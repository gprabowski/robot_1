#pragma once

#include <chrono>
#include <vector>

#include <glad/glad.h>

#include <geometry.hpp>
#include <glfw_impl.hpp>
#include <mock_data.hpp>

#include <atomic>

namespace pusn {

namespace internal {
// each of them needs to store:
//    * geometry
//    * API object reference

struct obstacle_info {
  float width{2.f}, height{1.f};
  math::vec2 pos{0.f, 0.f};
};

struct simulation_settings {
  float l1{30.f}, l2{20.f}, width{5.f}, first_angle{0.f}, second_angle{0.f},
      density{0.25f};
  std::vector<obstacle_info> obstacles;

  math::vec2 start_point{0.f, 0.f};
  math::vec2 end_point{0.f, 0.f};
};

struct square_center {
  api_agnostic_geometry geometry{{{math::vec3(-0.5, 0.0, -0.5), {}, {}},
                                  {math::vec3(0.5, 0.0, -0.5), {}, {}},
                                  {math::vec3(0.5, 0.0, 0.5), {}, {}},
                                  {math::vec3(-0.5, 0.0, 0.5), {}, {}}},
                                 {0, 1, 2, 2, 3, 0}};
  glfw_impl::renderable api_renderable;
};

struct square_right {
  api_agnostic_geometry geometry{{{math::vec3(0.0, 0.0, -0.5), {}, {}},
                                  {math::vec3(1.0, 0.0, -0.5), {}, {}},
                                  {math::vec3(1.0, 0.0, 0.5), {}, {}},
                                  {math::vec3(0.0, 0.0, 0.5), {}, {}}},
                                 {0, 1, 2, 2, 3, 0}};
  glfw_impl::renderable api_renderable;
};

} // namespace internal

enum class input_mode { normal, start_point, end_point };

struct application_scene {
  input_mode imode{input_mode::normal};

  internal::simulation_settings settings;
  internal::square_center centered_model;
  internal::square_right right_model;

  std::array<unsigned char, 360 * 360> configurations;

  std::optional<std::reference_wrapper<internal::obstacle_info>>
      selected_obstacle;

  bool init();
  void run();

  bool check_settings();
  void render(input_state &input);
  bool handle_mouse(mouse_state &mouse);

  std::optional<std::thread> worker;
};

} // namespace pusn
