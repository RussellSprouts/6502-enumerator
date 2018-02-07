
#include "stdint.h"
#include "z3++.h"
#include <vector>
#include <map>
#include <thread>
#include <algorithm>

#include "operations.h"
#include "opcode.h"
#include "emulator.h"
#include "random_machine.h"
#include "abstract_machine.h"
#include "queue.h"

const float cycles(const instruction3 ops) {
  return operation_costs[std::get<0>(ops)]
    + operation_costs[std::get<1>(ops)]
    + operation_costs[std::get<2>(ops)];
}

uint8_t length(instruction3 ops) {
  constexpr opcode zero { (Operations)0, (AddrMode)0 };
  return std::get<2>(ops) == zero ? (std::get<1>(ops) == zero ? 1 : 2) : 3;
}

void print(opcode op) {
  std::cout << OpNames[op.op] << " " << AddrModeNames[op.mode];
}

void print(instruction3 ops) {
  opcode zero { (Operations)0, (AddrMode)0 };
  if (std::get<0>(ops) != zero) { print(std::get<0>(ops)); }
  if (std::get<1>(ops) != zero) {
    std::cout << "; ";
    print(std::get<1>(ops));
  }
  if (std::get<2>(ops) != zero) {
    std::cout << "; ";
    print(std::get<2>(ops));
  }
}


bool equivalent(z3::solver &s, const abstract_machine &ma, const abstract_machine &mb) {
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
  
  switch (s.check()) {
  case z3::sat:
    s.pop();
    return false;
  case z3::unknown:
    // TODO: add to list of timed-out comparisons.
    s.pop();
    return false;
  default:
    s.pop();
    return true;
  }
}

bool isCanonical(instruction3 ops) {
  opcode zero { (Operations)0, (AddrMode)0 };
  opcode one = std::get<0>(ops);
  opcode two = std::get<1>(ops);
  opcode three = std::get<2>(ops);
  opcode opsArr[3] { one, two, three };
  int abs = 7;
  int zp = 0xA;
  bool c0 = false;
  for (int i = 0; i < 3; i++) {
    if (opsArr[i] == zero) { break; }
    uint8_t val = opsArr[i].mode & 0x0F;
    switch (opsArr[i].mode & 0xF0) {
    case 0x00: // Immediate
      if (val == 0 || 
          val == 4 || 
          val == 6) {
        c0 = true;
      } else if (!c0 && (val == 1 || val == 5)) {
        return false;
      }
      break;
    case 0x10: // Absolute
    case 0x20: // AbsoluteX
    case 0x30: // AbsoluteY
    case 0x70: // Indirect
      if (val > abs) { return false; }
      else if (val == abs) { abs++; }
      break;
    case 0x40: // ZeroPage
    case 0x50: // ZeroPageX
    case 0x60: // ZeroPageY
    case 0x80: // IndirectX
    case 0x90: // IndirectY
      if (val > zp) { return false; }
      else if (val == zp) { zp++; }
      break;
    }
  }
  return true;
}

int enumerate_worker(std::multimap<uint32_t, instruction3> &buckets, uint32_t i_min, uint32_t i_max) {
  
  constexpr uint16_t nMachines = 2;
  opcode zero = opcode { (Operations)0, (AddrMode)0 };

  for (uint32_t i = i_min; i < i_max; i++) {
    uint32_t hash = 0;
    for (uint16_t m = 0; m < nMachines; m++) {
      random_machine machine(0xFFA4BCAD + m);
      machine.instruction(opcodes[i]);
      hash ^= machine.hash();
    }
    buckets.insert(std::make_pair(hash, std::make_tuple(opcodes[i], zero, zero)));
  }
  for (uint32_t i = i_min; i < i_max; i++) {
    for (uint32_t j = 0; j < sizeof(opcodes)/sizeof(opcodes[0]); j++) {
      uint32_t hash = 0;
      for (uint16_t m = 0; m < nMachines; m++) {
        random_machine machine(0xFFA4BCAD + m);
        machine.instruction(opcodes[i]);
        machine.instruction(opcodes[j]);
        hash ^= machine.hash();
      }
      buckets.insert(std::make_pair(hash, std::make_tuple(opcodes[i], opcodes[j], zero)));
    }
  }
  return 1;
}

void enumerate_concurrent(std::multimap<uint32_t, instruction3> &combined_buckets) {

  std::cout << "Starting " << N_THREADS << " threads." << std::endl;

  work_queue queue;
  constexpr int N_TASKS = (sizeof opcodes / sizeof opcodes[0]);

  std::multimap<uint32_t, instruction3> buckets[N_TASKS];
 
  int n_max = (sizeof opcodes) / (sizeof opcodes[0]);
  //int n_max = 32;
  int step = n_max / N_TASKS;

  for (int i = 0; i < N_TASKS-1; i++) {
    queue.add(std::bind(enumerate_worker, std::ref(buckets[i]), i*step, (i+1)*step));
  }
  queue.add(std::bind(enumerate_worker, std::ref(buckets[N_TASKS-1]), (N_TASKS-1)*step, n_max));

  queue.run();

  std::cout << "Finished hashing. Merging results" << std::endl; 

  for (int i = 0; i < N_TASKS; i++) {
    std::cout << "  Adding results from " << i << std::endl;
    combined_buckets.insert(buckets[i].begin(), buckets[i].end());
    buckets[i].clear();
  }
}

bool compare_by_cycles(const instruction3 &a, const instruction3 &b) {
  return cycles(a) < cycles(b);
}
bool compare_by_length(const instruction3 &a, const instruction3 &b) {
  return length(a) < length(b);
}

int process_hashes_worker(const std::multimap<uint32_t, instruction3> &combined_buckets, uint64_t hash_min, uint64_t hash_max, bool try_split);

int process_sequences(std::vector<instruction3> &sequences, bool try_split) {

  if (sequences.size() <= 1) {
    return 0; // this instruction sequence must be unique.
  }
  int l = cycles(sequences[0]);
  bool differentLengths = false;
  for (auto sequence : sequences) {
    if (cycles(sequence) != l) {
      differentLengths = true;
      break;
    }
  }
  if (!differentLengths) {
    return 0; // all of these instructions have the same cost -- no optimizations are possible.
  }

  if (try_split) {
    std::multimap<uint32_t, instruction3> buckets;

    constexpr int nMachines = 128;

    for (const auto& seq : sequences) {
      uint32_t hash = 0;
      for (int m = 0; m < nMachines; m++) {
        random_machine machine(0x56346d56 + m*1001);
        machine.instruction(seq);
        hash ^= machine.hash();
      }
      random_machine machine(0);
      machine.instruction(seq);
      hash ^= machine.hash();
      random_machine machine2(0xFFFFFFFF);
      machine2.instruction(seq);
      hash ^= machine2.hash();
      buckets.insert(std::make_pair(hash, seq));
    }
    return process_hashes_worker(buckets, 0, 0x100000000, false);
  }

  std::sort(sequences.begin(), sequences.end(), compare_by_cycles);

  std::map<float, size_t> seq_cycles;
  float cost = -1;
  for (size_t i = 0; i < sequences.size(); i++) {
    auto c = cycles(sequences[i]);
    if (cost != c) {
      seq_cycles[c] = i;
      cost = c;
    }
  }

  int nComparisons = 0;

  z3::context ctx;
  z3::solver s(ctx);

  std::vector<abstract_machine> machines{};
  for (const auto &seq : sequences) {
    abstract_machine m(ctx);
    m.instruction(seq);
    m.simplify();
    machines.push_back(m);
  }

  // Check instructions starting from the end
  for (ssize_t i = sequences.size() - 1; i >= 0; i--) {
    const instruction3 seq = sequences.at(i);
    const auto c = cycles(seq);
    if (!isCanonical(seq)) { continue; }
    // Compare with instructions starting at the beginning
    for (size_t j = 0; j < seq_cycles[c]; j++) {
      nComparisons++;
      if (equivalent(s, machines[i], machines[j])) {
        print(seq);
        std::cout << " <-> ";
        print(sequences[j]);
        std::cout << std::endl;
        break;
      }
    }
  }
  return nComparisons;
}

int process_hashes_worker(const std::multimap<uint32_t, instruction3> &combined_buckets, uint64_t hash_min, uint64_t hash_max, bool try_split) {
  auto it = combined_buckets.lower_bound(hash_min);
  auto end = hash_max == 0x100000000 ? combined_buckets.end() : combined_buckets.lower_bound(hash_max);

  if (try_split) {
    std::cout << "  Processing hashes from " << std::hex << hash_min << " " << hash_max << std::endl;
  }
  int nComparisons = 0;

  int64_t last = -1;
  std::vector<instruction3> sequences;
  for (; it != end && it != combined_buckets.end(); ++it) {
    uint32_t hash = it->first;
    instruction3 ops = it->second;
    if (hash != last) {
      // sequences now contains a list of instruction sequences which are possibly equivalent
      nComparisons += process_sequences(sequences, try_split);
      sequences.clear();
    }
    sequences.push_back(ops);
    last = hash;
  }
  nComparisons += process_sequences(sequences, try_split);
  if (try_split) { std::cout << "Comparisons 0x" << nComparisons << std::endl; }
  return nComparisons;
}

void process_hashes_concurrent(const std::multimap<uint32_t, instruction3> &combined_buckets) {
  std::cout << "Starting processing of hashes" << std::endl;

  constexpr int N_TASKS = 1024;
  work_queue queue;

  uint64_t hash_max = 0x100000000;
  uint64_t step = hash_max / N_TASKS;
  for (int i = 0; i < N_TASKS-1; i++) {
    queue.add(std::bind(process_hashes_worker, std::ref(combined_buckets), i*step, (i+1)*step, true));
  }
  queue.add(std::bind(process_hashes_worker, std::ref(combined_buckets), (N_TASKS-1)*step, hash_max, true));

  queue.run();
}

int main() {
  try {
    for (int m = 0; m < 128; m++) {
      random_machine machine(0x56346d56 + m*1001);
      std::cout << (machine._ccS ? '.' : ' ')
        << (machine._ccV ? '.' : ' ')
        << (machine._ccC ? '.' : ' ')
        << (machine._ccI ? '.' : ' ')
        << (machine._ccD ? '.' : ' ')
        << (machine._ccZ ? '.' : ' ')
        << std::endl;
    }
    
    std::multimap<uint32_t, instruction3> combined_buckets;
    enumerate_concurrent(combined_buckets);
    process_hashes_concurrent(combined_buckets);
    
  } catch (z3::exception & ex) {
    std::cout << "unexpected error: " << ex << "\n";
  }
}
