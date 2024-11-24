cmake_minimum_required( VERSION 3.25.1 )

project( anemone CXX )
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

# build dependencies
add_subdirectory( lib )

# get python
find_package( PythonLibs REQUIRED )
include_directories( ${PYTHON_INCLUDE_DIRS} )

add_executable( ${PROJECT_NAME} src/main.cpp src/config.cpp src/log.cpp src/luahooks.cpp src/pythonhooks.cpp src/component.cpp src/util.cpp )

target_link_libraries( ${PROJECT_NAME} dependencies )
target_link_libraries( ${PROJECT_NAME} ${PYTHON_LIBRARIES} )