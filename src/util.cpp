#include "main.hpp"

bool utils::check_lua( lua_State * L, int r )
{
    if ( r != LUA_OK )
    {
        std::string error_msg = lua_tostring(L, -1);
        std::cout << error_msg << std::endl;
        return 1;
    }

    return 0;
}
