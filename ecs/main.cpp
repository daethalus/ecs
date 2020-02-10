#include "ecs.h"

#include <iostream>
#include <chrono>


int main() {

	auto t0 = std::chrono::high_resolution_clock::now();

	auto t1 = std::chrono::high_resolution_clock::now();

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
}