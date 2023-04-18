#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
using namespace std;
using namespace std::chrono;

constexpr int MAX_THREAD = 64;

mutex m;
atomic_int ai = 0;
int sum = 0;

void lock()
{
	int lock_value = 0;
	while (!atomic_compare_exchange_strong(&ai, &lock_value, 1)) lock_value = 0;
}

void unlock()
{
	ai = 0;
}

// 결과
// 쓰레드가 늘어날수록 기하급수적으로 느려진다.
// 
// 왜?
// 메모리 버스에 locking하고 동작하는데, 이 동작의 오버헤드가 엄청나게 심하다.
// 여러 쓰레드가 locking하면 더더욱 느려진다.
// 쓰레드가 많아지면 convoying 현상이 일어난다.
// -> 뮤텍스가 낫다는 결론?

void out_th(int threadid, int num_thread)
{
	for (int i = 0; i < 50000000 / num_thread; ++i) {
		lock();
		sum += 2;
		unlock();
	}
}

int main()
{
	//register int sum = 0;
	for (int num_thread = 1; num_thread <= MAX_THREAD; num_thread *= 2) {
		sum = 0;
		vector<thread> workers;

		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num_thread; ++i)
			workers.emplace_back(out_th, i, num_thread);
		for (auto& t : workers) t.join();
		auto end_t = high_resolution_clock::now();
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		cout << "thread num : " << num_thread << ", sum : " << sum << ", " << exec_ms << "ms" << endl;
	}
	//int num_core = thread::hardware_concurrency();
	//// 논리 프로세서의 개수를 받아온다.


}