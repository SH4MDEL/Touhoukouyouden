#pragma once
#include "scene.h"

void Scene::Recv()
{
    char buf[BUF_SIZE];
    size_t received;
    auto recvResult = g_socket.receive(buf, BUF_SIZE, received);

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
    static char remainPacketBuffer[BUF_SIZE];

    memcpy(remainPacketBuffer + remainPacketSize, packet, io_byte);
    // ���� ���� ���ۿ� �̹��� ���� ��Ŷ�� �̾� ����
    remainPacketSize += io_byte;
    // �̹��� ���� ������ �����ŭ ����
    while (remainPacketSize > 0) {
        // ���� ������ ����� 0���� Ŭ ��
        int packetSize = remainPacketBuffer[0];

        if (remainPacketSize < packetSize) break;
        // �ϳ��� ������ ��Ŷ�� ����� ���� �������
        // ���ڶ� �� Ż��

        ProcessPacket(remainPacketBuffer);
        // ��Ŷ ó��

        packet += packetSize;
        remainPacketSize -= packetSize;
        if (remainPacketSize != 0) {
            memcpy(remainPacketBuffer, packet, remainPacketSize);
        }
    }
}