#include "Parser.h"

#include <cstddef>
#include <cassert>
#include <iostream>
#include <vector>
#include <numeric>

std::size_t Parser::parse_int(std::size_t &idx) {
	idx++;
	assert(isdigit(pattern[idx]));
	std::size_t ret = pattern[idx] - '0';
	while(isdigit(pattern[++idx])) {
		ret = ret*10 + pattern[idx] - '0';
	}
	return ret;
}


/* 
 * BNF 
 * expr := factor | factor '|' expr
 * factor := primary | primary factor
 * primary := atom | '(' expr ')' | '(' expr ')*'
 * atom :=  alpha | alpha'?' | '.' | alpha'*' | alpha'+' | alpha'{'num','num'}'
 */

void Parser::expr(std::size_t &idx) {
	auto check_union = [&](){
		std::size_t d = 1;
		for(std::size_t i = idx; d > 0 && i < pattern.size(); ++i) {
			if(pattern[i] == '(') ++d;
			if(pattern[i] == ')') --d;
			if(pattern[i] == '|' && d == 1) return true;
		}
		return false;
	};
	if(!check_union()){
		factor(idx);
		return;
	}
	std::size_t src = tnfa.size()-1;
	std::vector<std::size_t> dst = std::vector<std::size_t>();
	tnfa.add_state();
	tnfa.add_edge(src, src+1);
	factor(idx);
	dst.push_back(tnfa.size()-1);
	while(pattern[idx] == '|') {
		tnfa.add_state();
		tnfa.add_edge(src, tnfa.size()-1);
		factor(++idx);
		dst.push_back(tnfa.size()-1);
	}
	tnfa.add_state();
	std::size_t gather_state = tnfa.size()-1;
	for(auto e : dst) tnfa.add_edge(e, gather_state);
}

void Parser::factor(std::size_t &idx) {
	while(idx < length && pattern[idx] != ')' && pattern[idx] != '|') {
		primary(idx);
	}
}

void Parser::primary(std::size_t &idx) {
	if(pattern[idx] == '('){
		++idx;
		auto is_kleene_star = [&](){
			auto idx_tmp = idx;
			for(std::size_t d = 1; d > 0 && idx_tmp < pattern.size(); ++idx_tmp) {
				if(pattern[idx_tmp] == '(') ++d;
				if(pattern[idx_tmp] == ')') --d;
			}
			if(idx_tmp >= length)return false;
			return pattern[idx_tmp] == '*';
		};
		if(is_kleene_star()) {
			auto src = tnfa.size()-1;
			tnfa.add_state();
			tnfa.add_edge(src, src+1);
			expr(idx);
			assert(pattern[idx++] == ')');
			assert(pattern[idx++] == '*');
			auto dst = tnfa.size();
			tnfa.add_edge(dst-1, src+1);
			tnfa.add_state();
			tnfa.add_edge(dst-1, dst);
			tnfa.add_edge(src, dst);
		} else {
			expr(idx);
			assert(pattern[idx] == ')');
			++idx;
		}
	} else {
		atom(idx);
	}
}

void Parser::atom(std::size_t &idx) {
	std::size_t src = tnfa.size()-1;
	tnfa.add_state();
	std::size_t dst = tnfa.size()-1;
	if(pattern[idx] == '.') {
		for(std::size_t i = 0; i < SIGMA; ++i) tnfa.add_edge(src, dst, static_cast<char>(i)); 
		++idx;
		switch(pattern[idx]) {
			case '?':
				tnfa.add_edge(src, dst);
				break;
			case '*':
				tnfa.add_edge(src, dst);
				for(std::size_t i = 0; i < SIGMA; ++i) tnfa.add_edge(dst, dst, static_cast<char>(i)); 
				break;
			case '+': 
				for(std::size_t i = 0; i < SIGMA; ++i) tnfa.add_edge(dst, dst, static_cast<char>(i)); 
				break;
			case '{': {
				std::size_t x = parse_int(idx);
				assert(pattern[idx] == ',');
				std::size_t y = parse_int(idx);
				assert(pattern[idx] == '}');
				assert(y > x);
				tnfa.add_state();
				for(std::size_t i = 0; i < y-x-1; i++) {
					for(std::size_t i = 0; i < SIGMA; ++i) tnfa.add_edge(src, dst, static_cast<char>(i)); 
					tnfa.add_edge(src, dst);
					tnfa.add_state();
				}
				for(std::size_t i = 0; i < x; i++) {
					for(std::size_t i = 0; i < SIGMA; ++i) tnfa.add_edge(src, dst, static_cast<char>(i)); 
					tnfa.add_state();
				}
			} break;
			default:
				--idx;
		}
		++idx;
	} else {
		if(pattern[idx] == '\\') ++idx;
		char label = pattern[idx];
		tnfa.add_edge(src, dst, label);
		++idx;
		switch(pattern[idx]) {
			case '?':
				tnfa.add_edge(src, dst);
				break;
			case '*': 
				tnfa.add_edge(src, dst);
				tnfa.add_edge(dst, dst, label);
				break;
			case '+': 
				tnfa.add_edge(dst, dst, label);
				break;
			case '{': {
				std::size_t x = parse_int(idx);
				assert(pattern[idx] == ',');
				std::size_t y = parse_int(idx);
				assert(pattern[idx] == '}');
				assert(y > x);
				tnfa.add_state();
				src++; dst++;
				for(std::size_t i = 0; i < y-x-1; i++) {
					tnfa.add_edge(src, dst, label);
					tnfa.add_edge(src, dst);
					tnfa.add_state();
					src++; dst++;
				}
				for(std::size_t i = 0; i < x; i++) {
					tnfa.add_edge(src, dst, label);
					tnfa.add_state();
					src++; dst++;
				}
			} break;
			default:
				--idx;
		}
		++idx;
	}
}


Parser::Parser(const std::string pattern_):
	pattern(pattern_), length(pattern.size()), tnfa()
{
	size_t idx = 0;
	expr(idx);
	tnfa.add_rev_edges();
	/*
	std::cerr << "----TNFA----" << std::endl;
	std::cerr << tnfa.m_state_size << " " << std::accumulate(tnfa.m_edges.begin(), tnfa.m_edges.end(), 0, [](std::size_t acc, std::vector<Edge>& a) { return acc+a.size();}) << std::endl;
	for(std::size_t i = 0; i < tnfa.m_state_size; ++i) {
		tnfa.dump_edges(i);
	}
	std::cerr << "------------" << std::endl;
	*/
}

TNFA& Parser::get_tnfa() {
	return tnfa;
}
