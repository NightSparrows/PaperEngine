
#pragma warning(push)
#pragma warning(disable: 4251)

#include "Application.h"

#include <PaperEngine/core/Logger.h>
#include <PaperEngine/events/ApplicationEvent.h>
#include <PaperEngine/debug/Instrumentor.h>
#include <PaperEngine/utils/Clock.h>

namespace PaperEngine {

	Application* Application::s_instance = nullptr;

	Application::Application(const ApplicationSpecification& spec)
	{
		PE_CORE_ASSERT(!s_instance, "Application already created.");
		s_instance = this;

		m_window = Window::Create(WindowProps(spec.name, spec.width, spec.height));
		m_window->init();
		m_window->set_event_callback(PE_BIND_EVENT_FN(Application::on_event));

	}

	Application::~Application()
	{
		//delete m_imguiLayer;
	}

	void Application::run()
	{
		PE_PROFILE_BEGIN_SESSION("Runtime", "PaperEngineProfile-Runtime.json");
		//m_imguiLayer = new ImGuiLayer();
		//push_overlay(m_imguiLayer);

		m_imguiLayer = ImGuiLayer::Create();
		m_imguiLayer->on_attach();

		m_running = true;
		Clock clock;
		while (m_running) {

			auto delta_time = clock.reset();

			{
				PE_PROFILE_SCOPE("Render");

				// update

				if (m_window->get_context().beginFrame()) {
					{
						PE_PROFILE_SCOPE("Layer Update");
						for (auto& layer : m_layerManager) {
							layer->on_update(delta_time);
						}
					}
					// render logic
					m_imguiLayer->on_update(delta_time);
					m_imguiLayer->begin_frame();
					for (auto& layer : m_layerManager) {
						layer->on_imgui_render();
					}
					m_window->on_update();
					m_imguiLayer->end_frame();

					PaperEngine::CommandBufferHandle cmd = PaperEngine::CommandBuffer::Create({ .isPrimary = true });

					PaperEngine::TextureHandle swapchainTexture =
						PaperEngine::Application::Get().get_window().get_context().get_swapchain_texture(PaperEngine::Application::Get().get_window().get_context().get_current_swapchain_index());
					cmd->open();
					// prepare swapchain to ready to 
					cmd->setTextureState(swapchainTexture, PaperEngine::TextureState::Present);
					cmd->close();

					PaperEngine::Application::Get().get_window().get_context().executeCommandBuffer(cmd);

					m_window->get_context().endFrame();
				}

			}


		}

		m_layerManager.cleanUp();

		m_imguiLayer->on_detach();

		m_window->cleanUp();
		PE_PROFILE_END_SESSION();
	}

	PE_API void Application::push_layer(Layer* layer)
	{
		m_layerManager.push_layer(layer);
		layer->on_attach();
	}

	PE_API void Application::push_overlay(Layer* layer)
	{
		m_layerManager.push_overlay(layer);
		layer->on_attach();
	}

	void Application::on_event(Event& e)
	{
		EventDispatcher dispatcher(e);
#ifdef PE_DEBUG
		dispatcher.dispatch<WindowCloseEvent>([this](Event& e) {
			m_running = false;
			return true;
			});
#endif // PE_DEBUG
		dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {return this->on_window_resize(e); });

		// always process imgui layer first
		m_imguiLayer->on_event(e);
		for (auto it = m_layerManager.rbegin(); it != m_layerManager.rend(); ++it) {
			if (e.Handled)
				break;
			(*it)->on_event(e);
		}
	}

	bool Application::on_window_resize(WindowResizeEvent& e)
	{
		return false;
	}

	void Application::Shutdown() {
		if (!s_instance)
			return;
		s_instance->m_running = false;
	}
	// Explicitly export unique_ptr specialization
	template class PE_API std::unique_ptr<Window>;
}


#pragma warning(pop)