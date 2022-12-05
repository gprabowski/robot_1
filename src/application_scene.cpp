#include <application_scene.hpp>

#include <vector>

#include <math.hpp>

#include <mock_data.hpp>

namespace pusn {

void generate_milling_tool(api_agnostic_geometry &out) {}

bool application_scene::init() {
  // model for obstacles
  glfw_impl::fill_renderable(centered_model.geometry.vertices,
                             centered_model.geometry.indices,
                             centered_model.api_renderable);
  glfw_impl::add_program_to_renderable("resources/default",
                                       centered_model.api_renderable);

  // model for robot
  glfw_impl::fill_renderable(right_model.geometry.vertices,
                             right_model.geometry.indices,
                             right_model.api_renderable);
  glfw_impl::add_program_to_renderable("resources/default",
                                       right_model.api_renderable);

  return true;
}

void application_scene::render(input_state &input) {

  // 1. get camera info
  glDepthFunc(GL_LESS);

  const auto view = math::get_view_matrix(
      {0.f, 50.f, 0.f}, math::vec3{0.f, 50.f, 0.f} + math::vec3{0.f, -1.f, 0.f},
      {0.f, 0.f, -1.f});

  auto half_v_x = glfw_impl::last_frame_info::viewport_area.x / 2.f;
  auto half_v_y = glfw_impl::last_frame_info::viewport_area.y / 2.f;

  const auto proj = math::get_ortho_matrix(
      -settings.density * half_v_x, settings.density * half_v_x,
      -settings.density * half_v_y, settings.density * half_v_y, -100.f, 100.f);

  // 2. render robot arms
  // 2.1 first arm
  glDisable(GL_CULL_FACE);
  const auto model_first_arm_m = math::get_model_matrix(
      {}, {settings.l1, 1.f, settings.width},
      glm::radians(glm::vec3{0.f, settings.first_angle, 0.f}));

  glfw_impl::use_program(right_model.api_renderable.program.value());

  glfw_impl::set_uniform("model", right_model.api_renderable.program.value(),
                         model_first_arm_m);
  glfw_impl::set_uniform("view", right_model.api_renderable.program.value(),
                         view);
  glfw_impl::set_uniform("proj", right_model.api_renderable.program.value(),
                         proj);
  glfw_impl::render(right_model.api_renderable, right_model.geometry);
  // 2.2 second arm
  // find its starting position
  glm::vec4 tmp_point{settings.l1, 0.f, 0.f, 1.f};
  const auto rot_m =
      glm::toMat4(glm::quat({0.f, glm::radians(settings.first_angle), 0.f}));
  tmp_point = rot_m * tmp_point;

  const auto model_second_arm_m = math::get_model_matrix(
      {tmp_point.x, tmp_point.y, tmp_point.z},
      {settings.l2, 1.f, settings.width},
      glm::radians(
          glm::vec3{0.f, settings.first_angle + settings.second_angle, 0.f}));

  glfw_impl::use_program(right_model.api_renderable.program.value());

  glfw_impl::set_uniform("model", right_model.api_renderable.program.value(),
                         model_second_arm_m);
  glfw_impl::set_uniform("view", right_model.api_renderable.program.value(),
                         view);
  glfw_impl::set_uniform("proj", right_model.api_renderable.program.value(),
                         proj);
  glfw_impl::render(right_model.api_renderable, right_model.geometry);
  glEnable(GL_CULL_FACE);

  glfw_impl::use_program(0);
}
} // namespace pusn
