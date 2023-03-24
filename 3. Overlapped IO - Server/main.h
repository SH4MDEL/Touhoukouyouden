#pragma once
#include "stdafx.h"
#include "server.h"
#include "exp_over.h"

void CALLBACK SendCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED send_over, DWORD recv_flag);
void CALLBACK RecvCallback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED recv_over, DWORD recv_flag);

void TranslatePacket(unsigned long long sid, packet* packetBuf);