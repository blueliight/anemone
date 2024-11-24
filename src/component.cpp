#include "main.hpp"

//
// Lua CPU
//

LuaCPU::LuaCPU( const char * path, ApplicationState * state )
{
    L = luaL_newstate();
    luaL_openlibs( L );
    luahooks::register_hooks( L );

    lua_pushstring( L, "state" );
    lua_pushlightuserdata( L, state );
    lua_settable( L, LUA_REGISTRYINDEX );

    // build components defined in lua files
    if ( utils::check_lua(L, luaL_dofile(L, path )) )
        throw std::runtime_error( "Error in lua file." );


}

void LuaCPU::exec( unsigned char c )
{
    lua_pushstring( L, "opc" );
    lua_gettable( L, LUA_REGISTRYINDEX );
    lua_geti( L, -1, c );

    if ( lua_isfunction(L, -1) )
    {
        lua_call( L, 0, 0 );
    }
    else {
        throw std::runtime_error( "Non-function value in opcode table" );
    }

    lua_pop( L, 1 );
}

void LuaCPU::exec_custom( unsigned char c )
{   
    lua_getfield( L, LUA_REGISTRYINDEX, "parser" );
    lua_pushinteger( L, c );             
    lua_call( L, 1, 0 );
}

bool LuaCPU::validate()
{
    lua_pushstring( L, "opc" );
    lua_gettable( L, LUA_REGISTRYINDEX );
    bool is_valid = lua_istable( L, -1 );
    return is_valid;
}

bool LuaCPU::validate_custom()
{           
    lua_getfield( L, LUA_REGISTRYINDEX, "parser" );
    bool is_valid = lua_isfunction( L, -1 );
    return is_valid;
}

//
// Python CPU
//

PythonCPU::PythonCPU( const char * path, ApplicationState * state )
{
    PyObject *pName, *pModule;

    Py_Initialize();

    pName = PyUnicode_DecodeFSDefault( path );
    pModule = PyImport_Import( pName );
    Py_DECREF( pName );

    if ( pModule == NULL )
        throw new std::runtime_error( std::string( "Unable to initialize module ", path ) );

    Pyfunction

    Py_DECREF( pModule );

}

void PythonCPU::exec( unsigned char c )
{

}

void PythonCPU::exec_custom( unsigned char c )
{

}

bool PythonCPU::validate()
{
    return true;
}

bool PythonCPU::validate_custom()
{
    return true;
}