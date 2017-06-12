## Sources #####################################################################
file(GLOB_RECURSE SRCS *.cpp)


## Library names ##############################################################
set(_LIB_VERSION_ "${VERSION}")
set(_LIB_SOVERSION_ "0")
set(PC_FILE "lib${PROJECT_NAME}.pc")

set(SHARED_TARGET ${PROJECT_NAME})
set(STATIC_TARGET ${PROJECT_NAME}-static)
set(TMP_TARGET ${PROJECT_NAME}-tmp)

## Generate the pc file ########################################################
configure_file(${PC_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${PC_FILE} @ONLY)

## Defines #####################################################################

## Link libraries ##############################################################
if (ENABLE_STATIC)

    # First build everest-only objects 
    add_library(${TMP_TARGET} ${SRCS})
    add_dependencies(${TMP_TARGET} ${HTTPPARSER})

    set(STATIC_TARGET_LIBRARY ${CMAKE_BINARY_DIR}/src/lib${PROJECT_NAME}.a)

    add_custom_target(combined
            COMMAND ar -x $<TARGET_FILE:${TMP_TARGET}>
            COMMAND ar -x ${HTTPPARSER_LIBRARIES}
            COMMAND ar -qcs ${STATIC_TARGET_LIBRARY} *.o
            COMMAND rm ./*.o
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/
            DEPENDS ${TMP_TARGET} ${HTTPPARSER}
            )

    add_custom_target(run ALL DEPENDS combined)

    add_library(${STATIC_TARGET} STATIC IMPORTED GLOBAL)
	add_dependencies(${STATIC_TARGET} run)

    set_target_properties(${STATIC_TARGET} PROPERTIES
                          IMPORTED_LOCATION ${STATIC_TARGET_LIBRARY}
                          VERSION     ${_LIB_VERSION_})

    install(FILES       ${STATIC_TARGET_LIBRARY}
            COMPONENT   ${STATIC_TARGET}
            DESTINATION ${LIB_INSTALL_DIR})


endif(ENABLE_STATIC)

add_library(${SHARED_TARGET} SHARED ${SRCS})
set_target_properties(${SHARED_TARGET} PROPERTIES
                      SOVERSION   ${_LIB_SOVERSION_}
                      VERSION     ${_LIB_VERSION_})

target_compile_definitions(${SHARED_TARGET} PUBLIC "-DEVEREST_DLL")
add_dependencies(${SHARED_TARGET} ${HTTPPARSER})

target_link_libraries(${SHARED_TARGET} ${HTTPPARSER_LIBRARIES})

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

    file(GLOB HEADERS_INTERNALS internals/*.hpp)
    install(FILES       ${HEADERS_INTERNALS}
            COMPONENT   ${SHARED_TARGET}-dev
            DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}/internals
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)


endif(ENABLE_HEADERS)