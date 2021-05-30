#include <fstream>
#include <vector>
#include <map>
#include <array>
#include <queue>
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
#include "file_stream.h"
#include <gperftools/profiler.h>

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

typedef struct execution_hash {
  typedef std::array<uint8_t, 8> buffer_t;

  uint64_t alwaysIncluded;

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

  static execution_hash from_buffer(uint8_t buffer[8]) {
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

static const random_machine initial_machines[16] {
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

execution_hash hash(instruction_seq seq) {
  random_machine rms[16];
  memcpy(rms, initial_machines, sizeof(rms));
  emulator<random_machine> emu;
  for (auto instruction : seq.instructions) {
    for (auto &rm : rms) {
      emu.instruction(rm, instruction);
    }
  }

  const uint32_t seed = 0x18480949;
  fnv_hash hash_all(seed);
  for (auto &rm : rms) {
    hash_all.add(rm.hash());
  }
  
  return execution_hash {
    .alwaysIncluded = hash_all.hash64(),
  };
}

typedef struct hash_output_file {
  static const int hash_size_used = 8;
  static const int hash_size = 8;
  static const int instructions_size = 14;
  static const int total_size = 22;
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

  static instruction_seq seq_from_buffer(const uint8_t buf[total_size]) {
    instruction_seq seq;
    // For each instruction in the sequence
    for (int i = hash_output_file::hash_size; i < hash_output_file::total_size; i += 2) {
      uint16_t data = buf[i] | (buf[i + 1] << 8);
      instruction ins;
      ins.data = data;
      if (ins.name() == instruction_name::NONE) { break; }
      seq = seq.add(get_instruction_info(ins));
    }
    return seq;
  }
} hash_output_file;

typedef struct hash_sequence {
  execution_hash hash;
  instruction_seq seq;

  static hash_sequence from_buffer(uint8_t *buffer) {
    return hash_sequence {
      execution_hash::from_buffer(buffer),
      hash_output_file::seq_from_buffer(buffer),
    };
  }

  static bool get_sort_bit(const uint8_t *buffer, const uint8_t bit) {
    if (bit < 8) {
      instruction_seq seq = hash_output_file::seq_from_buffer(buffer);
      return (seq.bytes >> (bit % 8)) & 1;
    } else {
      const int byte = buffer[bit/8 - 1];
      return (byte >> (bit % 8)) & 1;
    }
  }
} hash_sequence;

typedef struct hash_input_file {
  file_stream<hash_sequence, hash_output_file::total_size> stream;
  uint8_t cost;

  explicit hash_input_file(uint8_t cost) : cost(cost), stream("out/result-" + std::to_string(cost) + ".dat") {
  }

  // Assumes the input file is sorted by hash. Reads until it
  // finds items with the given hash, and returns a vector of
  // the results. Consumes input, so each hash argument must
  // be greater than the last.
  std::vector<instruction_seq> get_hashes(uint64_t hash) {
    std::vector<instruction_seq> result;
    while (stream.has_next()) {
      if (stream.peek().hash.alwaysIncluded < hash) {
        stream.next();
      } else if (stream.peek().hash.alwaysIncluded == hash) {
        result.push_back(stream.next().seq);
      } else {
        break;
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

  instruction_seq needle;
  needle = needle.add(get_instruction_info(instruction(instruction_name::ADC, addr_mode::ZERO_PAGE, 0)));
  needle = needle.add(get_instruction_info(instruction(instruction_name::LDA, addr_mode::IMMEDIATE, 0)));

  printf("%016" PRIx64 " ", hash);
  for (int i = hash_output_file::hash_size; i < hash_output_file::total_size; i += 2) {
    uint16_t data = buffer[i] + (buffer[i + 1] << 8);
    instruction ins;
    ins.data = data;
    if (ins.name() == instruction_name::NONE) { break; }
    const auto info = get_instruction_info(ins);
    std::cout << info.desc << " " << addr_mode_operand_name(ins.mode(), ins.number()) << "; ";
  }
  std::cout << std::endl;

  instruction_seq seq = hash_output_file::seq_from_buffer(buffer);
  if (seq.contains(needle)) {
    std::cout << "FOUND THE NEEDLE" << std::endl;
  }

  std::cout << "CANONICAL:       ";
  for (instruction ins : seq.instructions) {
    if (ins.name() == instruction_name::NONE) { break; }
    const auto info = get_instruction_info(ins);
    std::cout << info.desc << " " << addr_mode_operand_name(ins.mode(), ins.number()) << "; ";
  }
  std::cout << std::endl;
}

typedef struct output_file_manager {
  const uint8_t start;  
  std::vector<hash_output_file> outfiles;

  explicit output_file_manager(uint8_t start) : start(start) {
    for (int i = start; i <= max_cost; i++) {
      outfiles.push_back(hash_output_file(i, false));
    }
  }

  hash_output_file  &get_file(uint8_t cost) {
    return outfiles.at(cost - start);
  }
} output_file_manager;

typedef struct input_file_manager {
  uint8_t end;
  std::vector<hash_input_file> infiles;

  explicit input_file_manager(uint8_t end) : end(end) {
    for (int i = 0; i < end; i++) {
      infiles.push_back(hash_input_file(i));
    }
  }
} input_file_manager;

typedef struct optimization_output_file {
  std::ofstream file;
  optimization_output_file(std::string path) : file(path, std::ofstream::out | std::ofstream::binary | std::ofstream::app) {}
} optimization_output_file;

void process_sequences(z3::context &ctx, uint64_t hash, const std::vector<instruction_seq> &smaller_sequences, const std::vector<instruction_seq> &sequences) {
  z3::solver solver(ctx);
  for (size_t i = 0; i < sequences.size(); i++) {
    if (!sequences[i].is_canonical()) { 
      continue;
    }
    abstract_machine ma(ctx);
    ma.instructions(sequences[i]);

    // Compare against smaller sequences for optimizations
    for (size_t j = 0; j < smaller_sequences.size(); j++) {
      abstract_machine mb(ctx);
      mb.instructions(smaller_sequences[j]);
      const auto result = canBeDifferent(solver, ma, mb);
      if (result == z3::unsat) {
        std::cout << "OPTS!: ";
        for (const auto &ins : sequences[i].instructions) {
          if (ins.name() == instruction_name::NONE) { break; }
          const auto info = get_instruction_info(ins);
          std::cout << info.desc << " " << addr_mode_operand_name(ins.mode(), ins.number()) << "; ";
        }
        std::cout << " AND ";
        for (const auto &ins : smaller_sequences[j].instructions) {
          if (ins.name() == instruction_name::NONE) { break; }
          const auto info = get_instruction_info(ins);
          std::cout << info.desc << " " << addr_mode_operand_name(ins.mode(), ins.number()) << "; ";
        }
        std::cout << std::endl;
        goto next_sequence;
      } else if (result == z3::unknown) {
        std::cout << "UNKNOWN!" << std::endl;
      }
    }

    // Compare against sequences with the same cost but fewer bytes for equivalences.
    for (size_t j = 0; j < i && sequences[j].bytes < sequences[i].bytes; j++) { 
      abstract_machine mb(ctx);
      mb.instructions(sequences[j]);
      const auto result = canBeDifferent(solver, ma, mb);
      if (result == z3::unsat) {
        std::cout << "SAME!: ";
        for (const auto &ins : sequences[i].instructions) {
          if (ins.name() == instruction_name::NONE) { break; }
          const auto info = get_instruction_info(ins);
          std::cout << info.desc << " " << addr_mode_operand_name(ins.mode(), ins.number()) << "; ";
        }
        std::cout << " AND ";
        for (const auto &ins : sequences[j].instructions) {
          if (ins.name() == instruction_name::NONE) { break; }
          const auto info = get_instruction_info(ins);
          std::cout << info.desc << " " << addr_mode_operand_name(ins.mode(), ins.number()) << "; ";
        }
        std::cout << std::endl;
        goto next_sequence;
      } else if (result == z3::unknown) {
        std::cout << "UNKNOWN!" << std::endl;
      }
    }

    next_sequence:;
  }
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: " << std::endl;
    return 1;
  }

  std::cout << "SIZE: " << sizeof(random_machine) << std::endl;

  setup_instructions();

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
      total_instructions++;
      instruction_seq seq;
      seq = seq.add(instruction);
      auto hash_result = hash(seq);
      outfiles[seq.cycles].write(hash_result, seq);
    }

    std::cout << "Seeded " << total_instructions << " total instructions" << std::endl;
  } else if (arg1 == "view") {
    std::string arg2(argv[2]);
    std::ifstream view_file(arg2, std::ifstream::binary | std::ifstream::in);

    std::cout << "Opening " << arg2 << std::endl;
    while (!view_file.eof()) {
      char buffer[hash_output_file::total_size * 256];
      view_file.read(buffer, hash_output_file::total_size * 256);
      std::streamsize dataSize = view_file.gcount();
      for (int j = 0; j < dataSize; j += hash_output_file::total_size) {
        display_hash_result((uint8_t*)(buffer + j));
      }
    }
  } else if (arg1 == "find-opts") {
    std::string arg2(argv[2]);
    std::cout << "Finding optimizations with length " << arg2 << std::endl;
    input_file_manager in_manager(std::stoi(arg2));
    
    file_stream<hash_sequence, hash_output_file::total_size> view_file_stream("out/result-" + arg2 + ".dat");

    std::cout << "Opening " << arg2 << std::endl;
    std::vector<instruction_seq> sequences;
    z3::context ctx;
    uint64_t last_hash = 0;

    while (view_file_stream.has_next()) {
      hash_sequence hs = view_file_stream.next();

      if (hs.hash.alwaysIncluded != last_hash) {
        // We just finished a set of equivalent instructions.
        std::vector<instruction_seq> smaller_sequences;
        for (auto &file : in_manager.infiles) {
          const auto from_file = file.get_hashes(last_hash);
          smaller_sequences.insert(smaller_sequences.end(), from_file.begin(), from_file.end());
        }
        process_sequences(ctx, last_hash, smaller_sequences, sequences);
        sequences.clear();
      }

      sequences.push_back(hs.seq);
      last_hash = hs.hash.alwaysIncluded;
    }

    std::cout << "Last sequences" << std::endl;
    std::vector<instruction_seq> smaller_sequences;
    for (auto &file : in_manager.infiles) {
      std::vector<instruction_seq> from_file = file.get_hashes(last_hash);
      smaller_sequences.insert(smaller_sequences.end(), from_file.begin(), from_file.end());
    }
    process_sequences(ctx, last_hash, smaller_sequences, sequences);
  } else if (arg1 == "sort") {
    std::string file_name = std::string("out/result-") + argv[2] + ".dat"; 
    // sort the file by hash
    std::cout << "Sorting file:" << std::endl;
    radix_sort<hash_sequence>(file_name.c_str(), hash_output_file::total_size, hash_output_file::hash_size_used * 8 + 8);
  } else if (arg1 == "enum") {
    int target = std::stoi(argv[2]);
    output_file_manager output_files(target + 1);
    std::string file_name = std::string("out/result-") + argv[2] + ".dat";
    std::cout << "Processing sequences with length " << target << std::endl;
 
    // sort the file by hash
    std::cout << "Sorting file:" << std::endl;
    radix_sort<hash_sequence>(file_name.c_str(), hash_output_file::total_size, hash_output_file::hash_size_used * 8 + 8);

    ProfilerStart("gperf-profile.log");

    std::ifstream view_file(file_name, std::ifstream::binary | std::ifstream::in);
    // For each instruction type
    for (const auto &ins_info : instructions) {
      std::cout << "INSTRUCTION: " << (int)ins_info.ins.name() << " " << (int)ins_info.ins.number() << std::endl;
      view_file.clear();
      view_file.seekg(0, std::ifstream::beg);
      // For each section of the file
      while (!view_file.eof()) {
        char buffer[hash_output_file::total_size * 256];
        view_file.read(buffer, hash_output_file::total_size * 256);
        std::streamsize data_size = view_file.gcount();
        // For each instruction sequence hash in the file
        for (int buffer_start = 0; buffer_start < data_size; buffer_start += hash_output_file::total_size) {
          const uint8_t *buffer_ptr = (uint8_t*)(buffer + buffer_start);
          instruction_seq seq = hash_output_file::seq_from_buffer(buffer_ptr).add(ins_info);
          auto hash_result = hash(seq);
          output_files.get_file(seq.cycles).write(hash_result, seq);
        }
      }
    }

    ProfilerStop();
  }
}
