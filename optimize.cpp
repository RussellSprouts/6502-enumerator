
#include "instructions2.h"

enum class used_variables : uint64_t {
  absolute  = 0xF,
  absolute0 = 0x1,
  absolute1 = 0x2,
  absolute2 = 0x4,
  absolute3 = 0x8,

  zp = 0xF0,
  zp0 = 0x10,
  zp1 = 0x20,
  zp2 = 0x40,
  zp3 = 0x80,

  immediate = 0xF00,
  immediate0 = 0x100,
  immediate1 = 0x200,
  immediate2 = 0x400,
  immediate3 = 0x800,
};

/*
NEEDS:
- Canonicalization and rewriting of instruction sequences.
- SSA form of instruction sequence, to allow building a dag of instruction orderings.

Canonicalization: 

for example, the instruction sequence:

  lda #1
  ldx #0
  sta absolute0, x
  ldx #1
  stx absolute0

  Can be represented like this:

  a0, s0, n0, mem0 = start()
  a1, s1, n1       = lda(#1)
  x1, s2, n2       = ldx(#0)
  mem1             = sta(a1, absolute0, x1, mem0)
  x2, s3, n3       = ldx(#1)
  mem2             = stx(x2, absolute0, mem1)
                     end(mem2, a1, x2, s3, n3)


  a0      s0     n0      mem0 = start()
   |       |     |        |
   x       x     x        |
                          |
  a1      s1     n1       |   = lda(#1)
   \       |      |       |
    \      x      x       |
     +--------------------+
  x1 |    s2     n2       |   = ldx(#0)
   | |     |      |       \
   x \     x      x        ---------+---------------+
      -----------------             |               |
  mem1                 \      = sta(a1, absolute0, mem0)
     \                 |
      +----------------)---------------------------+
                       |                           |
  x2      s3     n3    |      = ldx(#1)            |
   \       |      |    |                           |
    \      |      |    |                           |
     ------)------)----)---+--------               |
           |      |    |   |        \
  mem2     |      |    |   |   = sta(x2, absolute0, mem1)
    \      |      |    |   |
     ------+------+----+---+---------+----+---+---+---+
                                     |    |   |   |   |
                                end(mem2, a1, x2, s3, n3)

  Which forms this dag:

              o
             / \
        +----   ----+
      lda(#1)     ldx(#0)
          |         | 
       sta(absolute0, x)
               |
            ldx(#1)
               |
          stx(absolute0)

  A topological sort of this graph gives
  all of the possible orderings of the instructions,
  which doesn't leave much room for changes.

  Now, we can apply optimizations:
  1. "ldx #0 ; sta absolute0, x", where x is not live,
     is the same as sta absolute0.

              o
             / \
         +---   ----+
      lda(#1)     ldx(#1)
         |          | 
   sta(absolute0)   |
              |     |
            stx(absolute0)
  2. "lda #1 ; ldx #1" is "lda #1 ; tax"

     lda #1
     tax
     sta absolute0
     stx absolute0

  3. "sta absolute0 ; stx absolute0" is "stx absolute0"

     lda #1
     tax
     stx absolute0
*/
