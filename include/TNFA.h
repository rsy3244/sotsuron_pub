#pragma once

#include <vector>
#include <cstddef>
#include <string>
#include <optional>

class Parser;
class Matcher;

class Edge {
public:
	std::size_t src, dst;
	std::optional<char> label;

	Edge() = default;
	Edge(std::size_t src_, std::size_t dst_, std::optional<char> label_): src(src_), dst(dst_), label(label_) {}
};

class TNFA {
	friend Parser;
	friend Matcher;
	std::size_t m_state_size;
	std::vector<std::vector<Edge>> m_edges;
	std::vector<std::vector<Edge>> m_rev_edges;

	void add_state();
	void add_edge(std::size_t src, std::size_t dst);
	void add_edge(std::size_t src, std::size_t dst, char label);
	void add_rev_edges();

	void dump_edges(std::size_t idx);

public:
	TNFA();
	~TNFA() = default;
	std::size_t size() const;
	std::vector<Edge> get_edges(std::size_t src) const;
};
