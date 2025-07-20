#include "arc.h"
#include <algorithm>
#include <cassert>
#include <iostream>

arc::arc(CACHE* cache) : arc(cache, cache->NUM_SET, cache->NUM_WAY) {}

arc::arc(CACHE* cache, long sets, long ways)
  : replacement(cache), NUM_WAY(ways), NUM_SET(sets),
    T1(sets), T2(sets), B1(sets), B2(sets), p(sets, 0) {}

long arc::find_victim(uint32_t cpu, uint64_t instr_id, long set,
                      const champsim::cache_block* current_set,
                      champsim::address ip, champsim::address full_addr,
                      access_type type)
{
    auto& t1 = T1[set];
    auto& t2 = T2[set];
    auto& b1 = B1[set];
    auto& b2 = B2[set];
    auto& pval = p[set];

    // If cache is full, evict based on ARC rule
    if ((long)(t1.size() + t2.size()) >= NUM_WAY) {
        int victim_way;

        if (!t1.empty() && (t1.size() > pval || t2.empty())) {
            victim_way = t1.back();
            t1.pop_back();
            b1.push_front(victim_way);
        } else {
            victim_way = t2.back();
            t2.pop_back();
            b2.push_front(victim_way);
        }

        // Limit B1 and B2 sizes
        const size_t ghost_limit = NUM_WAY;
        if (b1.size() > ghost_limit) b1.pop_back();
        if (b2.size() > ghost_limit) b2.pop_back();

        return victim_way;
    }

    // Cache not full — use invalid block
    for (long way = 0; way < NUM_WAY; ++way) {
        if (!current_set[way].valid)
            return way;
    }

    // All valid but not full? Fallback
    return 0;
}


void arc::replacement_cache_fill(uint32_t cpu, long set, long way,
                                 champsim::address full_addr, champsim::address ip,
                                 champsim::address victim_addr, access_type type)
{
    auto& t1 = T1[set];
    auto& t2 = T2[set];
    auto& b1 = B1[set];
    auto& b2 = B2[set];
    auto& pval = p[set];

    // If way was recently in B1, promote to T2 and increase p
    if (auto it = std::find(b1.begin(), b1.end(), way); it != b1.end()) {
        b1.erase(it);
        t2.push_front(way);
        size_t delta = std::max<size_t>(1, b2.size() / std::max<size_t>(1, b1.size()));
        pval = std::min<uint64_t>(NUM_WAY, pval + delta);
    }
    // If way was in B2, promote to T2 and decrease p
    else if (auto it = std::find(b2.begin(), b2.end(), way); it != b2.end()) {
        b2.erase(it);
        t2.push_front(way);
        size_t delta = std::max<size_t>(1, b1.size() / std::max<size_t>(1, b2.size()));
        pval = std::max<int64_t>(0, static_cast<int64_t>(pval) - static_cast<int64_t>(delta));
    }
    // Else insert new block into T1
    else {
        t1.push_front(way);
    }
}



void arc::update_replacement_state(uint32_t cpu, long set, long way,
                                   champsim::address full_addr, champsim::address ip,
                                   champsim::address victim_addr, access_type type,
                                   uint8_t hit)
{
    auto& t1 = T1[set];
    auto& t2 = T2[set];

    if (hit) {
        // Promote from T1 to T2
        if (auto it = std::find(t1.begin(), t1.end(), way); it != t1.end()) {
            t1.erase(it);
            t2.push_front(way);
        }
        // Refresh position in T2
        else if (auto it = std::find(t2.begin(), t2.end(), way); it != t2.end()) {
            t2.erase(it);
            t2.push_front(way);
        } else {
            assert(false && "ARC: Hit on block not in T1 or T2");
        }
    }
}