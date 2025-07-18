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

#include "CDataFile.h"

using namespace reshade::api;

extern "C" __declspec(dllexport) const char *NAME = "DCS VREM";
extern "C" __declspec(dllexport) const char *DESCRIPTION = "DCS mod to enhance VR in DCS - v9.4.1";

// ***********************************************************************************************************************
// definition of all shader of the mod (whatever feature selected in GUI)
std::unordered_map<uint32_t, Shader_Definition> shader_by_hash =
{
	// ** fix for rotor **
	{ 0xC0CC8D69, Shader_Definition(action_replace, Feature::Rotor, L"AH64_rotorPS.cso", 0) },
	{ 0x349A1054, Shader_Definition(action_replace, Feature::Rotor, L"AH64_rotor2PS.cso", 0) },
	{ 0xD3E172D4, Shader_Definition(action_replace, Feature::Rotor, L"UH1_rotorPS.cso", 0) },
	// ** fix for IHADSS **
	{ 0x2D713734, Shader_Definition(action_replace, Feature::IHADSS, L"IHADSS_PNVS_PS.cso", 0) },
	{ 0xDF141A84, Shader_Definition(action_replace, Feature::IHADSS, L"IHADSS_PS.cso", 0) },
	{ 0x45E221A9, Shader_Definition(action_replace, Feature::IHADSS, L"IHADSS_VS.cso", 0) },
	// ** label masking and color/sharpen/deband **
	// to start spying texture for depthStencil (Vs associated with global illumination PS)
	// and inject modified CB CperFrame
	{ 0x4DDC4917, Shader_Definition(action_log | action_injectCB, Feature::GetStencil, L"", 0) },
	{ 0x57D037A0, Shader_Definition(action_injectCB, Feature::Sky, L"", 0) },
	// { 0x4DDC4917, Shader_Definition(action_log , Feature::GetStencil, L"", 0) },
	// global PS for all changes
	//{ 0xBAF1E52F, Shader_Definition(action_replace | action_injectText | action_log, Feature::Global, L"global_PS_2.cso", 0) },
	{ 0xBAF1E52F, Shader_Definition(action_replace | action_injectText, Feature::Global, L"global_PS_2.cso", 0) },
	//TODO VS associated with global PS 2 for draw increase : 8DB626CD
	{ 0x8DB626CD, Shader_Definition(action_log , Feature::VS_global2, L"", 0) },
	// Label PS 
	{ 0x6CEA1C47, Shader_Definition(action_replace | action_injectText, Feature::Label , L"labels_PS.cso", 0) },
	// ** NS430 **
	// to start spying texture for screen texture and disable frame (Vs associated with NS430 screen PS below)
	{ 0x52C97365, Shader_Definition(action_replace, Feature::NS430, L"VR_GUI_MFD_VS.cso", 0) },
	// to start spying texture for screen texture (Vs associated with NS430 screen EDF9F8DD for su25T&UH1, not same res. texture !)
	{ 0x8439C716, Shader_Definition(action_log, Feature::NS430, L"", 0) },
	// inject texture in global GUI and filter screen display (same shader for both)
	{ 0x99D562, Shader_Definition(action_replace | action_injectText, Feature::NS430 , L"VR_GUI_MFD_PS.cso", 0) },
	// disable NS430 frame, shared with some cockpit parts (can not be done by skip)
	{ 0xEFD973A1, Shader_Definition(action_replace, Feature::NS430 , L"NS430__framePS.cso", 0) },
	// disable NS430 screen background (done in shader because shared with other objects than NS430)
	{ 0x6EF95548, Shader_Definition(action_replace, Feature::NS430, L"NS430_screen_back.cso", 0) },
	// to filter out call for GUI and MFD
	{ 0x55288581, Shader_Definition(action_log, Feature::GUI, L"", 0) },
	//  ** identify game config **
	// to define if VR is active or not (2D mirror view of VR )
	{ 0x886E31F2, Shader_Definition(action_identify | action_log, Feature::VRMode, L"", 0) },
	// VS drawing cockpit parts to define if view is in welcome screen or map
	{ 0xA337E177, Shader_Definition(action_identify, Feature::mapMode, L"", 0) },
	//  ** reflection on instrument, done by GCOCKPITIBL of CperFrame **
	// A10C PS 
	{ 0xC9F547A7, Shader_Definition(action_injectCB , Feature::NoReflect , L"", 0) },
	// AH64 + F4 PS 
	{ 0x7BB48FB, Shader_Definition(action_injectCB , Feature::NoReflect , L"", 0) },
	
	//  ** NVG **
	{ 0xE65FAB66, Shader_Definition(action_replace , Feature::NVG , L"NVG_extPS.cso", 0) },
	//  ** identify render target ** (VS associated with first global PS)
	{ 0x936B2B6A, Shader_Definition(action_log , Feature::Effects , L"", 0) },
	
};

// ***********************************************************************************************************************
//init shared variables (path added in main)
std::string settings_iniFileName = "DCS_VREM.ini";
std::string technique_iniFileName = "DCS_VREM_techniques.ini";
CDataFile technique_iniFile;

//default value overwritten by setting file if exists
bool debug_flag;
struct global_shared shared_data;
// map to detect pipeline to process regarding the features enabled in GUI
std::unordered_map<uint32_t, Shader_Definition> pipeline_by_hash;
// map of pipeline detected, by using hash from previous map
std::unordered_map<uint64_t, Shader_Definition> pipeline_by_handle;
// std::unordered_map<uint32_t, resource> texture_resource_by_handle;

uint64_t last_replaced_shader = 0;
Feature last_feature = Feature::Null;

// ***********************************************************************************************************************
//init local variables
//to hanlde loading code for replaced shader
static thread_local std::vector<std::vector<uint8_t>> shader_code;
// TODO
bool flag_capture = false;
// bool flag_capture = true;
bool do_not_draw = false;
static thread_local std::vector<std::vector<uint8_t>> s_data_to_delete;


// *******************************************************************************************************
// on_create_pipeline() : called once per pipeline when 3D screen load, used to replace shader code if option setup for shader
//
static bool on_create_pipeline(device* device, pipeline_layout, uint32_t subobject_count, const pipeline_subobject* subobjects)
{
	bool replaced_stages = false;
	bool status = false;
	const device_api device_type = device->get_api();

	// Go through all shader stages that are in this pipeline and potentially replace the associated shader code
	for (uint32_t i = 0; i < subobject_count; ++i)
	{
		switch (subobjects[i].type)
		{
		case pipeline_subobject_type::vertex_shader:
		case pipeline_subobject_type::hull_shader:
		case pipeline_subobject_type::domain_shader:
		case pipeline_subobject_type::geometry_shader:
		case pipeline_subobject_type::pixel_shader:
		case pipeline_subobject_type::compute_shader:
		case pipeline_subobject_type::amplification_shader:
		case pipeline_subobject_type::mesh_shader:
		case pipeline_subobject_type::raygen_shader:
		case pipeline_subobject_type::any_hit_shader:
		case pipeline_subobject_type::closest_hit_shader:
		case pipeline_subobject_type::miss_shader:
		case pipeline_subobject_type::intersection_shader:
		case pipeline_subobject_type::callable_shader:
			replaced_stages |= load_shader_code_crosire(device_type, *static_cast<shader_desc*>(subobjects[i].data), s_data_to_delete);
			break;
		}
	}

	// Return whether any shader code was replaced
	return replaced_stages;
}

// *******************************************************************************************************
// on_after_create_pipeline() : clear data used to replace shader code in pipelines
//
static void on_after_create_pipeline(device*, pipeline_layout, uint32_t, const pipeline_subobject*, pipeline)
{
	// Free the memory allocated in the 'load_shader_code' call above
	s_data_to_delete.clear();
}


// *******************************************************************************************************
// on_init_pipeline_layout() : called once, to create the DX11 layout to use in push_constant
//
static void on_init_pipeline_layout(reshade::api::device* device, const uint32_t paramCount, const reshade::api::pipeline_layout_param* params, reshade::api::pipeline_layout layout)
{

	std::stringstream s;

	// generate data for constant_buffer or shader_resource_view
	for (uint32_t paramIndex = 0; paramIndex < paramCount; ++paramIndex) {
		// auto param = params[paramIndex];
		reshade::api::pipeline_layout_param param = params[paramIndex];


		//log infos
		log_init_pipeline_params(paramCount, params, layout,paramIndex, param);

		if (param.push_descriptors.type == descriptor_type::constant_buffer)
		{
			//index should be 2 for CB in DX11, but let's get it dynamically. Finally Not used...
			shared_data.CBIndex = paramIndex;

			// create pipeline layout for injecting VREM parameters in CB CBINDEX
			create_modified_CB_layout(device, CBINDEX, "VREM Cbuffer", MOD_CB_NB);
			
			// create pipeline layout for injecting CperFrame parameters in CB CPERFRAME_INDEX
			create_modified_CB_layout(device, CPERFRAME_INDEX, "CperFrame", CPERFRAME_CB_NB);

			// create pipeline layout for injecting def_uniforms parameters in CB DEF_UNIFORMS_INDEX
			// create_modified_CB_layout(device, DEF_UNIFORMS_INDEX, "def_uniforms", DEF_UNIFORMS_CB_NB);
		}

		else if (param.push_descriptors.type == descriptor_type::shader_resource_view)
		{
			// store the index of Ressource View in the pipeline (not used)
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

			if (result)  log_create_RVlayout();
			else log_error_creating_RVlayout();
		}
	}
}

// ***********************************************************************************************************************
// on_init_pipeline() : called once per pipeline, used to load new shader code in cloned pipelines, initialize the shader handling process and set some flags 
// create a new pipeline for shaders which need to be replaced 
static void on_init_pipeline(device* device, pipeline_layout layout, uint32_t subobjectCount, const pipeline_subobject* subobjects, pipeline pipelineHandle)
{

	// It is assumed all worthly shaders are in pipeline of only 1 object in DCS
	// only few shaders are to be modded
	for (uint32_t i = 0; i < subobjectCount; i++)
	{
		//if (subobjectCount == 1)
		{
			switch (subobjects[i].type)
			{
			case pipeline_subobject_type::vertex_shader:
			case pipeline_subobject_type::hull_shader:
			case pipeline_subobject_type::domain_shader:
			case pipeline_subobject_type::geometry_shader:
			case pipeline_subobject_type::pixel_shader:
			case pipeline_subobject_type::compute_shader:
			case pipeline_subobject_type::amplification_shader:
			case pipeline_subobject_type::mesh_shader:
			case pipeline_subobject_type::raygen_shader:
			case pipeline_subobject_type::any_hit_shader:
			case pipeline_subobject_type::closest_hit_shader:
			case pipeline_subobject_type::miss_shader:
			case pipeline_subobject_type::intersection_shader:
			case pipeline_subobject_type::callable_shader:
				//compute has and see if it is declared in shader mod list
				uint32_t hash = calculateShaderHash(subobjects[i].data);

				// auto it = shaders_by_hash.find(hash);
				std::unordered_map<uint32_t, Shader_Definition>::iterator it = pipeline_by_hash.find(hash);

				if (it != pipeline_by_hash.end()) {
					// shader is to be handled
					// add the shader entry in the map by pipeline handle

					//log
					log_init_pipeline(pipelineHandle, layout, subobjectCount, subobjects, i, hash, it);

					//create the entry for handling shader by pipeline instead of Hash
					Shader_Definition newShader(it->second.action, it->second.feature, it->second.replace_filename, it->second.draw_count);
					newShader.hash = hash;

					if (it->second.action & action_replace_bind)
					{
						// either replace the shader code or clone the existing pipeline and load the modded shader in it
						//load shader code
						bool status = load_shader_code(shader_code, it->second.replace_filename);
						if (!status) {
							// log error
							log_shader_code_error(pipelineHandle, hash, it);
						}
						else
						{
							// no error, clone pipeline
							//keep hash for debug messages
							newShader.hash = hash;
							// if not done, clone the pipeline to have a new version with fixed color for PS
							clone_pipeline(device, layout, subobjectCount, subobjects, pipelineHandle, shader_code, &newShader);
						}
					}
					// setup some global variables according to the feature
					if (it->second.action & action_identify)
					{
						/*
						// moved to bind as it is not working when VR is set but HMD is off
						if (it->second.feature == Feature::VRMode)
						{
							shared_data.cb_inject_values.VRMode = 1.0;
						}

						// identify if cockpit VS is used
						if (it->second.feature == Feature::mapMode)
						{
							shared_data.cb_inject_values.mapMode = 0.0;
						}
						*/
					}

					// store new shader to re use it later 
					pipeline_by_handle.emplace(pipelineHandle.handle, newShader);
				}
				break;
			}
		}
	}
	// else log_invalid_subobjectCount(pipelineHandle);
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

	/*
	if ((debug_flag && flag_capture))
	{
		std::stringstream s;

		s << "*** bind_pipeline(" << to_string(stages) << " :  , pipelineHandle: " << (void*)pipelineHandle.handle << ")";
		reshade::log::message(reshade::log::level::error, s.str().c_str());

	}
	*/

	if (to_process)
	{
		
		//find if the pipeline is related to a shader of the mod
		std::unordered_map<uint64_t, Shader_Definition>::iterator it;
		it = pipeline_by_handle.find(pipelineHandle.handle);

		if (it != pipeline_by_handle.end()) {

			// debug message
			log_pipeline_to_process(pipelineHandle, it);
			
			if (it->second.action & action_count)
			{
				// increment the associated counter (not working ? not used)
				
			}

			int count_displayVS = shared_data.count_display - 1;
			//if (it->second.feature == Feature::Label) count_displayVS++;

			if (it->second.action & action_injectText)
			{
				// inject texture using push_descriptor() if things has been initialized => draw index is > -1

				if (( it->second.feature == Feature::Global || it->second.feature == Feature::Label) && count_displayVS > -1 && shared_data.depthStencil_copy_started)
				{
		
					if (it->second.feature == Feature::Label) count_displayVS++;
					// stencil depth textures in shaders for color change and label masking 
					if (shared_data.depth_view[count_displayVS].created && shared_data.stencil_view[count_displayVS].created)
					{

						// push the texture for depth and stencil, descriptor initialized in copy_texture()
						//depth
						shared_data.update.binding = 0;
						shared_data.update.descriptors = &shared_data.depth_view[count_displayVS].texresource_view;
						commandList->push_descriptors(reshade::api::shader_stage::pixel, shared_data.saved_pipeline_layout_RV, 0, shared_data.update);

						//stencil
						shared_data.update.binding = 1; // t4 as 3 is defined in pipeline_layout
						shared_data.update.descriptors = &shared_data.stencil_view[count_displayVS].texresource_view;;
						commandList->push_descriptors(reshade::api::shader_stage::pixel, shared_data.saved_pipeline_layout_RV, 0, shared_data.update);

						// log infos
						log_texture_injected("depthStencil", count_displayVS);
					}
				}

				// inject texture for NS430 if feature activated
				// if (it->second.feature == Feature::NS430  && shared_data.count_display > -1 && shared_data.NS430_copy_started && shared_data.cb_inject_values.NS430Flag)
				if (it->second.feature == Feature::NS430 && shared_data.NS430_copy_started && shared_data.cb_inject_values.NS430Flag)
				{
					// NS430 texture in VR menu GUI aera shaders as only 1 copy is done per frame, use 0 for all draws
					if (shared_data.NS430_view[0].created)
					{
						// push the texture for NS430, descriptor initialized in copy_texture()
						shared_data.update.binding = 2; // t5
						shared_data.update.descriptors = &shared_data.NS430_view[0].texresource_view;
						commandList->push_descriptors(reshade::api::shader_stage::pixel, shared_data.saved_pipeline_layout_RV, 0, shared_data.update);

						// log infos
						log_texture_injected("NS430", count_displayVS);
					}
				}

			}	

			
			if (it->second.action & action_injectCB)
			{
				// inject constant buffer other than the one containing VREM setting
				
				//CPERFRAME/haze for global illumination :  need to modify value for haze but set orgi. value for reflection
				if (it->second.feature == Feature::GetStencil && shared_data.CB_copied[CPERFRAME_CB_NB])
				{

					//modify value for Haze
					shared_data.dest_CB_array[CPERFRAME_CB_NB][FOG_INDEX] = shared_data.orig_values[CPERFRAME_CB_NB][GATMINTENSITY_SAVE] * shared_data.cb_inject_values.hazeReduction;
					// other value
					shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_X] = shared_data.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_X_SAVE];
					shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_Y] = shared_data.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_Y_SAVE];
					// use push constant() to push CPerFrame 
					// pipeline_layout for CB initialized in init_pipeline() once for all
					commandList->push_constants(
						shader_stage::all,
						shared_data.saved_pipeline_layout_CB[CPERFRAME_CB_NB],
						0,
						0, // can not injecting only the haze value to be updated (so first = FOG_INDEX) because it is making the game crash...
						CPERFRAME_SIZE,
						&shared_data.dest_CB_array[CPERFRAME_CB_NB]
					);

					log_CB_injected("CPerFrame updated for fog, GCOCKPITIBL default");

					// last_replaced_shader = pipelineHandle.handle;
					last_feature = it->second.feature;

				}

				//CPERFRAME/haze for other shaders :  need to keep orig. value
				if (it->second.feature == Feature::Sky && shared_data.CB_copied[CPERFRAME_CB_NB])
				{

					//modify value for Haze
					shared_data.dest_CB_array[CPERFRAME_CB_NB][FOG_INDEX] = shared_data.orig_values[CPERFRAME_CB_NB][GATMINTENSITY_SAVE];
					//other value
					shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_X] = shared_data.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_X_SAVE] ;
					shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_Y] = shared_data.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_Y_SAVE] ;

					// use push constant() to push CPerFrame 
					// pipeline_layout for CB initialized in init_pipeline() once for all
					commandList->push_constants(
						shader_stage::all,
						shared_data.saved_pipeline_layout_CB[CPERFRAME_CB_NB],
						0,
						0, 
						CPERFRAME_SIZE,
						&shared_data.dest_CB_array[CPERFRAME_CB_NB]
					);

					log_CB_injected("CPerFrame original");

					// last_replaced_shader = pipelineHandle.handle;
					last_feature = it->second.feature;

				}

				//CPERFRAME/reflect for instrument
				if (it->second.feature == Feature::NoReflect && shared_data.CB_copied[CPERFRAME_CB_NB])
				{

					//modify value
					shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_X] = shared_data.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_X_SAVE] * shared_data.cb_inject_values.gCockpitIBL;
					shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_Y] = shared_data.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_Y_SAVE] * shared_data.cb_inject_values.gCockpitIBL;

					// use push constant() to push CPerFrame 
					// pipeline_layout for CB initialized in init_pipeline() once for all
					commandList->push_constants(
						shader_stage::all,
						shared_data.saved_pipeline_layout_CB[CPERFRAME_CB_NB],
						0,
						0,
						CPERFRAME_SIZE,
						&shared_data.dest_CB_array[CPERFRAME_CB_NB]
					);
					log_CB_injected("CPerFrame for GCOCKPITIBL");

					// last_replaced_shader = pipelineHandle.handle;
					last_feature = it->second.feature;
				}
				
			}
			
		
			// shader is to be handled
			// if (it->second.action & action_replace)
			if (it->second.action & action_replace || it->second.action & action_replace_bind )
			// ugly workaround
			//if (it->second.action & action_replace || it->second.action & action_replace_bind || (it->second.action & action_replace && it->second.feature == Feature::mapMode) )
			{

				// optimization : do not push CB if same shader is replaced again and no "count" action used for the shader 
				// possible because CB13 is not used by the game
				// if (last_replaced_shader != pipelineHandle.handle || it->second.action & action_count)
				if (last_feature != it->second.feature || it->second.action & action_count || shared_data.disable_optimisation)
				{
					// use push constant() to push the mod parameter in CB13,a sit is assumed a replaced shader will need mod parameters
					// pipeline_layout for CB initialized in init_pipeline() once for all
					commandList->push_constants(
						shader_stage::all,
						shared_data.saved_pipeline_layout_CB[MOD_CB_NB],
						0,
						0,
						CBSIZE,
						&shared_data.cb_inject_values
					);
					log_CB_injected("VREM CB");
					
				}
				if (it->second.action & action_replace_bind)
				{
					// shader is to be replaced by the new one created in on_Init_Pipeline
					commandList->bind_pipeline(stages, it->second.substitute_pipeline);

					// log infos
					log_pipeline_replaced(pipelineHandle, it);
				}
			}
			
			if (it->second.action & action_skip && shared_data.cb_inject_values.NS430Flag && it->second.feature == Feature::NS430)
			{
				// shader is to be skipped, setup a flag for next draw
				do_not_draw = true;
			}

			// setup variables regarding the action
			if (it->second.action & action_log)
			{
				/*
				// VS for A10C instrum : get CB values
				if (it->second.feature == Feature::NoReflect)
				{
					shared_data.track_for_CB[DEF_UNIFORMS_CB_NB] = true;
					log_start_monitor("def_uniforms CB");
				}
				*/
				
				// VS for illum : trigger logging of resources (eg texture) or other topics (eg count calls)
				if (it->second.feature == Feature::GetStencil)
				{	
					// engage tracking shader_resource_view in push_descriptors() to get depthStencil 
					shared_data.track_for_depthStencil = true;

					// log infos
					log_start_monitor("Depth Stencil");
				}

				// VS for NS430 : trigger logging of resources (eg texture) or other topics (eg count calls)
				if (it->second.feature == Feature::NS430)
				{
					// engage tracking shader_resource_view in push_descriptors() to get depthStencil 
					shared_data.track_for_NS430 = true;

					// log infos
					log_start_monitor("NS430");
				}

				// set flag for tracking render target if feature enabled and not in 2D
				// if (it->second.feature == Feature::Effects && shared_data.effects_feature && shared_data.count_draw > 1)
				// TODO test to make it work in 2D
				if (it->second.feature == Feature::Effects && (shared_data.effects_feature || shared_data.texture_needed))
				{
				
					// if (shared_data.render_target_view[shared_data.count_display].created)
					{
						// TODO : check if needed shared_data.render_effect = false;
						log_start_monitor("     shared_data.render_target_view[shared_data.count_display].created is true, Render target tracked");
						shared_data.track_for_render_target = true;
						// log infos
						log_start_monitor("Render target");


					}
					/* else
					{
						shared_data.track_for_render_target = false;
					} */
				}

				// PS for GUI : set flag
				if (it->second.feature == Feature::GUI)
				{
					shared_data.cb_inject_values.GUItodraw = 1.0;

					// log infos
					log_start_monitor("GUItodraw");
				}

				// PS for NS430 : reset flag
				if (it->second.feature == Feature::NS430)
				{
					shared_data.cb_inject_values.GUItodraw = 0.0;

					// log infos
					log_clear_action_log("NS430");
				}
				

				// PS global color : increase draw count & GUI flag (to avoid MFD)
				// setup flag to launch render effect and stop tracking render target
				
				// if (it->second.feature == Feature::Global)
				if (it->second.feature == Feature::VS_global2)
				{
					// handle the case where the Ps is called 2 time consecutivelly because of mirror view
					if (shared_data.last_feature != Feature::Global)
					{
						
						// if texure has been copied previously, increase draw count, otherwise do nothing, to avoid counting shader calls for MFD rendering
						if (shared_data.depthStencil_copy_started)
						{

							shared_data.count_display += 1;
							// log max of count_display to enable or not features for VR / Quad view
							shared_data.count_draw = max(shared_data.count_draw, shared_data.count_display);
							// it's stupid but I'm too lazy to change code now..
							shared_data.cb_inject_values.count_display = shared_data.count_display;
							// fix for quad view
							// if (shared_data.count_display == 2) shared_data.mirror_VR = 1;
							// if (shared_data.count_display == 3) shared_data.mirror_VR = 0;

							// log infos
							log_increase_count_display();


							if (shared_data.effects_feature)
							{
								// handle effects : setup flag for draw
								shared_data.render_effect = true;
								shared_data.track_for_render_target = false;

								// log infos
								log_effect_requested();
							}
						}
						else
						{
							// log infos
							log_not_increase_draw_count();
						}
					}

					// set up draw flag to avoid push_constant() doing effect before draw (it will be overwritten by the PS)
					shared_data.draw_passed = false;
				}
			}

			if (it->second.action & action_identify)
			{
				// setup some global variables according to the feature (if not possible to do it on init_pipeline)

				if (it->second.feature == Feature::mapMode)
				{
					shared_data.cb_inject_values.mapMode = 0.0;
				}

				// PS for mirror view : setup VR mode
				if (it->second.feature == Feature::VRMode)
				{
					shared_data.cb_inject_values.VRMode = 1.0;
					// identify which view was used before mirror view
					// defaut
					shared_data.mirror_VR = 0;
					// secure only 1 and 2 view processed
					if (shared_data.count_display == 1) shared_data.mirror_VR = 0;
					if (shared_data.count_display == 2) shared_data.mirror_VR = 1;
					log_mirror_view();
				}

			}
			shared_data.last_feature = it->second.feature;

		}
	}

}

// *******************************************************************************************************
// on_push_descriptors() : to be monitored in order to copy texture and engage effect
// called a lot !
static void on_push_descriptors(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
	
	// display_to_use = 0 => outer left, 1 = outer right, 2 = Inner left, 3 = inner right.
	short int display_to_use = shared_data.count_display - 1;
	
	// render effect part
	// do not engage effect if option not selected and not in cockpit
	if (shared_data.render_effect && shared_data.effects_feature && !shared_data.cb_inject_values.mapMode && shared_data.draw_passed)
	{
		// engage effect for each call of global pixel shader
		// ensure to wait next good context after rendering
		shared_data.render_effect = false;

		// do not engage effect if render target view is not identified 
		if (shared_data.render_target_view[display_to_use].created)
		{			
			//texture needed defined if at least 1 shader is using DEPTH or STENCIL, computed when reading technique list
			// if (shared_data.texture_needed && !shared_data.render_target_view[display_to_use].depth_exported_for_technique)
			if (shared_data.texture_needed)
			{
				// export DEPTH and STENCIL once for all effects (must be done in 2D too !!)
				// update DEPTH texture
				// shared_data.render_target_view[display_to_use].depth_exported_for_technique = true;
				shared_data.runtime->update_texture_bindings("DEPTH", shared_data.depth_view[display_to_use].texresource_view, shared_data.depth_view[display_to_use].texresource_view);
				// update STENCIL texture
				shared_data.runtime->update_texture_bindings("STENCIL", shared_data.stencil_view[display_to_use].texresource_view, shared_data.stencil_view[display_to_use].texresource_view);
				log_export_texture(display_to_use);
			}
		
			// render all activated techniques if not 2D mirror or in 2D (reshade is already rendering the effect) 
			// if (shared_data.cb_inject_values.VRMode)
			{
				
				for (int i = 0; i < shared_data.technique_vector.size(); ++i)
				{
					
					bool buffer_exported = false;
					
					// render if QV target are relevant for the technique 
					if ((display_to_use <= 1 && shared_data.technique_vector[i].quad_view_target == QVOUTER) ||
						(display_to_use > 1 && shared_data.technique_vector[i].quad_view_target == QVINNER) ||
						(shared_data.technique_vector[i].quad_view_target == QVALL)
						)
					{
						// send mask and global preprocessor definition
						if (!shared_data.render_target_view[display_to_use].compiled)
						{
							
							if (!buffer_exported)
							{
								// push render target resol for shader re compilation 
								int check = 0;
								// check += default_preprocessor(shared_data.runtime, "MSAAX", shared_data.cb_inject_values.AAxFactor, true, display_to_use);
								// check += default_preprocessor(shared_data.runtime, "MSAAY", shared_data.cb_inject_values.AAyFactor, true, display_to_use);
								check += default_preprocessor(shared_data.runtime, "MSAAX", shared_data.MSAAxfactor, true, display_to_use);
								check += default_preprocessor(shared_data.runtime, "MSAAY", shared_data.MSAAyfactor, true, display_to_use);

								// if (display_to_use <= 1)
								{
									check += default_preprocessor(shared_data.runtime, "BUFFER_WIDTH", shared_data.render_target_view[display_to_use].width, true, display_to_use);
									check += default_preprocessor(shared_data.runtime, "BUFFER_HEIGHT", shared_data.render_target_view[display_to_use].height, true, display_to_use);
									check += default_preprocessor(shared_data.runtime, "BUFFER_RCP_WIDTH", 1.0/shared_data.render_target_view[display_to_use].width, true, display_to_use);
									check += default_preprocessor(shared_data.runtime, "BUFFER_RCP_HEIGHT", 1.0 / shared_data.render_target_view[display_to_use].height, true, display_to_use);
								}
								/*
								else
								{
									check += default_preprocessor(shared_data.runtime, "BUFFER_WIDTH_QVIN", shared_data.render_target_view[display_to_use].width, false, display_to_use);
									check += default_preprocessor(shared_data.runtime, "BUFFER_HEIGHT_QVIN", shared_data.render_target_view[display_to_use].height, false, display_to_use);
									check += default_preprocessor(shared_data.runtime, "BUFFER_RCP_WIDTH_QVIN", 1.0 / shared_data.render_target_view[display_to_use].width, false, display_to_use);
									check += default_preprocessor(shared_data.runtime, "BUFFER_RCP_HEIGHT_QVIN", 1.0 / shared_data.render_target_view[display_to_use].height, false, display_to_use);
								}*/

							}

							// setup a timeout in order to wait first usage compilation
							std::this_thread::sleep_for(std::chrono::high_resolution_clock::duration(std::chrono::milliseconds(shared_data.compil_delay)));
							shared_data.render_target_view[display_to_use].compiled = true;
							log_wait();
						}

						// render all activated techniques if not 2D mirror or in 2D (reshade is already rendering the effect) 
						if (shared_data.cb_inject_values.VRMode)
						{
							// engage effect (will be compiled at the first launch)
							shared_data.runtime->render_technique(shared_data.technique_vector[i].technique, cmd_list, shared_data.render_target_view[display_to_use].texresource_view, shared_data.render_target_view[display_to_use].texresource_view);
							log_effect(shared_data.technique_vector[i], cmd_list, shared_data.render_target_view[display_to_use].texresource_view);
						}
					}
				}
			}

			// push back the outer texture instead of inner or wrong eye for effect in mirror view
			if (shared_data.mirror_VR != -1 && shared_data.texture_needed && !shared_data.VRonly_technique)
			{
				// update DEPTH texture 
				shared_data.runtime->update_texture_bindings("DEPTH", shared_data.depth_view[shared_data.mirror_VR].texresource_view, shared_data.depth_view[shared_data.mirror_VR].texresource_view);
				// update STENCIL texture
				shared_data.runtime->update_texture_bindings("STENCIL", shared_data.stencil_view[shared_data.mirror_VR].texresource_view, shared_data.stencil_view[shared_data.mirror_VR].texresource_view);
				log_export_texture(-1);

			}

		}
	}

	//handle only shader_resource_view when needed
	// handle depthStencil
	if (shared_data.track_for_depthStencil && update.type == descriptor_type::shader_resource_view &&  stages == shader_stage::pixel && update.count == 6)
	{
		
		//log infos
		log_push_descriptor(stages, layout, param_index, update);

		// copy depthStencil texture into shared_data
		bool status = copy_depthStencil(cmd_list, stages, layout, param_index, update);

		// stop tracking
		// shared_data.track_for_depthStencil = false;
	}

	// handle NS430
	if (shared_data.track_for_NS430 && update.type == descriptor_type::shader_resource_view && stages == shader_stage::pixel && update.count == 1)
	{
		//log infos
		log_push_descriptor(stages, layout, param_index, update);

		// Copy only if texture has good resolution
		device* dev = cmd_list->get_device();
		resource_view NS430_rv = static_cast<const reshade::api::resource_view*>(update.descriptors)[0];
		resource NS430_resource = dev->get_resource_from_view(NS430_rv);
		resource_desc NS430_res_desc = dev->get_resource_desc(NS430_resource);


		if (NS430_res_desc.texture.width == NS430_textSizeX && NS430_res_desc.texture.height == NS430_textSizeY ) 
		{	
			// copy NS430 texture into shared_data (only done once per frame)
			bool status = copy_NS430_text(cmd_list, stages, layout, param_index, update);

			// stop tracking
			shared_data.track_for_NS430 = false;
		}

	}

	//handle CB 
	// CB cPerFrame is generated once at the beginning of the frame, it is not needed to use a dedicated shader to track the push_descriptor command
	//only if needed
	// if (shared_data.cb_inject_values.hazeReduction != 1.0 && shared_data.misc_feature && !shared_data.CPerFrame_copied)
	if ( (shared_data.cb_inject_values.hazeReduction != 1.0 || shared_data.cb_inject_values.gCockpitIBL !=1.0)  && shared_data.misc_feature)
	{
		
		
		if (update.type == descriptor_type::constant_buffer && update.binding == CPERFRAME_INDEX && update.count == 1 && stages == shader_stage::pixel)
		{
			
			/*
			bool error = read_constant_buffer(cmd_list, update, "CPerFrame", 0, shared_data.dest_CB_CPerFrame, CPERFRAME_SIZE);
			if (!error)
			{
				//update gAtmIntensity
				shared_data.dest_CB_CPerFrame[FOG_INDEX] = shared_data.dest_CB_CPerFrame[FOG_INDEX] * shared_data.cb_inject_values.hazeReduction;
				shared_data.CPerFrame_copied = true;
			}
			*/
			bool error = read_constant_buffer(cmd_list, update, "CPerFrame", 0, shared_data.dest_CB_array[CPERFRAME_CB_NB], CPERFRAME_SIZE);
			if (!error)
			{
			
				// copy original value for gAtmIntensity
				shared_data.orig_values[CPERFRAME_CB_NB][GATMINTENSITY_SAVE] = shared_data.dest_CB_array[CPERFRAME_CB_NB][FOG_INDEX];

				// copy original value for gCockpitIBL.xy
				shared_data.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_X_SAVE] = shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_X];
				shared_data.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_Y_SAVE] = shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_Y];

				/*
				// update value for dynamic reflexion
				shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_X] = shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_X] * shared_data.cb_inject_values.gCockpitIBL;
				shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_Y] = shared_data.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_Y] * shared_data.cb_inject_values.gCockpitIBL;
				*/

				shared_data.CB_copied[CPERFRAME_CB_NB] = true;
			}

		}
	}

	// copy def_uniform
	/*
	if (shared_data.track_for_CB[DEF_UNIFORMS_CB_NB] == true && update.type == descriptor_type::constant_buffer && update.binding == DEF_UNIFORMS_INDEX && update.count == 1 && stages == shader_stage::pixel)
	{
		bool error = read_constant_buffer(cmd_list, update, "def_uniforms", 0, shared_data.dest_CB_array[DEF_UNIFORMS_CB_NB], DEF_UNIFORM_SIZE);
		if (!error)
		{
			// copy original value for opacityValue 
			shared_data.orig_values[DEF_UNIFORMS_CB_NB][OPACITY_SAVE] = shared_data.dest_CB_array[DEF_UNIFORMS_CB_NB][OPACITY_INDEX];
			shared_data.CB_copied[DEF_UNIFORMS_CB_NB] = true;
			
		}
		shared_data.track_for_CB[DEF_UNIFORMS_CB_NB] == false;
	}
	*/
}

//*******************************************************************************************************
// on_bind_render_targets_and_depth_stencil() : track render target
static void on_bind_render_targets_and_depth_stencil(command_list *cmd_list, uint32_t count, const resource_view* rtvs, resource_view dsv)
{
	
	// copy render target if tracking
// BUG: using this line with openknweeboard is blocking game !!!!
// 	if (shared_data.track_for_render_target && shared_data.count_display > -1 && !shared_data.cb_inject_values.mapMode && count > 0 && (shared_data.effects_feature || shared_data.texture_needed))

	if (shared_data.track_for_render_target && shared_data.count_display > -1 && !shared_data.cb_inject_values.mapMode && count > 0 && (shared_data.effects_feature ))
	{
		
		// only first render target view to get
		shared_data.render_target_view[shared_data.count_display].texresource_view = rtvs[0];
		shared_data.render_target_view[shared_data.count_display].created = true;

		// get resolution of render target
		device* dev = cmd_list->get_device();
		resource scr_resource = dev->get_resource_from_view(rtvs[0]);
		resource_desc src_resource_desc = dev->get_resource_desc(scr_resource);

		shared_data.render_target_view[shared_data.count_display].width = src_resource_desc.texture.width;
		shared_data.render_target_view[shared_data.count_display].height = src_resource_desc.texture.height;
		
		log_renderTarget_depth(count, rtvs, dsv, cmd_list);
	}

}
//*******************************************************************************************************
// clear tracking flags to avoid tracking resources if push_descriptor did not detect it...
static void clear_tracking_flags()
{

	if (shared_data.track_for_depthStencil || shared_data.track_for_NS430 || do_not_draw)
	{
		log_reset_tracking();
		shared_data.track_for_depthStencil = false;
		shared_data.track_for_NS430 = false;
		do_not_draw = false;
	}

	// shared_data.track_for_CB[DEF_UNIFORMS_CB_NB] = false;

	shared_data.draw_passed = true;
}

// *******************************************************************************************************
// On draw* : skip draw
//
static bool on_draw(command_list* commandList, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{

	bool skip = false;
	if (do_not_draw) skip = true;

	// log 
	log_ondraw(vertex_count, instance_count, first_vertex,first_instance);

	// clear tracking flags
	clear_tracking_flags();

	return skip;
}

// On draw* : skip draw
static bool on_draw_indexed(command_list* commandList, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
{

	bool skip = false;
	if (do_not_draw) skip = true;

	// log 
	log_on_draw_indexed(index_count, instance_count, first_index, vertex_offset, first_instance);

	// clear trackign flags
	clear_tracking_flags();

	return skip;
}

// On draw* : skip draw
static bool on_drawOrDispatch_indirect(command_list* commandList, indirect_command type, resource buffer, uint64_t offset, uint32_t draw_count, uint32_t stride)
{

	bool skip = false;
	if (do_not_draw) skip = true;

	// log 
	log_on_drawOrDispatch_indirect(type, buffer, offset, draw_count, stride);

	// clear trackign flags
	clear_tracking_flags();

	return skip;

}


//*******************************************************************************************************
// onReshadeReloadedEffects() : initialize technique list

static void on_reshade_reloaded_effects(effect_runtime* runtime)
{

	if (shared_data.effects_feature)
	{
		enumerateTechniques(runtime);
	}

	//initialize pre process variable (if needed)
	//init_preprocess(runtime);


}

//*******************************************************************************************************
// on_reshade_overlay() : initialize technique list

static void on_reshade_overlay(effect_runtime* runtime)
{
	
	//initialize pre process variable (if needed)
	init_preprocess(runtime);

}


// *******************************************************************************************************
// Cleanup : delete created things
//
//delete created pipeline (maybe not needed ?)
static void on_destroy_pipeline(
	reshade::api::device* device,
	reshade::api::pipeline pipeline) 
{
	
	std::unordered_map<uint64_t, Shader_Definition>::iterator it;
	it = pipeline_by_handle.find(pipeline.handle);

	if (it != pipeline_by_handle.end()) {
		device->destroy_pipeline(it->second.substitute_pipeline);
		log_destroy_pipeline(pipeline, it);
	}

	//delete resource and resource view if created (done here to get a reshade entry point for destroy*)
	if (shared_data.depthStencil_res[0].created)
	{
		shared_data.depthStencil_res[0].created = false;
		for (int i = 0; i < 4; i++)
		{
			device->destroy_resource(shared_data.depthStencil_res[i].texresource);
			device->destroy_resource_view(shared_data.depth_view[i].texresource_view);
			device->destroy_resource_view(shared_data.stencil_view[i].texresource_view);
		}
	}
}

//delete vectors and maps
static void clean_up2()
{

	shader_code.clear(); 
	pipeline_by_handle.clear();
	// texture_resource_by_handle.clear();
	pipeline_by_hash.clear();

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
					
			//add reshade install dir. in filenames
			WCHAR buf[MAX_PATH];
			const std::filesystem::path dllPath = GetModuleFileNameW(nullptr, buf, ARRAYSIZE(buf)) ? buf : std::filesystem::path();		// <installpath>/shadertoggler.addon64
			const std::filesystem::path basePath = dllPath.parent_path();																// <installpath>
			const std::string& settings_FileName = settings_iniFileName;
			settings_iniFileName = (basePath / settings_FileName).string();
			const std::string& settings_FileName2 = technique_iniFileName;
			technique_iniFileName = (basePath / settings_FileName2).string();
		
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
			reshade::register_event<reshade::addon_event::push_descriptors>(on_push_descriptors);
			reshade::register_event<reshade::addon_event::create_pipeline>(on_create_pipeline);
			reshade::register_event<reshade::addon_event::init_pipeline>(on_after_create_pipeline);

			reshade::register_event<reshade::addon_event::bind_render_targets_and_depth_stencil>(on_bind_render_targets_and_depth_stencil);

			reshade::register_event<reshade::addon_event::reshade_present>(on_present);


			reshade::register_event < reshade::addon_event::reshade_overlay> (on_reshade_overlay);
			reshade::register_event< reshade::addon_event::reshade_reloaded_effects>(on_reshade_reloaded_effects);
			reshade::register_event<reshade::addon_event::reshade_set_technique_state>(onReshadeSetTechniqueState);
	
			// setup GUI
			reshade::register_overlay(nullptr, &displaySettings);

			// load stored settings and init shared variables
			load_setting_IniFile();

			// init mod features regarding the selection of GUI
			init_mod_features();

		}
		break;
	case DLL_PROCESS_DETACH:

		reshade::unregister_event<reshade::addon_event::init_pipeline>(on_init_pipeline);
		reshade::unregister_event<reshade::addon_event::bind_pipeline>(on_bind_pipeline);
		reshade::unregister_event<reshade::addon_event::init_pipeline_layout>(on_init_pipeline_layout);
		reshade::unregister_event<reshade::addon_event::draw>(on_draw);
		reshade::unregister_event<reshade::addon_event::draw_indexed>(on_draw_indexed);
		reshade::unregister_event<reshade::addon_event::draw_or_dispatch_indirect>(on_drawOrDispatch_indirect);
		reshade::unregister_event<reshade::addon_event::push_descriptors>(on_push_descriptors);
		reshade::unregister_event<reshade::addon_event::create_pipeline>(on_create_pipeline);
		reshade::unregister_event<reshade::addon_event::init_pipeline>(on_after_create_pipeline);
		reshade::unregister_event<reshade::addon_event::bind_render_targets_and_depth_stencil>(on_bind_render_targets_and_depth_stencil);

		reshade::unregister_event< reshade::addon_event::reshade_reloaded_effects>(on_reshade_reloaded_effects);
		reshade::unregister_event<reshade::addon_event::reshade_set_technique_state>(onReshadeSetTechniqueState);
		
		reshade::unregister_event<reshade::addon_event::reshade_present>(on_present);

		clean_up2();
		reshade::unregister_overlay(nullptr, &displaySettings);

		reshade::unregister_addon(hModule);
		break;
	}

	return TRUE;
}



