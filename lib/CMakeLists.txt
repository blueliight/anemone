cmake_minimum_required( VERSION 3.25.1 )

project( dependencies CXX )
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

# lua
file( GLOB LUA_SOURCES lua/src/*.c )
set_source_files_properties( ${LUA_SOURCES} PROPERTIES LANGUAGE CXX )

add_library( ${PROJECT_NAME} STATIC ${LUA_SOURCES} )
set_target_properties( ${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX )

target_include_directories( ${PROJECT_NAME} PUBLIC "lua/src" )