#pragma once


namespace PaperEngine
{
	enum class ResourceUsage
	{
		/// <summary>
		/// 只在GPU裡使用
		/// </summary>
		Static,
		/// <summary>
		/// 每禎將CPU資料傳至GPU中
		/// 將強制cpu access mode to write
		/// </summary>
		FrameStreaming,
		/// <summary>
		/// 只在GPU裡使用，但每禎計算的都不一樣
		/// bindingSet沒有作用
		/// </summary>
		FrameStatic
	};
}

