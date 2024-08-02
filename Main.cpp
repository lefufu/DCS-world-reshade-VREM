///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
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

#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#define ImTextureID unsigned long long // Change ImGui texture ID type to that of a 'reshade::api::resource_view' handle

#include <vector>
#include <filesystem>
#include <unordered_map>


#include <imgui.h>
#include <reshade.hpp>
#include "crc32_hash.hpp"
#include "functions.h"
#include "global_shared.hpp"
#include "to_string.hpp"
#include "shader_definitions.h"

using namespace reshade::api;

extern "C" __declspec(dllexport) const char *NAME = "DCS VREM";
extern "C" __declspec(dllexport) const char *DESCRIPTION = "DCS mod to enhance VR in DCS";

// ***********************************************************************************************************************
// definition of shader of the mod
std::unordered_map<uint32_t, Shader_Definition> shaders_by_hash =
{
	// fix for rotor
	{ 0xC0CC8D69, Shader_Definition(action_replace, Feature::Rotor, L"AH64_rotorPS.cso", 0) },
	// fix for TADS display in IHADSS
	{ 0x0099D562, Shader_Definition(action_replace, Feature::IHADSS, L"IHADSS_PNVS_PS.cso", 0) },
	// to start spying texture for depthStencil (Vs associated with global illumination PS)
	{ 0x4DDC4917, Shader_Definition(action_log, Feature::GetStencil, L"", 0) },
	// global PS for color change, sharpen,..
	//{ 0x829504B1, Shader_Definition(action_replace | action_injectText, Feature::Global, L"global_PS_1.cso", 0) },
	{ 0xBAF1E52F, Shader_Definition(action_replace | action_injectText, Feature::Global, L"global_PS_2.cso", 0) },

	// to define if VR is active or not (2D mirror view of VR )
	{ 0x886E31F2, Shader_Definition(action_identify, Feature::VRMode, L"", 0) },
	//
	// testing
	{ 0x6CEA1C47, Shader_Definition(action_replace, Feature::Testing, L"GUI_textPS.cso", 0) }

};

// ***********************************************************************************************************************
//init shared variables
std::string settings_iniFileName = "DCS_VREM.ini";
//default value overwritten by setting file if exists
bool debug_flag = true;
struct global_shared shared_data;
std::unordered_map<uint32_t, Shader_Definition> shaders_by_handle;
std::unordered_map<uint32_t, resource> texture_resource_by_handle;

// ***********************************************************************************************************************
//init local variables
//to hanlde loading code for replaced shader
static thread_local std::vector<std::vector<uint8_t>> shader_code;
static  bool flag_capture = false;

// ***********************************************************************************************************************
// on_init_resource_view() : not needed, keep for debug messages in resource copy
//  once it will be known which resource is used by which shader...

static void on_init_resource_view(
	reshade::api::device* device,
	reshade::api::resource resource,
	reshade::api::resource_usage usage_type,
	const reshade::api::resource_view_desc& desc,
	reshade::api::resource_view view) 
{
	
	std::stringstream s;
	s << "init_resource_view("
		<< reinterpret_cast<void*>(view.handle)
		<< ", view type: " << to_string(desc.type) << " (0x" << std::hex << (uint32_t)desc.type << std::dec << ")"
		<< ", view format: " << to_string(desc.format) << " (0x" << std::hex << (uint32_t)desc.format << std::dec << ")"
		<< ", resource: " << reinterpret_cast<void*>(resource.handle)
		<< ", resource usage: " << to_string(usage_type) << " 0x" << std::hex << (uint32_t)usage_type << std::dec;

	// handle only texture
	if (resource.handle) {
		const auto resourceDesc = device->get_resource_desc(resource);
		s << ", resource type: " << to_string(resourceDesc.type);
		switch (resourceDesc.type) {
			case reshade::api::resource_type::texture_1d:
			case reshade::api::resource_type::texture_2d:
				s << ", texture format: " << to_string(resourceDesc.texture.format);
				s << ", texture width: " << resourceDesc.texture.width;
				s << ", texture height: " << resourceDesc.texture.height;
				texture_resource_by_handle.emplace(resource.handle, resource);
				break;
			case reshade::api::resource_type::texture_3d:
				s << ", texture format: " << to_string(resourceDesc.texture.format);
				s << ", texture width: " << resourceDesc.texture.width;
				s << ", texture height: " << resourceDesc.texture.height;
				s << ", texture depth: " << resourceDesc.texture.depth_or_layers;
				texture_resource_by_handle.emplace(resource.handle, resource);
				break;
		}
		s << ")";
		if (debug_flag) reshade::log_message(reshade::log_level::info, s.str().c_str());
		s.str("");
		s.clear();
	}
}


// *******************************************************************************************************
// on_init_pipeline_layout() : called once, to create the DX11 layout to use in push_constant
//
static void on_init_pipeline_layout(reshade::api::device* device, const uint32_t paramCount, const reshade::api::pipeline_layout_param* params, reshade::api::pipeline_layout layout)
{

	std::stringstream s;

	// generate data for constant_buffer or shader_resource_view
	for (uint32_t paramIndex = 0; paramIndex < paramCount; ++paramIndex) {
		auto param = params[paramIndex];

		//log infos
		if (debug_flag)
		{
			s << "* looping on  paramCount : param = " << to_string(paramIndex) << ", param.type = " << to_string(param.type) << ", param.push_descriptors.type = " << to_string(param.push_descriptors.type);
			reshade::log_message(reshade::log_level::info, s.str().c_str());
			s.str("");
			s.clear();
		}
		if (param.push_descriptors.type == descriptor_type::constant_buffer)
		{
			//index should be 2 for CB in DX11, but let's get it dynamically
			shared_data.CBIndex = paramIndex;

			// create a new pipeline_layout for just 1 constant buffer to be updated by push_constant(), cb number defined in CBINDEX
			// pipeline_layout_param
			// uint32_t 	binding 			OpenGL uniform buffer binding index. 
			// uint32_t 	dx_register_index	D3D10/D3D11/D3D12 constant buffer register index. 
			// uint32_t 	dx_register_space	D3D12 constant buffer register space. 
			// uint32_t 	count				Number of constants in this range (in 32-bit values). 
			// shader_stage visibility			Shader pipeline stages that can make use of the constants in this range. 

			reshade::api::pipeline_layout_param newParams;
			newParams.type = reshade::api::pipeline_layout_param_type::push_constants;
			newParams.push_constants.binding = 0;
			newParams.push_constants.count = 1;
			newParams.push_constants.dx_register_index = CBINDEX;
			newParams.push_constants.dx_register_space = 0;
			newParams.push_constants.visibility = reshade::api::shader_stage::all;

			bool  result = device->create_pipeline_layout(1, &newParams, &shared_data.saved_pipeline_layout_CB);

			if (result) {
				if (debug_flag)
				{
					s << "on_init_pipeline_layout: new pipeline created, hash =" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB.handle) << " ).  DX11 layout created for CB;";
					s << "dx_register_index=" << CBINDEX << ", CBIndex =" << shared_data.CBIndex << "; ";
					reshade::log_message(reshade::log_level::warning, s.str().c_str());
					s.str("");
					s.clear();
				}
			}
			else
			{
				s << "on_init_pipeline_layout(" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB.handle) << " ). !!! Error in creating DX11 layout for CB!!!;";
				reshade::log_message(reshade::log_level::warning, s.str().c_str());
				s.str("");
				s.clear();
			}
		}

		else if (param.push_descriptors.type == descriptor_type::shader_resource_view)
		{
			// store info Ressource View injection
			shared_data.RVIndex = paramIndex;

			// create a new pipeline_layout for just 1 rsource view to be updated by push_constant(), RV number defined in RVINDEX
			reshade::api::descriptor_range srv_range;
			srv_range.dx_register_index = RVINDEX;
			srv_range.count = UINT32_MAX; 
			srv_range.visibility = reshade::api::shader_stage::pixel;
			srv_range.type = reshade::api::descriptor_type::shader_resource_view;	

			const reshade::api::pipeline_layout_param params[] = {
				srv_range,
			};

			bool  result = device->create_pipeline_layout(std::size(params), params, &shared_data.saved_pipeline_layout_RV);

			if (result) {
				if (debug_flag)
				{
					s << "on_init_pipeline_layout: new pipeline created, hash =" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_RV.handle) << " ).  DX11 layout created for RV;";
					s << "dx_register_index=" << RVINDEX << ", RVIndex =" << shared_data.RVIndex << "; ";
					reshade::log_message(reshade::log_level::warning, s.str().c_str());
					s.str("");
					s.clear();
				}
			}
			else
			{
				s << "on_init_pipeline_layout(" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB.handle) << " ). !!! Error in creating DX11 layout for CB!!!;";
				reshade::log_message(reshade::log_level::warning, s.str().c_str());
				s.str("");
				s.clear();
			}
		}

	}
}

// ***********************************************************************************************************************
// on_init_pipeline() : called once per pipeline, used to load new shader code, initialize the shader handling process and set some flags 
// create a new pipeline for shaders which need to be replaced 
static void on_init_pipeline(device* device, pipeline_layout layout, uint32_t subobjectCount, const pipeline_subobject* subobjects, pipeline pipelineHandle)
{

	std::stringstream s;

	// It is assumed all worthly shaders are in pipeline of only 1 object in DCS
	// only few shaders are to be modded
	if (subobjectCount == 1)
	{
		switch (subobjects[0].type)
		{
		case pipeline_subobject_type::vertex_shader:
		case pipeline_subobject_type::pixel_shader:
		case pipeline_subobject_type::compute_shader:
		case pipeline_subobject_type::hull_shader:
		case pipeline_subobject_type::domain_shader:
			//compute has and see if it is declared in shader mod list
			uint32_t hash = calculateShaderHash(subobjects[0].data);
			auto it = shaders_by_hash.find(hash);

			if (it != shaders_by_hash.end()) {
				// shader is to be handled
				// add the shader entry in the map by pipeline handle

				if (debug_flag) {
					// logging infos
					s << "onInitPipeline, pipelineHandle: " << (void*)pipelineHandle.handle << "), ";
					s << "layout =  " << reinterpret_cast<void*>(layout.handle) << " ;";
					s << "subobjectCount =  " << subobjectCount << " ;";
					s << "Type = " << to_string(subobjects[0].type) << " ;";
					s << "hash to handle = " << std::hex << hash << " ;";
					s << "Action = " << to_string(it->second.action) << "; Feature = " << to_string(it->second.feature) << "; fileName =" << to_string(it->second.replace_filename) << ";";

					reshade::log_message(reshade::log_level::info, s.str().c_str());
					s.str("");
					s.clear();
				}

				//create the entry for handling shader by pipeline instead of Hash
				Shader_Definition newShader(it->second.action, it->second.feature, it->second.replace_filename, it->second.draw_count);

				if (it->second.action & action_replace)
				{
					// clone the existing pipeline and load the modded shader in it
					bool status = load_shader_code(shader_code, it->second.replace_filename);
					if (!status) {
						// log error
						s << "onInitPipeline, pipelineHandle: " << (void*)pipelineHandle.handle << "), ";
						s << "hash to handle = " << std::hex << hash << " ;";
						s << "!!! Error in loading code for :" << to_string(it->second.replace_filename) << "; !!!";

						reshade::log_message(reshade::log_level::error, s.str().c_str());
						s.str("");
						s.clear();
					}
					else {
						// if not done, clone the pipeline to have a new version with fixed color for PS
						clone_pipeline(device, layout, subobjectCount, subobjects, pipelineHandle, shader_code, &newShader);
						//keep hash for debug messages
						newShader.hash = hash;
					}
				}

				if (it->second.action & action_identify)
				{
					// setup some global variables according to the feature
					if (it->second.feature == Feature::VRMode)
					{
						shared_data.VRMode = true;
					}
				}

				// store new shader to re use it later 
				shaders_by_handle.emplace(pipelineHandle.handle, newShader);
			}
			break;
		}
	}
}

// *******************************************************************************************************
// on_bind_pipeline(): called for all pipeline calls, used to 
//    replace the original pipeline by the modified one (if mod feature) 
//    do a push_constants() to push mod parameters in cb13 if mod is replaced
//	  trigger future tracking of resource if needed
//    trigger future skip of draw if needed
//
static void on_bind_pipeline(command_list* commandList, pipeline_stage stages, pipeline pipelineHandle)
{

	
	
	// maybe not the most elegant way to filter shader, but this more readeable...
	bool to_process = false;
	if ((stages & pipeline_stage::vertex_shader) == pipeline_stage::vertex_shader) to_process = true;
	if ((stages & pipeline_stage::pixel_shader) == pipeline_stage::pixel_shader) to_process = true;
	if ((stages & pipeline_stage::compute_shader) == pipeline_stage::compute_shader) to_process = true;
	if ((stages & pipeline_stage::hull_shader) == pipeline_stage::hull_shader) to_process = true;
	if ((stages & pipeline_stage::domain_shader) == pipeline_stage::domain_shader) to_process = true;

	if (to_process)
	{
		
		//find if the pipeline is related to a shader of the mod
		std::unordered_map<uint32_t, Shader_Definition>::iterator it;
		it = shaders_by_handle.find(pipelineHandle.handle);

		if (it != shaders_by_handle.end()) {

			// debug message
			if (debug_flag && shared_data.s_do_capture) {
				std::stringstream s;
				s << "on_bind_pipeline : Pipeline handle to process found: " << reinterpret_cast<void*>(pipelineHandle.handle) << ", hash =" << std::hex << it->second.hash << ", associated cloned pipeline handle: " << reinterpret_cast<void*>(it->second.substitute_pipeline.handle) << ", feature =" << to_string(it->second.feature) << ";";

				reshade::log_message(reshade::log_level::info, s.str().c_str());
				// s.str("");
				// s.clear();
			}
			
			
			if (it->second.action & action_injectText)
			{
				// inject texture using push_descriptor()
				if (it->second.feature == Feature::Global)
				{
					// push the texture for depth and stencil
					//depth
					reshade::api::descriptor_table_update update;
					update.binding = 0; // t3 as 3 is defined in pipeline_layout
					update.count = 1;
					update.type = reshade::api::descriptor_type::shader_resource_view;
					update.descriptors = &shared_data.depth_view;
					commandList->push_descriptors(reshade::api::shader_stage::pixel, shared_data.saved_pipeline_layout_RV, 0, update);

					//stencil
					update.binding = 1; // t4 as 3 is defined in pipeline_layout
					update.descriptors = &shared_data.stencil_view;
					commandList->push_descriptors(reshade::api::shader_stage::pixel, shared_data.saved_pipeline_layout_RV, 0, update);

					// log infos
					if (debug_flag && shared_data.s_do_capture)
					{
						reshade::log_message(reshade::log_level::info, " => on_bind_pipeline : depthStencil textures injected;");
					}
				}
			}	
			
			// shader is to be handled
			if (it->second.action & action_replace)
			{
				
				//use push constant() to push the mod parameter in CB13
				commandList->push_constants(
					shader_stage::all,
					shared_data.saved_pipeline_layout_CB,
					0,
					0,
					CBSIZE,
					&shared_data.cb_inject_values
				);
				
				// shader is to be replaced by the new one created in on_Init_Pipeline
				commandList->bind_pipeline(stages, it->second.substitute_pipeline);

				// log infos
				if (debug_flag && shared_data.s_do_capture)
				{
					std::stringstream s;
					s << " => on_bind_pipeline : pipeline Pixel replaced (" << reinterpret_cast<void*>(pipelineHandle.handle) << ") by  " << reinterpret_cast<void*>(it->second.substitute_pipeline.handle) << ", feature =" << to_string(it->second.feature )<< ";";
					reshade::log_message(reshade::log_level::info, s.str().c_str());
					// s.str("");
					// s.clear();
				}
	
			}
			
			if (it->second.action & action_skip)
			{
				// shader is to be skipped, setup a flag for next draw
			}

			if (it->second.action & action_log)
			{
				// trigger logging of resources (eg texture) or other topics (eg count calls)
				if (it->second.feature == Feature::GetStencil)
				{	
					// engage tracking shader_resource_view in push_descriptors() to get depthStencil 
					shared_data.track_for_depthStencil = true;
				}
			}

		}
	}

}

// *******************************************************************************************************
// on_push_descriptors() : to be monitored in order to copy texture
// called a lot !
static void on_push_descriptors(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{

	//const std::shared_lock<std::shared_mutex> lock(shared_data.s_mutex);

	//handle only shader_resource_view when needed
	// 
	// handle depthStencil
	if (shared_data.track_for_depthStencil && update.type == descriptor_type::shader_resource_view &&  stages == shader_stage::pixel && update.count == 6)
	{
		
		//log infos
		
		if (debug_flag && shared_data.s_do_capture)
		{
			std::stringstream s;
			s << "on_push_descriptors(" << to_string(stages) << ", " << (void*)layout.handle << ", " << param_index << ", { " << to_string(update.type) << ", " << update.binding << ", " << update.count << " })";
			reshade::log_message(reshade::log_level::info, s.str().c_str());
			s.str("");
			s.clear();

			if (update.type == descriptor_type::shader_resource_view)
			{
				// add info on textures hash
				for (uint32_t i = 0; i < update.count; ++i)
				{
					auto item = static_cast<const reshade::api::resource_view*>(update.descriptors)[i];
					s << "=> on_push_descriptors(), resource_view[" << i << "],  handle = " << reinterpret_cast<void*>(item.handle) << " })";
					reshade::log_message(reshade::log_level::info, s.str().c_str());
					s.str("");
					s.clear();
				}
			}
			reshade::log_message(reshade::log_level::info, s.str().c_str());
		}

		// copy depthStencil texture into shared_data
		bool status = copy_depthStencil(cmd_list, stages, layout, param_index, update);

		// stop tracking
		shared_data.track_for_depthStencil = false;
	}

}

//*******************************************************************************************************
// clear tracking flags to avoid tracking resources if push_descriptor did not detect it...
static void clear_tracking_flags()
{
	shared_data.track_for_depthStencil = false;
}

// *******************************************************************************************************
// On draw* : skip draw
//
static bool on_draw(command_list* commandList, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{

	if (debug_flag && shared_data.s_do_capture && shared_data.track_for_depthStencil)
	{
		std::stringstream s;
		s << "draw(" << vertex_count << ", " << instance_count << ", " << first_vertex << ", " << first_instance << ")";
		reshade::log_message(reshade::log_level::info, s.str().c_str());
	}

	// clear trackign flags
	clear_tracking_flags();

	/*
	// check if for this command list the active shader handles are part of the blocked set. If so, return true
	if (!constant_color)
		return blockDrawCallForCommandList(commandList);
	else
		return false;
		*/
	return false;
}

// On draw* : skip draw
static bool on_draw_indexed(command_list* commandList, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
{


	if (debug_flag && shared_data.s_do_capture && shared_data.track_for_depthStencil)
	{
		std::stringstream s;
		s << "draw_indexed(" << index_count << ", " << instance_count << ", " << first_index << ", " << vertex_offset << ", " << first_instance << ")";
		reshade::log_message(reshade::log_level::info, s.str().c_str());
	}

	// clear trackign flags
	clear_tracking_flags();

	/*
	// check if for this command list the active shader handles are part of the blocked set. If so, return true
	if (!constant_color)
		return blockDrawCallForCommandList(commandList);
	else
		return false;
		*/
	return false;
}

// On draw* : skip draw
static bool on_drawOrDispatch_indirect(command_list* commandList, indirect_command type, resource buffer, uint64_t offset, uint32_t draw_count, uint32_t stride)
{

	if (debug_flag && shared_data.s_do_capture && shared_data.track_for_depthStencil)
	{
		std::stringstream s;
		s << "draw_indexed_indirect(" << (void*)buffer.handle << ", " << offset << ", " << draw_count << ", " << stride << ")";
		reshade::log_message(reshade::log_level::info, s.str().c_str());
	}

	// clear trackign flags
	clear_tracking_flags();

	/*
	// check if for this command list the active shader handles are part of the blocked set. If so, return true
	if (!constant_color)
		return blockDrawCallForCommandList(commandList);
	else
		return false;
		*/
	return false;
}

// *******************************************************************************************************
// Cleanup : delete created things
//
//delete created pipeline (maybe not needed ?)
static void on_destroy_pipeline(
	reshade::api::device* device,
	reshade::api::pipeline pipeline) 
{
	
	std::unordered_map<uint32_t, Shader_Definition>::iterator it;
	it = shaders_by_handle.find(pipeline.handle);

	if (it != shaders_by_handle.end()) {
		device->destroy_pipeline(it->second.substitute_pipeline);
		if (debug_flag )
		{
			std::stringstream s;
			s << "destroy_pipeline, master pipeline" << reinterpret_cast<void*>(pipeline.handle) << ", associated pipeline" << reinterpret_cast<void*>(it->second.substitute_pipeline.handle) << ")";
		}
	}

}

//delete vectors and maps
void cleanup()
{

	shader_code.clear(); 
	shaders_by_handle.clear();
	texture_resource_by_handle.clear();
	shaders_by_hash.clear();
}

// *******************************************************************************************************
// Main 
//
BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
					
			WCHAR buf[MAX_PATH];
			const std::filesystem::path dllPath = GetModuleFileNameW(nullptr, buf, ARRAYSIZE(buf)) ? buf : std::filesystem::path();		// <installpath>/shadertoggler.addon64
			const std::filesystem::path basePath = dllPath.parent_path();																// <installpath>
			const std::string& settings_FileName = settings_iniFileName;
			settings_iniFileName = (basePath / settings_FileName).string();
		
			if(!reshade::register_addon(hModule))
			{
				return FALSE;
			}

			reshade::register_event<reshade::addon_event::init_pipeline>(on_init_pipeline);
			reshade::register_event<reshade::addon_event::bind_pipeline>(on_bind_pipeline);
			reshade::register_event<reshade::addon_event::init_pipeline_layout>(on_init_pipeline_layout);
			reshade::register_event<reshade::addon_event::draw>(on_draw);
			reshade::register_event<reshade::addon_event::draw_indexed>(on_draw_indexed);
			reshade::register_event<reshade::addon_event::draw_or_dispatch_indirect>(on_drawOrDispatch_indirect);
			//   reshade::register_event<reshade::addon_event::init_resource_view>(on_init_resource_view);
			reshade::register_event<reshade::addon_event::push_descriptors>(on_push_descriptors);

			reshade::register_event<reshade::addon_event::reshade_present>(on_present);
	
			// setup GUI
			reshade::register_overlay(nullptr, &displaySettings);

			// load stored settings and init shared variables
			load_setting_IniFile();

		}
		break;
	case DLL_PROCESS_DETACH:

		reshade::unregister_event<reshade::addon_event::init_pipeline>(on_init_pipeline);
		//reshade::unregister_event<reshade::addon_event::bind_pipeline>(on_bind_pipeline);
		reshade::unregister_event<reshade::addon_event::init_pipeline_layout>(on_init_pipeline_layout);
		reshade::unregister_event<reshade::addon_event::draw>(on_draw);
		reshade::unregister_event<reshade::addon_event::draw_indexed>(on_draw_indexed);
		reshade::unregister_event<reshade::addon_event::draw_or_dispatch_indirect>(on_drawOrDispatch_indirect);
		//   reshade::unregister_event<reshade::addon_event::init_resource_view>(on_init_resource_view);
		reshade::unregister_event<reshade::addon_event::push_descriptors>(on_push_descriptors);

		reshade::unregister_event<reshade::addon_event::reshade_present>(on_present);

		cleanup();
		reshade::unregister_overlay(nullptr, &displaySettings);

		reshade::unregister_addon(hModule);
		break;
	}

	return TRUE;
}



