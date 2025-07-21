#ifndef REPLACEMENT_LARC_H
#define REPLACEMENT_LARC_H

#include <vector>

#include "cache.h"
#include "modules.h"

class larc : public champsim::modules::replacement
{
  static constexpr float CR_UPPERBOUND = 0.9f, CR_LOWERBOUND = 0.1f;

  long NUM_WAY;
  float cr;
  // 2D vectors for LRU queues
  std::vector<std::vector<short>> all_q; // Contains way numbers
  std::vector<std::vector<champsim::address>> all_qr; // Contains physical addresses

public:
  explicit larc(CACHE* cache);
  larc(CACHE* cache, long sets, long ways);

  // void initialize_replacement();
  long find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                   champsim::address full_addr, access_type type);
  void update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr,
                                access_type type, uint8_t hit);
  // void replacement_final_stats()
};

#endif
