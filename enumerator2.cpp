#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include "stdint.h"
#include "instructions2.h"
#include "emulator2.h"
#include "random_machine.h"
#include "z3++.h"
#include "abstract_machine.h"

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
  uint32_t alwaysIncluded;
  uint8_t a;
  uint8_t x;
  uint8_t y;
  uint8_t ccS;
  uint8_t ccV;
  uint8_t ccI;
  uint8_t ccD;
  uint8_t ccC;
  uint8_t ccZ;
} execution_hash;

execution_hash hash(instruction_seq seq) {
  random_machine rms[8] {
    random_machine(0),
    random_machine(0xffffffff),
    random_machine(0x12345678),
    random_machine(0xCAFEFACE),
    random_machine(0x98765432),
    random_machine(0x30986987),
    random_machine(0x28922535),
    random_machine(0x19012253)
  };
  for (auto instruction : seq.instructions) {
    for (auto &rm : rms) {
      rm.instruction(instruction);
    }
  }

  execution_hash result { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  for (auto rm : rms) {
    result.alwaysIncluded ^= rm.hash();
    result.a ^= rm._a;
    result.x ^= rm._x;
    result.y ^= rm._y;
    result.ccS <<= 1;
    result.ccS |= rm._ccS != 0;
    result.ccV <<= 1;
    result.ccV |= rm._ccV != 0;
    result.ccI <<= 1;
    result.ccI |= rm._ccI != 0;
    result.ccD <<= 1;
    result.ccD |= rm._ccD != 0;
    result.ccC <<= 1;
    result.ccC |= rm._ccC != 0;
    result.ccZ <<= 1;
    result.ccZ |= rm._ccZ != 0;
  }
  
  return result;
}

int main() {
  // Create all of the output files.
  std::vector<std::ofstream> outfiles;
  for (int i = 1; i <= max_cost; i++) {
    outfiles.push_back(std::ofstream("out/result-" + std::to_string(i) + ".dat", std::ofstream::binary | std::ofstream::out | std::ofstream::trunc));
  }

  instruction ins1(instruction_name::LDA, addr_mode::CONSTANT, 0);
  instruction ins2(instruction_name::AND, addr_mode::CONSTANT, 0);

  instruction_seq seq(0, 0);
  auto hash_result = hash(seq.add(instruction_info { ins1, 0, 0 }));
  std::cout << "hash:" << hash_result.alwaysIncluded << " a:" << (int)hash_result.a << " x:" << (int)hash_result.x << " y:" << (int)hash_result.y << std::endl;

  hash_result = hash(seq.add(instruction_info { ins2, 0, 0 }));
  std::cout << "hash:" << hash_result.alwaysIncluded << " a:" << (int)hash_result.a << " x:" << (int)hash_result.x << " y:" << (int)hash_result.y << std::endl;
  // std::cout << hash(seq) << std::endl;
  // std::cout << hash(seq.add(instruction_info { ins1, 0, 0 })) << std::endl;
  // std::cout << hash(seq.add(instruction_info { ins1, 0, 0 }).add(instruction_info { ins2, 0, 0 })) << std::endl;
}

// write: hash: 4 bytes.
// a x y 6 flags - 9 bytes
// instructions: 2 bytes per instruction
