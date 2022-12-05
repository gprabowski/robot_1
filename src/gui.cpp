#include <gui.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

#include <implot/implot.h>

#include <ImGuiFileDialog.h>

#include <chrono>

namespace pusn {
namespace gui {

struct gui_info {
  static std::string file_error_message;
};

std::string gui_info::file_error_message{""};

// utility structure for realtime plot
struct ScrollingBuffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  ScrollingBuffer(int max_size = 2000) {
    MaxSize = max_size;
    Offset = 0;
    Data.reserve(MaxSize);
  }
  void AddPoint(float x, float y) {
    if (Data.size() < MaxSize)
      Data.push_back(ImVec2(x, y));
    else {
      Data[Offset] = ImVec2(x, y);
      Offset = (Offset + 1) % MaxSize;
    }
  }
  void Erase() {
    if (Data.size() > 0) {
      Data.shrink(0);
      Offset = 0;
    }
  }
};

// utility structure for realtime plot
struct RollingBuffer {
  float Span;
  ImVector<ImVec2> Data;
  RollingBuffer() {
    Span = 10.0f;
    Data.reserve(2000);
  }
  void AddPoint(float x, float y) {
    float xmod = fmodf(x, Span);
    if (!Data.empty() && xmod < Data.back().x)
      Data.shrink(0);
    Data.push_back(ImVec2(xmod, y));
  }
};

void ShowDemo_RealtimePlots() {
  static ScrollingBuffer sdata1;
  static RollingBuffer rdata1;
  static float t = 0;
  t += ImGui::GetIO().DeltaTime;
  sdata1.AddPoint(t, chosen_api::last_frame_info::last_frame_time);
  rdata1.AddPoint(t, ImGui::GetIO().Framerate);

  static float history = 10.0f;
  ImGui::SliderFloat("History", &history, 1, 30, "%.1f s");
  rdata1.Span = history;

  static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;

  if (ImPlot::BeginPlot("##Scrolling", ImVec2(-1, 150))) {
    ImPlot::SetupAxes(NULL, NULL, flags, flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, t - history, t, ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 16.f);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::PlotShaded("Frame time in ms", &sdata1.Data[0].x, &sdata1.Data[0].y,
                       sdata1.Data.size(), -INFINITY, 0, sdata1.Offset,
                       2 * sizeof(float));
    ImPlot::EndPlot();
  }
  if (ImPlot::BeginPlot("##Rolling", ImVec2(-1, 150))) {
    ImPlot::SetupAxes(NULL, NULL, flags, flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 100);
    ImPlot::PlotLine("FPS", &rdata1.Data[0].x, &rdata1.Data[0].y,
                     rdata1.Data.size(), 0, 0, 2 * sizeof(float));
    ImPlot::EndPlot();
  }
}

// color theme copied from thecherno/hazel
void set_dark_theme() {
  auto &colors = ImGui::GetStyle().Colors;
  colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

  // Headers
  colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Buttons
  colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Frame BG
  colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
  colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
  colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
  colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
  colors[ImGuiCol_TabUnfocused] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

  // Title
  colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
}

bool init(chosen_api::window_t &w) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  // io.ConfigViewportsNoAutoMerge = true;
  // io.ConfigViewportsNoTaskBarIcon = true;

  ImGui::StyleColorsDark();

  // when viewports are enables we tweak WindowRounding/WIndowBg so platform
  // windows can look identical to regular ones
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(w.get(), true);
  ImGui_ImplOpenGL3_Init("#version 460");

  // fonts
  io.FontDefault = io.Fonts->AddFontFromFileTTF(
      //"fonts/opensans/static/OpenSans/OpenSans-Regular.ttf",
      "resources/fonts/jbmono/fonts/ttf/JetBrainsMono-Regular.ttf", 18.0f);
  set_dark_theme();

  return true;
}

void start_frame() {
  static bool show_demo = false;
  ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);

  if (show_demo) {
    ImGui::ShowDemoWindow(&show_demo);
    ImPlot::ShowDemoWindow();
  }
}

void update_viewport_info(std::function<void(void)> process_input) {
  // update viewport static info
  ImGui::Begin("Movement Finder");

  auto min = ImGui::GetWindowContentRegionMin();
  auto max = ImGui::GetWindowContentRegionMax();

  chosen_api::last_frame_info::viewport_area = {max.x - min.x, max.y - min.y};

  auto tmp = ImGui::GetWindowPos();
  chosen_api::last_frame_info::viewport_pos = {tmp.x, tmp.y};
  chosen_api::last_frame_info::viewport_pos = {tmp.x + min.x, tmp.y + min.y};

  ImVec2 cp = {chosen_api::last_frame_info::viewport_pos.x,
               chosen_api::last_frame_info::viewport_pos.y};

  ImVec2 ca = {chosen_api::last_frame_info::viewport_area.x,
               chosen_api::last_frame_info::viewport_area.y};

  if (ImGui::IsMouseHoveringRect(cp, {cp.x + ca.x, cp.y + ca.y})) {
    process_input();
  }

  ImGui::End();
}

void render_performance_window() {
  ImGui::Begin("Frame Statistics");
  ShowDemo_RealtimePlots();
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::Text("Last CPU frame %.3lf ms",
              glfw_impl::last_frame_info::last_frame_time);
  ImGui::End();
}

void render_settings_gui(internal::simulation_settings &settings) {
  ImGui::Begin("Settings");

  ImGui::SliderFloat("L1", &settings.l1, 0.f, 100.f);
  ImGui::SliderFloat("L2", &settings.l2, 0.f, 100.f);
  ImGui::SliderFloat("Width", &settings.width, 0.f, 100.f);
  ImGui::SliderFloat("First Angle", &settings.first_angle, -180.f, 180.f);
  ImGui::SliderFloat("Second Angle", &settings.second_angle, -180.f, 180.f);
  ImGui::SliderFloat("Field Density", &settings.density, 0.f, 500.f);

  ImGui::End();
}

void render_actions_gui(application_scene &scene) {
  ImGui::Begin("Perform Actions");

  if (ImGui::Button("Add Obstacle")) {
    scene.settings.obstacles.push_back({10.f, 10.f, {0.f, 0.f}});
  }

  ImGui::End();
}

void render_pos_helper(input_state &input, application_scene &scene) {
  ImGui::Begin("Position Helper");

  ImGui::Text(
      "X: %f Y: %f",
      input.mouse.last_pos.x - glfw_impl::last_frame_info::viewport_pos.x -
          glfw_impl::last_frame_info::viewport_area.x / 2.f,
      input.mouse.last_pos.y - glfw_impl::last_frame_info::viewport_pos.y -
          glfw_impl::last_frame_info::viewport_area.y / 2.f);

  auto half_v_x = glfw_impl::last_frame_info::viewport_area.x;
  auto half_v_y = glfw_impl::last_frame_info::viewport_area.y;

  ImGui::Text("Size X: %f Y: %f", half_v_x, half_v_y);

  static glm::vec2 last_reoriented{0.f, 0.f};
  if (input.mouse.reoriented.has_value()) {
    last_reoriented = input.mouse.reoriented.value();
  }
  ImGui::Text("Reoriented X: %f Y: %f", last_reoriented.x, last_reoriented.y);

  ImGui::End();
}
void render_obstacle_gui(application_scene &scene) {
  if (scene.selected_obstacle.has_value()) {
    ImGui::Begin("Obstacle Edit");
    auto &obstacle = scene.selected_obstacle.value().get();
    ImGui::SliderFloat("Width:", &obstacle.width, -100.f, 100.f);
    ImGui::SliderFloat("Height:", &obstacle.height, -100.f, 100.f);
    ImGui::SliderFloat2("Position:", glm::value_ptr(obstacle.pos), -500.f,
                        500.f);
    if (ImGui::Button("Delete Obstacle")) {
      scene.selected_obstacle.reset();
      std::erase_if(scene.settings.obstacles,
                    [&](auto &ob) { return ob.pos == obstacle.pos; });
    }
    ImGui::End();
  }
}

void render(input_state &input, application_scene &scene) {
  render_performance_window();
  render_settings_gui(scene.settings);
  render_actions_gui(scene);
  render_pos_helper(input, scene);
  render_obstacle_gui(scene);
  render_popups();
}

void end_frame() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // update and render additional platform windows
  // (platform functions may change the current opengl context so we
  // save/restore it to make it easier to paste this code elsewhere. For
  // this specific demo appp we could also call
  // glfwMakeCOntextCurrent(window) directly)
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

void render_popups() {
  // Always center this window when appearing
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal("File Corrupted", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("The file you have pointed to is corrupted or wrongly "
                "formatted!\n\n");
    ImGui::Text("%s\n", gui_info::file_error_message.c_str());
    ImGui::Separator();

    if (ImGui::Button("OK", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

} // namespace gui
} // namespace pusn
