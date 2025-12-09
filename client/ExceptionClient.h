#ifndef EXCEPTION_CLIENT_H
#define EXCEPTION_CLIENT_H

#include <stdexcept>
#include <string>

class ExceptionClient: public std::runtime_error {
public:
    explicit ExceptionClient(const std::string& msg):
            std::runtime_error("Error en Client: " + msg) {}
};

#endif
