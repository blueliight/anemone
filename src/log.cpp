#include "log.hpp"
#include <iostream>

void Logger::log_error_fatal()
{
    std::cout << "\x1b[31;49m!!! Fatal Error : " << this->buf.str() << "\u001b[0m" << std::endl;
    this->buf.str( std::string() );
}

void Logger::log_error()
{
    std::cout << "\x1b[31;49m!! Error : " << this->buf.str() << "\u001b[0m" << std::endl;
    this->buf.str( std::string() );
}

void Logger::log_warning()
{
    std::cout << "\x1b[33;49m! Warning : " << this->buf.str() << "\u001b[0m" << std::endl;
    this->buf.str( std::string() );
}

void Logger::log_OK()
{
    std::cout << "\x1b[32;49m• " << this->buf.str() << "\u001b[0m" << std::endl;
    this->buf.str( std::string() );
}