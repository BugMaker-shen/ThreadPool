#include "threadpool_v1/threadpool.hpp"

#include <memory>
#include <iostream>

using namespace std;
using ull = unsigned long long;

class MyTask : public Task {
public:
	MyTask(int begin, int end)
		: begin_(begin)
		, end_(end)
	{}

	const Any run() {
		std::cout << "tid = " << std::this_thread::get_id() << " begin MyTask::run()" << std::endl;
		ull sum = 0;
		for (int i = begin_; i <= end_; i++) {
			sum += i;
		}
		// std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "tid = " << std::this_thread::get_id() << " end MyTask::run()" << std::endl; 
		return Any(sum);
	}

private:
	int begin_;
	int end_;
};

int main() {
	{
		ThreadPool pool;
		pool.setMode(PoolMode::MODE_CACHED);
		pool.start(2);
		Result res1 = pool.submitTask(std::make_shared<MyTask>(1, 1000000));
		Result res2 = pool.submitTask(std::make_shared<MyTask>(1, 1000000));
		Result res3 = pool.submitTask(std::make_shared<MyTask>(1, 1000000));
		Result res4 = pool.submitTask(std::make_shared<MyTask>(1, 1000000));
		Result res5 = pool.submitTask(std::make_shared<MyTask>(1, 1000000));
		cout << res1.get().cast_<ull>() + res2.get().cast_<ull>() + res3.get().cast_<ull>() + res4.get().cast_<ull>() + res5.get().cast_<ull>() << endl;
	}
	cout << "main over" << endl;
	return 0;
}

