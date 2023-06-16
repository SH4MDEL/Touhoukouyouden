#pragma once
#include "stdafx.h"
#include "server.h"
#include "expOverlapped.h"
#include "timer.h"
#include "database.h"
#include "setting.h"

void WorkerThread(HANDLE hiocp);
void TimerThread(HANDLE hiocp);
void DatabaseThread(HANDLE hiocp);
void ProcessPacket(UINT cid, CHAR* packetBuf);