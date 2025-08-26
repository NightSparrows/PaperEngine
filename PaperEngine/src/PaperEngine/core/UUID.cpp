#include "UUID.h"

#include <random>

namespace PaperEngine {

	static std::random_device s_rd;
	static std::mt19937_64 s_gen(s_rd());
	static std::uniform_int_distribution<uint64_t> s_dist;
	
	UUID::UUID()
	{
		m_uuid = s_dist(s_gen);
	}

}
