// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include "App.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
	engine::App app{};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}