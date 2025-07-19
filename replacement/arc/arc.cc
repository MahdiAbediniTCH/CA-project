#include "arc.h"
#include <algorithm>
#include <cassert>

static void move_to_front(std::deque<long>& dq, long way) {
  auto it = std::find(dq.begin(), dq.end(), way);
  if (it != dq.end()) {
    dq.erase(it);
    dq.push_front(way);
  }
}

arc::arc(CACHE* cache)
  : arc(cache, cache->NUM_SET, cache->NUM_WAY) {}

arc::arc(CACHE* /*cache*/, long sets, long ways)
  : NUM_SET(sets)
  , NUM_WAY(ways)
  , state(sets)
{}

long arc::find_victim(uint32_t, uint64_t, long set,
                      const champsim::cache_block* current_set,
                      champsim::address, champsim::address,
                      access_type) {
  auto& S = state[set];
  // choose any invalid way first
  for (long way = 0; way < NUM_WAY; ++way) {
    if (!current_set[way].valid)
      return way;
  }
  // ARC decision: evict from T1 or T2
  if (!S.T1.empty() && (S.T1.size() > S.p || (!S.B2.empty() && S.T1.size() == S.p))) {
    return S.T1.back();
  }
  assert(!S.T2.empty());
  return S.T2.back();
}

void arc::replacement_cache_fill(uint32_t, long set, long way,
                                 champsim::address, champsim::address,
                                 champsim::address victim_addr,
                                 access_type) {
  // eviction has occurred: victim_addr identifies the tag evicted
  auto& S = state[set];
  // if evicted was in T1, move to B1; if in T2, move to B2
  // since we track ways, assume last victim removal done in update_replacement_state
  (void)victim_addr;
}

void arc::update_replacement_state(uint32_t, long set, long way,
                                   champsim::address full_addr,
                                   champsim::address, champsim::address,
                                   access_type, uint8_t hit) {
  auto& S = state[set];
  // cache hit
  if (hit) {
    if (std::find(S.T1.begin(), S.T1.end(), way) != S.T1.end()) {
      S.T1.erase(std::find(S.T1.begin(), S.T1.end(), way));
      S.T2.push_front(way);
    } else {
      move_to_front(S.T2, way);
    }
    return;
  }
  // cache miss: adjust p if in ghost lists
  auto it1 = std::find(S.B1.begin(), S.B1.end(), way);
  if (it1 != S.B1.end()) {
    long delta = std::max((long)S.B2.size() / (long)S.B1.size(), 1L);
    S.p = std::min(S.p + delta, NUM_WAY);
    S.B1.erase(it1);
  }
  auto it2 = std::find(S.B2.begin(), S.B2.end(), way);
  if (it2 != S.B2.end()) {
    long delta = std::max((long)S.B1.size() / (long)S.B2.size(), 1L);
    S.p = std::max(S.p - delta, 0L);
    S.B2.erase(it2);
  }
  // choose and remove victim from T1 or T2, add to ghost
  long victim = find_victim(0, 0, set, nullptr, 0, 0, access_type::LOAD);
  if (std::find(S.T1.begin(), S.T1.end(), victim) != S.T1.end()) {
    S.T1.pop_back();
    S.B1.push_front(victim);
  } else {
    S.T2.pop_back();
    S.B2.push_front(victim);
  }
  // insert new way into T1
  S.T1.push_front(way);
  // trim ghosts
  if ((long)S.B1.size() > NUM_WAY) S.B1.pop_back();
  if ((long)S.B2.size() > NUM_WAY) S.B2.pop_back();
}
