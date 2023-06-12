#pragma once
#include "stdafx.h"
#include "server.h"
#include "expOverlapped.h"
#include "database.h"

void WorkerThread(HANDLE hiocp);
void TimerThread(HANDLE hiocp);
void DatabaseThread(HANDLE hiocp);
void ProcessPacket(UINT cid, CHAR* packetBuf);