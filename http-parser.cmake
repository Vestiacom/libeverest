# Build configuration of the http-parser library

include(ExternalProject)

# Target name
set(HTTPPARSER libhttp-parser)

ExternalProject_add(
    ${HTTPPARSER}
    BUILD_IN_SOURCE 1
    PREFIX libhttp-parser 
    CONFIGURE_COMMAND ""
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/submodules/http-parser
    BUILD_COMMAND make package
	INSTALL_COMMAND make install PREFIX=${CMAKE_BINARY_DIR}/libhttp-parser 
)

#
ExternalProject_Get_Property(${HTTPPARSER} install_dir)
set(HTTPPARSER_INCLUDE_DIRS ${install_dir}/include)
set(HTTPPARSER_LIBRARIES ${CMAKE_SOURCE_DIR}/submodules/http-parser/libhttp_parser.a)
