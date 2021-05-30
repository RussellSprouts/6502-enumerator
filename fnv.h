#pragma once

#include "stdint.h"

typedef struct fnv_hash {
  uint64_t state = 14695981039346656037u;

  explicit fnv_hash(uint32_t seed) {
    add(seed);
  }

  fnv_hash& add(uint8_t value) {
    state = state ^ value;
    state = state * 1099511628211u;
    return *this;
  }

  fnv_hash& add(uint16_t value) {
    return this->add((uint8_t)value).add((uint8_t)(value >> 8));
  }

  fnv_hash& add(uint32_t value) {
    return this->add((uint16_t)value).add((uint16_t)(value >> 16));
  }

  fnv_hash& add(uint64_t value) {
    return this->add((uint32_t)value).add((uint32_t)(value >> 32));
  }

  uint64_t hash64() {
    return state;
  }

  uint32_t hash32() {
    auto hash = hash64();
    return hash ^ (hash >> 32);
  }

  uint16_t hash16() {
    auto hash = hash32();
    return hash ^ (hash >> 16);
  }

  uint8_t hash8() {
    auto hash = hash16();
    return hash ^ (hash >> 8);
  }
} fnv_hash;
