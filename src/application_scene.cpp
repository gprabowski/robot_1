#include <application_scene.hpp>

#include <vector>

#include <math.hpp>

#include <mock_data.hpp>

namespace pusn {

void generate_milling_tool(api_agnostic_geometry &out) {}

bool application_scene::init() {
  // Generate and add milling tool
  mock_data::buildVerticesSmooth(100, model.height, model.radius,
                                 model.geometry.vertices,
                                 model.geometry.indices);
  glfw_impl::fill_renderable(model.geometry.vertices, model.geometry.indices,
                             model.api_renderable);
  glfw_impl::add_program_to_renderable("resources/model", model.api_renderable);

  // ADD GRID
  glfw_impl::fill_renderable(grid.geometry.vertices, grid.geometry.indices,
                             grid.api_renderable);
  glfw_impl::add_program_to_renderable("resources/grid", grid.api_renderable);

  return true;
}

void application_scene::render(input_state &input) {

  // 1. get camera info
  glDepthFunc(GL_LESS);

  const auto view = math::get_view_matrix(
      input.camera.pos, input.camera.pos + input.camera.front, input.camera.up);

  const auto proj = math::get_projection_matrix(
      glm::radians(input.render_info.fov_y),
      glfw_impl::last_frame_info::viewport_area.x,
      glfw_impl::last_frame_info::viewport_area.y, input.render_info.clip_near,
      input.render_info.clip_far);

  // 2. render grid
  glDisable(GL_CULL_FACE);
  const auto model_grid_m =
      math::get_model_matrix(grid.placement.position, grid.placement.scale,
                             math::deg_to_rad(grid.placement.rotation));
  glfw_impl::use_program(grid.api_renderable.program.value());

  glfw_impl::set_uniform("model", grid.api_renderable.program.value(),
                         model_grid_m);
  glfw_impl::set_uniform("view", grid.api_renderable.program.value(), view);
  glfw_impl::set_uniform("proj", grid.api_renderable.program.value(), proj);
  glfw_impl::render(grid.api_renderable, grid.geometry);
  glEnable(GL_CULL_FACE);

  auto &placement = model.placement;
  const auto model_model_m =
      math::get_model_matrix(placement.position, placement.scale,
                             math::deg_to_rad(placement.rotation));
  glfw_impl::use_program(model.api_renderable.program.value());
  glfw_impl::set_uniform("model", model.api_renderable.program.value(),
                         model_model_m);
  glfw_impl::set_uniform("view", model.api_renderable.program.value(), view);
  glfw_impl::set_uniform("proj", model.api_renderable.program.value(), proj);
  glfw_impl::render(model.api_renderable, model.geometry);

  glfw_impl::use_program(0);
}
} // namespace pusn
