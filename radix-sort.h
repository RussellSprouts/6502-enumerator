#pragma once

#include <fstream>
#include <iostream>
#include "stdint.h"

// Implement this to get the bit with the given number
// from the buffer. 0 is the least significant bit.
static inline bool get_bit(const char *buf, const int bit) {
  const int byte = buf[bit/8];
  return (byte >> (bit % 8)) & 1;
}

static void radix_sort(const char *file, const uint8_t size, const uint8_t bits) {
  // Read the input file and output to out0 or out1.
  {
    std::ifstream input(file, std::ifstream::in | std::ifstream::binary);
    std::cout << "Initial read of input." << std::endl;
    std::ofstream out0("out0.tmp", std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
    std::ofstream out1("out1.tmp", std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    while (!input.eof()) {
      char buffer[4096];
      input.read(buffer, 4096);
      std::streamsize dataSize = input.gcount();
      for (int j = 0; j < dataSize; j += size) {
        if (get_bit(buffer + j, 0)) {
          out1.write(buffer + j, size);
        } else {
          out0.write(buffer + j, size);
        }
      }
    }
  }

  // Now read the previous outputs and output to new files.
  for (int i = 1; i < bits; i++) {
    std::cout << "Bit " << i << std::endl;
    std::ofstream out0(i % 2 ? "out0-2.tmp" : "out0.tmp", std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
    std::ofstream out1(i % 2 ? "out1-2.tmp" : "out1.tmp", std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    std::ifstream input0(i % 2 ? "out0.tmp" : "out0-2.tmp", std::ifstream::in | std::ifstream::binary);
    std::ifstream input1(i % 2 ? "out1.tmp" : "out1-2.tmp", std::ifstream::in | std::ifstream::binary);
    while (!input0.eof()) {
      char buffer[4096];
      input0.read(buffer, 4096);
      std::streamsize dataSize = input0.gcount();
      for (int j = 0; j < dataSize; j += size) {
        if (get_bit(buffer + j, i)) {
          out1.write(buffer + j, size);
        } else {
          out0.write(buffer + j, size);
        }
      }
    }

    while (!input1.eof()) {
      char buffer[4096];
      input1.read(buffer, 4096);
      std::streamsize dataSize = input1.gcount();
      for (int j = 0; j < dataSize; j += size) {
        if (get_bit(buffer + j, i)) {
          out1.write(buffer + j, size);
        } else {
          out0.write(buffer + j, size);
        }
      }
    }
  }

  {
    std::ifstream input0(bits % 2 ? "out0.tmp" : "out0-2.tmp", std::ifstream::in | std::ifstream::binary);
    std::ifstream input1(bits % 2 ? "out1.tmp" : "out1-2.tmp", std::ifstream::in | std::ifstream::binary);
    std::ofstream output(file, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

    while (!input0.eof()) {
      char buffer[4096];
      input0.read(buffer, 4096);
      std::streamsize dataSize = input0.gcount();
      output.write(buffer, dataSize);
    }

    while (!input1.eof()) {
      char buffer[4096];
      input1.read(buffer, 4096);
      std::streamsize dataSize = input1.gcount();
      output.write(buffer, dataSize);
    }
  }
}
