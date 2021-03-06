#include "ImGuiRenderer.h"
#include "Profiler/Profiler.h"

#include <vulkan/vulkan.hpp>

#define EVK_INTERNAL_STATE 1
#include <evk/evk.h>

struct InternalState {
	vk::DescriptorPool _PerFrameDescriptorPool;
};

ImGuiRenderer::ImGuiRenderer() {
	state = std::make_shared<InternalState>();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
																//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	
	SetStyle();
	//ImGui::StyleColorsDark();


	//Create DescriptorPool
	std::vector<vk::DescriptorPoolSize> poolSizes = {
			vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer, 512 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eInputAttachment, 512 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eStorageImage, 512 },
	};
	state->_PerFrameDescriptorPool = evk::GetState().device.createDescriptorPool(vk::DescriptorPoolCreateInfo()
		.setPoolSizes(poolSizes)
		.setMaxSets(1024)
	);


	ImGui_ImplGlfw_InitForVulkan(Window::Get().GetNativeWindow(), true);
	
	//Init Vulkan Info
	ImGui_ImplVulkan_InitInfo vulkanInfo = {};
	vulkanInfo.Instance = evk::GetState().instance;
	vulkanInfo.PhysicalDevice = evk::GetState().physicalDevice;
	vulkanInfo.Device = evk::GetState().device;
	vulkanInfo.QueueFamily = evk::GetState().queueFamily;
	vulkanInfo.Queue = evk::GetState().queue;
	vulkanInfo.DescriptorPool = state->_PerFrameDescriptorPool;
	vulkanInfo.MinImageCount = 2;
	vulkanInfo.ImageCount = 2;
	vulkanInfo.CheckVkResultFn = check_vk_result;
	vulkanInfo.PipelineCache = VK_NULL_HANDLE;
	vulkanInfo.Allocator = VK_NULL_HANDLE;
	ImGui_ImplVulkan_Init(&vulkanInfo, Passes::Present().state->pass);

	//Send Fonts to Vulkan
	Graphics::Transfer([](CmdBuffer& cmd) {
		//Add Default Font
		ImFontConfig font_config = {};
		font_config.OversampleH = 3;
		font_config.OversampleV = 3;
		font_config.RasterizerMultiply = 1.2f;
		font_config.PixelSnapH = false;
		static const ImWchar font_ranges[] = {0x0020, 0x00FF, 0};
		ImGui::GetIO().Fonts->AddFontFromFileTTF("Assets/Roboto-Medium.ttf", 14, &font_config, font_ranges);

		//Add Font Awesome 5 Font
		ImFontConfig icons_config = {};
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMaxAdvanceX = 12;
		icons_config.GlyphMinAdvanceX = 12;
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImGui::GetIO().Fonts->AddFontFromFileTTF("Assets/Font Awesome 5 Free-Solid-900.otf", 13.0f, &icons_config, icons_ranges);

		ImGui_ImplVulkan_CreateFontsTexture(cmd.state->cmd);

	});
}

ImGuiRenderer::~ImGuiRenderer() {
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	evk::GetState().device.destroyDescriptorPool(state->_PerFrameDescriptorPool);
}

void ImGuiRenderer::DrawToScreen(CmdBuffer& cmd, std::function<void()> Cb) {
	PROFILE_FUNC();

	{
		PROFILE_SCOPE("ImGui_ImplVulkan_NewFrame");
		ImGui_ImplVulkan_NewFrame();
	}
	{
		PROFILE_SCOPE("ImGui_ImplGlfw_NewFrame");
		ImGui_ImplGlfw_NewFrame();
	}
	{
		PROFILE_SCOPE("ImGui::NewFram");
		ImGui::NewFrame();
	}

	{
		PROFILE_SCOPE("ImGui::DockSpaceOverViewport");
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
	}

	Cb();

	ImGui::Render();

	cmd.present([&] {
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.state->cmd);
	});

	ImGui::EndFrame();
}

void ImGuiRenderer::SetStyle() {
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text] = Accent;
	colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
	colors[ImGuiCol_WindowBg] = PrimaryLight;
	colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
	colors[ImGuiCol_PopupBg] = PrimaryDark;
	colors[ImGuiCol_Border] = ImVec4(0.266f, 0.266f, 0.266f, 0.300f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
	colors[ImGuiCol_FrameBg] = PrimaryDark;
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.080f, 0.080f, 0.080f, 1.000f);
	colors[ImGuiCol_TitleBgActive] = PrimaryDark;
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.080f, 0.080f, 0.080f, 1.000f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.195f, 0.195f, 0.195f, 1.000f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
	colors[ImGuiCol_ScrollbarGrabActive] = Accent;
	colors[ImGuiCol_CheckMark] = Accent;
	colors[ImGuiCol_SliderGrab] = Accent;
	colors[ImGuiCol_SliderGrabActive] = Accent;
	colors[ImGuiCol_Button] = AccentDark;
	colors[ImGuiCol_ButtonHovered] = Accent;
	colors[ImGuiCol_ButtonActive] = AccentDark;
	colors[ImGuiCol_Header] = ImVec4(0,0,0,0);
	colors[ImGuiCol_HeaderHovered] = ImVec4(1, 1, 1, 0.25f);
	colors[ImGuiCol_HeaderActive] = ImVec4(1, 1, 1, 0.5f);
	colors[ImGuiCol_Separator] = PrimaryDark;
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	colors[ImGuiCol_SeparatorActive] = Accent;
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
	colors[ImGuiCol_ResizeGripActive] = Accent;
	colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.08f, 0.00f);
	colors[ImGuiCol_TabHovered] = PrimaryLight;
	colors[ImGuiCol_TabActive] = PrimaryLight;
	colors[ImGuiCol_TabUnfocused] = PrimaryDark;
	colors[ImGuiCol_TabUnfocusedActive] = PrimaryLight;
	colors[ImGuiCol_DockingPreview] = Accent;
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.180f, 0.180f, 0.180f, 1.000f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	colors[ImGuiCol_PlotLinesHovered] = Accent;
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
	colors[ImGuiCol_PlotHistogramHovered] = Accent;
	colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	colors[ImGuiCol_DragDropTarget] = Accent;
	colors[ImGuiCol_NavHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.000f, 0.000f, 0.000f, 1.000f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

	style->ChildRounding = 0.0f;
	style->IndentSpacing = 13.0f;
	style->FrameBorderSize = 0.0f;
	style->FrameRounding = 2.0f;
	style->GrabMinSize = 7.0f;
	style->PopupRounding = 2.0f;
	style->ScrollbarRounding = 12.0f;
	style->ScrollbarSize = 13.0f;
	style->TabBorderSize = 0.0f;
	style->TabRounding = 4.0f;
	style->WindowRounding = 4.0f;

	style->FramePadding = ImVec2(8, 4);

	style->WindowMenuButtonPosition = ImGuiDir_None;
}