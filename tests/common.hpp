#ifndef TESTS_COMMON_HPP
#define TESTS_COMMON_HPP

#include <string>
#include <vector>

// This is for testing purposes
#define private public

void runParallel(const std::string& command);
void runParallel(const std::vector<std::string> commands);

#endif // TESTS_COMMON_HPPk