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

struct simulation_settings {};

struct scene_grid {
  api_agnostic_geometry geometry{{{math::vec3(-1.0, 0.0, -1.0), {}, {}},
                                  {math::vec3(1.0, 0.0, -1.0), {}, {}},
                                  {math::vec3(1.0, 0.0, 1.0), {}, {}},
                                  {math::vec3(-1.0, 0.0, 1.0), {}, {}}},
                                 {0, 1, 2, 2, 3, 0}};
  scene_object_info placement;
  glfw_impl::renderable api_renderable;
};

struct model {
  float height{70.f};
  float radius{5.f};

  simulation_settings settings;

  scene_object_info placement;
  api_agnostic_geometry geometry;
  glfw_impl::renderable api_renderable;

  inline void reset() {
    geometry.vertices.clear();
    geometry.indices.clear();
    mock_data::buildVerticesSmooth(100, height, radius, geometry.vertices,
                                   geometry.indices);
    glfw_impl::fill_renderable(geometry.vertices, geometry.indices,
                               api_renderable);
  }
};

} // namespace internal

struct application_scene {
  internal::simulation_settings settings;
  internal::model model;
  internal::scene_grid grid;

  bool init();
  void render(input_state &input);
};

} // namespace pusn
