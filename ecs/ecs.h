#ifndef ECS_H
#define ECS_H

#include <vector>
#include <iostream>
#include <functional>
#include <typeinfo>
#include <unordered_map>
#include <queue>
#include <memory>

constexpr auto ENTITY_SIZE = 2000000;
constexpr auto COMPONENT_SIZE = 50;
constexpr auto NULL_ID = UINT_MAX;

using Entity = std::uint32_t;
using Component = std::uint32_t;
using BasicView = std::vector<Entity>;

class EntityIdPool {

public:
	std::vector<Entity> freeIds;
	Entity counter;

	EntityIdPool() {
		counter = 0;
	}
	Entity generateId() {
		if (freeIds.size() > 0) {
			auto id = freeIds.back();
			freeIds.pop_back();
			return id;
		}
		return counter++;
	}

	void freeEntity(Entity entity) {
		freeIds.push_back(entity);
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
		bool generated;
		return tryGetId(name, generated);
	}

	Component tryGetId(std::string name, bool& generated) {
		for (int x = 0; x < COMPONENT_SIZE; ++x) {
			if (components[x] == name) {
				generated = false;
				return x;
			}
		}
		auto id = counter++;
		components[id] = name;
		generated = true;
		return id;
	}
private:
	Component counter;
	std::vector<std::string>components;
};


class EntitySet {
public:
	EntitySet() {
		entities.resize(ENTITY_SIZE);
		for (auto x = 0; x < entities.size(); x++) {
			entities[x] = NULL_ID;
		}
		counter = 0;
	}

	int index(Entity entity) {
		return entities[entity];
	}

	void assure(Entity entity) {
		if (entities[entity] == NULL_ID) {
			if (freeIds.size() > 0) {
				entities[entity] = freeIds.back();
				freeIds.pop_back();
			} else {
				entities[entity] = counter++;
			}
		}
	}

	bool entityIn(Entity entity) {
		return entities[entity] != NULL_ID;
	}

	virtual int size() {
		return counter - freeIds.size();
	}

	virtual void internalRemove(Entity pos, Entity entity) {

	}

	void remove(Entity entity) {
		auto id = entities[entity];
		if (id == NULL_ID) return;

		freeIds.push_back(id);
		entities[entity] = NULL_ID;
		internalRemove(id, entity);
	}

	std::vector<Entity> entities;

	virtual BasicView view() {
		return entities;
	}

private:
	int counter;
	std::vector<Entity> freeIds;
};

template <typename Type>
struct PoolItem {
	Entity id;
	std::unique_ptr<Type> instance;
};


template <typename Type>
class PoolHandler : public EntitySet {
public:
	std::unique_ptr<Type>& get(Entity entity) {
		auto index = EntitySet::index(entity);
		if (index > -1) {
			return instances[index].instance;
		}		
		return std::unique_ptr<Type>();
	}

	BasicView view() override {
		BasicView view;
		if (instances.size() > 0) {
			for (std::size_t x = 0; x < instances.size(); ++x) {
				auto& item = instances[x];
				if (item.id != NULL_ID) {
					view.emplace_back(item.id);
				}
			}
		}
		return view;
	}

	void internalRemove(Entity pos, Entity entity) override {
		auto& poolInstance = instances[pos];
		for (const auto& removeEvent : removeEvents) {
			//removeEvent(entity, poolInstance.instance);
		}
		poolInstance.id = NULL_ID;
	}

	void assign(Entity entity, Type* type) {
		this->assure(entity);
		auto index = EntitySet::index(entity);
		if (instances.size() <= index) {
			instances.resize(index + 1);
		}
		instances[index] = { entity, std::unique_ptr<Type>(type)};

		for (const auto& assignEvent : assignEvents) {
			//assignEvent(entity, type);
		}
	}

	void registerOnAssignEvent(std::function<void(Entity, std::unique_ptr<Type>&)> assignEvent) {
		assignEvents.push_back(assignEvent);
	}

	void registerOnRemoveEvent(std::function<void(Entity, std::unique_ptr<Type>&)> removeEvent) {
		removeEvents.push_back(removeEvent);
	}

	std::vector<PoolItem<Type>> instances;

	std::vector<std::function<void(Entity, std::unique_ptr<Type>&)>> assignEvents;
	std::vector<std::function<void(Entity, std::unique_ptr<Type>&)>> removeEvents;
private:

};

struct PoolData {
	EntitySet* pool;
};

class Generic {
};

template <typename Type>
class Singleton : public Generic {
public:
	std::unique_ptr<Type> instance;

	std::unique_ptr<Type> & get() {		
		return instance;
	}
};

struct SingletonData {
	Generic* data;
};


class ECS {
public:

	~ECS() {
		for (const auto& po : pools) {
			delete po.pool;
		}
	}

	Entity create() {
		return entityIdPool.generateId();
	}

	int entityCount() {
		return entityIdPool.counter - entityIdPool.freeIds.size();
	}

	bool assigned(Entity entity, Component component) {
		return pools[component].pool->entityIn(entity);
	}

	template <typename Type>
	bool assigned(Entity entity) {
		return assigned(entity, getComponent<Type>());
	}

	template <typename Type>
	void assign(Entity entity, Type* instance) {
		auto comp = getComponent<Type>();
		auto& pool = this->assure<Type>(comp);
		pool.assign(entity, instance);
	}

	template <typename Type>
	void assign(Type* instance) {
		auto comp = getComponent<Type>();
		this->assureSingleton<Type>(comp).instance = std::unique_ptr<Type>(instance);
	}

	template <typename Type>
	void assign(Entity entity, int component, Type instance) {
		this->assure<Type>(component).assign(entity, instance);
	}

	template <typename Type>
	Component getComponent() {
		return getComponent<Type>(typeid(Type).name());
	}

	template <typename Type>
	Component getComponent(std::string name) {
		bool genarated = false;
		auto component = componentIdPool.tryGetId(name, genarated);
		if (genarated) {
			this->assure<Type>(component);
		}
		return component;
	}

	bool isComponent(std::string name) {
		return componentIdPool.isComponent(name);
	}

	void destroy(Entity entity) {
		for (auto x = 0; x < pools.size(); ++x) {
			pools[x].pool->remove(entity);
		}
		entityIdPool.freeEntity(entity);
	}

	template <typename Type>
	std::unique_ptr<Type> & get(Entity entity) {
		bool generated;
		auto component = componentIdPool.tryGetId(typeid(Type).name(), generated);
		return this->assure<Type>(component).get(entity);
	}

	template <typename Type>
	std::unique_ptr<Type>& get() {
		auto comp = getComponent<Type>();		
		return this->assureSingleton<Type>(comp).get();
	}

	template <typename Type>
	std::unique_ptr<Type>& get(Entity entity, Component component) {
		return this->assure<Type>(component).get(entity);
	}

	template <typename T>
	void remove(Entity entity) {
		return remove(entity, componentIdPool.tryGetId(typeid(T).name()));
	}

	void remove(Entity entity, Component component) {
		pools[component].pool->remove(entity);
	}

	/*template <typename Type>
	void sort(BasicView& basicBiew, std::function<bool(Type&, Type&)> predicate) {
		auto component = getComponent<Type>();
		std::sort(basicBiew.begin(), basicBiew.end(), [this, component, &predicate](auto v1, auto v2) {
			return predicate(this->get<Type>(v1, component), this->get<Type>(v2, component));
		});
	}*/


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
	const BasicView view() {
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

	template <typename Type>
	void registerOnAssignEvent(std::function<void(Entity, Type)> assignEvent) {
		auto component = getComponent<Type>();
		this->assure<Type>(component).registerOnAssignEvent(assignEvent);
	}

	template <typename Type>
	void registerOnRemoveEvent(std::function<void(Entity, Type)> removeEvent) {
		auto component = getComponent<Type>();
		this->assure<Type>(component).registerOnRemoveEvent(removeEvent);
	}

private:
	mutable std::vector<PoolData> pools;
	ComponentIdPool componentIdPool;
	EntityIdPool entityIdPool;
	std::unordered_map<Component, SingletonData> singletons;


	template <typename Type>
	Singleton<Type>& assureSingleton(Component component) {
		if (singletons.count(component) <= 0) {
			singletons[component] = { new Singleton<Type>() };
		}
		return static_cast<Singleton<Type>&>(*singletons[component].data);
	}

	template <typename Type>
	PoolHandler<Type>& assure(Component component) {
		if (pools.size() <= component) {
			pools.resize(component + 1);
		}
		if (pools[component].pool == nullptr) {
			pools[component] = { new PoolHandler<Type>() };
		}
		return static_cast<PoolHandler<Type>&>(*pools[component].pool);
	}

	BasicView internalView(std::vector<Component> comps) {
		if (comps.size() > 1) {
			BasicView entities;
			auto first = pools[comps[0]].pool;
			auto pos = 0;

			for (int x = 1; x < comps.size(); ++x) {
				if (pools[comps[x]].pool->size() < first->size()) {
					first = pools[comps[x]].pool;
					pos = x;
				}
			}
			comps.erase(comps.begin() + pos);

			for (const auto& entity : first->view()) {
				bool found = true;
				for (const auto& comp : comps) {
					if (!pools[comp].pool->entityIn(entity)) {
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
			return pools[comps[0]].pool->view();
		}
	}
};

#endif 

