#include "main.hpp"

void luahooks::register_hooks( lua_State *L )
{
    lua_register( L, "register_component", register_component );
    lua_register( L, "mem_alloc", mem_alloc );
    lua_register( L, "register_op_code_table", register_op_code_table );
    lua_register( L, "initialize_op_code_table", initialize_op_code_table );
    lua_register( L, "read_from_address", read_from_address );
    lua_register( L, "write_to_address", write_to_address );
    lua_register( L, "go_to_address", go_to_address);
    lua_register( L, "get_arg", get_arg );
    lua_register( L, "register_custom_instruction_parser", register_custom_instruction_parser );
    lua_register( L, "to_hex_string", to_hex_string );
}

/**
 * @brief Print a number in hex
 *
 * @param n integer: the integer to print as hex
 */
static int luahooks::to_hex_string(lua_State *L)
{
    assert( lua_gettop( L ) == 1 );
    assert( lua_isinteger( L, -1 ) );

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );
    assert( lua_islightuserdata(L, -1) == 1 );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop(L, 1);

    int is_number = 0;
    lua_Integer number = lua_tointegerx( L, -1, &is_number );
    lua_pop(L, 1);

    if ( !is_number )
    {
        state->logger.buf << "Error in print_hex: lua_tointegerx called on non-numerical value";
        state->logger.log_warning();
    }

    std::stringstream ss;
    ss << std::hex << number;

    lua_pushstring( L, ss.str().c_str() );
    return 1;
}

/**
 * @brief Registers a component
 * 
 * @param name string
 * @param component table
 */
static int luahooks::register_component( lua_State *L )
{

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );
    assert( lua_islightuserdata(L, -1) == 1 );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop(L, 1);

    if ( lua_gettop( L ) != 2  )
    {
        state->logger.buf << "register_component called with incorrect number of arguments.";
        state->logger.log_error();
        return 0;
    }

    if( !lua_istable(L, -1) )
    {
        state->logger.buf << "Attempting to register non-table value as component!";
        state->logger.log_error();
        return 0;
    }

    if ( !lua_isstring(L, -2) )
    {
        state->logger.buf << "Attempting to register component with invalid name"; // maybe write a better error message
        state->logger.log_error();
        return 0;
    }

    std::string name = std::string( lua_tostring(L, -2) );
    state->cpu_name.assign( name );

    lua_settable( L, LUA_REGISTRYINDEX );

    state->logger.buf << "Registered component " << name;
    state->logger.log_OK();

    return 0;
}

/**
 * @brief allocates a block of memory for the system and returns a userdata pointer to that block
 * 
 * @param size int: size of the block (int bytes)
 * @returns pushes userdata pointer onto the lua stack, so that the lua script can store it
 */
static int luahooks::mem_alloc( lua_State * L )
{
    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );
    assert( lua_islightuserdata(L, -1) == 1 );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop(L, 1);

    if ( lua_gettop( L ) != 1 )
    {
        state->logger.buf << "mem_alloc: called with invalid number of arguments";
        state->logger.log_error();
        lua_pop( L, lua_gettop(L) );
        lua_pushnil( L );
        return 1;
    }

    if ( !lua_isinteger(L, -1) )
    {
        state->logger.buf << "mem_alloc: called with invalid size argument";
        state->logger.log_error();
        lua_pop( L, 1 );
        lua_pushnil( L );
        return 1;
    }

    int block_size = lua_tointeger( L, -1 );
    DataBlock block = {
        block_size,
        (unsigned char *) std::malloc( block_size )
    };

    if ( block.data == NULL )
    {
        state->logger.buf << "mem_alloc: malloc failed!";
        state->logger.log_error();
        lua_pop(L, 1);
        lua_pushnil( L );
        return 1;
    }

    lua_pop( L, 1 );
    lua_pushlightuserdata( L, &block );
    return 1;
}

/**
 * @brief Provides a user-defined opcode table, which we push onto the registry
 * 
 * @param op_code_table table: array of function pointers
 */
static int luahooks::register_op_code_table( lua_State *L )
{
    assert( lua_gettop( L ) == 1 );
    assert( lua_istable( L, -1 ) );

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );
    assert( lua_islightuserdata(L, -1) == 1 );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );

    lua_pop( L, 1 );

    lua_pushstring( L, "opc" );
    lua_insert( L, -2 );

    assert( lua_istable( L, -1 ) );
    assert( lua_isstring( L, -2 ) );

    lua_settable( L, LUA_REGISTRYINDEX );
    return 0;

}

/**
 * @brief Modifies a given lua table so that it contains x pointers to a specified (no-op) function f
 * @param table array: the table to initialize
 * @param f function:
 * @param i int: the number of function pointers to write in the table
 */
static int luahooks::initialize_op_code_table( lua_State *L )
{
    assert( lua_gettop( L ) == 3 );

    assert( lua_istable( L, -3) );
    assert( lua_isfunction( L, -2 ) );
    assert( lua_isinteger( L, -1 ) );

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );
    assert( lua_islightuserdata(L, -1) == 1 );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop( L, 1 );

    int number_of_elements = lua_tointeger( L, -1 );
    lua_pop( L, 1);

    // target function at -1
    // target opcopde table at -2

    for ( int i = 0; i < number_of_elements; i ++  )
    {
        // duplicate the function pointer because seti pops it
        lua_pushnil( L );
        lua_copy( L, -2, -1);
        lua_seti( L, -3, i );
    }

    lua_pop( L, 2 );
    return 0;
}


/**
 * @brief 
 * @param memblock lightuserdata (pointer):
 * @param addr int:
 */
static int luahooks::read_from_address( lua_State *L )
{
    assert( lua_gettop( L ) == 2 );
    assert( lua_isinteger( L, -1 ) );
    assert( lua_islightuserdata( L, -2 ) );

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop( L, 1 );

    DataBlock * data_block = (DataBlock *) lua_touserdata( L, -2 );
    int addr = lua_tointeger( L, -1 );
    lua_pop( L, 2 );

    lua_pushinteger( L, data_block->data[addr] );
    return 1;
}

/**
 * @brief 
 * @param memblock lightuserdata (pointer):
 * @param addr int:
 * @param data int:
 */
static int luahooks::write_to_address( lua_State *L )
{
    assert( lua_gettop( L ) == 3 );
    assert( lua_isinteger( L, -1 ) );
    assert( lua_isinteger( L, -2 ) );
    assert( lua_islightuserdata( L, -3 ) );

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop( L, 1 );

    DataBlock * data_block = (DataBlock *) lua_touserdata( L, -3 );
    int addr = lua_tointeger( L, -2 );
    int data = lua_tointeger( L, -1 );
    lua_pop( L, 3 );

    data_block->data[addr] = data;
    return 0;
}

/**
 * @brief
 *
 * @param addr int: destination address to move to
 */
static int luahooks::go_to_address(lua_State *L)
{
    assert( lua_gettop(L) == 1 );
    assert( lua_isinteger(L, -1) );

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop( L, 1 );

    if ( state->active_bin_stream == nullptr )
    {
        state->logger.buf << "go_to_address called before a binary file was loaded. This function may only be called while the emuator is actively processing a binary file, for example within an opcode function.";
        state->logger.log_warning();
        lua_pop( L, 1 );
        return 0;
    }

    int address = lua_tointeger(L, -1);
    lua_pop( L, 1 );

    state->active_bin_stream->seekg( address );

    return 0;
}

/**
 * @brief get the next byte of data (an arg in immediate mode for the current instruction) and increment the active binary file stream
 *
 * @returns returns the next byte of data from the binary file
 */
static int luahooks::get_arg( lua_State * L )
{
    assert( lua_gettop( L ) == 1 );
    assert( lua_isinteger( L, -1 ) );

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop( L, 1 );

    if ( state->active_bin_stream == nullptr )
    {
        state->logger.buf <<  "get_arg called before a binary file was loaded. This function may only be called while the emulator is actively processing a binary file";
        state->logger.log_warning();
        return 0;
    }

    unsigned char arg = state->active_bin_stream->get();
    lua_pushinteger( L, arg) ;

    return 1;
}

/**
 * @brief registers a user-defined function to parse the binary
 * 
 * @param f function: the function to call, with the argument bye -- the current byte in the stream
 */
static int luahooks::register_custom_instruction_parser(lua_State *L)
{
    assert( lua_gettop( L ) == 1 );
    assert( lua_isfunction( L, -1 ) );

    lua_pushstring( L, "state" );
    lua_gettable( L, LUA_REGISTRYINDEX );

    ApplicationState * state = (ApplicationState *) lua_touserdata( L, -1 );
    lua_pop( L, 1 );

    // f at -1 

    lua_pushstring( L, "parser" );
    lua_pushnil( L );
    lua_copy( L, -3, -1 );

    // f at -1
    // "parser" at -2
    // f at -3

    assert( lua_isfunction(L, -1) );
    assert( lua_isstring(L, -2) );
    lua_settable( L, LUA_REGISTRYINDEX );
    lua_pop( L, 1 );

    std::cout << "registered" << std::endl;

    state->custom_instruction_parser = true;
    return 0;
}