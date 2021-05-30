#include "stdio.h"
#include "stdint.h"

/**
 * Streams items from a file.
 * The type t should have a static method from_buffer, which takes
 * a uint8_t* and returns t. Size is the size of each element.
 */
template<typename t, size_t size>
struct file_stream {
  std::queue<t> queue;
  std::ifstream file;
  bool done;

  explicit file_stream(std::string filename) : file(filename, std::ifstream::binary | std::ifstream::in) {
    refill_queue();
  }

  void refill_queue() {
    if (file.eof()) { return;}
    if (queue.empty()) {
      char buffer[size * 256];
      file.read(buffer, size * 256);
      std::streamsize dataSize = file.gcount();
      for (int j = 0; j < dataSize; j += size) {
        t item = t::from_buffer((uint8_t*)(buffer + j));
        queue.push(item);
      }
    }
  }

  bool has_next() {
    refill_queue();
    return !queue.empty();
  }

  t peek() {
    refill_queue();
    return queue.front();
  }

  t next() {
    refill_queue();
    t result = queue.front();
    queue.pop();
    return result;
  }
};