#include "Bitvector.h"
#include "parameter.h"

#include <x86intrin.h>

#include <iostream>
#include <cstddef>
#include <cassert>
#include <bitset>

//木の
//heavy path deccomposition
//centroid path decomposition
Bitvector::Bitvector(const std::size_t size): m_val((size+WORDSIZE-1)/WORDSIZE), m_size(size), m_capacity((size+WORDSIZE-1)/WORDSIZE) {}

Bitvector::Bitvector(const Bitvector& obj): m_val(obj.m_val), m_size(obj.m_size), m_capacity(obj.m_capacity) {}

void Bitvector::set(std::size_t idx, bool flag) {
	m_val[idx/WORDSIZE] |= (static_cast<Data>(1) << (idx % WORDSIZE));
}

bool Bitvector::get(std::size_t idx) const {
	return m_val[idx/WORDSIZE] & (static_cast<Data>(1) << (idx % WORDSIZE));
}

std::size_t Bitvector::size() const {
	return m_capacity;
}

void Bitvector::dump()const {
	for(std::size_t i = m_capacity; i-- > 0; ) {
		for(std::size_t j = 0; j < WORDSIZE; j+=4){
			std::cerr << std::bitset<4>(m_val[i]>> (WORDSIZE-j-4)) << " ";
		}
	}
	std::cerr << std::endl;
}

Bitvector& Bitvector::operator+=(const Bitvector& rhs) {
	assert(this->size() == rhs.size());
	Data carry = 0;
	for(std::size_t i = 0; i < m_capacity; ++i) {
		m_val[i] += rhs.m_val[i] + carry;
		carry = m_val[i] < rhs.m_val[i];
	}
	return *this;
}

Bitvector& Bitvector::operator-=(const Bitvector& rhs) {
	assert(this->size() == rhs.size());
	Data borrow = 0;
	for(std::size_t i = 0; i < m_capacity; ++i) {
		Data tmp = m_val[i] < rhs.m_val[i];
		m_val[i] -= rhs.m_val[i] + borrow;
		borrow = tmp;
	}
	return *this;
}

#ifdef ALLOW_SIMD_INSTRUCTION

Bitvector& Bitvector::operator|=(const Bitvector& rhs) {
	assert(this->size() == rhs.size());
	std::size_t align_size = (m_capacity / 4) * 4;
	for(std::size_t i = 0; i < align_size; i += 4) {
		auto l = _mm256_loadu_si256((__m256i *)(m_val.data() +i));
		auto r = _mm256_loadu_si256((__m256i *)(rhs.m_val.data() +i));
		auto ret = _mm256_or_si256(l, r);
		_mm256_storeu_si256((__m256i *)(m_val.data() +i), ret);
	}
	for(std::size_t i = align_size; i < m_capacity; ++i) {
		m_val[i] |= rhs.m_val[i];
	}
	return *this;
}

Bitvector& Bitvector::operator|=(const Data rhs) {
	m_val[0] |= rhs;
	return *this;
}

Bitvector& Bitvector::operator&=(const Bitvector& rhs) {
	assert(this->size() == rhs.size());
	std::size_t align_size = (m_capacity / 4) * 4;
	for(std::size_t i = 0; i < align_size; i += 4) {
		auto l = _mm256_loadu_si256((__m256i *)(m_val.data() +i));
		auto r = _mm256_loadu_si256((__m256i *)(rhs.m_val.data() +i));
		auto ret = _mm256_and_si256(l, r);
		_mm256_storeu_si256((__m256i *)(m_val.data() +i), ret);
	}
	for(std::size_t i = align_size; i < m_capacity; ++i) {
		m_val[i] &= rhs.m_val[i];
	}
	return *this;
}

Bitvector& Bitvector::operator^=(const Bitvector& rhs) {
	assert(this->size() == rhs.size());
	std::size_t align_size = (m_capacity / 4) * 4;
	for(std::size_t i = 0; i < align_size; i += 4) {
		auto l = _mm256_loadu_si256((__m256i *)(m_val.data() +i));
		auto r = _mm256_loadu_si256((__m256i *)(rhs.m_val.data() +i));
		auto ret = _mm256_xor_si256(l, r);
		_mm256_storeu_si256((__m256i *)(m_val.data() +i), ret);
	}
	for(std::size_t i = align_size; i < m_capacity; ++i) {
		m_val[i] ^= rhs.m_val[i];
	}
	return *this;
}

Bitvector Bitvector::operator~() const {
	Bitvector ret(*this);
	std::size_t align_size = (m_capacity / 4) * 4;
	for(std::size_t i = 0; i < align_size; i += 4) {
		auto l = _mm256_loadu_si256((__m256i *)(m_val.data() +i));
		auto r = _mm256_cmpeq_epi8(l, l);
		r = _mm256_xor_si256(l, r);
		_mm256_storeu_si256((__m256i *)(ret.m_val.data() +i), r);
	}
	for(std::size_t i = align_size; i < m_capacity; ++i) {
		ret.m_val[i] = ~m_val[i];
	}
	return *this;
}

Bitvector& Bitvector::operator=(Bitvector&& rhs) & noexcept{
	m_val = std::move(rhs.m_val);
	m_size = rhs.m_size;
	m_capacity = rhs.m_capacity;
	return *this;
}

Bitvector& Bitvector::operator<<=(const std::size_t rhs) {
	Data carry = 0;
	for(std::size_t i = 0; i < m_capacity; ++i) {
		m_val[i] = (m_val[i] << rhs)| carry;
		carry = m_val[i] >> (WORDSIZE - rhs);
	}
	return *this;
}

Bitvector& Bitvector::operator>>=(const std::size_t rhs) {
	Data borrow = 0;
	std::size_t rest = rhs;
	if(rhs >= m_size){
		for(auto &e : m_val) e = 0;
	}
	if(rest >= WORDSIZE) {
		std::size_t shift = rhs / WORDSIZE;
		Data tmp = m_val[m_capacity-1];
		for(std::size_t i = m_capacity-1; i-shift+1 > 0; --i) {
			m_val[i-shift] = tmp;
		}
		for(std::size_t i = 0; i < shift; ++i) {
			m_val[m_capacity-1-i] = 0;
		}
		rest %= WORDSIZE;
		if(rest == 0)return *this;
	}
	for(std::size_t i = m_capacity-1; i+1 > 0; --i) {
		m_val[i] = (m_val[i] >> rest) | borrow;
		borrow = m_val[i] << (WORDSIZE - rest);
	}
	return *this;
}


Bitvector operator+(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) += rhs; }
Bitvector operator-(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) -= rhs; }
Bitvector operator|(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) |= rhs; }
Bitvector operator|(const Bitvector& lhs, const Data rhs) { return Bitvector(lhs) |= rhs; }
Bitvector operator&(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) &= rhs; }
Bitvector operator^(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) ^= rhs; }
Bitvector operator<<(const Bitvector& lhs, const std::size_t rhs) { return Bitvector(lhs) <<= rhs; }
Bitvector operator>>(const Bitvector& lhs, const std::size_t rhs) { return Bitvector(lhs) >>= rhs; }

#else

Bitvector& Bitvector::operator|=(const Bitvector& rhs) {
	assert(this->size() == rhs.size());
	for(std::size_t i = 0; i < m_capacity; ++i) {
		m_val[i] |= rhs.m_val[i];
	}
	return *this;
}

Bitvector& Bitvector::operator|=(const Data rhs) {
	m_val[0] |= rhs;
	return *this;
}

Bitvector& Bitvector::operator&=(const Bitvector& rhs) {
	assert(this->size() == rhs.size());
	for(std::size_t i = 0; i < m_capacity; ++i) {
		m_val[i] &= rhs.m_val[i];
	}
	return *this;
}

Bitvector& Bitvector::operator^=(const Bitvector& rhs) {
	assert(this->size() == rhs.size());
	for(std::size_t i = 0; i < m_capacity; ++i) {
		m_val[i] ^= rhs.m_val[i];
	}
	return *this;
}

Bitvector Bitvector::operator~() const {
	Bitvector ret(*this);
	for(auto &e : ret.m_val) {
		e = ~e;
	}
	return ret;
}

Bitvector& Bitvector::operator=(Bitvector&& rhs) & noexcept{
	m_val = std::move(rhs.m_val);
	m_size = rhs.m_size;
	m_capacity = rhs.m_capacity;
	return *this;
}

Bitvector& Bitvector::operator<<=(const std::size_t rhs) {
	Data carry = 0;
	for(std::size_t i = 0; i < m_capacity; ++i) {
		m_val[i] = (m_val[i] << rhs)| carry;
		carry = m_val[i] >> (WORDSIZE - rhs);
	}
	return *this;
}

Bitvector& Bitvector::operator>>=(const std::size_t rhs) {
	Data borrow = 0;
	std::size_t rest = rhs;
	if(rhs >= m_size){
		for(auto &e : m_val) e = 0;
	}
	if(rest >= WORDSIZE) {
		std::size_t shift = rhs / WORDSIZE;
		Data tmp = m_val[m_capacity-1];
		for(std::size_t i = m_capacity-1; i-shift+1 > 0; --i) {
			m_val[i-shift] = tmp;
		}
		for(std::size_t i = 0; i < shift; ++i) {
			m_val[m_capacity-1-i] = 0;
		}
		rest %= WORDSIZE;
		if(rest == 0)return *this;
	}
	for(std::size_t i = m_capacity-1; i+1 > 0; --i) {
		m_val[i] = (m_val[i] >> rest) | borrow;
		borrow = m_val[i] << (WORDSIZE - rest);
	}
	return *this;
}

Bitvector operator+(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) += rhs; }
Bitvector operator-(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) -= rhs; }
Bitvector operator|(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) |= rhs; }
Bitvector operator|(const Bitvector& lhs, const Data rhs) { return Bitvector(lhs) |= rhs; }
Bitvector operator&(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) &= rhs; }
Bitvector operator^(const Bitvector& lhs, const Bitvector& rhs) { return Bitvector(lhs) ^= rhs; }
Bitvector operator<<(const Bitvector& lhs, const std::size_t rhs) { return Bitvector(lhs) <<= rhs; }
Bitvector operator>>(const Bitvector& lhs, const std::size_t rhs) { return Bitvector(lhs) >>= rhs; }

#endif


