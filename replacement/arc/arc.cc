#include "arc.h"
#include <cassert>

arc::arc(CACHE* cache)
  : arc(cache, cache->get_sets(), cache->get_ways()) {}

arc::arc(CACHE* cache, long sets, long ways)
  : NUM_SET(sets), NUM_WAY(ways), state(sets) {}

static void move_to_front(std::deque<long>& dq, long way) {
  auto it = std::find(dq.begin(), dq.end(), way);
  if (it != dq.end()) {
    dq.erase(it);
    dq.push_front(way);
  }
}

long arc::find_victim(uint32_t, uint64_t, long set,
                      const champsim::cache_block* current_set,
                      champsim::address, champsim::address,
                      access_type) {
  auto& S = state[set];
  // If any way invalid (empty) choose that first
  for (long way = 0; way < NUM_WAY; ++way) {
    if (! current_set[way].valid) return way;
  }
  // ARC replacement
  long victim;
  if (!S.T1.empty() && (S.T1.size() > S.p ||
      (!S.B2.empty() && S.T1.size() == S.p))) {
    victim = S.T1.back();
  } else {
    assert(!S.T2.empty());
    victim = S.T2.back();
  }
  return victim;
}

void arc::replacement_cache_fill(uint32_t, long set, long way,
                                 champsim::address full_addr,
                                 champsim::address, champsim::address victim_addr,
                                 access_type) {
  auto& S = state[set];
  // On fill, victim has been evicted from T1 or T2
  // Remove from ghost lists if needed
  // Add evicted tag to B1 or B2
  // Determine evicted tag's list by checking full_addr match? champSim doesn't expose tags here.
  // Instead, we rely on find_victim logic to know which structure we removed from:
  // This method is called after eviction; simply update B1/B2
  // But ChampSim passes victim_addr, we can infer T1/T2 membership by comparing previous lists

  // For simplicity, treat victim removal in update_replacement_state
}

void arc::update_replacement_state(uint32_t, long set, long way,
                                   champsim::address full_addr,
                                   champsim::address, champsim::address victim_addr,
                                   access_type, uint8_t hit) {
  auto& S = state[set];
  // Hit in cache
  if (hit) {
    // If in T1 move to T2; if in T2 move to front
    if (std::find(S.T1.begin(), S.T1.end(), way) != S.T1.end()) {
      S.T1.erase(std::find(S.T1.begin(), S.T1.end(), way));
      S.T2.push_front(way);
    } else {
      move_to_front(S.T2, way);
    }
    return;
  }
  // Miss: full_addr newly fetched, track in T1 initially
  // If in B1 => increase p
  auto it_b1 = std::find(S.B1.begin(), S.B1.end(), static_cast<long>(way));
  if (it_b1 != S.B1.end()) {
    long delta = std::max((long)S.B2.size() / (long)S.B1.size(), 1L);
    S.p = std::min(S.p + delta, NUM_WAY);
    S.B1.erase(it_b1);
  }
  // If in B2 => decrease p
  auto it_b2 = std::find(S.B2.begin(), S.B2.end(), static_cast<long>(way));
  if (it_b2 != S.B2.end()) {
    long delta = std::max((long)S.B1.size() / (long)S.B2.size(), 1L);
    S.p = std::max(S.p - delta, 0L);
    S.B2.erase(it_b2);
  }
  // Now perform replacement eviction to maintain size constraints
  // Choose victim using find_victim logic and move it to ghost list
  long victim = find_victim(0, 0, set, nullptr, 0, 0, access_type::LOAD);
  // If victim from T1
  if (std::find(S.T1.begin(), S.T1.end(), victim) != S.T1.end()) {
    S.T1.pop_back();
    S.B1.push_front(victim);
  } else {
    S.T2.pop_back();
    S.B2.push_front(victim);
  }
  // Finally insert new item into T1 at front
  S.T1.push_front(way);
  // Trim B1 and B2 to size NUM_WAY
  if ((long)S.B1.size() > NUM_WAY) S.B1.pop_back();
  if ((long)S.B2.size() > NUM_WAY) S.B2.pop_back();
}