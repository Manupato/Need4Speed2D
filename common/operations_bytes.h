#ifndef OPERATIONS_BYTES_H
#define OPERATIONS_BYTES_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include <arpa/inet.h>

#include "ISocket.h"
#include "peer_close_error.h"

class OperationsBytes {

public:
    OperationsBytes() = default;

    // Agregamos al buffer buf un byte
    static void add_one_byte(uint8_t code, std::vector<uint8_t>& buf);

    // Agrega al buffer un uint16_t
    static void add_two_bytes(uint16_t value, std::vector<uint8_t>& buf);

    // Agrega al buffer un uint32_t
    static void add_four_bytes(uint32_t value, std::vector<uint8_t>& buf);

    // Agrega al buffer un string
    static void add_string(const std::string& str, std::vector<uint8_t>& buf);

    // Recibe un byte de un socket
    static uint8_t receive_one_byte(class ISocket& skt);

    // Recibe un uint16_t de un socket
    static uint16_t receive_two_bytes(class ISocket& skt);

    // Recibe un uint32_t de un socket
    static uint32_t receive_four_bytes(class ISocket& skt);

    // Recibe un string de un socket
    static std::string receive_string(size_t length, class ISocket& skt);

    ~OperationsBytes() = default;
    OperationsBytes(const OperationsBytes& other) = delete;
    OperationsBytes& operator=(const OperationsBytes& other) = delete;
};

#endif  // OPERATIONS_BYTES_H
