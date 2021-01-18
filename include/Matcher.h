#pragma once

#include "TNFA.h"
#include "Bitmask.h"
#include "Bitvector.h"
#include "parameter.h"

#include <vector>
#include <cstddef>
#include <string>

class Matcher {
	std::size_t m_depth;
	std::size_t m_state_size;
	std::size_t m_vec_size;
	std::size_t m_accept;

	Bitvector CHR[SIGMA];
	Bitvector REP[SIGMA];

	std::vector<Bitmask> m_scatter;
	std::vector<Bitmask> m_gather;
	std::vector<Bitmask> m_propagate;

	std::vector<std::vector<Bitvector>> m_back_blk;
	std::vector<Bitvector> m_back_src;
	std::vector<Bitvector> m_back_dst;

	Bitvector state;

	void scatter(const std::size_t depth);
	void gather(const std::size_t depth);
	void propagate(const std::size_t depth);
	void back_eps(const std::size_t depth);
	void move(const char label);
	void eps_closure();

public:
	Matcher(TNFA& tnfa);
	~Matcher() = default;
	std::vector<std::size_t> match(const std::string &text);
	std::size_t depth() const;
	std::size_t state_size() const;
};
