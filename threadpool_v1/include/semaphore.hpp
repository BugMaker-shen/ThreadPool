#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
	Semaphore(int srcLimit = 0);
	~Semaphore();

	// P
	void wait();
	// V
	void post();

private:
	// 资源计数
	int srcLimit_;
	std::mutex mtx_;
	std::condition_variable cv_;
};

#endif