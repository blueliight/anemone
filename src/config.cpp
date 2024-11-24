#include "main.hpp"

int config::get_cvar( std::vector<cvar> *cvar_v, const char *name, CVarType type, cvar_value * dest )
{
    std::string s_name = std::string( name );

    for ( cvar cv : *cvar_v )
    {
        if( !cv.name.compare( s_name ) )
        {
            if ( cv.type == NONE  )
                return 1;

            if ( cv.type == type )
            {
                int i = type;
                dest->swap( cv.value );
                return 0;
            }
        }
    }

    return 1;
}

//
//  the following get_* functions are just wrappers for get_cvar()
//

int config::get_bool(std::vector<cvar> *cvar_v, const char *name, bool *dest)
{
    cvar_value cv = false;

    if ( config::get_cvar( cvar_v, name, CVarType::BOOL, &cv ) )
        return 1;

    bool b = std::get<bool>( cv );
    std::memcpy( dest, &b, sizeof( bool ) );
    return 0;
}

int config::get_int(std::vector<cvar> *cvar_v, const char *name, int *dest)
{
    cvar_value cv = 0;

    if ( config::get_cvar( cvar_v, name, CVarType::INT, &cv ) )
        return 1;

    int i = std::get<int>( cv );
    std::memcpy( dest, &i, sizeof( int ) );
    return 0;
}

int config::get_float(std::vector<cvar> *cvar_v, const char *name, float *dest)
{
    cvar_value cv = (float)0.0;

    if ( config::get_cvar( cvar_v, name, CVarType::FLOAT, &cv ) )
        return 1;

    float f = std::get<float>( cv );
    std::memcpy( dest, &f, sizeof( float ) );
    return 0;
}

int config::parse_file( char * file, std::vector<cvar> * dest, ApplicationState * state )
{
    std::ifstream cfg_stream( file );
    if ( cfg_stream.is_open() )
    {
        std::string line;

        while ( std::getline( cfg_stream, line ) )
        {
            // remove_if shifts undesired characters to the back of the string
            // erase removes the undesired characters from the modified string
            line.erase( std::remove_if( line.begin(), line.end(), isspace ), line.end() );
            if ( line[0] == '#' || line.empty() )
                continue;

            auto delimiterPos = line.find( "=" );
            auto name = line.substr( 0, delimiterPos );
            auto value = line.substr( delimiterPos+1, std::string::npos );
            
            cvar var;
            var.name = name;
            var.type = CVarType::NONE;

            // try converting to float first
            if ( value == "true" )
            {
                var.value = true;
                var.type = CVarType::BOOL;
            }
            else if ( value == "false" )
            {
                var.value = false;
                var.type = CVarType::BOOL;
            }
            else
            {
                // try conversion to int
                try {
                    var.value = std::stoi( value );
                    var.type = CVarType::INT;
                }
                catch ( std::exception e) 
                {
                    // try conversion to float
                    try 
                    {
                        var.value = std::stof( value );
                        var.type = CVarType::FLOAT;    
                    } catch (std::exception e) 
                    {
                        // finally, copy value as string
                        var.value = value;
                        var.type = CVarType::STRING;
                    }

                }
            }

            if( var.type == CVarType::NONE )
            {
                state->logger.buf << "cfg parser was unable to translate " << value << " to a cvar value!";
                state->logger.log_warning();
                var.value = 0;
            }

            dest->push_back( var );

        }
    }
    else
    {
        state->logger.buf << "Unable to load config file at " << file;
        state->logger.log_error_fatal();
        return 1;
    }

    return 0;
}