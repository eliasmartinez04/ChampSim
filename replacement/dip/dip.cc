#include "dip.h"

#include <algorithm>
#include <cassert>

dip::dip(CACHE* cache) : dip(cache, cache->NUM_SET, cache->NUM_WAY) {}

dip::dip(CACHE* cache, long sets, long ways)
    : replacement(cache), NUM_WAY(ways),
      last_used_cycles(static_cast<std::size_t>(sets * ways), 0)
{
}

long dip::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
                      const champsim::cache_block* current_set,
                      champsim::address ip, champsim::address full_addr,
                      access_type type)
{
  auto begin = std::next(std::begin(last_used_cycles), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);

  auto victim = std::min_element(begin, end);

  assert(begin <= victim);
  assert(victim < end);

  return std::distance(begin, victim);
}

void dip::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                 champsim::address full_addr,
                                 champsim::address ip,
                                 champsim::address victim_addr,
                                 access_type type)
{
  bool use_bip = false;

  if (set < 32) {
    // LRU leader set
    if (psel < PSEL_MAX)
      psel++;

    use_bip = false;
  }
  else if (set < 64) {
    // BIP leader set
    if (psel > 0)
      psel--;

    use_bip = true;
  }
  else {
    // Follower set
    use_bip = (psel >= PSEL_THRESHOLD);
  }

  if (use_bip) {
    if (bip_counter == 0)
      last_used_cycles.at(static_cast<std::size_t>(set * NUM_WAY + way)) = mru_cycle++;
    else
      last_used_cycles.at(static_cast<std::size_t>(set * NUM_WAY + way)) = lru_cycle--;

    bip_counter = (bip_counter + 1) % 32;
  }
  else {
    // Conventional LRU inserts at MRU
    last_used_cycles.at(static_cast<std::size_t>(set * NUM_WAY + way)) = mru_cycle++;
  }
}

void dip::update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                   champsim::address full_addr,
                                   champsim::address ip,
                                   champsim::address victim_addr,
                                   access_type type, uint8_t hit)
{
  if (hit && access_type{type} != access_type::WRITE)
    last_used_cycles.at(static_cast<std::size_t>(set * NUM_WAY + way)) = mru_cycle++;
}