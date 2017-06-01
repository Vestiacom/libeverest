#include "common.hpp"
#include <thread>

void runParallel(const std::string& command)
{
	std::thread t([&]() {
		int ret = system(command.c_str());
		(void)ret;
	});
	t.join();
}
