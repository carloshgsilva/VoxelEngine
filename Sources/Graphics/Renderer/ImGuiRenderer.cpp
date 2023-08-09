#include "ImGuiRenderer.h"
#include "Profiler/Profiler.h"
#include "Core/Window.h"

#include <evk/evk.h>
#include <imgui/imnodes.h>

ImGuiRenderer::ImGuiRenderer() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
                                                                //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    SetStyle();

    ImGui_ImplGlfw_InitForVulkan(Window::Get().GetNativeWindow(), true);

    //Send Fonts to Vulkan
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

    ImGui_ImplEvk_Init();
}

ImGuiRenderer::~ImGuiRenderer() {
    ImGui_ImplEvk_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();
}

void ImGuiRenderer::OnGUI(std::function<void()> cb) {
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    cb();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplEvk_PrepareRender(ImGui::GetDrawData());
}

void ImGuiRenderer::Draw() {
    ImGui_ImplEvk_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiRenderer::SetStyle() {
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text] = Accent;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
    colors[ImGuiCol_WindowBg] = PrimaryLight;
    colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
    colors[ImGuiCol_PopupBg] = PrimaryDark;
    colors[ImGuiCol_Border] = Border;
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
    style->IndentSpacing = 10.0f;
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