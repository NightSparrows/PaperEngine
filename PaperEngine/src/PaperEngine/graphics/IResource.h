#pragma once

#include <PaperEngine/core/Base.h>

namespace PaperEngine
{

	class IResource
	{
	public:

		enum class Usage
		{
			/// <summary>
			/// 上傳GPU後就不太會更改
			/// </summary>
			Static,
			/// <summary>
			/// 每個Frame都會streaming更新
			/// 自動map
			/// </summary>
			FrameStreaming,
			/// <summary>
			/// 在GPU裡面計算的
			/// </summary>
			FrameStatic
		};

	public:
			virtual ~IResource() = default;
	};

}
