#ifndef LOG_HPP
#define LOG_HPP

#include <sstream>
struct Logger
{
    std::stringstream buf; 
    void log_error_fatal();
    void log_error();
    void log_warning();
    void log_OK();
};

#endif