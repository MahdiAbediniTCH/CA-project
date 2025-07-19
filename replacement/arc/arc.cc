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

long arc::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
                      const champsim::cache_block* current_set,
                      champsim::address ip, champsim::address addr,
                      access_type type)
{
  // 1. Use any invalid way first
  for (long way = 0; way < NUM_WAY; ++way) {
    if (!current_set[way].valid) {
      return way;
    }
  }

  // 2. Determine eviction source based on adaptive target 'p'
  champsim::address victim_addr{};
  bool in_b2 = (std::find(B2.begin(), B2.end(), addr) != B2.end());
  bool evict_from_T1 = (!T1.empty() &&
                       (static_cast<long>(T1.size()) > p || (in_b2 && static_cast<long>(T1.size()) == p)));

  if (evict_from_T1) {
    // Evict LRU from T1
    victim_addr = T1.back();
    T1.pop_back();
    B1.push_front(victim_addr);
    if (static_cast<long>(B1.size()) > NUM_WAY) {
      B1.pop_back();
    }
  } else {
    // Evict LRU from T2
    victim_addr = T2.back();
    T2.pop_back();
    B2.push_front(victim_addr);
    if (static_cast<long>(B2.size()) > NUM_WAY) {
      B2.pop_back();
    }
  }

  // 3. Find the way index that matches the victim address
  for (long way = 0; way < NUM_WAY; ++way) {
    if (current_set[way].valid && current_set[way].address == victim_addr) {
      return way;
    }
  }

  // Fallback
  return 0;
}

void arc::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                 champsim::address addr, champsim::address ip,
                                 champsim::address victim_addr, access_type type)
{
  // Case: refill from B1 (ghost of T1) -> increase 'p'
  auto itB1 = std::find(B1.begin(), B1.end(), addr);
  if (itB1 != B1.end()) {
    long growth = std::max(1L, static_cast<long>(B2.size()) / std::max(1L, static_cast<long>(B1.size())));
    p = std::min<long>(NUM_WAY, p + growth);
    B1.erase(itB1);
    T2.push_front(addr);
    return;
  }

  // Case: refill from B2 (ghost of T2) -> decrease 'p'
  auto itB2 = std::find(B2.begin(), B2.end(), addr);
  if (itB2 != B2.end()) {
    long shrink = std::max(1L, static_cast<long>(B1.size()) / std::max(1L, static_cast<long>(B2.size())));
    p = std::max(0L, p - shrink);
    B2.erase(itB2);
    T2.push_front(addr);
    return;
  }

  // New block: insert into T1
  T1.push_front(addr);

  // Trim history lists
  if (static_cast<long>(B1.size()) > NUM_WAY) B1.pop_back();
  if (static_cast<long>(B2.size()) > NUM_WAY) B2.pop_back();
}

void arc::update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                   champsim::address addr, champsim::address ip,
                                   champsim::address victim_addr, access_type type,
                                   uint8_t hit)
{
  if (!hit) return;

  // On hit: promote from T1 to T2 or refresh in T2
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
