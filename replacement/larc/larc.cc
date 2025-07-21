#include "larc.h"

#include <iostream>
#include <algorithm>
#include <cassert>

larc::larc(CACHE* cache) : larc(cache, cache->NUM_SET, cache->NUM_WAY) {}

larc::larc(CACHE* cache, long sets, long ways) :
  replacement(cache),
  NUM_WAY(ways), 
  cr(static_cast<float>(ways) * CR_LOWERBOUND), 
  all_q(sets, std::vector<short>(ways)), 
  all_qr(sets, std::vector<champsim::address>(ways))
{
  // Initialize each inner vector to 0, 1, ..., NUM_WAY - 1
  for (auto& vec : all_q)
    std::iota(vec.begin(), vec.end(), 0); // fills vec with increasing values
}

long larc::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                      champsim::address full_addr, access_type type)
{
  // This method is called when a miss occurs
  // Update cr
  float c = static_cast<float>(NUM_WAY);
  cr = std::min(CR_UPPERBOUND * c, cr + (c / cr));

  // get reference to the inner vectors
  auto& q = all_q[set]; 
  auto& qr = all_qr[set];

  auto it = std::find(qr.begin(), qr.end(), full_addr);

  if (it != qr.end()) { // Check whether the address is in Q_r
    qr.erase(it);                // Remove from current position
    short selected_way = q.back();
    q.pop_back();
    q.insert(q.begin(), selected_way); // Insert at the front
    // std::cout << selected_way << std::endl;
    return selected_way;
  }
  // Champsim does not support write-through, therefore no bypass is allowed on write
  else if (access_type{type} == access_type::WRITE) {
    short selected_way = q.back();
    q.pop_back();
    q.insert(q.begin(), selected_way); // Insert at the front
    // std::cout << selected_way << std::endl;
    return selected_way;
  }
  else {
    qr.insert(qr.begin(), full_addr);
    if (static_cast<float>(qr.size()) > cr) {
      qr.pop_back();
    }
    // std::cout << NUM_WAY << std::endl;
    return NUM_WAY; // Bypass
  }

}

void larc::update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                   champsim::address victim_addr, access_type type, uint8_t hit)
{
  // Hit condition
  if (hit && access_type{type} != access_type::WRITE) { 
    // Update cr
    float c = static_cast<float>(NUM_WAY);
    cr = std::max(CR_LOWERBOUND * c, cr - (c / (c - cr)));

    auto& q = all_q[set]; // Reference to the inner vector

    // Find the element matching way
    auto it = std::find(q.begin(), q.end(), way);
    if (it != q.end()) {
      // Move the element to the front if found
      short value = *it;
      q.erase(it);                    // Remove from current position
      q.insert(q.begin(), value); // Insert at the front
    }
    else {
      std::cerr << "Could not find hit address in queue data" << std::endl;
    }
  }
}
