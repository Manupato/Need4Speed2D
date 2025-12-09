#include "operations_bytes.h"

void OperationsBytes::add_one_byte(uint8_t code, std::vector<uint8_t>& buf) { buf.push_back(code); }

void OperationsBytes::add_two_bytes(uint16_t value, std::vector<uint8_t>& buf) {
    value = htons(value);
    const auto pos = buf.size();
    buf.resize(pos + sizeof(value));
    std::memcpy(buf.data() + pos, &value, sizeof(value));
}

void OperationsBytes::add_four_bytes(uint32_t value, std::vector<uint8_t>& buf) {
    value = htonl(value);
    const auto pos = buf.size();
    buf.resize(pos + sizeof(value));
    std::memcpy(buf.data() + pos, &value, sizeof(value));
}

void OperationsBytes::add_string(const std::string& str, std::vector<uint8_t>& buf) {
    buf.insert(std::end(buf), str.begin(), str.end());
}

uint8_t OperationsBytes::receive_one_byte(ISocket& skt) {
    uint8_t byte = 0;
    if (skt.recvall(&byte, sizeof(byte)) == 0) {
        throw PeerCloseError(
                "OperationsBytes: El peer ha sido cerrado al intentar recibir un byte");
    }
    return byte;
}

uint16_t OperationsBytes::receive_two_bytes(ISocket& skt) {
    uint16_t value = 0;
    if (skt.recvall(&value, sizeof(value)) == 0) {
        throw PeerCloseError(
                "OperationsBytes: El peer ha sido cerrado al intentar recibir dos bytes");
    }
    return ntohs(value);
}
uint32_t OperationsBytes::receive_four_bytes(ISocket& skt) {
    uint32_t value = 0;
    if (skt.recvall(&value, sizeof(value)) == 0) {
        throw PeerCloseError(
                "OperationsBytes: El peer ha sido cerrado al intentar recibir cuatro bytes");
    }
    return ntohl(value);
}

std::string OperationsBytes::receive_string(size_t length, ISocket& skt) {
    std::string str(length, '\0');
    if (length > 0) {
        if (skt.recvall(str.data(), length) == 0) {
            throw PeerCloseError(
                    "OperationsBytes: El peer ha sido cerrado al intentar recibir un string");
        }
    }
    return str;
}
