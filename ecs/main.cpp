#include "ecs_old.h"
//#include "ecs.h"

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

	auto component = ecs.getComponent<ComponentTest>();
	auto anotherComp = ecs.getComponent<Another>();

	auto t0 = std::chrono::high_resolution_clock::now();

	for (int x = 0; x <= 999999; x++) {
		auto entity = ecs.create();
		//ecs.assign<ComponentTest>(entity, component, { x });
		//ecs.assign<ComponentTest>(entity, new ComponentTest{ x });
	}

	auto entities = ecs.view<ComponentTest>();
	for (auto entity : entities) {
		ecs.destroy(entity);
	}

	for (int x = 0; x <= 5; x++) {
		auto entity = ecs.create();
		//ecs.assign<ComponentTest>(entity, component, { x });
		//ecs.assign<Another>(entity, anotherComp, { x % 2 == 0 });
		//ecs.assign<ComponentTest>(entity, new ComponentTest{ x });
		//ecs.assign<Another>(entity, new Another{ x % 2 == 0});
	}

	

	for (int x = 0; x <= 999999; x++) {
		auto entity = ecs.create();
		//ecs.assign<ComponentTest>(entity, component, { x });
		ecs.assign<ComponentTest>(entity, new ComponentTest{ x });
	}

	auto t1 = std::chrono::high_resolution_clock::now();

	entities = ecs.view<ComponentTest>();

	int sum = 0;

	for (auto entity : entities) {		
		sum = sum + ecs.get<ComponentTest>(entity, component)->value;
	}

	std::cout << sum << std::endl;

	auto t2 = std::chrono::high_resolution_clock::now();


	auto t3 = std::chrono::high_resolution_clock::now();


	std::cout << "internal took to instace: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
		<< " milliseconds. to interate: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds and "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count()
		<< " milliseconds to delete "
		<< " \n";

	while (true) {
		_sleep(1000);
	}
};