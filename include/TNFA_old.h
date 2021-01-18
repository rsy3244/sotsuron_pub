#pragma once
#include "Bitvector.h"
#include "Bitmask.h"

#include <cstddef>
#include <string>
#include <vector>


class TNFA {
	const std::string pattern;
	const std::size_t length;
	std::size_t depth;
	std::size_t state_size;
	std::size_t vec_size;
	std::size_t accept;
	Bitvector CHR[128];
	Bitvector REP[128];

	std::vector<Bitvector> BLK[3];
	std::vector<Bitvector> SRC[3];
	std::vector<Bitvector> DST[3];
	std::vector<Bitmask> m_scatter;
	std::vector<Bitmask> m_gather;
	std::vector<Bitmask> m_propagate;

	Bitvector state;

	void scatter(const std::size_t depth);
	void gather(const std::size_t depth);
	void propagate(const std::size_t depth);
	void move(const char c);
	void eps_closure();

public:
	TNFA() = default;
	TNFA(const std::string  &pattern_);
	bool match(const std::string &text);

};

