#
#ifndef	__SEMAPHORE
#define	__SEMAPHORE

#include	<thread>
#include	<mutex>
#include	<condition_variable>

class	Semaphore {
private:
	std::mutex mtx;
	std::condition_variable cv;
	int count;
public:
	Semaphore (int count_ = 0) : count {count_} {}

void	Release () {
	std::unique_lock<std::mutex>lck (mtx);
	++count;
	cv. notify_one ();
}

void	acquire () {
	std::unique_lock <std::mutex> lck (mtx);
	while (count == 0) {
	   cv. wait (lck);
	}
	-- count;
}

bool	tryAcquire (int delay) {
	std::unique_lock <std::mutex> lck (mtx);
	if (count == 0) {
	   auto now = std::chrono::system_clock::now ();
           cv. wait_until (lck, now + std::chrono::milliseconds (delay));
	}
	if (count == 0)
	   return false;
	-- count;
	return true;
}
};
#endif
