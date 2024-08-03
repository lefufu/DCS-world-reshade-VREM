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

extern std::string settings_iniFileName;

//GUI variables to use in functions
extern bool debug_flag;


struct resource_trace {
	bool created = false;
	reshade::api::resource texresource;
};

struct resourceview_trace {
	bool created = false;
	reshade::api::resource_view texresource_view;
};

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
	// reshade::api::device *saved_device;
	reshade::api::descriptor_table_update CB_desc_table_update;

	// for frame debugging
	bool s_do_capture = false;
	bool button_capture = false;

	// for logging shader_resource_view in push_descriptors() to get depthStencil 
	bool track_for_depthStencil = false;

	// for logging VRmode (false = 2D)
	bool VRMode = false;

	// to handle parallel access
	std::shared_mutex s_mutex;

	// to copy depth Stencil texture
	//reshade::api::resource depthStencil_res;
	//reshade::api::resource_view stencil_view;
	//reshade::api::resource_view depth_view;
	//bool depthStencil_created = false;
	
	resource_trace depthStencil_res[4];
	resourceview_trace stencil_view[4];
	resourceview_trace depth_view[4];

	// counter for the current display
	short int count_display = -1;

};

extern struct global_shared shared_data;