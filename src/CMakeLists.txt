## Sources #####################################################################
file(GLOB_RECURSE SRCS *.cpp)


## Library names ##############################################################
set(_LIB_VERSION_ "${VERSION}")
set(_LIB_SOVERSION_ "0")
set(PC_FILE "lib${PROJECT_NAME}.pc")

set(STATIC_TARGET ${PROJECT_NAME}-static)
set(SHARED_TARGET ${PROJECT_NAME})

## Generate the pc file ########################################################
configure_file(${PC_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE} @ONLY)

## Link libraries ##############################################################

if (ENABLE_STATIC)

    add_library(${STATIC_TARGET} STATIC ${SRCS})
    set_target_properties(${STATIC_TARGET} PROPERTIES
                          OUTPUT_NAME ${SHARED_TARGET}
                          VERSION     ${_LIB_VERSION_})
    
    add_dependencies(${STATIC_TARGET} ${HTTPPARSER})

    target_link_libraries(${STATIC_TARGET} STATIC ${HTTPPARSER_LIBRARIES})

    install(TARGETS     ${STATIC_TARGET}
            COMPONENT   ${STATIC_TARGET}
            DESTINATION ${LIB_INSTALL_DIR})


endif(ENABLE_STATIC)

add_library(${SHARED_TARGET} SHARED ${SRCS})
set_target_properties(${SHARED_TARGET} PROPERTIES
                      SOVERSION   ${_LIB_SOVERSION_}
                      VERSION     ${_LIB_VERSION_})

add_dependencies(${SHARED_TARGET} ${HTTPPARSER})

target_link_libraries(${SHARED_TARGET} STATIC ${HTTPPARSER_LIBRARIES})

## Includes ####################################################################
include_directories(SYSTEM ${HTTPPARSER_INCLUDE_DIRS})


## Install #####################################################################
install(FILES       ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE}
        COMPONENT   ${SHARED_TARGET}
        DESTINATION ${LIB_INSTALL_DIR}/pkgconfig)

install(TARGETS     ${SHARED_TARGET}
        COMPONENT   ${SHARED_TARGET}
        DESTINATION ${LIB_INSTALL_DIR})


if (ENABLE_HEADERS)

    file(GLOB HEADERS *.hpp)
    install(FILES       ${HEADERS}
            COMPONENT   ${SHARED_TARGET}-dev
            DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

endif(ENABLE_HEADERS)