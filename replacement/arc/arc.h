#ifndef REPLACEMENT_ARC_H
#define REPLACEMENT_ARC_H

#include <vector>
#include <deque>
#include <algorithm>
#include "cache.h"
#include "modules.h"

class arc : public champsim::modules::replacement {
  long NUM_SET, NUM_WAY;
  // Per-set structures for ARC: T1, T2, B1, B2 and target size p
  struct SetState {
    std::deque<long> T1, T2, B1, B2;
    long p = 0;
  };
  std::vector<SetState> state;

public:
  explicit arc(CACHE* cache);
  arc(CACHE* cache, long sets, long ways);

  long find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
                   const champsim::cache_block* current_set,
                   champsim::address ip, champsim::address full_addr,
                   access_type type) override;

  void replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                              champsim::address full_addr, champsim::address ip,
                              champsim::address victim_addr,
                              access_type type) override;

  void update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                champsim::address full_addr, champsim::address ip,
                                champsim::address victim_addr,
                                access_type type, uint8_t hit) override;
};

#endif // REPLACEMENT_ARC_H