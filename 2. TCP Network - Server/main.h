#pragma once
#include "stdafx.h"
#include "server.h"

void Send(SOCKET socket, void* packetBuf);
int Recv(SOCKET socket);
void TranslatePacket(SOCKET socket, const packet& packetBuf);