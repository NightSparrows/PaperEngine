
#include <vulkan/vulkan.h>

#include <PaperEngine/core/Layer.h>

namespace PaperEngine {

	/// <summary>
	/// basic setup imgui for you
	/// do not export to user since this is 
	/// a native application layer
	/// </summary>
	class ImGuiLayer : public Layer {
	public:

		void on_attach() override;
		void on_detach() override;

		// need to be inside the command render pass
		void begin_frame();
		void end_frame();

	protected:
		VkDescriptorPool m_descriptorPool;
		bool m_swapchainRebuild{ false };
	};

}
