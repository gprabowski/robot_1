#include <application.hpp>
#include <logger.hpp>

#include <iostream>

#include <gui.hpp>

namespace pusn {

bool application::init(const std::string &window_title) {
  bool final_result{true};
  final_result &= logger::init();
  window = chosen_api::initialize(window_title, &input);
  final_result &= scene.init();
  final_result &= gui::init(window);
  viewport.setup();
  return final_result;
}

void application::render_gui() { gui::render(input, scene); }

void application::render_viewport() {
  static const glm::vec4 clear_color = {38.f / 255.f, 38.f / 255.f,
                                        38.f / 255.f, 1.00f};

  ImGui::Begin("Movement Finder");
  viewport.bind();
  viewport.left = true;
  auto s = ImGui::GetContentRegionAvail();
  if (viewport.width != s.x || viewport.height != s.y) {
    viewport.width = s.x;
    viewport.height = s.y;
    viewport.setup();
  }
  viewport.set_left();
  glViewport(0, 0, chosen_api::last_frame_info::viewport_area.x,
             chosen_api::last_frame_info::viewport_area.y);
  chosen_api::clear_color_and_depth(clear_color, 1.f);
  scene.render(input);
  GLuint t = viewport.color_left.value();
  ImGui::Image((void *)(uint64_t)t, s, {0, 1}, {1, 0});
  viewport.unbind();
  ImGui::End();

  glViewport(0, 0, chosen_api::last_frame_info::width,
             chosen_api::last_frame_info::height);
}

bool application::main_loop() {
  while (!chosen_api::should_close(window)) {
    chosen_api::before_frame();
    gui::start_frame();
    gui::update_viewport_info([&]() { input.process_new_input(); });
    render_viewport();
    render_gui();
    gui::end_frame();
    chosen_api::after_frame(window);
  }
  return true;
}

} // namespace pusn
