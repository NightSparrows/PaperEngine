#pragma once

#include <cstdint>

namespace PaperEngine {

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid) : m_uuid(uuid) {}
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_uuid; }

	private:
		uint64_t m_uuid;
	};

}

namespace std {
	template <typename T> struct hash;

	template<>
	struct hash<PaperEngine::UUID>
	{
		size_t operator()(const PaperEngine::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};
}
