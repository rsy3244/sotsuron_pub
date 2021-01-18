#include "Matcher.h"

#include <cstddef>
#include <cassert>
#include <queue>
#include <iostream>
#include <vector>

void Matcher::scatter(const std::size_t depth) {
	state |= ((m_scatter[depth].BLK - (state & m_scatter[depth].SRC)) & m_scatter[depth].DST);
}

void Matcher::gather(const std::size_t depth) {
	state |= ((m_gather[depth].BLK + (state & m_gather[depth].SRC)) & m_gather[depth].DST);
}

void Matcher::propagate(const std::size_t depth) {
	Bitvector a(m_propagate[depth].DST);
	a |= (state & m_propagate[depth].BLK);
	state |= (m_propagate[depth].BLK & ((~(a - m_propagate[depth].SRC)) ^ a));
}

void Matcher::back_eps(const std::size_t depth) {
	if(m_back_src.size() == 0) return;
	Bitvector a(state);
	a &= m_back_src[depth];
	for(std::size_t i = 0; i < m_back_blk[depth].size(); ++i) {
		// a = ((a & ~m_back_blk[depth][i]) | (a & m_back_blk[depth][i]) >> (1 << i));
		Bitvector tmp(a);
		tmp &= m_back_blk[depth][i];
		tmp >>= (1 << i);
		a &= ~m_back_blk[depth][i];
		a |= tmp;
	}
	a &= m_back_dst[depth];
	state |= a;
}

void Matcher::move(const char c) {
	std::size_t idx = static_cast<std::size_t>(c);
	state = (((state << static_cast<std::size_t>(1)) & CHR[idx]) | 1) | (state & REP[idx]);
}

void Matcher::eps_closure() {
	for(std::size_t k = m_depth-1; k+1 > 1; --k ) {
		propagate(k);
		back_eps(k);
		gather(k-1);
	}
	propagate(0);
	back_eps(0);
	for(std::size_t k = 1; k < m_depth; ++k) {
		scatter(k-1);
		propagate(k);
	}
}

Matcher::Matcher(TNFA& tnfa):
	m_depth(0), m_state_size(tnfa.size()), m_accept(tnfa.size()-1) { 
	for(auto &e : CHR) e = Bitvector(m_state_size);
	for(auto &e : REP) e = Bitvector(m_state_size);
	for(auto a : tnfa.m_edges) for(auto e : a) {
		if(e.label) {
			if(e.src == e.dst) REP[static_cast<std::size_t>(e.label.value())].set(e.dst, true);
			else CHR[static_cast<std::size_t>(e.label.value())].set(e.dst, true);
		}
	}
	std::vector<std::size_t> depth(m_state_size, 0);
	std::size_t now_depth = 0;
	for(std::size_t idx = 0; idx < m_state_size; ++idx) {
		std::size_t edge_size = 0;
		for(auto e : tnfa.m_edges[idx]) if(!e.label && e.src < e.dst) edge_size += 1;
		std::size_t rev_edge_size = 0;
		for(auto e : tnfa.m_rev_edges[idx]) if(!e.label && e.src < e.dst) rev_edge_size += 1;
		if(rev_edge_size > 1) now_depth -= 1;
		depth[idx] = now_depth;
		if(edge_size > 1) {
			now_depth += 1;
			m_depth = std::max(now_depth, m_depth);
		}
	}
	m_depth += 1;

	m_gather = std::vector<Bitmask>(m_depth, Bitmask(m_state_size));
	m_scatter = std::vector<Bitmask>(m_depth, Bitmask(m_state_size));
	m_propagate = std::vector<Bitmask>(m_depth, Bitmask(m_state_size));

	for(std::size_t d = m_depth-1; d+1 > 0; --d) {
		for(std::size_t idx = 0; idx < m_state_size; ++idx) {
			if(depth[idx] > d) m_gather[d].BLK.set(idx, true);
			if(depth[idx] != d) continue;
			for(auto e : tnfa.m_edges[idx]) {
				if(depth[e.dst] < d) { //これはg-edge
					m_gather[d-1].SRC.set(idx, true);
					m_gather[d-1].DST.set(e.dst, true);
					m_scatter[d-1].BLK.set(e.dst, true);
				}
				if(depth[e.dst] == d && !e.label && e.src < e.dst){ //これはp-edge
					m_propagate[d].BLK.set(e.src, true);
					m_propagate[d].BLK.set(e.dst, true);
				}
				if(depth[e.dst] > d) { //これはs-edge
					m_scatter[d].SRC.set(idx, true);
					m_scatter[d].DST.set(e.dst, true);
				}
			}
		}
	}

	std::vector<std::size_t> eps_block(m_state_size);
	std::size_t id = 1;
	for(std::size_t idx = 0; idx < m_state_size-1; ++idx) {
		if(m_propagate[m_depth-1].BLK.get(idx)) {
			for(; m_propagate[m_depth-1].BLK.get(idx); ++idx) {
				eps_block[idx] = id;
			}
			id += 1;
		}
	}

	for(std::size_t d = m_depth-2; d+1 > 0; --d) {
		for(std::size_t idx = 0; idx < m_state_size; ++idx) {
			// 直前の状態から同じ深さのp-edgeが繋がっている
			if(depth[idx] != d) continue;
			for(auto [src, dst, label] : tnfa.m_edges[idx]) {
				if(!label && d == depth[dst] && src < dst) {
					if(eps_block[src] != 0) {
						eps_block[dst] = eps_block[src];
					} else if(eps_block[dst] != 0) {
						eps_block[src] = eps_block[dst];
					} else {
						eps_block[src] = id;
						eps_block[dst] = id;
						id += 1;
					}
					m_propagate[d].BLK.set(src, true);
					m_propagate[d].BLK.set(dst, true);
				}
			}
			/*
			if(idx != 0 && m_propagate[d].BLK.get(idx-1) && m_propagate[d].BLK.get(idx)){
				if(eps_block[idx-1] != 0) {
					eps_block[idx] = eps_block[idx-1];
				}
				else {
					id++;
					eps_block[idx-1] = id;
					eps_block[idx] = id;
					m_propagate[d].BLK.set(idx, true);
				}
			}
			*/
			// s-edgeの遷移元（遷移先のどれかがg-edgeの遷移元までeps-blockで繋がっているか確認すればよい）
			if(m_scatter[d].SRC.get(idx)) {
				std::size_t next_id = eps_block[idx] == 0 ? id+1 : id;
				for(auto [src, dst, label] : tnfa.m_edges[idx]) {
					if(!label && src < dst) {
						for(idx = dst; !m_gather[d].SRC.get(idx); ++idx) {
							if(eps_block[idx] == 0) break;
						}
						if(eps_block[idx] == eps_block[dst] && eps_block[idx] != 0) {
							while(!m_gather[d].DST.get(idx))++idx;
							eps_block[idx] = next_id;
							m_propagate[d].BLK.set(idx, true);
							m_propagate[d].BLK.set(src, true);
							eps_block[src] = next_id;
							id = next_id+1;
							break;
						}
					}
				}
			}
		}
	}
	/*
	std::cerr << "depth : ";
	for(auto e : depth) std::cerr << " " << e;
	std::cerr << std::endl;
	std::cerr << id << "     : ";
	for(auto e : eps_block) std::cerr << " " << e;
	std::cerr << std::endl;
	*/

	std::vector<std::size_t> eps_src(id+1, m_state_size);
	std::vector<std::size_t> eps_dst(id+1);
	for(std::size_t idx = 0; idx < m_state_size; ++idx) {
		if(eps_block[idx] != 0) {
			eps_src[eps_block[idx]] = std::min(eps_src[eps_block[idx]], idx);
			eps_dst[eps_block[idx]] = std::max(eps_dst[eps_block[idx]], idx);
		}
	}
	for(std::size_t i = 1; i < id; ++i) {
		if(eps_src[i] == eps_dst[i]) m_propagate[depth[eps_src[i]]].BLK.set(eps_src[i], false);
		else if(eps_src[i] == m_state_size) continue;
		else {
			m_propagate[depth[eps_src[i]]].SRC.set(eps_src[i], true);
			m_propagate[depth[eps_dst[i]]].DST.set(eps_dst[i], true);
		}
	}

	/*
	for(std::size_t i = 0; i < m_depth; ++i){
		std::cerr << "SCATTER: " << i << std::endl;
		std::cerr << "BLK ";
		m_scatter[i].BLK.dump();
		std::cerr << "SRC ";
		m_scatter[i].SRC.dump();
		std::cerr << "DST ";
		m_scatter[i].DST.dump();
		std::cerr << "GATHER: " << i << std::endl;
		std::cerr << "BLK ";
		m_gather[i].BLK.dump();
		std::cerr << "SRC ";
		m_gather[i].SRC.dump();
		std::cerr << "DST ";
		m_gather[i].DST.dump();
		std::cerr << "PROPAGATE: " << i << std::endl;
		std::cerr << "BLK ";
		m_propagate[i].BLK.dump();
		std::cerr << "SRC ";
		m_propagate[i].SRC.dump();
		std::cerr << "DST ";
		m_propagate[i].DST.dump();
	}
	*/

	auto find_back_edge = [&](){
		for(auto a : tnfa.m_edges) for(auto e : a) {
			if(e.dst < e.src) return true;
		}
		return false;
	};
	if(find_back_edge()) {
		//Kleene-starがある場合の処理
		m_back_blk = std::vector<std::vector<Bitvector>>(m_depth);
		m_back_src = std::vector<Bitvector>(m_depth, Bitvector(m_state_size));
		m_back_dst = std::vector<Bitvector>(m_depth, Bitvector(m_state_size));
		for(auto a : tnfa.m_edges) for(auto e : a) {
			if(e.dst < e.src) {
				assert(!e.label);
				std::size_t distance = e.src - e.dst;
				m_back_src[depth[e.src]].set(e.src, true);
				m_back_dst[depth[e.src]].set(e.dst, true);
				auto ndigit = [](std::size_t num) -> std::size_t {
					if(num == 0) return 0;
					std::size_t ret = 0;
					while(num >>= 1) { ret++; }
					return ret;
				};
				m_back_blk[depth[e.dst]] = std::vector<Bitvector>(ndigit(distance)+1, Bitvector(m_state_size));
				for(std::size_t idx = 0; (std::size_t(1) << idx) <= distance; ++idx) {
					if(distance & (std::size_t(1) << idx)){
						for(std::size_t j = e.dst; j <= e.src; ++j){
							m_back_blk[depth[e.dst]][idx].set(j, true);
						}
					}
				}
			}

			/*
			for(std::size_t i = 0; i < m_depth; ++i) {
				std::cerr << "BACK: " << i << std::endl;
				std::cerr << "SRC ";
				m_back_src[i].dump();
				std::cerr << "DST ";
				m_back_dst[i].dump();
				std::cerr << "BLK " << std::endl;
				for(std::size_t j = 0; j < m_back_blk[i].size(); j++) {
					std::cerr << (1 << j) << ": ";
					m_back_blk[i][j].dump();
				}
			}
			*/
		}

	}

}

std::vector<std::size_t> Matcher::match(const std::string &text) {
	state = Bitvector(m_state_size);
	state.set(0, true);
	eps_closure();
	std::vector<std::size_t> ret;
	for(std::size_t i = 0; i < text.size(); i++) {
		if(state.get(m_accept)) ret.push_back(i);
		move(text[i]);
		eps_closure();
	}
	if(state.get(m_accept)) ret.push_back(text.size());
	return ret;
}

std::size_t Matcher::depth() const{
	return m_depth;
}
std::size_t Matcher::state_size() const{
	return m_state_size;
}
