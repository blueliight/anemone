#ifndef MAIN_HPP
#define MAIN_HPP

#include <cstdlib>
#include <istream>
#include <string>
#include <string>
#include "log.hpp"
#include "lua.h"
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <istream>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>
#include <iostream>
#include <variant>
#include <cstring>
#include <filesystem>
#include <algorithm>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <Python.h>

#include "log.hpp"

typedef struct DataBlock
{
    int size;
    unsigned char * data;
} DataBlock;

typedef struct ApplicationState
{
    bool custom_instruction_parser;
    DataBlock binary_target;
    Logger logger;
    std::istream * active_bin_stream;
    std::string cpu_name; // theres probably a better way to do this
} ApplicationState;

namespace utils
{
    bool check_lua( lua_State * L, int r );
}

namespace luahooks{
    static int register_component( lua_State * L );
    static int mem_alloc( lua_State * L );
    static int register_op_code_table( lua_State * L );
    static int initialize_op_code_table( lua_State * L );
    static int read_from_address( lua_State * L );
    static int write_to_address( lua_State * L );
    static int go_to_address( lua_State * L );
    static int get_arg( lua_State * L );
    static int register_custom_instruction_parser( lua_State * L );
    static int to_hex_string( lua_State * L );
    void register_hooks( lua_State * L );
}

namespace pythonhooks{
    int run_main( const char * path, ApplicationState * state );
}

const std::string APP_NAME = std::string( "anemone" );

//
// Wrappers for user-defined components
//

class UserDefinedCPU
{
    public:
        virtual void exec( unsigned char c ) = 0;
        virtual void exec_custom( unsigned char c ) = 0;
        virtual bool validate() = 0;
        virtual bool validate_custom() = 0;
};

class LuaCPU : public virtual UserDefinedCPU
{
    private:
        lua_State * L;
    public:
        LuaCPU( const char * path, ApplicationState * state );
        void exec( unsigned char c );
        void exec_custom( unsigned char c );
        bool validate();
        bool validate_custom();
};

class PythonCPU : public virtual UserDefinedCPU
{
    private:
        PyObject * opc;
        PyObject * parser;
        
    public:
        PythonCPU( const char * path, ApplicationState * state );
        void exec( unsigned char c );
        void exec_custom( unsigned char c );
        bool validate();
        bool validate_custom();
};

class GenericCPU : public virtual UserDefinedCPU
{
    public:
        void exec( unsigned char c );
        void exec_custom( unsigned char c );
        bool validate();
        bool validate_custom();
};

//
//  Config datatypes
//

//typedef 0 nulltype;
typedef std::variant<bool, int, float, std::string> cvar_value;

enum CVarType{
    NONE,
    BOOL,
    INT,
    FLOAT,
    STRING
};

typedef struct cvar
{
    std::string name;
    CVarType type;
    cvar_value value;
} cvar;

typedef struct EmulatorComponents
{
    std::filesystem::path cpu_path;
} EmulatorComponents;

namespace config{
    void init_settings( EmulatorComponents * dest, ApplicationState * state );
    int parse_file( char * file, std::vector<cvar> * cvar_v, ApplicationState * state );

    int get_cvar( std::vector<cvar> * cvar_v, const char * name, CVarType type, cvar_value * dest );
    int get_bool( std::vector<cvar> * cvar_v, const char * name, bool * dest  );
    int get_int( std::vector<cvar> * cvar_v, const char * name, int * dest );
    int get_float( std::vector<cvar> * cvar_v, const char * name, float * dest );

    template <typename T>
    int get_string(std::vector<cvar> *cvar_v, const char *name, T * dest)
    {
        cvar_value cv = std::string( "null" );

        if ( config::get_cvar( cvar_v, name, CVarType::STRING, &cv ) )
            return 1;

        std::string s = std::get<std::string>( cv );
        dest->assign( s );
        return 0;
    } 
}

//
// Function defs
//

int lua_exec_instr( unsigned char * instr );
int lua_parse_instr( unsigned char * instr );

int python_exec_instr( unsigned char * instr );
int python_parse_instr( unsigned char * instr );

#endif