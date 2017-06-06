## Sources #####################################################################
file(GLOB_RECURSE SRC ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE TESTS_SRC *.cpp)


## Setup target ################################################################
set(UT_PROJECT_NAME "${PROJECT_NAME}-tests")
add_executable(${UT_PROJECT_NAME} ${SRC} ${TESTS_SRC})

add_dependencies(${UT_PROJECT_NAME} ${HTTPPARSER})

## Defines #####################################################################
# Automatically create main function
# add_definitions(-DBOOST_TEST_IGNORE_NON_ZERO_CHILD_CODE) 


## Link libraries ##############################################################
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS unit_test_framework system filesystem)
# pkg_check_modules(DEPS REQUIRED libcurl)

include_directories(${SRC_DIR} ${TESTS_DIR})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${DEPS_INCLUDE_DIR})

target_link_libraries(${UT_PROJECT_NAME} ${Boost_LIBRARIES} ${DEPS_LIBRARIES} ${HTTPPARSER_LIBRARIES} ev.a pthread)


## Integrate with CTest runner #################################################
add_test(NAME libeverest-unittests COMMAND libeverest-tests --log_level=message --catch_system_errors=no)


## Install #####################################################################
install(TARGETS ${UT_PROJECT_NAME}
        COMPONENT tests
        DESTINATION bin)
