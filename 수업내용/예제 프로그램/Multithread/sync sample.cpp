#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
using namespace std;

// volatile int* sendData = nullptr;
// int가 volatile이지 주소가 volatile이 아니다.
int* volatile sendData = nullptr;

void receiver()
{
	while (!sendData);
	cout << *sendData;
	delete sendData;
}

void sender()
{
	sendData = new int{ 999 };
}

int main()
{
	thread t1(receiver);
	thread t2(sender);

	t1.join();
	t2.join();
}