#include <application_scene.hpp>

#include <math.hpp>
#include <mock_data.hpp>

#include <queue>
#include <vector>

namespace pusn {

bool is_inside(internal::obstacle_info &info, float px, float py) {
  return (std::abs(info.pos.x - px) <= (info.width / 2.f)) &&
         (std::abs(info.pos.y - py) <= (info.height / 2.f));
};

bool do_segment_obstacle_intersect(internal::obstacle_info &obstacle, float x0,
                                   float y0, float x1, float y1) {
  const auto osp =
      obstacle.pos - glm::vec2{obstacle.width / 2.f, obstacle.height / 2.f};
  std::array<glm::vec2, 4> rec{
      osp,
      {osp.x + obstacle.width, osp.y},
      {osp.x, osp.y + obstacle.height},
      {osp.x + obstacle.width, osp.y + obstacle.height}};

  return math::check_segment_intersection(x0, y0, x1, y1, rec[0].x, rec[0].y,
                                          rec[1].x, rec[1].y) ||
         math::check_segment_intersection(x0, y0, x1, y1, rec[0].x, rec[0].y,
                                          rec[2].x, rec[2].y) ||
         math::check_segment_intersection(x0, y0, x1, y1, rec[3].x, rec[3].y,
                                          rec[1].x, rec[1].y) ||
         math::check_segment_intersection(x0, y0, x1, y1, rec[3].x, rec[3].y,
                                          rec[2].x, rec[2].y) ||
         is_inside(obstacle, x0, y0) || is_inside(obstacle, x1, y1);
}

bool application_scene::is_config_correct(float alpha, float beta) {
  float x0{0.f}, y0{0.f};
  glm::vec4 tmp_point{settings.l1, 0.f, 0.f, 1.f};
  auto rot_m = glm::toMat4(glm::quat({0.f, glm::radians(alpha), 0.f}));
  tmp_point = rot_m * tmp_point;

  glm::vec4 tmp_point2{settings.l2, 0.f, 0.f, 1.f};
  rot_m = glm::toMat4(glm::quat({0.f, glm::radians(alpha + beta), 0.f}));
  tmp_point2 = rot_m * tmp_point2;
  tmp_point2 += tmp_point;

  float x1 = tmp_point.x, y1 = tmp_point.z;
  float x2 = tmp_point2.x, y2 = tmp_point2.z;

  for (auto &obstacle : settings.obstacles) {
    bool intersect1 = do_segment_obstacle_intersect(obstacle, x0, y0, x1, y1);
    bool intersect2 = do_segment_obstacle_intersect(obstacle, x1, y1, x2, y2);
    if (intersect1 || intersect2) {
      return false;
    }
  }
  return true;
}

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

  if (worker.has_value() && worker_done) {
    worker.value().join();
    worker.reset();
    worker_done = false;
  }

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

  if (imode == input_mode::normal) {
    if (mouse.just_pressed[mouse_state::mouse_button::right]) {

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
  } else {
    if (mouse.just_pressed[mouse_state::mouse_button::right]) {
      const auto len_x =
          glfw_impl::last_frame_info::viewport_area.x * settings.density / 2;
      const auto len_y =
          glfw_impl::last_frame_info::viewport_area.y * settings.density / 2;

      glm::vec2 tmp = {
          mouse.last_pos.x / glfw_impl::last_frame_info::viewport_area.x,
          mouse.last_pos.y / glfw_impl::last_frame_info::viewport_area.y};

      tmp -= glm::vec2{0.5f, 0.5f};
      tmp *= 2;
      tmp.x *= len_x;
      tmp.y *= len_y;

      if (imode == input_mode::start_point) {
        settings.start_point = tmp;
      } else if (imode == input_mode::end_point) {
        settings.end_point = tmp;
      }
      imode = input_mode::normal;
      return true;
    }
  }
  return false;
}

bool application_scene::find_configs() {
  start_configs.clear();
  end_configs.clear();

  // check if start and end points are in reach
  if (glm::length(settings.start_point) > (settings.l1 + settings.l2)) {
    return false;
  }

  if (glm::length(settings.end_point) > (settings.l1 + settings.l2)) {
    return false;
  }

  // set arms to initial position
  auto sp = settings.start_point;
  auto ep = settings.end_point;

  sp.y *= -1;
  ep.y *= -1;

  // first option
  auto find_second = [](float l1, float l2, glm::vec2 p) {
    return glm::degrees(
        std::acos((p.x * p.x + p.y * p.y - l1 * l1 - l2 * l2) / (2 * l1 * l2)));
  };

  auto find_first = [](float l1, float l2, glm::vec2 p, float second) {
    auto res =
        glm::degrees(std::atan((p.y / p.x)) -
                     std::atan((l2 * std::sin(glm::radians(second))) /
                               (l1 + l2 * std::cos(glm::radians(second)))));
    if (p.x < 0) {
      res += 180;
      if (res > 360) {
        res -= 360;
      }
    }

    return res;
  };

  settings.second_angle = find_second(settings.l1, settings.l2, sp);
  settings.first_angle =
      find_first(settings.l1, settings.l2, sp, settings.second_angle);

  float second_angle_2 = 360.f - settings.second_angle;
  float first_angle_2 =
      find_first(settings.l1, settings.l2, sp, second_angle_2);

  if (is_config_correct(settings.first_angle, settings.second_angle)) {
    start_configs.push_back({static_cast<int>(settings.first_angle),
                             static_cast<int>(settings.second_angle)});
  }

  if (is_config_correct(first_angle_2, second_angle_2)) {
    settings.first_angle = first_angle_2;
    settings.second_angle = second_angle_2;
    start_configs.push_back(
        {static_cast<int>(first_angle_2), static_cast<int>(second_angle_2)});
  }

  // FOR END POINT

  auto end_second_angle = find_second(settings.l1, settings.l2, ep);
  auto end_first_angle =
      find_first(settings.l1, settings.l2, ep, end_second_angle);

  float end_second_angle_2 = 360.f - end_second_angle;
  float end_first_angle_2 =
      find_first(settings.l1, settings.l2, ep, end_second_angle_2);

  if (is_config_correct(end_first_angle, end_second_angle)) {
    end_configs.push_back({static_cast<int>(end_first_angle),
                           static_cast<int>(end_second_angle)});
  }

  if (is_config_correct(end_first_angle_2, end_second_angle_2)) {
    end_configs.push_back({static_cast<int>(end_first_angle_2),
                           static_cast<int>(end_second_angle_2)});
  }

  if (start_configs.size() == 0 || end_configs.size() == 0) {
    return false;
  }

  return true;
}

void application_scene::fill_texture() {
  configurations.fill({0.2f, 0.2f, 0.2f});
  auto set = [&](int x, int y, const glm::vec3 &val) {
    configurations[y * 360 + x] = val;
  };

  for (int tx = 0; tx < 360; ++tx) {
    for (int ty = 0; ty < 360; ++ty) {
      if (!is_config_correct(tx, ty)) {
        set(tx, ty, {0.f, 0.f, 0.f});
      }
    }
  }

  glfw_impl::fill_texture<glm::vec3>(configurations_texture, 360u, 360u,
                                     configurations.data());
}

} // namespace pusn
