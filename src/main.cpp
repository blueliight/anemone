#include "main.hpp"

int load_binary_to_target( ApplicationState * state, const char * file )
{
    std::filebuf bin_buffer;
    if( bin_buffer.open( file, std::ios::in ) )
    {
        std::istream bin_stream( &bin_buffer );
        bin_stream.seekg( 0, bin_stream.end );
        if ( bin_stream.tellg() > state->binary_target.size )
        {
            state->logger.buf << "Binary file " << file << " is longer than reserved memory space (" << state->binary_target.size << " bytes )";
            state->logger.log_warning();
        }

        bin_stream.seekg( 0, bin_stream.beg );

        bin_stream.read( (char*)state->binary_target.data, state->binary_target.size );
        std::cout << "Read " << bin_stream.gcount() << " bytes from binary file" << std::endl;
        bin_buffer.close();

        return 0;
    }

    state->logger.buf << "Error loading binary file " << file;
    state->logger.log_error_fatal();
    return 1;
}

int main( int argc, char ** argv )
{
    std::cout << "xyzemu v0.0.1" << std::endl;
    
    if ( argc == 2 )
    {
        ApplicationState app_state = {
            false,
            {
                0,
                NULL
            },
            Logger(),
            nullptr,
            std::string( "null" )
        };

        std::vector<cvar> cvar_settings;
        if ( config::parse_file( argv[1], &cvar_settings, &app_state ))
            return 1;

        std::filesystem::path system_path;
        if (config::get_string( &cvar_settings, "system", &system_path  ))
            return 1;

        app_state.logger.buf << "Loaded system file " << system_path.string();
        app_state.logger.log_OK();

        std::filesystem::path bin_path;
        if ( config::get_string( &cvar_settings, "binary", &bin_path) )
            return 1;

        app_state.logger.buf << "Loaded binary file " << bin_path.string();
        app_state.logger.log_OK();

        UserDefinedCPU * cpu;

        //
        // setup lua environment
        //
        if ( system_path.extension() == ".lua" )
        {
            try{
                
                cpu = new LuaCPU( system_path.c_str(), &app_state );
            
            } catch ( std::exception e )
            {
                app_state.logger.buf << "Error while initializing Lua system: " << e.what();
                app_state.logger.log_error_fatal();
                return 1;
            }
        }

        //
        // setup python environment
        //
        if ( system_path.extension() == ".py" )
        {
            try{
                cpu = new PythonCPU( system_path.c_str(), &app_state );
            } catch ( std::exception e )
            {
                app_state.logger.buf << "Error while initializing Python system: " << e.what();
                app_state.logger.log_error_fatal();
                return 1;
            }
        }

        app_state.logger.buf << "Successfully processed system file " << system_path.string();
        app_state.logger.log_OK();

        //
        // load the binary
        //
        std::filebuf bin_buffer;
        bin_buffer.open( bin_path.c_str(), std::ios::in );
        if ( !bin_buffer.is_open() )
        {
            app_state.logger.buf << "Unable to open binary file " << bin_path;
            app_state.logger.log_error_fatal();
            return 1;
        }

        std::istream bin_stream( &bin_buffer );
        app_state.active_bin_stream = &bin_stream;

        //
        // start executing binary
        //

        if ( app_state.custom_instruction_parser )
        {
            if ( !cpu->validate_custom() )
            {
                app_state.logger.buf << "Unable to find custom instruction parser in file " << bin_path.c_str();
                app_state.logger.log_error_fatal();
                return 1;
            }
        }
        else{
            if ( !cpu->validate() )
            {
                app_state.logger.buf << "Unable to find opcode table in file " << bin_path.c_str();
                app_state.logger.log_error_fatal();
                return 1;
            }
        }

        bool should_exit = false;

        while ( !should_exit )
        {
            unsigned char c = bin_stream.get();

            if ( bin_stream.eof() )
            {
                std::cout << "End of binary file reached." << std::endl;
                bin_buffer.close();
                return 0;
            }

            try{
                if ( app_state.custom_instruction_parser )
                    cpu->exec_custom( c );
                else
                    cpu->exec( c );
            } catch ( std::exception e ) {
                app_state.logger.buf << "In main loop, Exception encountered: " << e.what();
                app_state.logger.log_error_fatal();

                bin_buffer.close();
                return 1;
            }

            std::cout << c << std::endl;
            std::cin.get();
        }

        bin_buffer.close();
        return 0;

    }
    else
        std::cout << "Usage: xyzemu <config file>.cfg" << std::endl;

    return 0;
}