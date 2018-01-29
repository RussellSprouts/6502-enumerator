#include <functional>
#include <vector>
#include <mutex>

using work_item = std::function<int()>;

static const int N_THREADS = std::thread::hardware_concurrency();

struct work_queue {
  
  std::vector<work_item> vec;
  std::mutex mutex;
  unsigned int i = 0;
  bool running = false;
  int result = 0;

  void add(work_item f) {
    mutex.lock();
    if (running) {
      throw "Cannot add after the work queue has started.";
    }
    vec.push_back(f);
    mutex.unlock();
  }

  bool get_task(work_item *out) {
    mutex.lock();
    bool result = false;
    if (i < vec.size()) {
      *out = vec[i++];
      result = true;
    }
    mutex.unlock();
    return result;
  }

  void thread_worker() {
    work_item f;
    while (get_task(&f)) {
      result += f();
    }
  }

  void run() {
    running = true;
    std::thread threads[N_THREADS];
    for (int i = 0; i < N_THREADS; i++) {
      threads[i] = std::thread(&work_queue::thread_worker, this);
    }

    for (int i = 0; i < N_THREADS; i++) {
      threads[i].join();
    }
  }
};
