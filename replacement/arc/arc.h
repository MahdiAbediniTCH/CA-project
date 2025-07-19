#ifndef REPLACEMENT_ARC_H
#define REPLACEMENT_ARC_H

#include <deque>
#include <cstdint>
#include "cache.h"
#include "modules.h"

class arc : public champsim::modules::replacement
{
  long NUM_WAY;
  long NUM_SET;

  // ARC lists (store block addresses)
  std::deque<champsim::address> T1; // Recently used once, in cache
  std::deque<champsim::address> T2; // Frequently used, in cache
  std::deque<champsim::address> B1; // Recently evicted from T1
  std::deque<champsim::address> B2; // Recently evicted from T2

  int p = 0; // Adaptive target size for T1
  uint64_t cycle = 0; // Optional global cycle counter

public:
  arc(CACHE* cache);
  arc(CACHE* cache, long sets, long ways);

  long find_victim(uint32_t cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                   champsim::address full_addr, access_type type);

  void replacement_cache_fill(uint32_t cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr,
                              access_type type);

  void update_replacement_state(uint32_t cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr,
                                access_type type, uint8_t hit);
};

#endif
