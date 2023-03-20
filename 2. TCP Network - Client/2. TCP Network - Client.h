#pragma once

#include "resource.h"

extern void InitServer();
extern void	Send(void* packetBuf);
extern void	Recv();
extern void	TranslatePacket(const packet& packetBuf);