#include "arc.h"
#include <algorithm>
#include <cassert>

arc::arc(CACHE* cache)
  : arc(cache, cache->NUM_SET, cache->NUM_WAY) {}

arc::arc(CACHE* cache, long sets, long ways)
  : replacement(cache), NUM_SET(sets), NUM_WAY(ways)
{
  // Initialize adaptive parameter
  p = 0;
}

//------------------------------------------------------------------------------
// 1) find_victim: pick an invalid way or choose the LRU victim from T1/T2
//    but do NOT mutate T1/T2/B1/B2 here
//------------------------------------------------------------------------------
uint32_t arc::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set,
                          const BLOCK* current_set, uint64_t ip, uint64_t addr,
                          access_type type)
{
  // 1. Search for an invalid line
  for (uint32_t way = 0; way < NUM_WAY; ++way) {
    if (!current_set[way].valid)
      return way;
  }

  // 2. Fully occupied: decide T1 vs T2 eviction
  bool is_ghost_hit_in_b2 = (std::find(B2.begin(), B2.end(), addr) != B2.end());
  bool evict_from_T1 =
       !T1.empty() &&
      (static_cast<long>(T1.size()) > p ||
       (is_ghost_hit_in_b2 && static_cast<long>(T1.size()) == p));

  uint64_t victim_addr = evict_from_T1 ? T1.back() : T2.back();

  // 3. Find the way index that holds this victim address
  for (uint32_t way = 0; way < NUM_WAY; ++way) {
    if (current_set[way].valid && current_set[way].address == victim_addr)
      return way;
  }

  // 4. Fallback: should never happen
  return NUM_WAY;  // indicates bypass (not used in ARC, but compliant)
}


void arc::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                 uint64_t full_addr, uint64_t ip,
                                 uint64_t victim_addr, access_type type)
{
  // Case 2: B1 hit → grow p
  auto itB1 = std::find(B1.begin(), B1.end(), full_addr);
  if (itB1 != B1.end()) {
    long growth = std::max(1L, static_cast<long>(B2.size()) /
                                std::max(1L, static_cast<long>(B1.size())));
    p = std::min<long>(NUM_WAY, p + growth);

    bool evict_from_T1 = !T1.empty() &&
                         (static_cast<long>(T1.size()) > p);
    if (evict_from_T1) {
      auto v = T1.back(); T1.pop_back();
      B1.push_front(v);
      if ((long)B1.size() > NUM_WAY) B1.pop_back();
    } else {
      auto v = T2.back(); T2.pop_back();
      B2.push_front(v);
      if ((long)B2.size() > NUM_WAY) B2.pop_back();
    }

    B1.erase(itB1);
    T2.push_front(full_addr);
    return;
  }

  // Case 3: B2 hit → shrink p
  auto itB2 = std::find(B2.begin(), B2.end(), full_addr);
  if (itB2 != B2.end()) {
    long shrink = std::max(1L, static_cast<long>(B1.size()) /
                                 std::max(1L, static_cast<long>(B2.size())));
    p = std::max(0L, p - shrink);

    bool evict_from_T1 = !T1.empty() &&
                         (static_cast<long>(T1.size()) > p ||
                          static_cast<long>(T1.size()) == p);
    if (evict_from_T1) {
      auto v = T1.back(); T1.pop_back();
      B1.push_front(v);
      if ((long)B1.size() > NUM_WAY) B1.pop_back();
    } else {
      auto v = T2.back(); T2.pop_back();
      B2.push_front(v);
      if ((long)B2.size() > NUM_WAY) B2.pop_back();
    }

    B2.erase(itB2);
    T2.push_front(full_addr);
    return;
  }

  // Case 4: New block
  long size_T1 = (long)T1.size();
  long size_T2 = (long)T2.size();
  long size_B1 = (long)B1.size();
  long size_B2 = (long)B2.size();

  bool full_T1_B1 = (size_T1 + size_B1) >= NUM_WAY;
  bool full_history = (size_T1 + size_T2 + size_B1 + size_B2) >= NUM_WAY;
  bool overflow_history = (size_T1 + size_T2 + size_B1 + size_B2) >= 2 * NUM_WAY;

  if (full_T1_B1) {
    if (size_T1 < NUM_WAY) {
      B1.pop_back(); // Evict ghost
    } else {
      auto v = T1.back(); T1.pop_back();
      B1.push_front(v);
      if ((long)B1.size() > NUM_WAY) B1.pop_back();
      T1.push_front(full_addr);
      return;
    }
  } else if (full_history) {
    if (overflow_history) {
      B2.pop_back();
    }
    bool evict_from_T1 = !T1.empty() &&
                         (static_cast<long>(T1.size()) > p);
    if (evict_from_T1) {
      auto v = T1.back(); T1.pop_back();
      B1.push_front(v);
      if ((long)B1.size() > NUM_WAY) B1.pop_back();
    } else {
      auto v = T2.back(); T2.pop_back();
      B2.push_front(v);
      if ((long)B2.size() > NUM_WAY) B2.pop_back();
    }
  }

  T1.push_front(full_addr);
}


void arc::update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                   uint64_t addr, uint64_t ip,
                                   uint64_t victim_addr, access_type type,
                                   bool hit)
{
  if (!hit)
    return;

  // On hit: promote from T1 → T2 or refresh in T2
  auto itT1 = std::find(T1.begin(), T1.end(), addr);
  if (itT1 != T1.end()) {
    T1.erase(itT1);
    T2.push_front(addr);
    return;
  }

  auto itT2 = std::find(T2.begin(), T2.end(), addr);
  if (itT2 != T2.end()) {
    T2.erase(itT2);
    T2.push_front(addr);
  }
}
