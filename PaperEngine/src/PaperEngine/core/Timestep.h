#pragma once


namespace PaperEngine {

	class Timestep
	{
	public:

		template<typename Rep, typename Period>
		Timestep(std::chrono::duration<Rep, Period> duration)
			: m_time(std::chrono::duration_cast<std::chrono::nanoseconds>(duration))
		{
		}

	private:
		std::chrono::nanoseconds m_time;
	};

}
