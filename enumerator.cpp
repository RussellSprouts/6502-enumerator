
#include "stdint.h"
#include "include/z3++.h"
#include <vector>
#include <map>
#include <unordered_set>
#include <thread>
#include <algorithm>

#include "operations.h"
#include "opcode.h"
#include "emulator.h"
#include "random_machine.h"
#include "abstract_machine.h"
#include "queue.h"

uint8_t length(instruction_seq ops) {
  return ops.ops[2] == opcode::zero ? (ops.ops[1] == opcode::zero ? 1 : 2) : 3;
}

void print(opcode op) {
  std::cout << OpNames[op.op] << " " << AddrModeNames[op.mode];
}

void print(instruction_seq ops) {
  for (int i = 0; i < instruction_seq::max_length; i++) {
    if (ops.ops[i] == opcode::zero) { return; }
    print(ops.ops[i]);
    std::cout << "; ";
  }
}

const float cycles(const instruction_seq ops) {
  float cost = 0;
  for (int i = 0; i < instruction_seq::max_length; i++) {
    cost += operation_costs[ops.ops[i]];
  }
  return cost;
}

z3::check_result equivalent(z3::solver &s, const abstract_machine &ma, const abstract_machine &mb) {
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

/**
 * Generates a bit mask showing the operands
 * in use in the given set of instructions.
 */
uint16_t operand_mask(instruction_seq ops) {
  uint16_t result = 0;
  for (int i = 0; i < instruction_seq::max_length; i++) {
    if (ops.ops[i] == opcode::zero) break;
    uint8_t operand = ops.ops[i].mode & 0xF;
    // handle immediate values
    switch (operand) {
      case ImmediateC0: case ImmediateC0Plus1:
        result |= 0x1;
        break;
      case ImmediateC1: case ImmediateC1Plus1:
        result |= 0x2;
        break;
      case ImmediateC0PlusC1:
        result |= 0x3;
        break;
      case 0x0E: case Immediate0: case Immediate1:
        // no operand, don't do anything.
        break;
      default:
        // for everything else, just use the value as a bit number.
        result |= 1 << operand;
    }
  }
  return result;
}

/**
 * The candidate instruction sequence can't use an operand
 * that the original sequence doesn't.
 */
inline bool is_possible_optimization_by_operand_masks(uint16_t original, uint16_t candidate) {
  return (original & candidate) == candidate;
}

bool is_canonical(instruction_seq ops) {
  int abs = 7;
  int zp = 0xA;
  bool c0 = false;
  for (int i = 0; i < instruction_seq::max_length; i++) {
    if (ops.ops[i] == opcode::zero) { break; }
    uint8_t val = ops.ops[i].mode & 0x0F;
    switch (ops.ops[i].mode & 0xF0) {
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

const static int N_INSTRUCTIONS = (sizeof opcodes) / (sizeof opcodes[0]);

void enumerate_recursive(uint32_t i_min, uint32_t i_max, const random_machine &m1, const random_machine &m2, instruction_seq path, int depth, std::multimap<uint32_t, instruction_seq> &buckets, const std::unordered_set<instruction_seq> &non_optimal) {
  for (uint32_t i = i_min; i < i_max; i++) {
    instruction_seq new_path = path.append(opcodes[i]);
    if (new_path.in(non_optimal)) {
      //std::cout << "Skipping non-optimal ";
      //print(new_path);
      //std::cout << std::endl;
      continue;
    }
    random_machine m1_copy = m1;
    random_machine m2_copy = m2;
    m1_copy.instruction(opcodes[i]);
    m2_copy.instruction(opcodes[i]);
    uint32_t hash = m1_copy.hash() ^ m2_copy.hash();
    buckets.insert(std::make_pair(hash, new_path));
    //std::cout << "done." << std::endl;
    if (depth > 1) {
      enumerate_recursive(0, N_INSTRUCTIONS, m1_copy, m2_copy, new_path, depth - 1, buckets, non_optimal);
    }
  }
}

void enumerate_worker(uint32_t i_min, uint32_t i_max, int depth, std::multimap<uint32_t, instruction_seq> &buckets, const std::unordered_set<instruction_seq> &non_optimal) {
  
  std::cout << i_min << std::endl;

  random_machine m1(0xFFA4BCAD);
  random_machine m2(0x4572849E);
  instruction_seq path;
  enumerate_recursive(i_min, i_max, m1, m2, path, depth, buckets, non_optimal);
}

void enumerate_concurrent(int depth, std::multimap<uint32_t, instruction_seq> &combined_buckets, const std::unordered_set<instruction_seq> &non_optimal) {
  std::cout << "Starting " << N_THREADS << " threads." << std::endl;

  work_queue<std::multimap<uint32_t, instruction_seq>> queue;
  constexpr int N_TASKS = (sizeof opcodes / sizeof opcodes[0]);
 
  // divide into separate work items for each opcode.
  int n_max = (sizeof opcodes) / (sizeof opcodes[0]);
  int step = n_max / N_TASKS;

  for (int i = 0; i < N_TASKS-1; i++) {
    queue.add([=](auto &buckets) {
      enumerate_worker(i * step, (i + 1) * step, depth, buckets, non_optimal);
    });
  }
  queue.add([=](auto &buckets) {
    enumerate_worker((N_TASKS - 1) * step, n_max, depth, buckets, non_optimal);
  });

  queue.run();
  
  std::cout << "Finished hashing. Merging results" << std::endl; 
  for (auto &buckets : queue.stores) {
    std::cout << "Processing some buckets (" << buckets.size() << ")" << std::endl;
    combined_buckets.insert(buckets.begin(), buckets.end());
    buckets.clear();
  }
  
}

bool compare_by_cycles(const instruction_seq &a, const instruction_seq &b) {
  return cycles(a) < cycles(b);
}
bool compare_by_length(const instruction_seq &a, const instruction_seq &b) {
  return length(a) < length(b);
}

struct process_hashes_thread_context {
  std::vector<std::pair<instruction_seq, instruction_seq>> optimizations;
  std::vector<std::pair<instruction_seq, instruction_seq>> timed_out;
  z3::context context;
  z3::solver solver;

  process_hashes_thread_context()
    : context(), solver(context) {
    z3::params p(context);
    p.set(":timeout", 1000u);
    solver.set(p);
  }
};

int process_hashes_worker(const std::multimap<uint32_t, instruction_seq> &combined_buckets, process_hashes_thread_context &ctx, uint64_t hash_min, uint64_t hash_max, bool try_split);

int process_sequences(std::vector<instruction_seq> &sequences, process_hashes_thread_context &thread_ctx, bool try_split) {

  if (sequences.size() <= 1) {
    return 0; // this instruction sequence must be unique.
  }
  int l = cycles(sequences[0]);
  bool different_costs = false;
  for (auto sequence : sequences) {
    if (cycles(sequence) != l) {
      different_costs = true;
      break;
    }
  }
  if (!different_costs) {
    return 0; // all of these instructions have the same cost -- no optimizations are possible.
  }

  z3::solver solver(thread_ctx.context);
  z3::params p(thread_ctx.context);
  p.set(":timeout", 1000u);
  solver.set(p);

  if (try_split) {
    std::multimap<uint32_t, instruction_seq> buckets;

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
    return process_hashes_worker(buckets, thread_ctx, 0, 0x100000000, false);
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

  std::vector<abstract_machine> machines{};
  std::vector<uint16_t> operand_masks;
  for (const auto &seq : sequences) {
    abstract_machine m(thread_ctx.context);
    m.instruction(seq);
    m.simplify();
    machines.push_back(m);

    operand_masks.push_back(operand_mask(seq));
  }

  // Check instructions starting from the end
  for (ssize_t i = sequences.size() - 1; i >= 0; i--) {
    const instruction_seq seq = sequences.at(i);
    const auto c = cycles(seq);
    if (!is_canonical(seq)) { continue; }
    for (size_t j = 0; j < seq_cycles[c]; j++) {
      // if the candidate uses an unknown that wasn't introduced in the original,
      // it can't be an optimization.
      if (!is_possible_optimization_by_operand_masks(operand_masks[i], operand_masks[j])) {
        // std::cout << "Skipping (mask) ";
        // print(seq);
        // std::cout << "[" << operand_masks[i] << "] <?> ";
        // print(sequences[j]);
        // std::cout << "[" << operand_masks[j] << "]" << std::endl;
        continue;
      }
      nComparisons++;
      auto equivalence = equivalent(solver, machines[i], machines[j]);
      if (equivalence == z3::unsat) {
        thread_ctx.optimizations.push_back(std::make_pair(seq, sequences[j]));
        print(seq);
        std::cout << " <-> ";
        print(sequences[j]);
        std::cout << " " << operand_masks[i];
        std::cout << std::endl;
        break;
      } else if (equivalence == z3::unknown) {
        thread_ctx.timed_out.push_back(std::make_pair(seq, sequences[j]));
        print(seq);
        std::cout << " <?> ";
        print(sequences[j]);
        std::cout << " " << operand_masks[i];
        std::cout << " (TIMED OUT)" << std::endl;
        
      }
    }
  }
  return nComparisons;
}

int process_hashes_worker(const std::multimap<uint32_t, instruction_seq> &combined_buckets, process_hashes_thread_context &thread_ctx, uint64_t hash_min, uint64_t hash_max, bool try_split) {
  auto it = combined_buckets.lower_bound(hash_min);
  auto end = hash_max == 0x100000000 ? combined_buckets.end() : combined_buckets.lower_bound(hash_max);

  if (try_split) {
    std::cout << "  Processing hashes from " << std::hex << hash_min << " " << hash_max << std::endl;
  }
  int nComparisons = 0;

  int64_t last = -1;
  std::vector<instruction_seq> sequences;
  for (; it != end && it != combined_buckets.end(); ++it) {
    uint32_t hash = it->first;
    instruction_seq ops = it->second;
    if (hash != last) {
      // sequences now contains a list of instruction sequences which are possibly equivalent
      nComparisons += process_sequences(sequences, thread_ctx, try_split);
      sequences.clear();
    }
    sequences.push_back(ops);
    last = hash;
  }
  nComparisons += process_sequences(sequences, thread_ctx, try_split);
  if (try_split) { std::cout << "Comparisons 0x" << nComparisons << std::endl; }
  return nComparisons;
}

void retry_timed_out() {

}

void process_hashes_concurrent(const std::multimap<uint32_t, instruction_seq> &combined_buckets, std::vector<std::pair<instruction_seq, instruction_seq>> &optimizations, uint64_t hash_min, uint64_t hash_max) {
  std::cout << "Starting processing of hashes" << std::endl;

  constexpr int N_TASKS = 1024;
  work_queue<process_hashes_thread_context> queue;

  uint64_t step = (hash_max - hash_min) / N_TASKS;
  for (int i = 0; i < N_TASKS-1; i++) {
    queue.add([=, &combined_buckets](auto &ctx) {
      process_hashes_worker(combined_buckets, ctx, hash_min + i * step, hash_min + (i + 1) * step, true);
    });
  }
  queue.add([=, &combined_buckets](auto& ctx) {
    process_hashes_worker(combined_buckets, ctx, hash_min + (N_TASKS - 1) * step, hash_max, true);
  });

  queue.run();

  std::cout << "Done processing hashes." << std::endl;

  std::vector<std::pair<instruction_seq, instruction_seq>> timed_out;

  for (auto &thread_context : queue.stores) {
    std::cout << "One thread found " << thread_context.optimizations.size() << std::endl;
    optimizations.insert(
      optimizations.end(),
      thread_context.optimizations.begin(),
      thread_context.optimizations.end());
    timed_out.insert(
      timed_out.end(),
      thread_context.timed_out.begin(),
      thread_context.timed_out.end());
  }

  std::cout << "TIMED OUT COMPARISONS: " << timed_out.size() << std::endl;
  for (auto &pair : timed_out) {
    std::cout << "    ";
    print(pair.first);
    std::cout << "<?> ";
    print(pair.second);
    std::cout << std::endl;
  }
}

int main() {
  try {
    std::unordered_set<instruction_seq> non_optimal;
    std::multimap<uint32_t, instruction_seq> combined_buckets;
    std::vector<std::pair<instruction_seq, instruction_seq>> optimizations;
    enumerate_concurrent(2, combined_buckets, non_optimal);
    process_hashes_concurrent(combined_buckets, optimizations, 0, 0x100000000);
    std::cout << "Done processing, now results" << std::endl;
    for (const auto &pair : optimizations) {
      const auto &original = pair.first;
      non_optimal.insert(original);
    }
    combined_buckets.clear();
    enumerate_concurrent(3, combined_buckets, non_optimal);
    std::cout << "Done with enumeration -- now processing" << std::endl;
    uint32_t lastHash = 0xFFFFFFFF;
    for (const auto &pair : combined_buckets) {
      const auto hash = pair.first;
      const auto seq = pair.second;
      if (hash != lastHash) {
        std::cout << "-" << std::endl;
        lastHash = hash;
      }
      print(seq);
      std::cout << std::endl;
    }
    process_hashes_concurrent(combined_buckets, optimizations, 0, 0x100000000);

    std::cout << "Size: " << non_optimal.size() << std::endl;

  } catch (z3::exception & ex) {
    std::cout << "unexpected error: " << ex << "\n";
  }
}
