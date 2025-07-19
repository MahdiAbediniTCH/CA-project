#include "tplru.h"

#include <iostream>
#include <algorithm>
#include <cassert>

tplru::tplru(CACHE* cache) : tplru(cache, cache->NUM_SET, cache->NUM_WAY) {}

tplru::tplru(CACHE* cache, long sets, long ways) : replacement(cache), NUM_WAY(ways), all_btrees(static_cast<std::size_t>(sets * ways), 0) {}

long tplru::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                      champsim::address full_addr, access_type type)
{
  std::vector<short> btree(all_btrees.begin() + set * NUM_WAY, all_btrees.end() + (set+1) * NUM_WAY);
  long ind = 1;
  while (ind < NUM_WAY) {
    ind = ind * 2 + btree[ind];
  }
  std::cout << ind - NUM_WAY << std::endl;
  return ind - NUM_WAY;
}

void tplru::replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip, champsim::address victim_addr,
                                 access_type type)
{
  // Mark the way as being used on the current cycle
  long ind = way + NUM_WAY;
  while (ind > 1) {
    all_btrees[set * NUM_WAY + ind / 2] = (ind % 2) ? 0 : 1;
    ind = ind / 2;
  }
}

void tplru::update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                   champsim::address victim_addr, access_type type, uint8_t hit)
{
  // Mark the way as being used on the current cycle
  if (hit && access_type{type} != access_type::WRITE) { // Skip this for writeback hits
    long ind = way + NUM_WAY;
    while (ind > 1) {
      all_btrees[set * NUM_WAY + ind / 2] = (ind % 2) ? 0 : 1;
      ind = ind / 2;
    }
  }
}
