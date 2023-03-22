#pragma once

#include "resource.h"

extern void InitServer();
extern void	Send(void* packetBuf);
extern bool	Recv();
extern void	TranslatePacket(const packet& packetBuf);