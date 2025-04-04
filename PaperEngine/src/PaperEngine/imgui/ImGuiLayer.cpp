
#include <Platform/Vulkan/VulkanContext.h>
#include <Platform/Vulkan/VulkanUtils.h>

#include <PaperEngine/imgui/ImGuiLayer.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <PaperEngine/core/Application.h>
#include <Platform/Vulkan/VulkanRenderer.h>

namespace PaperEngine {

    class ImGuiLayerImpl {
    public:
        VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
    };

    static void check_vk_result(VkResult err)
    {
        if (err == VK_SUCCESS)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    ImGuiLayer::ImGuiLayer()
    {
		m_impl = new ImGuiLayerImpl();
    }

    ImGuiLayer::~ImGuiLayer()
    {
        delete m_impl;
    }

    void ImGuiLayer::on_attach()
	{
        /// create descriptor pool
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE},
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 0;
        for (VkDescriptorPoolSize& pool_size : pool_sizes)
            pool_info.maxSets += pool_size.descriptorCount;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        CHECK_VK_RESULT(vkCreateDescriptorPool(VulkanContext::GetDevice(), &pool_info, nullptr, &m_impl->descriptorPool));

		/// vulkan setup
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(Application::Get().get_window().get_native_window()), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.ApiVersion = PE_VULKAN_API_VERSION;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
        init_info.Instance = VulkanContext::GetInstance();
        init_info.PhysicalDevice = VulkanContext::GetPhysicalDevice();
        init_info.Device = VulkanContext::GetDevice();
        init_info.QueueFamily = VulkanContext::GetQueueFamily();
        init_info.Queue = VulkanContext::GetQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = m_impl->descriptorPool;
        init_info.RenderPass = VulkanRenderer::Get().get_render_pass();    // TODO: insert my final render pass
        init_info.Subpass = 0;
        init_info.MinImageCount = VulkanContext::GetImageCount();
        init_info.ImageCount = VulkanContext::GetImageCount();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);

        if (!ImGui_ImplVulkan_CreateFontsTexture()) {
			PE_CORE_ERROR("[ImguiLayer] Failed to create font texture");
        }
		/// end vulkan setup
	}

    void ImGuiLayer::on_detach()
    {
		vkDeviceWaitIdle(VulkanContext::GetDevice());
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        vkDestroyDescriptorPool(VulkanContext::GetDevice(), m_impl->descriptorPool, nullptr);
    }

    void ImGuiLayer::begin_frame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }

    void ImGuiLayer::end_frame()
    {
		ImGui::ShowDemoWindow();

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        ImGui_ImplVulkan_RenderDrawData(draw_data, VulkanContext::GetCurrentCmdBuffer());

    }

}
