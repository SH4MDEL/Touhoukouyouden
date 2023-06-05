#pragma once
#include "stdafx.h"
#include "server.h"
#include "exp_over.h"
#include "Database.h"

void WorkerThread(HANDLE hiocp);
void TimerThread(HANDLE hiocp);
void ProcessPacket(UINT cid, CHAR* packetBuf);