///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// to_string for debug messages
// 
// (c) Lefuneste.
//
// All rights reserved.
// https://github.com/xxx
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
//  * Redistributions of source code must retain the above copyright notice, this
//	  list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and / or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This software is using part of code or algorithms provided by
// * Crosire https://github.com/crosire/reshade  
// * FransBouma https://github.com/FransBouma/ShaderToggler
// * ShortFuse https://github.com/clshortfuse/renodx
// 
/////////////////////////////////////////////////////////////////////////

/*
 * Copyright (C) 2021 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#pragma once

#include <reshade.hpp>
#include <cassert>
#include <sstream>
#include <shared_mutex>
#include <unordered_set>
#include <string.h>

#include "shader_definitions.h"

using namespace reshade::api;

namespace
{
	/*
	std::shared_mutex s_mutex;
	std::unordered_set<uint64_t> s_samplers;
	std::unordered_set<uint64_t> s_resources;
	std::unordered_set<uint64_t> s_resource_views;
	std::unordered_set<uint64_t> s_pipelines;
	*/

	inline auto to_string(shader_stage value)
	{
		switch (value)
		{
		case shader_stage::vertex:
			return "vertex";
		case shader_stage::hull:
			return "hull";
		case shader_stage::domain:
			return "domain";
		case shader_stage::geometry:
			return "geometry";
		case shader_stage::pixel:
			return "pixel";
		case shader_stage::compute:
			return "compute";
		case shader_stage::amplification:
			return "amplification";
		case shader_stage::mesh:
			return "mesh";
		case shader_stage::raygen:
			return "raygen";
		case shader_stage::any_hit:
			return "any_hit";
		case shader_stage::closest_hit:
			return "closest_hit";
		case shader_stage::miss:
			return "miss";
		case shader_stage::intersection:
			return "intersection";
		case shader_stage::callable:
			return "callable";
		case shader_stage::all:
			return "all";
		case shader_stage::all_graphics:
			return "all_graphics";
		case shader_stage::all_ray_tracing:
			return "all_raytracing";
		default:
			return "unknown";
		}
	}
	inline auto to_string(pipeline_stage value)
	{
		switch (value)
		{
		case pipeline_stage::vertex_shader:
			return "vertex_shader";
		case pipeline_stage::hull_shader:
			return "hull_shader";
		case pipeline_stage::domain_shader:
			return "domain_shader";
		case pipeline_stage::geometry_shader:
			return "geometry_shader";
		case pipeline_stage::pixel_shader:
			return "pixel_shader";
		case pipeline_stage::compute_shader:
			return "compute_shader";
		case pipeline_stage::amplification_shader:
			return "amplification_shader";
		case pipeline_stage::mesh_shader:
			return "mesh_shader";
		case pipeline_stage::input_assembler:
			return "input_assembler";
		case pipeline_stage::stream_output:
			return "stream_output";
		case pipeline_stage::rasterizer:
			return "rasterizer";
		case pipeline_stage::depth_stencil:
			return "depth_stencil";
		case pipeline_stage::output_merger:
			return "output_merger";
		case pipeline_stage::all:
			return "all";
		case pipeline_stage::all_graphics:
			return "all_graphics";
		case pipeline_stage::all_ray_tracing:
			return "all_ray_tracing";
		case pipeline_stage::all_shader_stages:
			return "all_shader_stages";
		default:
			return "unknown";
		}
	}
	inline auto to_string(descriptor_type value)
	{
		switch (value)
		{
		case descriptor_type::sampler:
			return "sampler";
		case descriptor_type::sampler_with_resource_view:
			return "sampler_with_resource_view";
		case descriptor_type::shader_resource_view:
			return "shader_resource_view";
		case descriptor_type::unordered_access_view:
			return "unordered_access_view";
		case descriptor_type::constant_buffer:
			return "constant_buffer";
		case descriptor_type::acceleration_structure:
			return "acceleration_structure";
		default:
			return "unknown";
		}
	}
	inline auto to_string(dynamic_state value)
	{
		switch (value)
		{
		default:
		case dynamic_state::unknown:
			return "unknown";
		case dynamic_state::alpha_test_enable:
			return "alpha_test_enable";
		case dynamic_state::alpha_reference_value:
			return "alpha_reference_value";
		case dynamic_state::alpha_func:
			return "alpha_func";
		case dynamic_state::srgb_write_enable:
			return "srgb_write_enable";
		case dynamic_state::primitive_topology:
			return "primitive_topology";
		case dynamic_state::sample_mask:
			return "sample_mask";
		case dynamic_state::alpha_to_coverage_enable:
			return "alpha_to_coverage_enable";
		case dynamic_state::blend_enable:
			return "blend_enable";
		case dynamic_state::logic_op_enable:
			return "logic_op_enable";
		case dynamic_state::color_blend_op:
			return "color_blend_op";
		case dynamic_state::source_color_blend_factor:
			return "src_color_blend_factor";
		case dynamic_state::dest_color_blend_factor:
			return "dst_color_blend_factor";
		case dynamic_state::alpha_blend_op:
			return "alpha_blend_op";
		case dynamic_state::source_alpha_blend_factor:
			return "src_alpha_blend_factor";
		case dynamic_state::dest_alpha_blend_factor:
			return "dst_alpha_blend_factor";
		case dynamic_state::logic_op:
			return "logic_op";
		case dynamic_state::blend_constant:
			return "blend_constant";
		case dynamic_state::render_target_write_mask:
			return "render_target_write_mask";
		case dynamic_state::fill_mode:
			return "fill_mode";
		case dynamic_state::cull_mode:
			return "cull_mode";
		case dynamic_state::front_counter_clockwise:
			return "front_counter_clockwise";
		case dynamic_state::depth_bias:
			return "depth_bias";
		case dynamic_state::depth_bias_clamp:
			return "depth_bias_clamp";
		case dynamic_state::depth_bias_slope_scaled:
			return "depth_bias_slope_scaled";
		case dynamic_state::depth_clip_enable:
			return "depth_clip_enable";
		case dynamic_state::scissor_enable:
			return "scissor_enable";
		case dynamic_state::multisample_enable:
			return "multisample_enable";
		case dynamic_state::antialiased_line_enable:
			return "antialiased_line_enable";
		case dynamic_state::depth_enable:
			return "depth_enable";
		case dynamic_state::depth_write_mask:
			return "depth_write_mask";
		case dynamic_state::depth_func:
			return "depth_func";
		case dynamic_state::stencil_enable:
			return "stencil_enable";
		case dynamic_state::front_stencil_read_mask:
			return "front_stencil_read_mask";
		case dynamic_state::front_stencil_write_mask:
			return "front_stencil_write_mask";
		case dynamic_state::front_stencil_reference_value:
			return "front_stencil_reference_value";
		case dynamic_state::front_stencil_func:
			return "front_stencil_func";
		case dynamic_state::front_stencil_pass_op:
			return "front_stencil_pass_op";
		case dynamic_state::front_stencil_fail_op:
			return "front_stencil_fail_op";
		case dynamic_state::front_stencil_depth_fail_op:
			return "front_stencil_depth_fail_op";
		case dynamic_state::back_stencil_read_mask:
			return "back_stencil_read_mask";
		case dynamic_state::back_stencil_write_mask:
			return "back_stencil_write_mask";
		case dynamic_state::back_stencil_reference_value:
			return "back_stencil_reference_value";
		case dynamic_state::back_stencil_func:
			return "back_stencil_func";
		case dynamic_state::back_stencil_pass_op:
			return "back_stencil_pass_op";
		case dynamic_state::back_stencil_fail_op:
			return "back_stencil_fail_op";
		case dynamic_state::back_stencil_depth_fail_op:
			return "back_stencil_depth_fail_op";
		}
	}
	inline auto to_string(resource_usage value)
	{
		switch (value)
		{
		default:
		case resource_usage::undefined:
			return "undefined";
		case resource_usage::index_buffer:
			return "index_buffer";
		case resource_usage::vertex_buffer:
			return "vertex_buffer";
		case resource_usage::constant_buffer:
			return "constant_buffer";
		case resource_usage::stream_output:
			return "stream_output";
		case resource_usage::indirect_argument:
			return "indirect_argument";
		case resource_usage::depth_stencil:
		case resource_usage::depth_stencil_read:
		case resource_usage::depth_stencil_write:
			return "depth_stencil";
		case resource_usage::render_target:
			return "render_target";
		case resource_usage::shader_resource:
		case resource_usage::shader_resource_pixel:
		case resource_usage::shader_resource_non_pixel:
			return "shader_resource";
		case resource_usage::unordered_access:
			return "unordered_access";
		case resource_usage::copy_dest:
			return "copy_dest";
		case resource_usage::copy_source:
			return "copy_source";
		case resource_usage::resolve_dest:
			return "resolve_dest";
		case resource_usage::resolve_source:
			return "resolve_source";
		case resource_usage::acceleration_structure:
			return "acceleration_structure";
		case resource_usage::general:
			return "general";
		case resource_usage::present:
			return "present";
		case resource_usage::cpu_access:
			return "cpu_access";
		}
	}
	inline auto to_string(query_type value)
	{
		switch (value)
		{
		case query_type::occlusion:
			return "occlusion";
		case query_type::binary_occlusion:
			return "binary_occlusion";
		case query_type::timestamp:
			return "timestamp";
		case query_type::pipeline_statistics:
			return "pipeline_statistics";
		case query_type::stream_output_statistics_0:
			return "stream_output_statistics_0";
		case query_type::stream_output_statistics_1:
			return "stream_output_statistics_1";
		case query_type::stream_output_statistics_2:
			return "stream_output_statistics_2";
		case query_type::stream_output_statistics_3:
			return "stream_output_statistics_3";
		default:
			return "unknown";
		}
	}
	inline auto to_string(acceleration_structure_type value)
	{
		switch (value)
		{
		case acceleration_structure_type::top_level:
			return "top_level";
		case acceleration_structure_type::bottom_level:
			return "bottom_level";
		default:
		case acceleration_structure_type::generic:
			return "generic";
		}
	}
	inline auto to_string(acceleration_structure_copy_mode value)
	{
		switch (value)
		{
		case acceleration_structure_copy_mode::clone:
			return "clone";
		case acceleration_structure_copy_mode::compact:
			return "compact";
		case acceleration_structure_copy_mode::serialize:
			return "serialize";
		case acceleration_structure_copy_mode::deserialize:
			return "deserialize";
		default:
			return "unknown";
		}
	}
	inline auto to_string(acceleration_structure_build_mode value)
	{
		switch (value)
		{
		case acceleration_structure_build_mode::build:
			return "build";
		case acceleration_structure_build_mode::update:
			return "update";
		default:
			return "unknown";
		}
	}
}

inline auto to_string(reshade::api::pipeline_layout_param_type value) {
	switch (value) {
	case reshade::api::pipeline_layout_param_type::push_constants:               return "push_constants";
	case reshade::api::pipeline_layout_param_type::descriptor_table:             return "descriptor_table";
	case reshade::api::pipeline_layout_param_type::push_descriptors:             return "push_descriptors";
	case reshade::api::pipeline_layout_param_type::push_descriptors_with_ranges: return "push_descriptors_with_ranges";
	default:                                                                     return "unknown";
	}
}

inline auto to_string(reshade::api::resource_type value) {
	switch (value) {
	case reshade::api::resource_type::buffer:     return "buffer";
	case reshade::api::resource_type::texture_1d: return "texture_1d";
	case reshade::api::resource_type::texture_2d: return "texture_2d";
	case reshade::api::resource_type::texture_3d: return "texture_3d";
	case reshade::api::resource_type::surface:    return "surface";
	default:
	case reshade::api::resource_type::unknown:    return "unknown";
	}
}

inline auto to_string(reshade::api::format value) {
	switch (value) {
	case reshade::api::format::r1_unorm:              return "r1_unorm";
	case reshade::api::format::l8_unorm:              return "l8_unorm";
	case reshade::api::format::a8_unorm:              return "a8_unorm";
	case reshade::api::format::r8_typeless:           return "r8_typeless";
	case reshade::api::format::r8_uint:               return "r8_uint";
	case reshade::api::format::r8_sint:               return "r8_sint";
	case reshade::api::format::r8_unorm:              return "r8_unorm";
	case reshade::api::format::r8_snorm:              return "r8_snorm";
	case reshade::api::format::l8a8_unorm:            return "l8a8_unorm";
	case reshade::api::format::r8g8_typeless:         return "r8g8_typeless";
	case reshade::api::format::r8g8_uint:             return "r8g8_uint";
	case reshade::api::format::r8g8_sint:             return "r8g8_sint";
	case reshade::api::format::r8g8_unorm:            return "r8g8_unorm";
	case reshade::api::format::r8g8_snorm:            return "r8g8_snorm";
	case reshade::api::format::r8g8b8a8_typeless:     return "r8g8b8a8_typeless";
	case reshade::api::format::r8g8b8a8_uint:         return "r8g8b8a8_uint";
	case reshade::api::format::r8g8b8a8_sint:         return "r8g8b8a8_sint";
	case reshade::api::format::r8g8b8a8_unorm:        return "r8g8b8a8_unorm";
	case reshade::api::format::r8g8b8a8_unorm_srgb:   return "r8g8b8a8_unorm_srgb";
	case reshade::api::format::r8g8b8a8_snorm:        return "r8g8b8a8_snorm";
	case reshade::api::format::r8g8b8x8_unorm:        return "r8g8b8x8_unorm";
	case reshade::api::format::r8g8b8x8_unorm_srgb:   return "r8g8b8x8_unorm_srgb";
	case reshade::api::format::b8g8r8a8_typeless:     return "b8g8r8a8_typeless";
	case reshade::api::format::b8g8r8a8_unorm:        return "b8g8r8a8_unorm";
	case reshade::api::format::b8g8r8a8_unorm_srgb:   return "b8g8r8a8_unorm_srgb";
	case reshade::api::format::b8g8r8x8_typeless:     return "b8g8r8x8_typeless";
	case reshade::api::format::b8g8r8x8_unorm:        return "b8g8r8x8_unorm";
	case reshade::api::format::b8g8r8x8_unorm_srgb:   return "b8g8r8x8_unorm_srgb";
	case reshade::api::format::r10g10b10a2_typeless:  return "r10g10b10a2_typeless";
	case reshade::api::format::r10g10b10a2_uint:      return "r10g10b10a2_uint";
	case reshade::api::format::r10g10b10a2_unorm:     return "r10g10b10a2_unorm";
	case reshade::api::format::r10g10b10a2_xr_bias:   return "r10g10b10a2_xr_bias";
	case reshade::api::format::b10g10r10a2_typeless:  return "b10g10r10a2_typeless";
	case reshade::api::format::b10g10r10a2_uint:      return "b10g10r10a2_uint";
	case reshade::api::format::b10g10r10a2_unorm:     return "b10g10r10a2_unorm";
	case reshade::api::format::l16_unorm:             return "l16_unorm";
	case reshade::api::format::r16_typeless:          return "r16_typeless";
	case reshade::api::format::r16_uint:              return "r16_uint";
	case reshade::api::format::r16_sint:              return "r16_sint";
	case reshade::api::format::r16_unorm:             return "r16_unorm";
	case reshade::api::format::r16_snorm:             return "r16_snorm";
	case reshade::api::format::r16_float:             return "r16_float";
	case reshade::api::format::l16a16_unorm:          return "l16a16_unorm";
	case reshade::api::format::r16g16_typeless:       return "r16g16_typeless";
	case reshade::api::format::r16g16_uint:           return "r16g16_uint";
	case reshade::api::format::r16g16_sint:           return "r16g16_sint";
	case reshade::api::format::r16g16_unorm:          return "r16g16_unorm";
	case reshade::api::format::r16g16_snorm:          return "r16g16_snorm";
	case reshade::api::format::r16g16_float:          return "r16g16_float";
	case reshade::api::format::r16g16b16a16_typeless: return "r16g16b16a16_typeless";
	case reshade::api::format::r16g16b16a16_uint:     return "r16g16b16a16_uint";
	case reshade::api::format::r16g16b16a16_sint:     return "r16g16b16a16_sint";
	case reshade::api::format::r16g16b16a16_unorm:    return "r16g16b16a16_unorm";
	case reshade::api::format::r16g16b16a16_snorm:    return "r16g16b16a16_snorm";
	case reshade::api::format::r16g16b16a16_float:    return "r16g16b16a16_float";
	case reshade::api::format::r32_typeless:          return "r32_typeless";
	case reshade::api::format::r32_uint:              return "r32_uint";
	case reshade::api::format::r32_sint:              return "r32_sint";
	case reshade::api::format::r32_float:             return "r32_float";
	case reshade::api::format::r32g32_typeless:       return "r32g32_typeless";
	case reshade::api::format::r32g32_uint:           return "r32g32_uint";
	case reshade::api::format::r32g32_sint:           return "r32g32_sint";
	case reshade::api::format::r32g32_float:          return "r32g32_float";
	case reshade::api::format::r32g32b32_typeless:    return "r32g32b32_typeless";
	case reshade::api::format::r32g32b32_uint:        return "r32g32b32_uint";
	case reshade::api::format::r32g32b32_sint:        return "r32g32b32_sint";
	case reshade::api::format::r32g32b32_float:       return "r32g32b32_float";
	case reshade::api::format::r32g32b32a32_typeless: return "r32g32b32a32_typeless";
	case reshade::api::format::r32g32b32a32_uint:     return "r32g32b32a32_uint";
	case reshade::api::format::r32g32b32a32_sint:     return "r32g32b32a32_sint";
	case reshade::api::format::r32g32b32a32_float:    return "r32g32b32a32_float";
	case reshade::api::format::r9g9b9e5:              return "r9g9b9e5";
	case reshade::api::format::r11g11b10_float:       return "r11g11b10_float";
	case reshade::api::format::b5g6r5_unorm:          return "b5g6r5_unorm";
	case reshade::api::format::b5g5r5a1_unorm:        return "b5g5r5a1_unorm";
	case reshade::api::format::b5g5r5x1_unorm:        return "b5g5r5x1_unorm";
	case reshade::api::format::b4g4r4a4_unorm:        return "b4g4r4a4_unorm";
	case reshade::api::format::a4b4g4r4_unorm:        return "a4b4g4r4_unorm";
	case reshade::api::format::s8_uint:               return "s8_uint";
	case reshade::api::format::d16_unorm:             return "d16_unorm";
	case reshade::api::format::d16_unorm_s8_uint:     return "d16_unorm_s8_uint";
	case reshade::api::format::d24_unorm_x8_uint:     return "d24_unorm_x8_uint";
	case reshade::api::format::d24_unorm_s8_uint:     return "d24_unorm_s8_uint";
	case reshade::api::format::d32_float:             return "d32_float";
	case reshade::api::format::d32_float_s8_uint:     return "d32_float_s8_uint";
	case reshade::api::format::r24_g8_typeless:       return "r24_g8_typeless";
	case reshade::api::format::r24_unorm_x8_uint:     return "r24_unorm_x8_uint";
	case reshade::api::format::x24_unorm_g8_uint:     return "x24_unorm_g8_uint";
	case reshade::api::format::r32_g8_typeless:       return "r32_g8_typeless";
	case reshade::api::format::r32_float_x8_uint:     return "r32_float_x8_uint";
	case reshade::api::format::x32_float_g8_uint:     return "x32_float_g8_uint";
	case reshade::api::format::bc1_typeless:          return "bc1_typeless";
	case reshade::api::format::bc1_unorm:             return "bc1_unorm";
	case reshade::api::format::bc1_unorm_srgb:        return "bc1_unorm_srgb";
	case reshade::api::format::bc2_typeless:          return "bc2_typeless";
	case reshade::api::format::bc2_unorm:             return "bc2_unorm";
	case reshade::api::format::bc2_unorm_srgb:        return "bc2_unorm_srgb";
	case reshade::api::format::bc3_typeless:          return "bc3_typeless";
	case reshade::api::format::bc3_unorm:             return "bc3_unorm";
	case reshade::api::format::bc3_unorm_srgb:        return "bc3_unorm_srgb";
	case reshade::api::format::bc4_typeless:          return "bc4_typeless";
	case reshade::api::format::bc4_unorm:             return "bc4_unorm";
	case reshade::api::format::bc4_snorm:             return "bc4_snorm";
	case reshade::api::format::bc5_typeless:          return "bc5_typeless";
	case reshade::api::format::bc5_unorm:             return "bc5_unorm";
	case reshade::api::format::bc5_snorm:             return "bc5_snorm";
	case reshade::api::format::bc6h_typeless:         return "bc6h_typeless";
	case reshade::api::format::bc6h_ufloat:           return "bc6h_ufloat";
	case reshade::api::format::bc6h_sfloat:           return "bc6h_sfloat";
	case reshade::api::format::bc7_typeless:          return "bc7_typeless";
	case reshade::api::format::bc7_unorm:             return "bc7_unorm";
	case reshade::api::format::bc7_unorm_srgb:        return "bc7_unorm_srgb";
	case reshade::api::format::r8g8_b8g8_unorm:       return "r8g8_b8g8_unorm";
	case reshade::api::format::g8r8_g8b8_unorm:       return "g8r8_g8b8_unorm";
	case reshade::api::format::intz:                  return "intz";
	case reshade::api::format::unknown:
	default:                                          return "unknown";
	}
}

inline auto to_string(reshade::api::resource_view_type value) {
	switch (value) {
	case reshade::api::resource_view_type::buffer:                       return "buffer";
	case reshade::api::resource_view_type::texture_1d:                   return "texture_1d";
	case reshade::api::resource_view_type::texture_1d_array:             return "texture_1d_array";
	case reshade::api::resource_view_type::texture_2d:                   return "texture_2d";
	case reshade::api::resource_view_type::texture_2d_array:             return "texture_2d_array";
	case reshade::api::resource_view_type::texture_2d_multisample:       return "texture_2d_multisample";
	case reshade::api::resource_view_type::texture_2d_multisample_array: return "texture_2d_multisample_array";
	case reshade::api::resource_view_type::texture_3d:                   return "texture_3d";
	case reshade::api::resource_view_type::texture_cube:                 return "texture_cube";
	case reshade::api::resource_view_type::texture_cube_array:           return "texture_cube_array";
	case reshade::api::resource_view_type::acceleration_structure:       return "acceleration_structure";
	case reshade::api::resource_view_type::unknown:
	default:                                                             return "unknown";
	}
}

inline auto to_string(reshade::api::pipeline_subobject_type value) {
	switch (value) {
	case reshade::api::pipeline_subobject_type::vertex_shader:           return "vertex_shader";
	case reshade::api::pipeline_subobject_type::hull_shader:             return "hull_shader";
	case reshade::api::pipeline_subobject_type::domain_shader:           return "domain_shader";
	case reshade::api::pipeline_subobject_type::geometry_shader:         return "geometry_shader";
	case reshade::api::pipeline_subobject_type::pixel_shader:            return "pixel_shader";
	case reshade::api::pipeline_subobject_type::compute_shader:          return "compute_shader";
	case reshade::api::pipeline_subobject_type::input_layout:            return "input_layout";
	case reshade::api::pipeline_subobject_type::stream_output_state:     return "stream_output_state";
	case reshade::api::pipeline_subobject_type::blend_state:             return "blend_state";
	case reshade::api::pipeline_subobject_type::rasterizer_state:        return "rasterizer_state";
	case reshade::api::pipeline_subobject_type::depth_stencil_state:     return "depth_stencil_state";
	case reshade::api::pipeline_subobject_type::primitive_topology:      return "primitive_topology";
	case reshade::api::pipeline_subobject_type::depth_stencil_format:    return "depth_stencil_format";
	case reshade::api::pipeline_subobject_type::render_target_formats:   return "render_target_formats";
	case reshade::api::pipeline_subobject_type::sample_mask:             return "sample_mask";
	case reshade::api::pipeline_subobject_type::sample_count:            return "sample_count";
	case reshade::api::pipeline_subobject_type::viewport_count:          return "viewport_count";
	case reshade::api::pipeline_subobject_type::dynamic_pipeline_states: return "dynamic_pipeline_states";
	case reshade::api::pipeline_subobject_type::max_vertex_count:        return "max_vertex_count";
	case reshade::api::pipeline_subobject_type::amplification_shader:    return "amplification_shader";
	case reshade::api::pipeline_subobject_type::mesh_shader:             return "mesh_shader";
	case reshade::api::pipeline_subobject_type::raygen_shader:           return "raygen_shader";
	case reshade::api::pipeline_subobject_type::any_hit_shader:          return "any_hit_shader";
	case reshade::api::pipeline_subobject_type::closest_hit_shader:      return "closest_hit_shader";
	case reshade::api::pipeline_subobject_type::miss_shader:             return "miss_shader";
	case reshade::api::pipeline_subobject_type::intersection_shader:     return "intersection_shader";
	case reshade::api::pipeline_subobject_type::callable_shader:         return "callable_shader";
	case reshade::api::pipeline_subobject_type::libraries:               return "libraries";
	case reshade::api::pipeline_subobject_type::shader_groups:           return "shader_groups";
	case reshade::api::pipeline_subobject_type::max_payload_size:        return "max_payload_size";
	case reshade::api::pipeline_subobject_type::max_attribute_size:      return "max_attribute_size";
	case reshade::api::pipeline_subobject_type::max_recursion_depth:     return "max_recursion_depth";
	case reshade::api::pipeline_subobject_type::flags:                   return "flags";
	default:
	case reshade::api::pipeline_subobject_type::unknown:                 return "unknown";
	}
}

inline auto to_string(uint32_t action) {
	switch (action) {
	case action_replace: return "replace";
	case action_skip: return "skip";
	case action_log: return "log";
	case action_identify: return "identify";
	case action_injectText: return "injectText";
	};
}

inline auto to_string(Feature feature) {
	switch (feature) {
	case Feature::Rotor: return "Rotor";
	case Feature::Global: return "Global";
	case Feature::Label: return "Label";
	case Feature::Testing: return "Testing";
	case Feature::GetStencil: return "GetStencil";
	case Feature::IHADSS: return "IHADSS";	
	case Feature::VRMode: return "VRMode";
	case Feature::mapMode: return "MapMode";
	case Feature::Haze: return "Haze";
	case Feature::NoReflect: return "NoReflect";
	case Feature::HazeMSAA2x: return "HazeMSAA2x";
		
	}
}


// Fonction de conversion wchar_t* vers std::string
inline std::string to_string(const wchar_t* wcharStr) {

	char string[MAXNAMECHAR];
	size_t convertedChars = 0;
	wcstombs_s(&convertedChars, string, MAXNAMECHAR, wcharStr, _TRUNCATE);

	return string;

}
