#pragma once

#include "parameter.h"
#include "TNFA.h"

#include <cstddef>
#include <string>
#include <vector>


class Parser {
	const std::string pattern;
	const std::size_t length;
	
	TNFA tnfa;

	std::size_t parse_int(std::size_t &idx);

	void expr(std::size_t &idx);
	void factor(std::size_t &idx);
	void primary(std::size_t &idx);
	void atom(std::size_t &idx);
	

public:
	Parser(const std::string pattern_);
	~Parser() = default;
	TNFA& get_tnfa();
};
