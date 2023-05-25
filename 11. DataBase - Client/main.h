#pragma once
#include "stdafx.h"
#include "framework.h"

extern void	Send(void* packetBuf);
extern void	Recv();
extern void	TranslatePacket(char* buf, size_t io_byte);
extern void ProcessPacket(char* buf);