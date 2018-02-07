#include <functional>
#include <vector>
#include <mutex>


static const int N_THREADS = std::thread::hardware_concurrency();

template<typename ThreadStorage>
struct work_queue {
  
  using work_item = std::function<void(ThreadStorage&)>;
  std::vector<ThreadStorage> stores;
  std::vector<work_item> vec;
  std::mutex mutex;
  unsigned int i = 0;
  bool running = false;

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

  void thread_worker(ThreadStorage &store) {
    work_item f;
    while (get_task(&f)) {
      f(store);
    }
  }

  void run() {
    running = true;
    for (int i = 0; i < N_THREADS; i++) {
      ThreadStorage t;
      stores.push_back(t);
    }

    std::thread threads[N_THREADS - 1];
    for (int i = 0; i < N_THREADS - 1; i++) {
      threads[i] = std::thread(&work_queue::thread_worker, this, std::ref(stores[i]));
    }
    thread_worker(stores[N_THREADS - 1]);

    for (int i = 0; i < N_THREADS-1; i++) {
      threads[i].join();
    }
  }
};
