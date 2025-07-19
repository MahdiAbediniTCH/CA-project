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
long arc::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
                      const champsim::cache_block* current_set,
                      champsim::address ip, champsim::address addr,
                      access_type type)
{
  // (a) any invalid line first
  for (long way = 0; way < NUM_WAY; ++way) {
    if (!current_set[way].valid)
      return way;
  }

  // (b) fully occupied: decide whether to evict from T1 or T2
  bool is_ghost_hit_in_b2 = 
      (std::find(B2.begin(), B2.end(), addr) != B2.end());
  bool evict_from_T1 =
       !T1.empty() &&
      ( static_cast<long>(T1.size()) > p
        || (is_ghost_hit_in_b2 && static_cast<long>(T1.size()) == p)
      );

  // pick the LRU address from the chosen list
  champsim::address victim_addr = 
      evict_from_T1 ? T1.back()
                    : T2.back();

  // (c) find which way holds that address
  for (long way = 0; way < NUM_WAY; ++way) {
    if (current_set[way].valid &&
        current_set[way].address == victim_addr)
    {
      return way;
    }
  }

  // (d) fallback (shouldn't happen)
  return 0;
}


//------------------------------------------------------------------------------
// 2) replacement_cache_fill: on a miss (cold or ghost), update ARC state
//       Cases: B1‐hit (grow p), B2‐hit (shrink p), new miss
//------------------------------------------------------------------------------
void arc::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                 champsim::address addr, champsim::address ip,
                                 champsim::address victim_addr,
                                 access_type type)
{
  // ----- Case 2: ghost‐hit in B1 -----
  auto itB1 = std::find(B1.begin(), B1.end(), addr);
  if (itB1 != B1.end()) {
    // (a) grow p toward recency
    long growth = std::max(1L,
      static_cast<long>(B2.size()) / std::max(1L, static_cast<long>(B1.size()))
    );
    p = std::min<long>(NUM_WAY, p + growth);

    // (b) REPLACE: evict one LRU line from T1 or T2 into B1/B2
    bool is_ghost_b2 = false;  // incoming was from B1
    bool evict_from_T1 =
         !T1.empty() &&
        ( static_cast<long>(T1.size()) > p
          || (is_ghost_b2 && static_cast<long>(T1.size()) == p)
        );
    if (evict_from_T1) {
      auto v = T1.back();  T1.pop_back();
      B1.push_front(v);
      if (static_cast<long>(B1.size()) > NUM_WAY) B1.pop_back();
    }
    else {
      auto v = T2.back();  T2.pop_back();
      B2.push_front(v);
      if (static_cast<long>(B2.size()) > NUM_WAY) B2.pop_back();
    }

    // (c) remove from B1 and install into T2
    B1.erase(itB1);
    T2.push_front(addr);
    return;
  }

  // ----- Case 3: ghost‐hit in B2 -----
  auto itB2 = std::find(B2.begin(), B2.end(), addr);
  if (itB2 != B2.end()) {
    // (a) shrink p toward frequency
    long shrink = std::max(1L,
      static_cast<long>(B1.size()) / std::max(1L, static_cast<long>(B2.size()))
    );
    p = std::max(0L, p - shrink);

    // (b) REPLACE
    bool is_ghost_b2 = true;   // incoming was from B2
    bool evict_from_T1 =
         !T1.empty() &&
        ( static_cast<long>(T1.size()) > p
          || (is_ghost_b2 && static_cast<long>(T1.size()) == p)
        );
    if (evict_from_T1) {
      auto v = T1.back();  T1.pop_back();
      B1.push_front(v);
      if (static_cast<long>(B1.size()) > NUM_WAY) B1.pop_back();
    }
    else {
      auto v = T2.back();  T2.pop_back();
      B2.push_front(v);
      if (static_cast<long>(B2.size()) > NUM_WAY) B2.pop_back();
    }

    // (c) remove from B2 and install into T2
    B2.erase(itB2);
    T2.push_front(addr);
    return;
  }

  // ----- Case 4: completely new block -----
  long size_T1 = static_cast<long>(T1.size());
  long size_T2 = static_cast<long>(T2.size());
  long size_B1 = static_cast<long>(B1.size());
  long size_B2 = static_cast<long>(B2.size());

  bool primary_full  = (size_T1 + size_B1) >= NUM_WAY;
  bool history_full  = (size_T1 + size_T2 + size_B1 + size_B2) >= NUM_WAY;
  bool history_overflow = (size_T1 + size_T2 + size_B1 + size_B2) >= 2 * NUM_WAY;

  if (primary_full) {
    if (size_T1 < NUM_WAY) {
      // drop oldest ghost from B1
      B1.pop_back();
    }
    else {
      // evict LRU of T1 → B1
      auto v = T1.back();  T1.pop_back();
      B1.push_front(v);
      if (static_cast<long>(B1.size()) > NUM_WAY) B1.pop_back();
      // done; no further REPLACE
      T1.push_front(addr);
      return;
    }
  }
  else if (history_full) {
    if (history_overflow) {
      // trim history
      B2.pop_back();
    }
    // now inline REPLACE (incoming is not ghost, treat as b2=false)
    bool is_ghost_b2 = false;
    bool evict_from_T1 =
         !T1.empty() &&
        ( static_cast<long>(T1.size()) > p
          || (is_ghost_b2 && static_cast<long>(T1.size()) == p)
        );
    if (evict_from_T1) {
      auto v = T1.back();  T1.pop_back();
      B1.push_front(v);
      if (static_cast<long>(B1.size()) > NUM_WAY) B1.pop_back();
    }
    else {
      auto v = T2.back();  T2.pop_back();
      B2.push_front(v);
      if (static_cast<long>(B2.size()) > NUM_WAY) B2.pop_back();
    }
  }

  // finally insert the new line into T1
  T1.push_front(addr);
}