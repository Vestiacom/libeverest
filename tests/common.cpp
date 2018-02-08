#include "common.hpp"
#include <thread>
#include <iostream>
#include <unistd.h>

void runParallel(const std::string& command)
{
	std::thread t([&]() {
		int ret = system(command.c_str());
		(void)ret;
	});
	t.detach();
}

void runParallel(const std::vector<std::string> commands)
{
	int pid = fork();
	if (pid == 0) {
		for (auto command : commands) {
			std::cout << "Running: " << command << std::endl;
			int ret = system(command.c_str());
			if (ret != 0) {
				std::cout << "Test command: " << command
				          << " failed with: " << ret
				          << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		exit(EXIT_SUCCESS);
	}

}