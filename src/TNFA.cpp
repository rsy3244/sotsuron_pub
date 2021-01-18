#include "TNFA.h"
#include "Parser.h"
#include "parameter.h"

#include <iostream>
#include <cassert>
#include <optional>
#include <vector>
#include <string>
	
void TNFA::add_state() {
	m_state_size += 1;
	m_edges.push_back(std::vector<Edge>());
}

void TNFA::add_edge(std::size_t src, std::size_t dst) {
	m_edges[src].emplace_back(src, dst, std::nullopt);
}

void TNFA::add_edge(std::size_t src, std::size_t dst, char label) {
	m_edges[src].emplace_back(src, dst, std::optional<char>(label));
}

void TNFA::add_rev_edges() {
	m_rev_edges.resize(m_edges.size());
	for(auto a: m_edges) {
		for(auto e : a) {
			m_rev_edges[e.dst].emplace_back(e);
		}
	}
}

void TNFA::dump_edges(std::size_t idx) {
	for(auto e : m_edges[idx]){
		std::cerr << e.src << " " << e.dst;
		if(e.label) std::cerr << " " << e.label.value();
		std::cerr << std::endl;
	}
}

TNFA::TNFA():
	m_state_size(1), m_edges(std::vector<std::vector<Edge>>(1,std::vector<Edge>())) {}

std::size_t TNFA::size() const {
	return m_state_size;
}

std::vector<Edge> TNFA::get_edges(std::size_t src) const {
	return m_edges[src];
}

