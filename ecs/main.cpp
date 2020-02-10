#include "ecs.h"

#include <iostream>
#include <chrono>

struct ComponentTest {
	int value;
};

struct Another {
	bool pair;
};

int main() {

	ECS ecs;

	auto t0 = std::chrono::high_resolution_clock::now();

	for (int x = 0; x <= 99599; x++) {
		auto entity = ecs.create();
		ecs.assign<ComponentTest>(entity, { x });	
	}

	for (int x = 0; x <= 100; x++) {
		auto entity = ecs.create();
		ecs.assign<ComponentTest>(entity, { x });
		ecs.assign<Another>(entity, { x % 2 == 0});
	}

	auto t1 = std::chrono::high_resolution_clock::now();

	auto entities = ecs.view<Another, ComponentTest>();

	int sum = 0;
	auto component = ecs.getComponent<ComponentTest>();

	for (auto entity : entities) {
		ecs.get<ComponentTest>(entity, component).value;
	}

	auto t2 = std::chrono::high_resolution_clock::now();

	auto t3 = std::chrono::high_resolution_clock::now();


	std::cout << "internal took to instace: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
		<< " milliseconds. to create entities: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds and "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count()
		<< " milliseconds to iterate "
		<< " \n";
};