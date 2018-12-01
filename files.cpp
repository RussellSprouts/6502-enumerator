#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "sys/stat.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"

std::string int_to_hex(int i) {
  std::stringstream stream;
  stream << std::setfill('0') << std::setw(2) << std::hex << i;
  return stream.str();
}

static FILE* files[256*256];

void makeFiles() {
  mkdir("out", S_IRWXU);
  for (int i = 0; i < 256; i++) {
    std::string hex = "out/" + int_to_hex(i);
    mkdir(hex.c_str(), S_IRWXU);
    for (int j = 0; j < 256; j++) {
      std::string file = hex + "/" + int_to_hex(j);
      FILE *desc = fopen(file.c_str(), "wb");
      if (desc == NULL) {
        int err = errno;
        std::cerr << "Error opening file: " << strerror(err) << " " << file << std::endl;
        exit(-1);
      }
      fclose(desc);
      std::cout << file << std::endl;
    }
  }
}

int main(int argc, char **argv) {
  makeFiles();
}
