#ifndef ECS_H
#define ECS_H

#include <vector>
#include <iostream>
#include <functional>
#include <typeinfo>
#include <unordered_map>
#include <queue>
#include <memory>

constexpr auto ENTITY_SIZE = 100000;
constexpr auto COMPONENT_SIZE = 50;

using Entity = std::uint32_t;
using Component = std::uint32_t;
using BasicView = std::vector<Entity>;

class EntityIdPool {

public:
	std::queue<Entity> freeIds;
	Entity counter;

	EntityIdPool() {
		counter = 0;
	}
	Entity generateId() {
		if (freeIds.size() > 0) {
			auto id = freeIds.front();
			freeIds.pop();
			return id;
		}
		return counter++;
	}

	void freeEntity(Entity entity) {
		freeIds.push(entity);
	}
};

class ComponentIdPool {
public:
	ComponentIdPool() : components(COMPONENT_SIZE) {
		counter = 0;
	}

	bool isComponent(std::string name) {
		for (int x = 0; x < COMPONENT_SIZE; ++x) {
			if (components[x] == name) {
				return true;
			}
		}
		return false;
	}

	Component tryGetId(std::string name) {
		for (int x = 0; x < COMPONENT_SIZE; ++x) {
			if (components[x] == name) {
				return x;
			}
		}
		auto id = counter++;
		components[id] = name;
		return id;
	}
private:
	Component counter;
	std::vector<std::string>components;
};


class EntitySet {
public:
	std::vector<Entity> in;
	EntitySet() : entities(ENTITY_SIZE){
		counter = 0;
	}

	int index(Entity entity) {
		return entities[entity];
	}
	
	void assure(Entity entity) {
		if (entities[entity] == 0) {
			entities[entity] = ++counter;
			in.push_back(entity);
		}
	}

	bool entityIn(Entity entity){
		return entities[entity] != 0;
	}

	int size() {
		return counter;
	}

private:
	int counter;
	std::vector<int> entities;	
};


template <typename Type>
class PoolHandler : public EntitySet {
public:
	Type & get(Entity entity) {
		return instances[EntitySet::index(entity)];
	}

	void assign(Entity entity, Type type) {
		this->assure(entity);
		auto index = EntitySet::index(entity);
		if (instances.size() <= index) {
			instances.resize(index + 1);
		}
		instances[index] = type;
	}

private:
	std::vector<Type> instances;
};

struct PoolData {
	EntitySet* pool;
};

class ECS {
public:

	ECS() {

	}

	~ECS() {
		for (auto po : pool) {
			delete po.pool;
		}
	}

	Entity create() {
		return entityIdPool.generateId();
	}

	int entityCount() {
		return entityIdPool.counter - entityIdPool.freeIds.size();
	}

	bool assigned(int entity, Component component) {
		return false;
	}

	template <typename Type>
	bool assigned(int entity) {
	//	return componentPool[getComponent<Type>()].objects[entity] != nullptr;
		return false;
	}

	template <typename Type>
	void assign(int entity, Type instance) {
		auto comp = getComponent<Type>();
		this->assure<Type>(comp).assign(entity, instance);
	}

	template <typename Type>
	Component getComponent() {
		return getComponent(typeid(Type).name());
	}

	Component getComponent(std::string name) {
		return componentIdPool.tryGetId(name);
	}

	bool isComponent(std::string name) {
		return componentIdPool.isComponent(name);
	}

	template <typename Type>
	void assign(Entity entity, int component, Type object) {

		

		// componentPool[component].objects[entity] = object;
	}

	void destroy(Entity entity) {
		//for (auto x = 0; x < COMPONENT_SIZE; ++x) {
		//	auto component = componentPool[x].objects[entity];
		//	componentPool[x].objects[entity] = nullptr;
		//	if (component != nullptr) {
		//		//toDelete.push(component);				
		//	}
		//}
		entityIdPool.freeEntity(entity);
	}

	template <typename Type>
	Type& get(Entity entity) {

		auto component = componentIdPool.tryGetId(typeid(Type).name());

		return this->assure<Type>(component).get(entity);
		//return this->get<T>(entity, );
	}

	template <typename Type>
	Type & get(Entity entity, Component component) {
		return this->assure<Type>(component).get(entity);
	}

	void* get(Entity entity, Component component) {
		//return pool[component].objects[entity];
		return nullptr;
	}

	template <typename T>
	bool remove(Entity entity) {
		return remove(entity, componentIdPool.tryGetId(typeid(T).name()));
	}

	bool remove(Entity entity, Component component) {
		/*auto comp = pool[component].objects[entity];
		pool[component].objects[entity] = nullptr;
		if (comp != nullptr) {
			toDelete.push(comp);
			return true;
		}*/
		return false;
	}

	template <typename Type>
	void sort(BasicView& basicBiew, std::function<bool(Type*, Type*)> predicate) {
		auto component = getComponent<Type>();
		std::sort(basicBiew.begin(), basicBiew.end(), [this, component, &predicate](auto v1, auto v2) {
			return predicate(this->get<Type>(v1, component), this->get<Type>(v2, component));
		});
	}


	template <typename T>
	void listComponents(std::vector<Component>& vcomp, T component) {
		vcomp.push_back(component);
	}

	template <typename T, typename... Args>
	void listComponents(std::vector<Component>& vcomp, T component, Args... components) {
		vcomp.push_back(component);
		listComponents(vcomp, components...);
	}

	template<typename... Components>
	typename std::enable_if<sizeof...(Components) == 0>::type find(std::vector<Component>& componentIds) {
	}


	template<typename Type, typename... Components>
	void find(std::vector<Component>& componentIds) {
		componentIds.push_back(getComponent<Type>());
		find<Components...>(componentIds);
	}

	template <typename... Components>
	BasicView view() {
		std::vector<Component> componentIds;
		find<Components...>(componentIds);
		return internalView(componentIds);
	}


	template <typename... Args>
	BasicView view(Args... components) {
		std::vector<Component> comps;
		listComponents(comps, components...);
		return internalView(comps);
	}

private:
	mutable std::vector<PoolData> pool;
	ComponentIdPool componentIdPool;
	EntityIdPool entityIdPool;

	
	template <typename Type>
	PoolHandler<Type> & assure(Component component) {
		if (pool.size() <= component) {
			pool.resize(component + 1);
			pool[component] = { new PoolHandler<Type>()};
		}
		return static_cast<PoolHandler<Type>&>(*pool[component].pool);
	}

	BasicView internalView(std::vector<Component> comps) {		

		if (comps.size() > 1) {
			BasicView entities;
			auto first = pool[comps[0]].pool;		
			auto pos = 0;

			for (int x = 1; x < comps.size(); x++) {
				if (pool[comps[x]].pool->size() < first->size()) {
					first = pool[comps[x]].pool;
					pos = x;
				}
			}
			comps.erase(comps.begin() + pos);

			for (auto entity : first->in) {
				bool found = true;
				for (auto comp : comps) {
					if (!pool[comp].pool->entityIn(entity)) {
						found = false;
						break;
					}
				}
				if (found) {
					entities.push_back(entity);
				}
			}
			return entities;
		} else {
			return pool[comps[0]].pool->in;			
		}
	}
};

#endif 

