//#include "ecs_old.h"
#include "ecs.h"

#include <iostream>
#include <chrono>
#include <unordered_map>
#include "position.h"
#include <vector>
#include <unordered_set>
#include <memory>
#include <thread>


struct Game {
	int value = 0;
};


struct Test {
	int asd = 0;
};





int main() {
	ECS ecs;

	int x = 0;
	while (x < 1000000) {
		auto entity = ecs.create();
		ecs.assign<Game>(entity, new Game{ x });
		x++;
	}

	std::thread t([&ecs]() {
		auto& value = ecs.get<Game>();		
		while (true) {
			auto view = ecs.view<Game>();
			long sum = 0;
			int nulls = 0;
			for (auto entity : view) {
				auto &game = ecs.get<Game>(entity);				
				if (game) {
					if (game->value >= 0) {
						sum = sum + game->value;
					} else {
						std::cout << "wtf" << std::endl;
					}
					
				} else {
					nulls++;
				}
			}
			std::cout << sum << " nulls " << nulls << std::endl;
			_sleep(10);
		}		
	});
	t.detach();

	_sleep(1000);

	auto view = ecs.view<Game>();
	for (auto entity : view) {
		auto& game = ecs.get<Game>(entity);
		if ((game->value % 2) == 0) {
			ecs.destroy(entity);
		}
	}

	std::cout << "deleted" << std::endl;

	_sleep(5000);

	x = 0;

	while (x < 500000) {
		auto entity = ecs.create();
		ecs.assign<Game>(entity, new Game{ x });
		x++;
	}

	std::cout << "created" << std::endl;

	//ecs.destroy(entity);

	

	auto t0 = std::chrono::high_resolution_clock::now();

	auto t1 = std::chrono::high_resolution_clock::now();


	auto t2 = std::chrono::high_resolution_clock::now();

	auto t3 = std::chrono::high_resolution_clock::now();


	/*std::cout << "internal took to instace: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
		<< " milliseconds. to interate: "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count()
		<< " milliseconds and "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count()
		<< " milliseconds to find using map "
		<< " \n";*/

	while (true) {
		_sleep(1000);
	}
};