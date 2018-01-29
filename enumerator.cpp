
#include "stdint.h"
#include "z3++.h"
#include <vector>
#include <map>
#include <thread>
#include <algorithm>

#include "operations.h"
#include "opcode.h"
#include "emulator.h"
#include "concrete_machine.h"
#include "abstract_machine.h"
#include "queue.h"

typedef std::tuple<opcode, opcode> instruction2;
typedef std::tuple<opcode, opcode, opcode> instruction3;
typedef std::tuple<opcode, opcode, opcode, opcode> instruction4;

void addEquivalence(abstract_machine & m1, concrete_machine & m2, z3::solver & s) {
  //std::cout << "substitute example\n";
  z3::context & c = s.ctx();
  z3::expr x = m1._a;
  //std::cout << x << std::endl;

  z3::expr a(c), initialA(c);
  a   = c.bv_const("a", 8);
  initialA = c.num_val(m2._a, a.get_sort());
  Z3_ast from[] = { a };
  Z3_ast to[]   = { initialA };
  z3::expr new_f(c);
  new_f = to_expr(c, Z3_substitute(c, x, 1, from, to));
  new_f = new_f.simplify();
  //std::cout << new_f << std::endl;
}

float cycles(instruction3 ops) {
  return operation_costs[std::get<0>(ops)]
    + operation_costs[std::get<1>(ops)]
    + operation_costs[std::get<2>(ops)];
}

uint8_t length(instruction3 ops) {
  constexpr opcode zero { (Operations)0, (AddrMode)0 };
  return std::get<2>(ops) == zero ? (std::get<1>(ops) == zero ? 1 : 2) : 3;
}

bool equivalent(z3::context &c, instruction3 a, instruction3 b) {
  constexpr opcode zero_op { (Operations)0, (AddrMode)0 };
  constexpr emulator<abstract_machine> emu;
  abstract_machine ma(c);
  abstract_machine mb(c);
  auto inst = std::get<0>(a);
  emu.instruction(ma, inst.op, inst.mode);
  inst = std::get<1>(a);
  if (inst != zero_op) {
    emu.instruction(ma, inst.op, inst.mode);
  }
  inst = std::get<2>(a);
  if (inst != zero_op) {
    emu.instruction(ma, inst.op, inst.mode);
  }
  
  inst = std::get<0>(b);
  emu.instruction(mb, inst.op, inst.mode);
  inst = std::get<1>(b);
  if (inst != zero_op) {
    emu.instruction(mb, inst.op, inst.mode);
  }
  inst = std::get<2>(b);
  if (inst != zero_op) {
    emu.instruction(mb, inst.op, inst.mode);
  }

  z3::solver s(c);
  s.add(!(
    ma._memory == mb._memory &&
    ma._a == mb._a &&
    ma._x == mb._x &&
    ma._y == mb._y &&
    ma._sp == mb._sp &&
    ma._ccS == mb._ccS &&
    ma._ccV == mb._ccV &&
    ma._ccD == mb._ccD &&
    ma._ccI == mb._ccI &&
    ma._ccC == mb._ccC &&
    ma._ccZ == mb._ccZ &&
    ma._earlyExit == mb._earlyExit
  ));
  
  if (s.check() == z3::unsat) {
    std::cout << "Same!" << std::endl;
    return true;
  }
  //std::cout << "Different" << std::endl;
  return false;
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
    uint8_t val = opsArr[i].op & 0x0F;
    switch (opsArr[i].op & 0xF0) {
    case 0: // Immediate
      if (val == 0 || 
          val == 4 || 
          val == 6) {
        c0 = true;
      } else if (!c0 && (val == 1 || val == 5)) {
        return false;
      }
      break;
    case 1: // Absolute
    case 2: // AbsoluteX
    case 3: // AbsoluteY
    case 7: // Indirect
      if (val > abs) { return false; }
      else if (val == abs) { abs++; }
      break;
    case 4: // ZeroPage
    case 5: // ZeroPageX
    case 6: // ZeroPageY
    case 8: // IndirectX
    case 9: // IndirectY
      if (val > zp) { return false; }
      else if (val == zp) { zp++; }
      break;
    }
  }
  return true;
}

int enumerate_worker(std::multimap<uint32_t, instruction3> &buckets, uint32_t i_min, uint32_t i_max) {
  
  const emulator<concrete_machine> emu;
  constexpr uint16_t nMachines = 2;
  opcode zero = opcode { (Operations)0, (AddrMode)0 };

  for (uint32_t i = i_min; i < i_max; i++) {
    uint32_t hash = 0;
    for (uint16_t m = 0; m < nMachines; m++) {
      concrete_machine machine(0xFFA4BCAD + m);
      emu.instruction(machine, opcodes[i].op, opcodes[i].mode);
      hash ^= machine.hash();
    }
    buckets.insert(std::make_pair(hash, std::make_tuple(opcodes[i], zero, zero)));
  }
  for (uint32_t i = i_min; i < i_max; i++) {
    for (uint32_t j = 0; j < sizeof(opcodes)/sizeof(opcodes[0]); j++) {
      uint32_t hash = 0;
      for (uint16_t m = 0; m < nMachines; m++) {
        concrete_machine machine(0xFFA4BCAD + m);
        emu.instruction(machine, opcodes[i].op, opcodes[i].mode);
        emu.instruction(machine, opcodes[j].op, opcodes[j].mode);
        hash ^= machine.hash();
      }
      buckets.insert(std::make_pair(hash, std::make_tuple(opcodes[i], opcodes[j], zero)));
    }
  }

  for (uint32_t i = i_min; i < i_max; i++) {
    std::cout << "i: " << i << std::endl;
    for (uint32_t j = 0; j < sizeof(opcodes)/sizeof(opcodes[0]); j++) {
      for (uint32_t k = 0; k < sizeof(opcodes)/sizeof(opcodes[0]); k++) {
        uint32_t hash = 0;
        for (uint16_t m = 0; m < nMachines; m++) {
          concrete_machine machine(0xFFA4BCAD + m);
          emu.instruction(machine, opcodes[i].op, opcodes[i].mode);
          emu.instruction(machine, opcodes[j].op, opcodes[j].mode);
          emu.instruction(machine, opcodes[k].op, opcodes[k].mode);
          hash ^= machine.hash();
        }
        buckets.insert(std::make_pair(hash, std::make_tuple(opcodes[i], opcodes[j], opcodes[k])));
      }
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

void run(const emulator<concrete_machine> &emu, concrete_machine &machine, const instruction3 seq) {
  const opcode zero = opcode { (Operations)0, (AddrMode)0 };

  if (std::get<0>(seq) != zero) { emu.instruction(machine, std::get<0>(seq)); }
  if (std::get<1>(seq) != zero) { emu.instruction(machine, std::get<1>(seq)); }
  if (std::get<2>(seq) != zero) { emu.instruction(machine, std::get<2>(seq)); }
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
    emulator<concrete_machine> emu;

    constexpr int nMachines = 16;

    for (const auto& seq : sequences) {
      uint32_t hash = 0;
      for (int m = 0; m < nMachines; m++) {
        concrete_machine machine(0x56346d56 + m);
        run(emu, machine, seq);
        hash ^= machine.hash();
      }
      concrete_machine machine(0);
      run(emu, machine, seq);
      hash ^= machine.hash();
      concrete_machine machine2(0xFFFFFFFF);
      run(emu, machine2, seq);
      hash ^= machine2.hash();
      buckets.insert(std::make_pair(hash, seq));
    }
    // std::cout << "size " << buckets.size() << std::endl;
    return process_hashes_worker(buckets, 0, 0x100000000, false);
  }

  std::sort(sequences.begin(), sequences.end(), compare_by_cycles);

  int nComparisons = 0;
  // iterate backwards
  for (auto it = sequences.rbegin(); it != sequences.rend(); ++it) {
    auto seq = *it;
    if (isCanonical(seq)) {
      for (auto it2 = sequences.begin(); it2 != sequences.end(); ++it2) {
        if (cycles(*it2) >= cycles(seq)) { break; }
        nComparisons++;
        // These two instruction sequences might be equivalent.
        //print(seq);
        //std::cout << " ?= ";
        //print(*it2);
        //std::cout << std::endl;
      }
    }
  }
  return nComparisons;
}

int process_hashes_worker(const std::multimap<uint32_t, instruction3> &combined_buckets, uint64_t hash_min, uint64_t hash_max, bool try_split) {
  auto it = combined_buckets.lower_bound(hash_min);
  auto end = hash_max == 0x100000000 ? combined_buckets.end() : combined_buckets.lower_bound(hash_max);

  if (try_split)
    std::cout << "  Processing hashes from " << std::hex << hash_min << " " << hash_max << std::endl;
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
  queue.add(std::bind(process_hashes_worker, std::ref(combined_buckets), (N_THREADS-1)*step, hash_max, true));

  queue.run();
}

/*


  z3::context c;

  uint32_t last = 0;
  std::vector<uint64_t> sequences;
  for (const auto &p : buckets) {
    if (p.first != last) {
      if (sequences.size() > 1) {
        int l = length(sequences[0]);
        bool differentLengths = false;
        for (auto sequence : sequences) {
          if (length(sequence) != l) {
            differentLengths = true;
            break;
          }
        }
        bool hasCanonical = false;
        for (auto sequence : sequences) {
          auto tup = int2tuple(sequence);
          if (isCanonical(tup)) {
            hasCanonical = true;
            break;
          }
        }
        if (differentLengths && hasCanonical) {
          for (auto sequence : sequences) {
            auto tup = int2tuple(sequence);
            for (auto sequence2 : sequences) {
              if (sequence != sequence2) {
                if (equivalent(c, tup, int2tuple(sequence2))) {
                  print(tup); std::cout << std::endl;
                  print(int2tuple(sequence2)); std::cout << std::endl;
                }
              }
            }
          }
          std::cout << "---------------" << std::endl;
          std::cout << sequences.size() << std::endl; 
        }
      }
      sequences.clear();
    }
    sequences.push_back(p.second);
    last = p.first;
  }





void loadEmulator() {
  z3::context c;

  {
    z3::solver s(c);
    // ensure abstract and concrete machines are the same.
    for (int seed = 0; seed < 200; seed += 13) {
      std::cout << "seed: " << seed << std::endl;
      
      concrete_machine initialMachine(seed);
      emulator<concrete_machine> c_emu;
      emulator<abstract_machine> a_emu;

      z3::expr a = c.num_val(initialMachine._a, c.bv_sort(8));
      z3::expr x = c.num_val(initialMachine._x, c.bv_sort(8));
      z3::expr y = c.num_val(initialMachine._y, c.bv_sort(8));
      z3::expr sp = c.num_val(initialMachine._sp, c.bv_sort(8));
      z3::expr ccS = c.bool_val(initialMachine._ccS);
      z3::expr ccV = c.bool_val(initialMachine._ccV);
      z3::expr ccC = c.bool_val(initialMachine._ccC);
      z3::expr ccD = c.bool_val(initialMachine._ccD);
      z3::expr ccI = c.bool_val(initialMachine._ccI);
      z3::expr ccZ = c.bool_val(initialMachine._ccZ);
      z3::expr absolute0 = c.num_val(initialMachine.absolute0, c.bv_sort(16));
      z3::expr absolute1 = c.num_val(initialMachine.absolute1, c.bv_sort(16));
      z3::expr absolute2 = c.num_val(initialMachine.absolute2, c.bv_sort(16));
      z3::expr zp0 = c.num_val(initialMachine.zp0, c.bv_sort(8));
      z3::expr zp1 = c.num_val(initialMachine.zp1, c.bv_sort(8));
      z3::expr zp2 = c.num_val(initialMachine.zp2, c.bv_sort(8));
      z3::expr zp3 = c.num_val(initialMachine.zp3, c.bv_sort(8));
      z3::expr c0 = c.num_val(initialMachine.c0, c.bv_sort(8));
      z3::expr c1 = c.num_val(initialMachine.c1, c.bv_sort(8));
      z3::expr memory = mkArray(c);
      for (uint32_t i = 0; i < 0x10000; i++) {
        memory = z3::store(memory, i, initialMachine.read(i));
      }
      for (uint32_t i = 0; i < sizeof(opcodes)/sizeof(opcodes[0]); i++) {
        abstract_machine m1(c);
        concrete_machine m2(seed);
        m1._a = a;
        m1._x = x;
        m1._y = y;
        m1._sp = sp;
        m1._ccS = ccS;
        m1._ccV = ccV;
        m1._ccC = ccC;
        m1._ccD = ccD;
        m1._ccI = ccI;
        m1._ccZ = ccZ;
        m1.absolute0 = absolute0;
        m1.absolute1 = absolute1;
        m1.absolute2 = absolute2;
        m1.zp0 = zp0;
        m1.zp1 = zp1;
        m1.zp2 = zp2;
        m1.zp3 = zp3;
        m1.c0 = c0;
        m1.c1 = c1;
        m1._memory = memory;
        
        a_emu.instruction(m1, opcodes[i].op, opcodes[i].mode);
        c_emu.instruction(m2, opcodes[i].op, opcodes[i].mode);
        addEquivalence(m1, initialMachine, s);
        if (s.check() != z3::sat) {
          std::cout << "different: " << OpNames[opcodes[i].op] << " " << opcodes[i].mode << std::endl;
        }
      }
    }
  }

  for (uint32_t i = 0; i < sizeof(opcodes)/sizeof(opcodes[0]); i++) {
    abstract_machine m1(c);

    e1.instruction(opcodes[i].op, opcodes[i].mode);
    z3::solver s(c);


    concrete_machine cm1(0);
    a_emu.instruction(m1, opcodes[i].op, opcodes[i].mode);
    for (int j = 0; j < i; j++) {
 
      concrete_machine cm2(0);
      emulator<concrete_machine> ce2(cm2);
      ce2.instruction(opcodes[j].op, opcodes[j].mode);

      if (cm1._a != cm2._a || cm1._x != cm2._x || cm1._y != cm2._y) {
        continue;
      }
   
      abstract_machine m2(c);
      emulator<abstract_machine> e2(m2);

      e2.instruction(opcodes[j].op, opcodes[j].mode);

      s.push();
      s.add(!(
        m1._memory == m2._memory &&
        m1._a == m2._a &&
        m1._x == m2._x &&
        m1._y == m2._y &&
        m1._sp == m2._sp &&
        m1._ccS == m2._ccS &&
        m1._ccV == m2._ccV &&
        m1._ccD == m2._ccD &&
        m1._ccI == m2._ccI &&
        m1._ccC == m2._ccC &&
        m1._ccZ == m2._ccZ &&
        m1._earlyExit == m2._earlyExit
      ));

      if (s.check() == z3::unsat) {
        std::cout << "Same: " << OpNames[(int)opcodes[i].op] << "," << AddrModeNames[opcodes[i].mode] << std::endl;;
        std::cout << "and:  " << OpNames[(int)opcodes[j].op] << "," << (int)opcodes[j].mode << std::endl;;
      }
      s.pop();
    }
  }
}*/

int main() {
  try {
    std::multimap<uint32_t, instruction3> combined_buckets;
    enumerate_concurrent(combined_buckets);
    process_hashes_concurrent(combined_buckets);
  } catch (z3::exception & ex) {
    std::cout << "unexpected error: " << ex << "\n";
  }
}
