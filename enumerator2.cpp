#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <iostream>
#include "stdint.h"
#include "inttypes.h"
#include "instructions2.h"
#include "emulator2.h"
#include "random_machine.h"
#include "z3++.h"
#include "abstract_machine.h"
#include "fnv.h"
#include "radix-sort.h"

constexpr int max_cost = 140;

z3::check_result canBeDifferent(z3::solver &s, const abstract_machine &ma, const abstract_machine &mb) {
  s.push();
  s.add(!(
    ma._earlyExit == mb._earlyExit &&
    ma._ccS == mb._ccS &&
    ma._ccV == mb._ccV &&
    ma._ccD == mb._ccD &&
    ma._ccI == mb._ccI &&
    ma._ccC == mb._ccC &&
    ma._ccZ == mb._ccZ &&
    ma._a == mb._a &&
    ma._x == mb._x &&
    ma._y == mb._y &&
    ma._sp == mb._sp &&
    ma._memory == mb._memory
  ));
  
  auto result = s.check();
  s.pop();
  return result;
}

void write_seq(std::vector<std::ofstream> &outfiles, instruction_seq &seq) {
  //outfiles[seq.cycles].write(seq.data(), seq.data_size());
}

typedef struct execution_hash {
  typedef std::array<uint8_t, 18> buffer_t;

  uint64_t alwaysIncluded;
  uint16_t a;
  uint16_t x;
  uint16_t y;
  uint16_t ccS;
  uint16_t ccV;
  uint16_t ccC;
  uint16_t ccZ;

  buffer_t as_buffer() const {
    buffer_t result = {0};
    result[0] = alwaysIncluded;
    result[1] = alwaysIncluded >> 8;
    result[2] = alwaysIncluded >> 16;
    result[3] = alwaysIncluded >> 24;
    result[4] = alwaysIncluded >> 32;
    result[5] = alwaysIncluded >> 40;
    result[6] = alwaysIncluded >> 48;
    result[7] = alwaysIncluded >> 56;
    return result;
  }

  static execution_hash from_buffer(buffer_t buffer) {
    execution_hash result;
    result.alwaysIncluded = buffer[0]
      | (((uint64_t)buffer[1]) << 8)
      | (((uint64_t)buffer[2]) << 16)
      | (((uint64_t)buffer[3]) << 24)
      | (((uint64_t)buffer[4]) << 32)
      | (((uint64_t)buffer[5]) << 40)
      | (((uint64_t)buffer[6]) << 48)
      | (((uint64_t)buffer[7]) << 56);
    return result;
  }
} execution_hash;

execution_hash hash(instruction_seq seq) {
  random_machine rms[16] {
    // machines with all 0s or all 1s
    random_machine(0),
    random_machine(0xffffffff),
    // simple patterns
    random_machine(0x12345678),
    random_machine(0xCAFEFACE),
    // hex digits of PI
    random_machine(0x243F6A88),
    random_machine(0x85A308D3),
    random_machine(0x13198A2E),
    random_machine(0x03707344),
    random_machine(0xA4093822),
    random_machine(0x299F31D0),
    random_machine(0x082EFA98),
    random_machine(0xEC4E6C89),
    random_machine(0x452821E6),
    random_machine(0x38D01377),
    random_machine(0xBE5466CF),
    random_machine(0x34E90C6C),
  };
  for (auto instruction : seq.instructions) {
    for (auto &rm : rms) {
      rm.instruction(instruction);
    }
  }

  const uint32_t seed = 0x18480949;
  fnv_hash hash_all(seed);
  fnv_hash hash_a(seed);
  fnv_hash hash_x(seed);
  fnv_hash hash_y(seed);
  fnv_hash hash_ccS(seed);
  fnv_hash hash_ccV(seed);
  fnv_hash hash_ccC(seed);
  fnv_hash hash_ccZ(seed);
  for (auto rm : rms) {
    hash_all.add(rm.hash());
    hash_a.add(rm._a);
    hash_x.add(rm._x);
    hash_y.add(rm._y);
    hash_ccS.add((uint8_t)(rm._ccS != 0));
    hash_ccV.add((uint8_t)(rm._ccV != 0));
    hash_ccC.add((uint8_t)(rm._ccC != 0));
    hash_ccZ.add((uint8_t)(rm._ccZ != 0));
  }
  
  return execution_hash {
    .alwaysIncluded = hash_all.hash64(),
    .a = hash_a.hash16(),
    .x = hash_x.hash16(),
    .y = hash_y.hash16(),
    .ccS = hash_ccS.hash16(),
    .ccV = hash_ccV.hash16(),
    .ccC = hash_ccC.hash16(),
    .ccZ = hash_ccZ.hash16(),
  };
}

typedef struct hash_output_file {
  static const int hash_size_used = 8;
  static const int hash_size = 18;
  static const int instructions_size = 14;
  static const int total_size = 32;
  std::ofstream file;

  hash_output_file(uint8_t cost, bool trunc) : file("out/result-" + std::to_string(cost) + ".dat", std::ofstream::binary | std::ofstream::out | (trunc ? std::ofstream::trunc : std::ofstream::app)) {}

  void write(const execution_hash &hash, const instruction_seq &seq) {
    write_hash(hash);
    write_instructions(seq);
  }

  void write_hash(const execution_hash &hash) {
    auto data = hash.as_buffer();
    file.write((char*)data.data(), data.size());
  }

  void write_instructions(const instruction_seq &seq) {
    for (auto &instruction : seq.instructions) {
      file.put(instruction.data);
      file.put(instruction.data >> 8);
    }
  }
} hash_output_file;

typedef struct hash_input_file {
  std::ifstream file;
  execution_hash last;

  hash_input_file(uint8_t cost) : file("out/result-" + std::to_string(cost) + ".dat", std::ifstream::binary | std::ifstream::in) {
    last.alwaysIncluded = 0;
  }

  // Assumes the input file is sorted by hash. Reads until it
  // finds items with the given hash, and returns a vector of
  // the results. Consumes input, so each hash argument must
  // be greater than the last.
  std::vector<execution_hash> get_hashes(uint64_t hash) {
    std::vector<execution_hash> result;
    char buffer[hash_output_file::total_size];

    if (last.alwaysIncluded == hash) {
      result.push_back(last);
    }

    while (!file.eof() && last.alwaysIncluded <= hash) {
      std::cout << "hash: " << last.alwaysIncluded << std::endl;
      file.read(buffer, hash_output_file::total_size);
      execution_hash::buffer_t hash_buffer;
      for (uint32_t i = 0; i < hash_buffer.size(); i++) {
        hash_buffer[i] = buffer[i];
      }
      last = execution_hash::from_buffer(hash_buffer);
      if (last.alwaysIncluded == hash) {
        result.push_back(last);
      }
    }

    return result;
  }
} hash_input_file;

void display_hash_result(uint8_t *buffer) {
  // diplay hash
  uint64_t hash = 0;
  for (int i = 7; i >= 0; i--) {
    hash <<= 8;
    hash |= buffer[i];
  }

  printf("%016" PRIx64 " ", hash);
  for (int i = hash_output_file::hash_size; i < hash_output_file::total_size; i += 2) {
    uint16_t data = buffer[i] + (buffer[i + 1] << 8);
    instruction ins;
    ins.data = data;
    if (ins.name() == instruction_name::NONE) { break; }
    for (const auto &info : instructions) {
      if (ins.name() == info.ins.name() && ins.mode() == info.ins.mode()) {
        std::cout << info.desc << " " << addr_mode_operand_name(ins.mode(), ins.number()) << "; ";
      }
    }
  }  
  std::cout << std::endl;
}

typedef struct output_file_manager {
  uint8_t start;  
  std::vector<hash_output_file> outfiles;

  output_file_manager(uint8_t start) : start(start) {
    for (int i = start; i <= max_cost; i++) {
      outfiles.push_back(hash_output_file(i, false));
    }
  }

  hash_output_file &get_file(uint8_t cost) {
    return outfiles[cost - start];
  }
} output_file_manager;

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << std::endl;
    return 1;
  }

  // Create all of the output files.
  std::vector<hash_output_file> outfiles;
  std::string arg1(argv[1]);
  if (arg1 == "init") {
    for (int i = 0; i <= max_cost; i++) {
      outfiles.push_back(hash_output_file(i, true));
    }

    std::cout << "Initializing" << std::endl;
    instruction_seq empty_seq;
    auto hash_result = hash(empty_seq);
    outfiles[0].write(hash_result, empty_seq);

    int total_instructions = 0;
    for (auto &instruction : instructions) {
      int variants = addr_mode_variants(instruction.ins.mode());
      for (int variant = 0; variant < variants; variant++) {
        total_instructions++;
        instruction_seq seq;
        instruction_info instruction_variant = instruction;
        instruction_variant.ins = instruction_variant.ins.number(variant);
        seq = seq.add(instruction_variant);
        auto hash_result = hash(seq);
        outfiles[seq.cycles].write(hash_result, seq);
      }
    }

    std::cout << "Seeded " << total_instructions << " total instructions" << std::endl;
  } else if (arg1 == "view") {
    std::string arg2(argv[2]);
    std::ifstream view_file(arg2, std::ifstream::binary | std::ifstream::in);

    std::cout << "Opening " << arg2 << std::endl;
    while (!view_file.eof()) {
      char buffer[4096];
      view_file.read(buffer, 4096);
      std::streamsize dataSize = view_file.gcount();
      for (int j = 0; j < dataSize; j += hash_output_file::total_size) {
        display_hash_result((uint8_t*)(buffer + j));
      }
    }
  } else {
    int target = std::stoi(argv[1]);
    output_file_manager output_files(target + 1);
    std::string file_name = std::string("out/result-") + argv[1] + ".dat";
    std::cout << "Processing sequences with length " << target << std::endl;
 
    // sort the file by hash
    std::cout << "Sorting file:" << std::endl;
    radix_sort(file_name.c_str(), hash_output_file::total_size, hash_output_file::hash_size_used * 8);

    std::ifstream view_file(file_name, std::ifstream::binary | std::ifstream::in);
    // For each instruction type
    for (const auto &ins_info : instructions) {
      std::cout << "INSTRUCTION: " << (int)ins_info.ins.name() << std::endl;
      int variants = addr_mode_variants(ins_info.ins.mode());
      // For each variant of the instruction
      for (int variant = 0; variant < variants; variant++) {
        std::cout << "VARIANT: " << variant << std::endl;

        view_file.clear();
        view_file.seekg(0, std::ifstream::beg);
        // For each section of the file
        while (!view_file.eof()) {
          char buffer[4096];
          view_file.read(buffer, 4096);
          std::streamsize data_size = view_file.gcount();
          // For each instruction sequence hash in the file
          for (int buffer_start = 0; buffer_start < data_size; buffer_start += hash_output_file::total_size) {
            uint8_t *buffer_ptr = (uint8_t*)(buffer + buffer_start);
            instruction_seq seq;
            // For each instruction in the sequence
            for (int i = hash_output_file::hash_size; i < hash_output_file::total_size; i += 2) {
              uint16_t data = buffer_ptr[i] | (buffer_ptr[i + 1] << 8);
              instruction ins;
              ins.data = data;
              if (ins.name() == instruction_name::NONE) { break; }
              for (const auto info : instructions) {
                if (ins.name() == info.ins.name() && ins.mode() == info.ins.mode()) {
                  seq = seq.add(info);
                  break;
                }
              }
            }

            instruction_info next_instruction = ins_info;
            next_instruction.ins = next_instruction.ins.number(variant);
            seq = seq.add(next_instruction);
            auto hash_result = hash(seq);
            output_files.get_file(seq.cycles).write(hash_result, seq);
          }
        }
      }
    }
  }
}
