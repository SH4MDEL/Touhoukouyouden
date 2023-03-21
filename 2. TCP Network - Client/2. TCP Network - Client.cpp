// 1. Game Client.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "framework.h"
#include "2. TCP Network - Client.h"

#include "mainScene.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
Framework g_framework;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

#define USE_CONSOLE_WINDOW
#ifdef USE_CONSOLE_WINDOW
#ifdef UNICODE
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif
#endif

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // TODO: 여기에 코드를 입력합니다.
    cout << "서버 IP를 입력하세요: ";
    cin >> g_serverIP;


    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY2TCPNETWORKCLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY2TCPNETWORKCLIENT));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            if (msg.message == WM_QUIT) break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 프레임워크 가동
        g_framework.FrameAdvance();
    }

    return (int)msg.wParam;
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = Framework::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MY2TCPNETWORKCLIENT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;//MAKEINTRESOURCEW(IDC_MY1GAMECLIENT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd;

    DWORD dwStyle =
        WS_OVERLAPPED
        | WS_CAPTION
        | WS_SYSMENU
        | WS_MINIMIZEBOX
        | WS_BORDER;

    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    RECT getWinSize;
    GetWindowRect(GetDesktopWindow(), &getWinSize);

    RECT rc;
    rc.left = rc.top = 0;
    rc.right = GetSystemMetrics(SM_CXSCREEN);
    rc.bottom = GetSystemMetrics(SM_CYSCREEN);
    AdjustWindowRect(&rc, dwStyle, FALSE);

    POINT ptClientWorld;
    ptClientWorld.x = (getWinSize.right - GetSystemMetrics(SM_CXSCREEN)) / 2;
    ptClientWorld.y = (getWinSize.bottom - GetSystemMetrics(SM_CYSCREEN)) / 2;

    hWnd = CreateWindowW(
        szWindowClass,
        szTitle,
        dwStyle,
        ptClientWorld.x,
        ptClientWorld.y,
        rc.right - rc.left,
        rc.bottom - rc.top,
        nullptr,
        nullptr,
        hInstance,
        nullptr);

    if (!hWnd) return false;
    if (!g_framework.OnCreate(hInstance, hWnd, rc)) return false;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return true;
}

void InitServer()
{
    wcout.imbue(locale("korean"));
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 0), &WSAData);

    g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);

    SOCKADDR_IN server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, g_serverIP.c_str(), &server_addr.sin_addr);
    connect(g_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    unsigned long noblock = 1;
    ioctlsocket(g_socket, FIONBIO, &noblock);

    cs_packet_login packet;
    packet.size = sizeof(cs_packet_login);
    packet.type = CS_PACKET_LOGIN;
    Send(&packet);
#ifdef NETWORK_DEBUG
    cout << "CS_PACKET_LOGIN 송신" << endl;
#endif
}

void Send(void* packetBuf)
{
    int retval, remain;
    remain = reinterpret_cast<packet*>(packetBuf)->size;
    while (remain > 0) {
        retval = send(g_socket, reinterpret_cast<char*>(packetBuf) + reinterpret_cast<packet*>(packetBuf)->size - remain,
            remain, 0);
        remain -= retval;
    }
}

bool Recv()
{
    packet pk;
    int retval = recv(g_socket, reinterpret_cast<char*>(&pk), sizeof(packet), 0);
    if (retval == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {

        }
#ifdef NETWORK_DEBUG
        else {
            std::cout << "Socket Error in recv" << std::endl;
        }
        return false;
#endif // NETWORK_DEBUG
    }
    TranslatePacket(pk);
    return true;
}

void TranslatePacket(const packet& packetBuf)
{

    int retval;
    switch (packetBuf.type)
    {
    case SC_PACKET_LOGIN_CONFIRM:
    {
        sc_packet_login_confirm pk;
        retval = recv(g_socket, reinterpret_cast<char*>(&pk) + 2, packetBuf.size - 2, 0);
        g_playerID = pk.id;
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_LOGIN_CONFIRM 수신" << endl;
#endif
        break;
    }
    case SC_PACKET_OBJECT_INFO:
    {
        sc_packet_object_info pk;
        retval = recv(g_socket, reinterpret_cast<char*>(&pk) + 2, packetBuf.size - 2, 0);
        MainScene* scene = (MainScene*)g_framework.GetScene();
        scene->GetBoard()->Move(scene->GetPlayer()->GetBoardPosition(), pk.coord);
#ifdef NETWORK_DEBUG
        cout << "SC_PACKET_OBJECT_INFO 수신" << endl;
#endif
        break;
    }
    }
}
