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
extern "C" __declspec(dllexport) const char *DESCRIPTION = "DCS mod to enhance VR in DCS - v2.0";

// ***********************************************************************************************************************
// definition of shader of the mod
std::unordered_map<uint32_t, Shader_Definition> shaders_by_hash =
{
	// ** fix for rotor **
	{ 0xC0CC8D69, Shader_Definition(action_replace, Feature::Rotor, L"AH64_rotorPS.cso", 0) },
	{ 0x460B6238, Shader_Definition(action_replace, Feature::Rotor, L"AH64_rotor2PS.cso", 0) },
	{ 0x492932EA, Shader_Definition(action_replace, Feature::Rotor, L"UH1_rotorPS.cso", 0) },
	// ** fix for IHADSS **
	{ 0x2D713734, Shader_Definition(action_replace, Feature::IHADSS, L"IHADSS_PNVS_PS.cso", 0) },
	{ 0x6AA19B8F, Shader_Definition(action_replace, Feature::IHADSS, L"IHADSS_PS.cso", 0) },
	{ 0x45E221A9, Shader_Definition(action_replace, Feature::IHADSS, L"IHADSS_VS.cso", 0) },
	// ** label masking and color/sharpen/deband **
	// to start spying texture for depthStencil (Vs associated with global illumination PS)
	{ 0x4DDC4917, Shader_Definition(action_log, Feature::GetStencil, L"", 0) },
	// global PS for all changes
	{ 0xBAF1E52F, Shader_Definition(action_replace | action_injectText | action_log, Feature::Global, L"global_PS_2.cso", 0) },
	// Label PS 
	{ 0x6CEA1C47, Shader_Definition(action_replace | action_injectText, Feature::Label , L"labels_PS.cso", 0) },
	// ** NS430 **
	// Obsolete - to start spying texture for screen texture and disable frame (Vs associated with NS430 screen PS below)
	{ 0x52C97365, Shader_Definition(action_replace, Feature::NS430, L"VR_GUI_MFD_VS.cso", 0) },
	// to start spying texture for screen texture (Vs associated with NS430 screen EDF9F8DD for su25T&UH1, not same res. texture !)
	{ 0x8439C716, Shader_Definition(action_log, Feature::NS430, L"", 0) },
	// inject texture in global GUI and filter screen display (same shader for both)
	{ 0x99D562, Shader_Definition(action_replace | action_injectText, Feature::NS430 , L"VR_GUI_MFD_PS.cso", 0) },
	// disable NS430 frame, shared with some cockpit parts (can not be done by skip)
	{ 0xD391F41C, Shader_Definition(action_replace, Feature::NS430 , L"NS430__framePS.cso", 0) },
	// disable NS430 screen background (done in shader because shared with other objects than NS430)
	{ 0x6EF95548, Shader_Definition(action_replace, Feature::NS430, L"NS430_screen_back.cso", 0) },
	// to filter out call for GUI and MFD
	{ 0x55288581, Shader_Definition(action_log, Feature::GUI, L"", 0) },
	 
	// 
	//  ** identify game config **
	// to define if VR is active or not (2D mirror view of VR )
	{ 0x886E31F2, Shader_Definition(action_identify, Feature::VRMode, L"", 0) },
	// VS drawing cockpit parts to define if view is in welcome screen or map
	{ 0xA337E177, Shader_Definition(action_identify, Feature::mapMode, L"", 0) },
	//  ** haze control : illum PS: used for Haze control, to define which draw (eye + quad view) is current.  **
	{ 0x51302692, Shader_Definition(action_replace | action_identify, Feature::Haze, L"illumNoAA_PS.cso", 0) },
	{ 0x88AF23C6, Shader_Definition(action_replace | action_identify, Feature::HazeMSAA2x, L"illumMSAA2x_PS.cso", 0) },
	{ 0xA055EDE4, Shader_Definition(action_replace, Feature::Haze, L"illumMSAA2xB_PS.cso", 0) },
	//  ** A10C cockpit instrument **
	{ 0x4D8EB0B7, Shader_Definition(action_replace , Feature::NoReflect , L"A10C_instrument.cso", 0) },
	//  ** NVG **
	{ 0xE65FAB66, Shader_Definition(action_replace , Feature::NVG , L"NVG_extPS.cso", 0) }
};

// ***********************************************************************************************************************
//init shared variables
std::string settings_iniFileName = "DCS_VREM.ini";
//default value overwritten by setting file if exists
bool debug_flag;
struct global_shared shared_data;
std::unordered_map<uint32_t, Shader_Definition> shaders_by_handle;
std::unordered_map<uint32_t, resource> texture_resource_by_handle;

uint32_t last_replaced_shader = 0;

// ***********************************************************************************************************************
//init local variables
//to hanlde loading code for replaced shader
static thread_local std::vector<std::vector<uint8_t>> shader_code;
bool flag_capture = false;
bool do_not_draw = false;

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
			//index should be 2 for CB in DX11, but let's get it dynamically. Not used.
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

			//logs
			if (result) log_create_CBlayout();
			else log_error_creating_CBlayout();
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
			// auto it = shaders_by_hash.find(hash);
			std::unordered_map<uint32_t, Shader_Definition>::iterator it = shaders_by_hash.find(hash);

			if (it != shaders_by_hash.end()) {
				// shader is to be handled
				// add the shader entry in the map by pipeline handle

				//log
				log_init_pipeline(pipelineHandle, layout, subobjectCount, subobjects, hash, it);

				//create the entry for handling shader by pipeline instead of Hash
				Shader_Definition newShader(it->second.action, it->second.feature, it->second.replace_filename, it->second.draw_count);

				if (it->second.action & action_replace)
				{
					// clone the existing pipeline and load the modded shader in it
					bool status = load_shader_code(shader_code, it->second.replace_filename);
					if (!status) {
						// log error
						log_shader_code_error(pipelineHandle, hash, it);
					}
					else {
						//keep hash for debug messages
						newShader.hash = hash;
						// if not done, clone the pipeline to have a new version with fixed color for PS
						clone_pipeline(device, layout, subobjectCount, subobjects, pipelineHandle, shader_code, &newShader);

					}
				}

				if (it->second.action & action_identify)
				{
					// setup some global variables according to the feature
					if (it->second.feature == Feature::VRMode)
					{
						shared_data.cb_inject_values.VRMode = 1.0;
					}

					if (it->second.feature == Feature::mapMode)
					{
						shared_data.cb_inject_values.mapMode = 0.0;
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
			log_pipeline_to_process(pipelineHandle, it);
			
			if (it->second.action & action_count)
			{
				// increment the associated counter
				
			}
			
			if (it->second.action & action_injectText)
			{
				// inject texture using push_descriptor() if things has been initialized => draw index is > -1

				if (( it->second.feature == Feature::Global || it->second.feature == Feature::Label) && shared_data.count_display > -1 && shared_data.depthStencil_copy_started)
				{
					// stencil depth textures in shaders for color change and label masking 
					if (shared_data.depth_view[shared_data.count_display].created && shared_data.stencil_view[shared_data.count_display].created)
					{

						// push the texture for depth and stencil, descriptor initialized in copy_texture()
						//depth
						shared_data.update.binding = 0;
						shared_data.update.descriptors = &shared_data.depth_view[shared_data.count_display].texresource_view;
						commandList->push_descriptors(reshade::api::shader_stage::pixel, shared_data.saved_pipeline_layout_RV, 0, shared_data.update);

						//stencil
						shared_data.update.binding = 1; // t4 as 3 is defined in pipeline_layout
						shared_data.update.descriptors = &shared_data.stencil_view[shared_data.count_display].texresource_view;;
						commandList->push_descriptors(reshade::api::shader_stage::pixel, shared_data.saved_pipeline_layout_RV, 0, shared_data.update);

						// log infos
						log_texture_injected("depthStencil");
					}
				}

				// inject texture for NS430 if feature activated
				// if (it->second.feature == Feature::NS430  && shared_data.count_display > -1 && shared_data.NS430_copy_started && shared_data.cb_inject_values.NS430Flag)
				if (it->second.feature == Feature::NS430 && shared_data.NS430_copy_started && shared_data.cb_inject_values.NS430Flag)
				{
					// NS430 texture in VR menu GUI aera shaders as only 1 copy is done per frame, use 0 for all draws
					// if (shared_data.NS430_view[shared_data.count_display].created)
					if (shared_data.NS430_view[0].created)
					{
						// push the texture for NS430, descriptor initialized in copy_texture()
						shared_data.update.binding = 2; // t5
						// shared_data.update.descriptors = &shared_data.NS430_view[shared_data.count_display].texresource_view;
						shared_data.update.descriptors = &shared_data.NS430_view[0].texresource_view;
						commandList->push_descriptors(reshade::api::shader_stage::pixel, shared_data.saved_pipeline_layout_RV, 0, shared_data.update);

						// log infos
						log_texture_injected("NS430");
					}
				}

			}	
			
			// shader is to be handled
			if (it->second.action & action_replace)
			{
				
				// optimization : do not push CB if same shader is replaced again and no "count" action used for the shader 
				// possible because CB13 is not used by the game
				if (last_replaced_shader != pipelineHandle.handle || it->second.action & action_count)
				{
					// use push constant() to push the mod parameter in CB13,a sit is assumed a replaced shader will need mod parameters
					// pipeline_layout for CB initialized in init_pipeline() once for all
					commandList->push_constants(
						shader_stage::all,
						shared_data.saved_pipeline_layout_CB,
						0,
						0,
						CBSIZE,
						&shared_data.cb_inject_values
					);
				}
				
				// shader is to be replaced by the new one created in on_Init_Pipeline
				commandList->bind_pipeline(stages, it->second.substitute_pipeline);

				// log infos
				log_pipeline_replaced(pipelineHandle, it);	
				last_replaced_shader = pipelineHandle.handle;
			}
			
			if (it->second.action & action_skip && shared_data.cb_inject_values.NS430Flag && it->second.feature == Feature::NS430)
			{
				// shader is to be skipped, setup a flag for next draw
				do_not_draw = true;
			}

			// setup variables regarding the action
			if (it->second.action & action_log)
			{
				// VS for illum : trigger logging of resources (eg texture) or other topics (eg count calls)
				if (it->second.feature == Feature::GetStencil)
				{	
					// engage tracking shader_resource_view in push_descriptors() to get depthStencil 
					// const std::unique_lock<std::shared_mutex> lock(shared_data.s_mutex);
					shared_data.track_for_depthStencil = true;

					// log infos
					log_start_monitor("Depth Stencil");
				}

				// VS for NS430 : trigger logging of resources (eg texture) or other topics (eg count calls)
				if (it->second.feature == Feature::NS430)
				{
					// engage tracking shader_resource_view in push_descriptors() to get depthStencil 
					// const std::unique_lock<std::shared_mutex> lock(shared_data.s_mutex);
					shared_data.track_for_NS430 = true;

					// log infos
					log_start_monitor("NS430");
				}

				// PS for GUI : set flag
				if (it->second.feature == Feature::GUI)
				{
					shared_data.cb_inject_values.GUItodraw = 1.0;

					// log infos
					log_start_monitor("GUItodraw");
				}

				// PS for GUI : reset flag
				if (it->second.feature == Feature::NS430)
				{
					shared_data.cb_inject_values.GUItodraw = 0.0;

					// log infos
					log_clear_action_log("GUItodraw");
				}
				

				// PS global color : increase draw count & GUI flag (to avoid MFD)
				if (it->second.feature == Feature::Global)
				{
					// if texure has been copied previously, increase draw count, otherwise do nothing, to avoid counting shader calls for MFD rendering
					if (shared_data.depthStencil_copy_started)
					{
						shared_data.count_display += 1;
						// it's stupid but I'm too lazy to change code now..
						shared_data.cb_inject_values.count_display = shared_data.count_display;

						// log infos
						log_increase_draw_count();

					}
					else
					{	
						// log infos
						log_not_increase_draw_count();
					}
				}
			}

			if (it->second.action & action_identify)
			{
				// setup some global variables according to the feature (if not possible to do it on init_pipeline)

				if (it->second.feature == Feature::mapMode)
				{
					shared_data.cb_inject_values.mapMode = 0.0;
				}

				// PS for no MSAA : setup the ratio for masking
				if (it->second.feature == Feature::Haze)
				{

					shared_data.cb_inject_values.AAxFactor = 1.0;
					shared_data.cb_inject_values.AAyFactor = 1.0;

					// log infos
					log_MSAA();
				}

				// PS for no MSAA : setup the ratio for masking
				if (it->second.feature == Feature::HazeMSAA2x)
				{

					shared_data.cb_inject_values.AAxFactor = 2.0;
					shared_data.cb_inject_values.AAyFactor = 1.0;

					//log
					log_MSAA();
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
		log_push_descriptor(stages, layout, param_index, update);

		// copy depthStencil texture into shared_data
		bool status = copy_depthStencil(cmd_list, stages, layout, param_index, update);

		// stop tracking
		shared_data.track_for_depthStencil = false;
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

		if (debug_flag && flag_capture)
		{
			std::stringstream s;
			s << "   on_push_descriptor, text. width= " << NS430_res_desc.texture.width << ", texture.height =" << NS430_res_desc.texture.height << ";";
			reshade::log_message(reshade::log_level::info, s.str().c_str());
			s.str("");
			s.clear();
		}

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
		reshade::unregister_event<reshade::addon_event::bind_pipeline>(on_bind_pipeline);
		reshade::unregister_event<reshade::addon_event::init_pipeline_layout>(on_init_pipeline_layout);
		reshade::unregister_event<reshade::addon_event::draw>(on_draw);
		reshade::unregister_event<reshade::addon_event::draw_indexed>(on_draw_indexed);
		reshade::unregister_event<reshade::addon_event::draw_or_dispatch_indirect>(on_drawOrDispatch_indirect);
		reshade::unregister_event<reshade::addon_event::push_descriptors>(on_push_descriptors);

		reshade::unregister_event<reshade::addon_event::reshade_present>(on_present);

		cleanup();
		reshade::unregister_overlay(nullptr, &displaySettings);

		reshade::unregister_addon(hModule);
		break;
	}

	return TRUE;
}



