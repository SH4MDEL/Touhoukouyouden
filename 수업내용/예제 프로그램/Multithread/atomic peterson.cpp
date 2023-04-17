#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
#include <unordered_map>
using namespace std;

atomic<int> victim;
atomic<bool> flag[2] = { false, false };
volatile int sum;

void lock(int tid)
{
	int other = 1 - tid;
	flag[tid] = true;
	victim = tid;
	while (flag[other] && victim == tid);

}

void unlock(int tid)
{
	flag[tid] = false;
}

void receiver(int tid)
{
	for (int i = 0; i < 25000000; ++i) {
		lock(tid);
		sum += 2;
		unlock(tid);
	}
}

int main()
{
	thread t1(receiver, 0);
	thread t2(receiver, 1);

	t1.join();
	t2.join();

	cout << sum << endl;

	// Out of order Execution
	// Write Buffering
	// Write Buffer에 기록된 값을 먼저 확인하기 때문에
	// 싱글코어에서는 문제가 발생하지 않는다.
}