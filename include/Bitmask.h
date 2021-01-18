#pragma once

#include "Bitvector.h"

#include <cstddef>
#include <vector>


class Bitmask {
	std::size_t size;
public:
	Bitvector BLK;
	Bitvector SRC;
	Bitvector DST;

	Bitmask()=default;
	Bitmask(std::size_t size_);
	Bitmask(const std::vector<bool> &blk, const std::vector<bool> &src, const std::vector<bool> &dst);
};
