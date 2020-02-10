#ifndef ECS_H
#define ECS_H

#include <vector>
#include <iostream>
#include <functional>
#include <typeinfo>
#include <unordered_map>
#include <queue>

constexpr auto ENTITY_SIZE = 10000;
constexpr auto COMPONENT_SIZE = 50;

using Entity = std::uint32_t;
using Component = std::uint32_t;
using BasicView = std::vector<Entity>;

class EntityIdPool {
private:
	std::queue<Entity> freeIds;
public:
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
	ComponentIdPool() : components(50) {
		counter = 0;
	}
	Component tryGetId(std::string name) {
		/*auto ite = components.find(name);
		if (ite != components.end()) {
			return ite->second;
		} else {
			auto id = ++counter;
			components[name] = id;
			return id;
		}*/
		for (int x = 0; x < 50; ++x) {
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


class ECS {
public:	

	ECS() : componentPool(COMPONENT_SIZE) {
	}

	Entity create() {
		return entityIdPool.generateId();
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
		
	void assign(Entity entity, int component, void* object) {
		componentPool[component].objects[entity] = object;
	}

	template <typename T>
	void listComponents(std::vector<int>& vcomp, T component) {
		vcomp.push_back(component);
	}

	template <typename T, typename... Args>
	void listComponents(std::vector<int> &vcomp, T component, Args... components) {
		vcomp.push_back(component);
		listComponents(vcomp, components...);
	}

	void destroy(Entity entity) {
		for (auto x = 0; x < 2; ++x) {
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
		return static_cast<T*>(componentPool[component].objects[entity]);
	}

	template <typename T>
	bool remove(Entity entity) {
		return remove(entity, componentIdPool.tryGetId(typeid(T).name()));
	}

	bool remove(Entity entity, Component component) {
		auto comp = componentPool[component].objects[entity];
		componentPool[component].objects[entity] = nullptr;
		if (comp != nullptr) {
			delete comp;
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

	template <typename... Args>
	BasicView viewx(Args... components) {
		std::vector<int> comps;
		BasicView entities;
		listComponents(comps, components...);
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

	template <typename... Args>
	void view(std::function<void(Entity)> predicate, Args... components) {
		std::vector<int> comps;
		listComponents(comps, components...);
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
					predicate(ent);
				}
			}
		} else {
			auto first = componentPool[comps[0]].objects;
			for (int ent = 0; ent < entityIdPool.counter; ++ent) {
				if (first[ent] == nullptr) continue;
				predicate(ent);
			}
		}		
	}

private:
	std::vector<ComponentPool> componentPool;
	ComponentIdPool componentIdPool;
	EntityIdPool entityIdPool;
};

#endif 

