#pragma once
#include "scene.h"

void Scene::Recv()
{
    char buf[BUFSIZE];
    size_t received;
    auto recvResult = g_socket.receive(buf, BUFSIZE, received);

    if (recvResult == sf::Socket::Error) {
#ifdef NETWORK_DEBUG
        cout << "Recv Error!" << endl;
#endif // NETWORK_DEBUG
        exit(-1);
    }
    if (recvResult == sf::Socket::Disconnected) {
#ifdef NETWORK_DEBUG
        cout << "Disconnected!" << endl;
#endif // NETWORK_DEBUG
        exit(-1);
    }
    if (recvResult != sf::Socket::NotReady) {
        if (received > 0) {
            TranslatePacket(buf, received);
        }
    }
}

void Scene::TranslatePacket(char* buf, size_t io_byte)
{
    char* packet = buf;
    static size_t remainPacketSize = 0;
    static char remainPacketBuffer[BUFSIZE];

    memcpy(remainPacketBuffer + remainPacketSize, packet, io_byte);
    // 이전 남은 버퍼에 이번에 받은 패킷을 이어 붙임
    remainPacketSize += io_byte;
    // 이번에 받은 데이터 사이즈만큼 더함
    while (remainPacketSize > 0) {
        // 남은 데이터 사이즈가 0보다 클 시
        int packetSize = remainPacketBuffer[0];

        if (remainPacketSize < packetSize) break;
        // 하나의 온전한 패킷을 만들기 위한 사이즈보다
        // 모자랄 시 탈출

        ProcessPacket(remainPacketBuffer);
        // 패킷 처리

        packet += packetSize;
        remainPacketSize -= packetSize;
        if (remainPacketSize != 0) {
            memcpy(remainPacketBuffer, packet, remainPacketSize);
        }
    }
}