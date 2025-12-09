#ifndef SERVER_ERROR_H
#define SERVER_ERROR_H

#include <stdexcept>
#include <string>
#include <utility>

class ServerError: public std::runtime_error {
public:
    explicit ServerError(const std::string& message): std::runtime_error(message) {}
    explicit ServerError(const char* message): std::runtime_error(message) {}
};

#endif  // SERVER_ERROR_H
