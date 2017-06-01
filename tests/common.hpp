#ifndef TESTS_COMMON_HPP
#define TESTS_COMMON_HPP

void runParallel(const std::string& command)
{
	std::thread t([&]() {
		int ret = system(command.c_str());
		(void)ret;
	});
	t.join();
}

#endif // TESTS_COMMON_HPPk