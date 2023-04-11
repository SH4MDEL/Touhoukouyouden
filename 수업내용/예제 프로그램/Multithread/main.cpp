#include <thread>
#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
using namespace std;
using namespace std::chrono;

constexpr int MAX_THREAD = 16;

mutex m;
volatile int sum = 0;
struct NUM {
	alignas(64) volatile int sum;

};

volatile NUM t_sum[MAX_THREAD];

void out_th(int threadid, int num_thread)
{
	for (int i = 0; i < 50000000 / num_thread; ++i) {
		t_sum[threadid].sum += 2;
	}
}

int main()
{
	//register int sum = 0;
	for (int num_thread = 1; num_thread <= MAX_THREAD; num_thread *= 2) {
		sum = 0;
		for (auto& v : t_sum) v.sum = 0;

		vector<thread> workers;

		auto start_t = high_resolution_clock::now();
		for (int i = 0; i < num_thread; ++i)
			workers.emplace_back(out_th, i, num_thread);
		for (auto& t : workers) t.join();
		auto end_t = high_resolution_clock::now();
		for (auto v : t_sum) sum += v.sum;
		auto exec_t = end_t - start_t;
		auto exec_ms = duration_cast<milliseconds>(exec_t).count();

		cout << sum << ", " << exec_ms << endl;
	}
	//int num_core = thread::hardware_concurrency();
	//// 논리 프로세서의 개수를 받아온다.


}