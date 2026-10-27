// Exceptions, iostreams and the gthread primitives libstdc++ uses.
// amigaos.exp builds it for each thread model.  With -athread=single,
// std::thread throws, so the thread is only started when the model
// has threads.
// { dg-do run }

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <bits/gthr.h>
static std::mutex mtx;
static std::timed_mutex tmtx;
static std::once_flag once;
static std::condition_variable cv;
int main()
{
  try { throw std::runtime_error("boom"); }
  catch (const std::exception &e) { std::cout << e.what() << '\n'; }
  std::call_once(once, [] { std::cout << "once\n"; });
  {
    // A timed wait before any thread exists.
    std::unique_lock<std::mutex> l(mtx);
    cv.wait_for(l, std::chrono::milliseconds(10));
  }
  if (__gthread_active_p ())
    {
      std::thread t([] { std::lock_guard<std::mutex> l(mtx); cv.notify_one(); });
      t.join();
    }
  if (tmtx.try_lock_for(std::chrono::milliseconds(1)))
    tmtx.unlock();
  return 0;
}
