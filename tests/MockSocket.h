#ifndef MOCK_SOCKET_H
#define MOCK_SOCKET_H

#include <gmock/gmock.h>

#include "../common/ISocket.h"

class MockSocket: public ISocket {
public:
    MOCK_METHOD(int, sendall, (const void* data, unsigned int sz), (override));
    MOCK_METHOD(int, recvall, (void* data, unsigned int size), (override));

    MOCK_METHOD(int, close, (), (override));

    MOCK_METHOD(bool, is_stream_send_closed, (), (const, override));
    MOCK_METHOD(bool, is_stream_recv_closed, (), (const, override));
};

#endif
