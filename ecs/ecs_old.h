#ifndef ECS_H
#define ECS_H

#include <vector>
#include <iostream>
#include <functional>
#include <typeinfo>
#include <unordered_map>
#include <queue>

constexpr auto ENTITY_SIZE = 1080000;
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


class ComponentPool {
public:
	ComponentPool() : objects(ENTITY_SIZE){}
	std::vector<void*> objects;
};

struct EntityNode {
	Entity id;
	std::vector<bool> entities;
	EntityNode() : entities(ENTITY_SIZE) {
	}
};


class ECS {
public:	

	ECS() : componentPool(COMPONENT_SIZE) {
	}

	Entity create() {
		return entityIdPool.generateId();
	}

	int entityCount() {
		return entityIdPool.counter - entityIdPool.freeIds.size();		
	}
	
	bool assigned(int entity, Component component) {
		return componentPool[component].objects[entity] != nullptr;
	}

	template <typename Type>
	bool assigned(int entity) {
		return componentPool[getComponent<Type>()].objects[entity] != nullptr;
	}

	template <typename Type>
	void assign(int entity, void* object) {		
		componentPool[getComponent<Type>()].objects[entity] = object;
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
		
	void assign(Entity entity, int component, void* object) {
		componentPool[component].objects[entity] = object;
	}

	void destroy(Entity entity) {
		for (auto x = 0; x < COMPONENT_SIZE; ++x) {
			auto component = componentPool[x].objects[entity];
			componentPool[x].objects[entity] = nullptr;
			if (component != nullptr) {
				delete component;
			}
		}
		entityIdPool.freeEntity(entity);
	}

	template <typename T>
	T* get(Entity entity) {
		return this->get<T>(entity, componentIdPool.tryGetId(typeid(T).name()));
	}

	template <typename T>
	T* get(Entity entity, Component component) {
		return static_cast<T*>(this->get(entity, component));
	}
	
	void* get(Entity entity, Component component) {
		return componentPool[component].objects[entity];
	}

	template <typename T>
	bool remove(Entity entity) {
		return remove(entity, componentIdPool.tryGetId(typeid(T).name()));
	}

	bool remove(Entity entity, Component component) {
		auto comp = componentPool[component].objects[entity];
		componentPool[component].objects[entity] = nullptr;
		if (comp != nullptr) {
			toDelete.push(comp);
			return true;
		}
		return false;
	}

	template <typename Type>
	void sort(BasicView &basicBiew, std::function<bool(Type*, Type*)> predicate) {
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

	void memoryCleanup() {
		int c = 0;
		while (!toDelete.empty()) {
			c++;
			auto comp = toDelete.front();
			delete comp;
			toDelete.pop();
			if (c > 25) {
				break;
			}
		}		
	}

	EntityNode* createNode(Entity entity) {
		auto node = EntityNode();
		node.id = entity;
		nodes.push_back(node);
		return &node;
	}

	EntityNode& getNode(Entity parent) {
		int index = -1;
		for (int x = 0; x < nodes.size(); x++) {
			if (nodes[x].id == parent) {
				index = 0;
			}
		}
		return nodes[index];
	}

	void removeNode(Entity entity) {
		for (int x = 0; x < nodes.size(); x++) {
			if (nodes[x].id == entity) {
				nodes.erase(nodes.begin() + x);
				break;
			}
		}
	}

	void addToNode(Entity parent, Entity child) {		
		getNode(parent).entities[child] = true;
	}

private:
	std::vector<ComponentPool> componentPool;

	std::vector<EntityNode> nodes;

	ComponentIdPool componentIdPool;
	EntityIdPool entityIdPool;
	std::queue<void*> toDelete;

	BasicView internalView(std::vector<Component> comps) {
		BasicView entities;
		if (comps.size() > 1) {
			auto first = componentPool[comps[0]].objects;
			for (Entity ent = 0; ent < entityIdPool.counter; ++ent) {
				bool found = false;

				if (first[ent] == nullptr) continue;

				for (int comp = 1; comp < comps.size(); ++comp) {
					if (componentPool[comps[comp]].objects[ent] != nullptr) {
						found = true;
					} else {
						found = false;
						break;
					}
				}
				if (found) {
					entities.push_back(ent);
				}
			}
		} else {
			auto first = componentPool[comps[0]].objects;
			for (int ent = 0; ent < entityIdPool.counter; ++ent) {
				if (first[ent] == nullptr) continue;
				entities.push_back(ent);
			}
		}
		return entities;
	}
};

#endif 

