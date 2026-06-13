#include "FluidEngine.h"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <time.h>

int main() {
	srand((unsigned int)time(NULL));

	try {
		FluidEngine engine;
		engine.run();
	}
	catch (const std::runtime_error& e) {
		std::cout << e.what() << std::endl;
	}

	return 0; 
}