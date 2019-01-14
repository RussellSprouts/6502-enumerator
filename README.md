
# A peephole superoptimizer for the 6502

A peephole superoptimizer uses superoptimization techniques
to automatically discover replacement patterns for instruction
sequences. See [this paper](https://theory.stanford.edu/~aiken/publications/papers/asplos06.pdf) for an overview of the idea and an application
to x86 processors.

The original paper has these (simplified) steps:

1. Harvest sequences from existing binaries to find sequences
   that are good targets for optimization.
2. Run the sequences a few times to get a fingerprint of their behavior.
3. Run all of the instruction sequences up to a certain length and
   get finderprints of their behavior.
4. Use a theorem prover to check sequences with the same fingerprint
   to see if they are actually equivalent.

In 2006, they were able to harvest millions of sequences, and enumerate
all of the sequences up to 3 opcodes. With the benefit of 12 years of
processor improvements and a simpler instruction set in 6502, I hope to
reach at least length 4. Right now I am skipping the harvest step --
I don't have a good corpus of permissively licensed binaries for the 6502.
Instead I am checking equivalence between the enumerated instructions.

## Machine model

Not all machine instructions are supported. The main limitation is that jumps and branches must jump outside of the sequence (no loops). This also means that the jsr and brk instructions are not supported, because they have an unknown effect on the machine state before returning.

The 6502 has 16-bit addresses, as well as special instructions to access the zero-page 0x0000 to 0x00FF. In an instruction sequence, there can be 3 possibly different 16-bit addresses labeled Absolute0 to Absolute2, and 4 possibly different zero-page addresses labeled Zp0 to Zp3. For instructions that take immediate arguments, there are two 8-bit immediates C0 and C1 which have arbitrary values, and operands can include 0, 1, C0, C1, C0+1, C1+1, and C0+C1. This can find some constant folding optimizations.

Currently, all of memory is considered when determining equivalence. This does not model the hardware stack well -- once a value is popped, it should be treated as possibly dead, because an interrupt might override it. We should immediately change it to an arbitrary value again. After running, we should ignore values off the top of the stack for equivalence, to find optimizations that eliminate spills to the stack. A software stack could also be added. Accessing the hardware stack would look like `lda Stack, x`, and Stack would act like the other absolute variables. Values popped from the software stack won't immediately decay to unknown, but won't be counted when checking equivalence.

## Techniques

Performing checks with the theorem prover is very expensive, so there are several techniques to eliminate them:

1. Finger-printing: Test each sequence on a small number of inital machine states, and hash the results. This quick test will group together all instruction sequences which might be equivalent. The original paper was optimizing for the same machine, so it could execute the instruction sequences on the processor. It sandboxed memory accesses to wrap within a 256 byte range. We are cross-compiling, so the code is emulated. We give it access to an entire virtual memory space of 64k, but we use a hash function to implement read for the initial state and save only the addresses written to.
2. Canonicalization: Instruction sequences with the same shape but different names for the unknowns can be considered together -- simply rename the first absolute address to Absolute0, the first zero-page address to Zp0, etc. The original paper enumerated only the canonical sequences, and then found the hash that would result from each possible variant. In this implementation, the enumeration is a small percentage of the runtime, so all sequences are generated, but when finding optimizations, only we only check canonical sequences against the other sequences with the same fingerprint.
3. Pruning during enumeration: If any subsequence of a sequence is known to be non-optimal, then skip the sequence. In order to make the most effective use of this, the enumerator will have to be changed to enumerate in cost order instead of number of instructions.
3. Only compare if there are possible gains. If all of the sequences in a group are the same, skip the group. Sort each sequence by cost, then only compare sequences with sequences that are cheaper. The orignal paper used the execution of test machine states to approximate the time cost of each sequence. The 6502 has a simpler execution model where most instructions have fixed cycle cost, and some have a 1 or 2 cycle penalty based on runtime conditions, like taking a branch or crossing a page boundary. This allows a simple cost to be assigned to each instruction, with possible cycle penalties represented as fractions of a cycle. The cost of a sequence is simply the sum of cost of the instructions. In the future, the solver could be used to find cases where the penalties must or cannot happen. Besides time, other models like code size should have some weight.
4. Operand masks: Each instruction sequences uses the unknown operands Absolute0, Zp0, etc. The candidate cheaper sequence cannot use more operands than were provided in the input, so if there are any, skip that sequence.
5. Future work: data flow checks. One useful fact is that if a sequence has an output -- either register, processor flag, or memory location -- that it never takes as an input, then it must change that output for some inputs. It cannot preserve it in all cases. That means we only need to check sequences that also have that output.

Here are some sample replacement templates:

```
lsr Absolute0; bmi Absolute1 -> lsr Absolute0
```

```
clc; sec; -> sec
```

etc.
