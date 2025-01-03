///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// global variables shared between modules/functions
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

#pragma once

#include "reshade.hpp"
#include "mod_injection.h"
#include <string>
#include <shared_mutex>
#include "shader_definitions.h"

constexpr size_t CHAR_BUFFER_SIZE = 256;

using namespace reshade::api;

extern std::string settings_iniFileName;

//GUI variables to use in functions
extern bool debug_flag;
extern bool flag_capture;

struct resource_trace {
	bool created = false;
	bool copied = false;
	reshade::api::resource texresource;
};

struct resourceview_trace {
	bool created = false;
	reshade::api::resource_view texresource_view;
};

struct technique_trace {
	effect_technique technique;
	std::string name;
	std::string eff_name;
	bool technique_status;
};


// maximum resources or reource_views per draw, to copy DepthStencil texture
// should be 2x per eye x 2 for quad view + margin. No added view/resources for AA   
#define MAXVIEWSPERDRAW 6

// a class to host all global variables shared between reshade on_* functions. 
// 
struct __declspec(uuid("6EAA737E-90F1-453E-A062-BF8FE390EE21")) global_shared
{
	// index of constant buffers collected in global DX11 pipepline layout, should be 2
	uint32_t CBIndex = 2;
	// index of resource views collected in global DX11 pipepline layout, should be 1
	uint32_t RVIndex = 1;

	// this struct is containing the constant buffer to be injected, see 'mod_parameter.h'
	struct ShaderInjectData cb_inject_values;
	uint64_t cb_inject_size = CBSIZE;

	// DX11 pipeline_layout for CB
	reshade::api::pipeline_layout saved_pipeline_layout_CB;

	// DX11 pipeline_layout for ressource view
	reshade::api::pipeline_layout saved_pipeline_layout_RV;

	// to be used in push_constant
	reshade::api::descriptor_table_update CB_desc_table_update;

	// for frame debugging
	bool button_capture = false;

	// to handle parallel access (not used)
	std::shared_mutex s_mutex;

	// to avoid "holes" in 

	// to copy texture
	reshade::api::descriptor_table_update update;
	// depth Stencil texture
	resource_trace depthStencil_res[MAXVIEWSPERDRAW];
	resourceview_trace stencil_view[MAXVIEWSPERDRAW];
	resourceview_trace depth_view[MAXVIEWSPERDRAW];
	bool depthStencil_copy_started;
	// for logging shader_resource_view in push_descriptors() to get depthStencil 
	bool track_for_depthStencil = false;

	// technique : copy render target, flag for draw
	// resource_trace render_target_res[MAXVIEWSPERDRAW];
	resourceview_trace render_target_view[MAXVIEWSPERDRAW];

	bool track_for_render_target = false;
	bool render_effect = false;
	uint32_t count_draw = 0;


	// for technique init
	char technique_init = -1;

	// for technique refresh
	bool button_technique = false;

	// copy from one function to another
	reshade::api::effect_runtime* runtime;
	reshade::api::command_list* commandList;

	//map of technique selected 
	std::vector<technique_trace> technique_vector;

	// render target (all(0)/outer(1)/inner(2)) for effect
	int effect_target_QV = 0;

	//NS430 texture
	resource_trace NS430_res[MAXVIEWSPERDRAW];
	resourceview_trace NS430_view[MAXVIEWSPERDRAW];
	bool NS430_copy_started;
	// for logging shader_resource_view in push_descriptors() to get screen texture 
	bool track_for_NS430 = false;

	// counter for the current display (eye + quad view)
	short int count_display = 0;

	// to avoid "holes" in count_display, as the PS used to increment can be called 2 time consecutivelly
	Feature last_feature = Feature::Null;

	// to used as seed for random
	uint32_t frame_counter = 0;

	// counter for skip seems not working !
	uint32_t counter_testing = 0;

	// for logging render targets  
	bool track_for_rt = false;

	//disable optimisation
	bool disable_optimisation = false;

	//effect

	// flag for features
	bool effects_feature = false;
	bool color_feature = false;
	bool sharpenDeband_feature = false;
	bool misc_feature = false;
	bool helo_feature = false;
	bool NS430_feature = false;
	// bool debug_feature = false;
	// initial value of flag above, to display 'relaunch' message
	bool init_effects_feature = false;
	bool init_color_feature = false;
	bool init_sharpenDeband_feature = false;
	bool init_misc_feature = false;
	bool init_helo_feature = false;
	bool init_NS430_feature = false;
	bool init_debug_feature = false;
};

extern struct global_shared shared_data;

extern bool do_not_draw;

struct __declspec(uuid("7251932A-ADAF-4DFC-B5CB-9A4E8CD5D6EB")) device_data
{
	effect_runtime* main_runtime = nullptr;
};
