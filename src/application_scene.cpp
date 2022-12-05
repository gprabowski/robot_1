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

  // fill the obstacle list
  settings.obstacles.push_back({20.f, 20.f, {100.f, 100.f}});
  settings.obstacles.push_back({20.f, 20.f, {-100.f, -100.f}});
  settings.obstacles.push_back({20.f, 20.f, {-100.f, 100.f}});
  settings.obstacles.push_back({20.f, 20.f, {100.f, -100.f}});

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

  // 3. render obstacles
  for (auto &obstacle : settings.obstacles) {
    glDisable(GL_CULL_FACE);

    const auto obstacle_m = math::get_model_matrix(
        {obstacle.pos.x, 0.f, obstacle.pos.y},
        {obstacle.width, 1.f, obstacle.height}, {0.f, 0.f, 0.f});

    glfw_impl::use_program(centered_model.api_renderable.program.value());

    glfw_impl::set_uniform("color",
                           centered_model.api_renderable.program.value(),
                           glm::vec3{1.f, 1.f, 1.f});
    if (selected_obstacle.has_value()) {
      if (obstacle.pos == selected_obstacle.value().get().pos) {
        glfw_impl::set_uniform("color",
                               centered_model.api_renderable.program.value(),
                               glm::vec3{1.f, 0.f, 0.f});
      }
    }

    glfw_impl::set_uniform(
        "model", centered_model.api_renderable.program.value(), obstacle_m);
    glfw_impl::set_uniform("view",
                           centered_model.api_renderable.program.value(), view);
    glfw_impl::set_uniform("proj",
                           centered_model.api_renderable.program.value(), proj);

    glfw_impl::render(centered_model.api_renderable, centered_model.geometry);
  }

  glfw_impl::use_program(0);
}

bool application_scene::handle_mouse(mouse_state &mouse) {
  if (mouse.just_pressed[mouse_state::mouse_button::left]) {

    const auto view = math::get_view_matrix({0.f, 50.f, 0.f},
                                            math::vec3{0.f, 50.f, 0.f} +
                                                math::vec3{0.f, -1.f, 0.f},
                                            {0.f, 0.f, -1.f});

    auto v_x = glfw_impl::last_frame_info::viewport_area.x;
    auto v_y = glfw_impl::last_frame_info::viewport_area.y;

    const auto proj = math::get_ortho_matrix(
        -settings.density * v_x / 2.f, settings.density * v_x / 2.f,
        -settings.density * v_y / 2.f, settings.density * v_y / 2.f, -100.f,
        100.f);

    for (auto &obstacle : settings.obstacles) {
      const auto obstacle_m = math::get_model_matrix(
          {obstacle.pos.x, 0.f, obstacle.pos.y},
          {obstacle.width, 1.f, obstacle.height}, {0.f, 0.f, 0.f});

      glm::vec4 tmp_obstacle_pos =
          proj * view * obstacle_m * glm::vec4{0.f, 0.f, 0.f, 1.f};

      glm::vec2 obstacle_pos = tmp_obstacle_pos / tmp_obstacle_pos.w;

      obstacle_pos = (obstacle_pos + glm::vec2{1.0f, 1.0f}) / 2.f;
      obstacle_pos.y = 1 - obstacle_pos.y;

      obstacle_pos = {obstacle_pos.x * v_x, obstacle_pos.y * v_y};

      if (glm::length(mouse.last_pos - obstacle_pos) < 50.f) {
        selected_obstacle = std::ref<internal::obstacle_info>(obstacle);
        return true;
      }
    }
    selected_obstacle.reset();
  }

  if (mouse.reoriented.has_value() && selected_obstacle.has_value()) {
    const auto len_x =
        glfw_impl::last_frame_info::viewport_area.x * settings.density / 2;
    const auto len_y =
        glfw_impl::last_frame_info::viewport_area.y * settings.density / 2;

    glm::vec2 tmp = {mouse.reoriented.value().x /
                         glfw_impl::last_frame_info::viewport_area.x,
                     mouse.reoriented.value().y /
                         glfw_impl::last_frame_info::viewport_area.y};

    tmp -= glm::vec2{0.5f, 0.5f};
    tmp *= 2;
    tmp.x *= len_x;
    tmp.y *= len_y;

    selected_obstacle.value().get().pos = tmp;
  }

  return false;
}

} // namespace pusn
