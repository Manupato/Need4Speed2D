#ifndef PEER_CLOSE_ERROR_H
#define PEER_CLOSE_ERROR_H

#include <stdexcept>
#include <string>
#include <utility>

class PeerCloseError: public std::runtime_error {
public:
    explicit PeerCloseError(const std::string& message): std::runtime_error(message) {}
    explicit PeerCloseError(const char* message): std::runtime_error(message) {}
};

#endif  // PEER_CLOSE_ERROR_H
