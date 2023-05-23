#include "App.hpp"
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <magic_enum.hpp>
#include <string>
#include <thread>
#include <vector>
#include <wiringPi.h>

import Defer;

std::atomic_bool shouldTerminate{false};

void sigHandle(int sig) {
	if (shouldTerminate) {
		std::clog << "Received " << sig << " again, exiting..." << std::endl;
		std::exit(1);
	}
	std::clog << "Received " << sig << ", terminating..." << std::endl;
	shouldTerminate = true;
}

auto main(int argc, char const* argv[]) -> int {
	std::ios_base::sync_with_stdio(false);
	signal(SIGTERM, sigHandle); // on Docker, SIGTERM is sent when the container is stopped
	signal(SIGINT, sigHandle);

	std::vector<std::string> args;
	args.reserve(argc);
	for (int i = 0; i < argc; ++i) {
		args.emplace_back(argv[i]);
	}

	mg_init_library(MG_FEATURES_DEFAULT);
	Defer defer{[]() { mg_exit_library(); }};
	std::cout << "Civetweb version: " << mg_version() << std::endl;

#ifdef __aarch64__
	wiringPiSetup();
#endif

#ifdef __aarch64__
	int   major{};
	char* minor;
	wiringPiVersion(&major, &minor);
	std::cout << "WirePi version: " << major << "." << minor << std::endl;
#else
	int major{}, minor{};
	wiringPiVersion(&major, &minor);
	std::cout << "WirePi version: " << major << "." << minor << std::endl;
#endif

	App app{args};

	app.start();

#ifndef NDEBUG
	std::cout << "Press ENTER to end the program";
	std::cout.flush();
	std::cin.get();
#else
	std::cout << "Press CTRL+C to end the program";
	std::cout.flush();
	while (!shouldTerminate) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
#endif

	app.stop();

	return EXIT_SUCCESS;
}
