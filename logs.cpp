///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// Some of log writing commands put in functions in a dedicated file to clean code
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


#include <imgui.h>
#include <reshade.hpp>
#include <string.h>

// #include "CDataFile.h"

#include "functions.h"
#include "global_shared.hpp"
#include "mod_injection.h"
#include "to_string.hpp"
#include <bitset>


using namespace reshade::api;

//to bypass frame capture (when crash at game launch)
const bool FORCE_LOG = false;

// *******************************************************************************************************
/// <summary>
/// Loads infos when push_descriptor() is called
/// </summary>

void log_push_descriptor(shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << "on_push_descriptors(" << to_string(stages) << ", " << (void*)layout.handle << ", " << param_index << ", { " << to_string(update.type) << ", " << update.binding << ", " << update.count << " })";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		if (update.type == descriptor_type::shader_resource_view)
		{
			// add info on textures hash
			for (uint32_t i = 0; i < update.count; ++i)
			{
				auto item = static_cast<const reshade::api::resource_view*>(update.descriptors)[i];
				s << "=> on_push_descriptors(), resource_view[" << i << "],  handle = " << reinterpret_cast<void*>(item.handle) << " })";
				reshade::log::message(reshade::log::level::info, s.str().c_str());
				s.str("");
				s.clear();
			}
		}
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}


// *******************************************************************************************************
/// <summary>
/// Loads infos when resource and resources view are created for a texture
/// </summary>
/// 
void log_resource_created(std:: string texture_name, device* dev, resource_desc check_new_res)
{

	// if ((debug_flag && flag_capture) || FORCE_LOG)
	if ((debug_flag ) || FORCE_LOG)
	{

		// display resource info
		
		std::stringstream s;
		// s << " => copy_depthStencil: for draw " << shared_data.count_display << ": create stencil/depth resource, type: " << to_string(check_new_res.type);
		s << " => copy_" << texture_name << ": for draw " << shared_data.count_display << ": create stencil/depth resource, type: " << to_string(check_new_res.type);

		switch (check_new_res.type) {
		default:
		case reshade::api::resource_type::unknown:
			s << "!!! error: resource_type not texture* !!!";
			break;

		case reshade::api::resource_type::texture_1d:
		case reshade::api::resource_type::texture_2d:
		case reshade::api::resource_type::texture_3d:
			s << ", texture format: " << to_string(check_new_res.texture.format);
			s << ", texture width: " << check_new_res.texture.width;
			s << ", texture height: " << check_new_res.texture.height;
			s << ", texture depth: " << check_new_res.texture.depth_or_layers;
			s << ", texture samples: " << check_new_res.texture.samples;
			s << ", texture levels: " << check_new_res.texture.levels;
			s << ", usage: " << to_string(check_new_res.usage);
			break;
		}
		s << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		//display resource view infos
		resource_view_desc check_new_res2 = dev->get_resource_view_desc(shared_data.depth_view[shared_data.count_display].texresource_view);
		// s << " => copy_depthStencil: create depth resource view , type: " << to_string(check_new_res2.type);
		s << " => copy_" << texture_name << ": create depth resource view , type: " << to_string(check_new_res2.type);

		switch (check_new_res2.type) {
		default:
		case reshade::api::resource_view_type::unknown:
			s << "!!! error: resource_type not texture* !!!";
			break;

		case reshade::api::resource_view_type::texture_1d:
		case reshade::api::resource_view_type::texture_2d:
		case reshade::api::resource_view_type::texture_3d:
		case reshade::api::resource_view_type::texture_2d_multisample:
			s << ", view format: " << to_string(check_new_res2.format);
			break;
		}
		s << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		check_new_res2 = dev->get_resource_view_desc(shared_data.stencil_view[shared_data.count_display].texresource_view);
		// s << " => copy_depthStencil: create depth resource view , type: " << to_string(check_new_res2.type);
		s << " => copy_" << texture_name << ": create stencil resource view , type: " << to_string(check_new_res2.type);

		switch (check_new_res2.type) {
		default:
		case reshade::api::resource_view_type::unknown:
			s << "!!! error: resource_type not texture* !!!";
			break;

		case reshade::api::resource_view_type::texture_1d:
		case reshade::api::resource_view_type::texture_2d:
		case reshade::api::resource_view_type::texture_3d:
		case reshade::api::resource_view_type::texture_2d_multisample:
			s << ", view format: " << to_string(check_new_res2.format);
			break;
		}
		s << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

	}
}

// *******************************************************************************************************
/// <summary>
/// Loads infos : starting creating resource and resources view
/// </summary>
void log_creation_start(std::string texture_name)
{

	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		s << " = > copy_" << texture_name << "( "<< shared_data.count_display << ") : create resource";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}


// *******************************************************************************************************
/// <summary>
/// Loads infos texture copy done
/// </summary>
/// 
void log_copy_texture(std::string texture_name)
{
	// if (debug_flag && shared_data.s_do_capture)
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " = > copy_" << texture_name << ": for draw (" << shared_data.count_display << ") : resource copied";
		reshade::log::message(reshade::log::level::info, s.str().c_str());

	}
}

// *******************************************************************************************************
/// <summary>
/// Log MSAA identified
/// </summary>
/// 
void log_MSAA()
{
	// log infos
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : flag MSAA2x, Xfactor = " << shared_data.cb_inject_values.AAxFactor << ", Yfactor = " << shared_data.cb_inject_values.AAyFactor << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}
// *******************************************************************************************************
/// <summary>
/// Log draw count increased
/// </summary>
/// 
void log_increase_count_display()
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : update draw count : " << shared_data.count_display << ", mapMode = " << shared_data.cb_inject_values.mapMode << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log draw count not increased
/// </summary>
/// 
void log_not_increase_draw_count()
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : depthstencil not copied => do not update draw count, texture not copied;";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log start monitor parameters
/// </summary>
/// 
void log_start_monitor(std::string texture_name)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : start monitor " << texture_name << ", draw : " << shared_data.count_display << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log pipeline is replaced
/// </summary>
/// 
void log_pipeline_replaced(pipeline pipelineHandle, std::unordered_map<uint64_t, Shader_Definition>::iterator it)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : pipeline shader replaced (" << reinterpret_cast<void*>(pipelineHandle.handle) << ") by  " << reinterpret_cast<void*>(it->second.substitute_pipeline.handle) << ", hash =" << std::hex << it->second.hash << ", feature =" << to_string(it->second.feature) << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log texture is injected
/// </summary>
/// 
void log_texture_injected(std::string texture_name, int count_displayVS)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : "<< texture_name << " textures injected for draw index : ";
		s << count_displayVS << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}
// *******************************************************************************************************
/// <summary>
/// Log the pipeline to process
/// </summary>
/// 
void log_pipeline_to_process(pipeline pipelineHandle, std::unordered_map<uint64_t, Shader_Definition>::iterator it)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << "on_bind_pipeline : Pipeline handle to process found: " << reinterpret_cast<void*>(pipelineHandle.handle) << ", hash =";
		s << std::hex << it->second.hash << ", associated cloned pipeline handle: " << reinterpret_cast<void*>(it->second.substitute_pipeline.handle);
		s << ", feature =" << to_string(it->second.feature) << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log on draw call
/// </summary>
/// 
void log_ondraw( uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
	if ((debug_flag && flag_capture && (shared_data.track_for_depthStencil || shared_data.track_for_NS430 || shared_data.render_effect)) || FORCE_LOG)
	{
		std::stringstream s;
		s << "draw(" << vertex_count << ", " << instance_count << ", " << first_vertex << ", " << first_instance << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		reshade::log::message(reshade::log::level::info, "Clear tracking flags");
		s.str("");
		s.clear();

		if (shared_data.render_effect)
		{

		}
	}
}

// *******************************************************************************************************
/// <summary>
/// Log on draw indexed call
/// </summary>
/// 
void log_on_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
{
	if ((debug_flag && flag_capture && (shared_data.track_for_depthStencil || shared_data.track_for_NS430 || shared_data.render_effect)) || FORCE_LOG)
	{
		std::stringstream s;
		s << "draw_indexed(" << index_count << ", " << instance_count << ", " << first_index << ", " << vertex_offset << ", " << first_instance << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		reshade::log::message(reshade::log::level::info, "Clear tracking flags");
	}
}
// *******************************************************************************************************
/// <summary>
/// Log on draw or dispatch indirect call
/// </summary>
/// 
void log_on_drawOrDispatch_indirect( indirect_command type, resource buffer, uint64_t offset, uint32_t draw_count, uint32_t stride)
{
	if ((debug_flag && flag_capture && (shared_data.track_for_depthStencil || shared_data.track_for_NS430 || shared_data.render_effect)) || FORCE_LOG)
	{
		std::stringstream s;
		s << "draw_indexed_indirect(" << (void*)buffer.handle << ", " << offset << ", " << draw_count << ", " << stride << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		reshade::log::message(reshade::log::level::info, "Clear tracking flags");
	}
}

// *******************************************************************************************************
/// <summary>
/// Log destroy pipeline
/// </summary>
/// 
/// 
void log_destroy_pipeline(reshade::api::pipeline pipeline, std::unordered_map<uint64_t, Shader_Definition>::iterator it)
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		s << "destroy_pipeline, master pipeline" << reinterpret_cast<void*>(pipeline.handle) << ", associated pipeline" << reinterpret_cast<void*>(it->second.substitute_pipeline.handle) << ")";
	}
}

// *******************************************************************************************************
/// <summary>
/// Log error when loading shader code in init_pipeline
/// </summary>
/// 
/// 
void log_shader_code_error(pipeline pipelineHandle, uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it)
{

	std::stringstream s;

	s << "onInitPipeline, pipelineHandle: " << (void*)pipelineHandle.handle << "), ";
	s << "hash to handle = " << std::hex << hash << " ;";
	s << "!!! Error in loading code for :" << to_string(it->second.replace_filename) << "; !!!";

	reshade::log::message(reshade::log::level::error, s.str().c_str());
	s.str("");
	s.clear();
}

// *******************************************************************************************************
/// <summary>
/// Log error when loading shader code in create_pipeline
/// </summary>
/// 
/// 
void log_shader_code_error_oncreate(uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it)
{

	std::stringstream s;

	s << "onCreatePipeline, hash to handle = " << std::hex << hash << " ;";
	s << "!!! Error in loading code for :" << to_string(it->second.replace_filename) << "; !!!";

	reshade::log::message(reshade::log::level::error, s.str().c_str());
	s.str("");
	s.clear();
}

// *******************************************************************************************************
/// <summary>
/// Log init pipeline
/// </summary>
/// 
/// 
void log_init_pipeline(pipeline pipelineHandle, pipeline_layout layout, uint32_t subobjectCount, const pipeline_subobject* subobjects, uint32_t i, uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it)
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		// logging infos
		s << "onInitPipeline, pipelineHandle: " << (void*)pipelineHandle.handle << "), ";
		s << "layout =  " << reinterpret_cast<void*>(layout.handle) << " ;";
		s << "subobjectCount =  " << subobjectCount << " ;";
		s << "index =  " << i << " ;";
		s << "Type = " << to_string(subobjects[i].type) << " ;";
		s << "hash to handle = " << std::hex << hash << " ;";
		s << "Action = " << to_string(it->second.action) << "; Feature = " << to_string(it->second.feature) << "; fileName =" << to_string(it->second.replace_filename) << ";";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	}
}
// *******************************************************************************************************
/// <summary>
/// Log init pipeline params
/// </summary>
/// 
/// 
void log_init_pipeline_params(const uint32_t paramCount, const reshade::api::pipeline_layout_param* params, reshade::api::pipeline_layout layout, uint32_t paramIndex, reshade::api::pipeline_layout_param param)
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		s << "* looping on  paramCount : param = " << to_string(paramIndex) << ", param.type = " << to_string(param.type) << ", param.push_descriptors.type = " << to_string(param.push_descriptors.type);
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	}
}
// *******************************************************************************************************
/// <summary>
/// Log create Constant Buffer layout
/// </summary>
/// 
/// 
void log_create_CBlayout(std::string CBName, int CB_number)
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		// s << "on_init_pipeline_layout: new pipeline created, hash =" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB.handle) << " ).  DX11 layout created for CB " << CBName <<";";
		s << "on_init_pipeline_layout: new pipeline created, hash =" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB[CB_number].handle) << " ).  DX11 layout created for CB " << CBName << ";";
		s << "dx_register_index=" << CBINDEX << ", CBIndex =" << shared_data.CBIndex << "; ";
		reshade::log::message(reshade::log::level::error, s.str().c_str());
		s.str("");
		s.clear();
	}
}

// *******************************************************************************************************
/// <summary>
/// Log error creating Constant Buffer layout
/// </summary>
/// 
/// 

void log_error_creating_CBlayout(std::string CBName, int CB_number)
{
	std::stringstream s; 
	
	s << "on_init_pipeline_layout(" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB[CB_number].handle) << " ). !!! Error in creating DX11 layout for CB " << CBName << "!!!;";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
	s.str("");
	s.clear();
}

// *******************************************************************************************************
/// <summary>
/// Log create Resource View layout
/// </summary>
/// 
/// 

void log_create_RVlayout()
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		s << "on_init_pipeline_layout: new pipeline created, hash =" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_RV.handle) << " ).  DX11 layout created for RV;";
		s << "dx_register_index=" << RVINDEX << ", RVIndex =" << shared_data.RVIndex << "; ";
		reshade::log::message(reshade::log::level::warning, s.str().c_str());
		s.str("");
		s.clear();
	}
}

// *******************************************************************************************************
/// <summary>
/// Log error creating Resource View layout
/// </summary>
/// 
/// 

void log_error_creating_RVlayout()
{
	std::stringstream s;
	s << "on_init_pipeline_layout(" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_RV.handle) << " ). !!! Error in creating DX11 layout for RV !!!;";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
	s.str("");
	s.clear();
}


// *******************************************************************************************************
/// <summary>
/// Log reset tracking
/// </summary>
/// 
/// 
void log_reset_tracking()
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		reshade::log::message(reshade::log::level::info, " -> on_draw*: All tracking resetted");
	}
}

// *******************************************************************************************************
/// <summary>
/// Log reset variable in action_log
/// </summary>
/// 
/// 
void log_clear_action_log(std::string varname)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : reset variable :" << varname << " ;";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log reset tracking
/// </summary>
/// 
/// 
void log_CB_injected(std::string CBName)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " -> on_bind_pipeline: CB injected :" << CBName << " ;";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// error message if pipeline has more than 1 object
/// </summary>
/// 
void log_invalid_subobjectCount(pipeline pipelineHandle)
{
	std::stringstream s;

	s << "on_init_pipeline_layout(" << reinterpret_cast<void*>(&pipelineHandle) << " ). !!! Error more than 1 object in pipeline !!!;";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

// *******************************************************************************************************
/// <summary>
/// Log shader code replacement in create_pipeline
/// </summary>
/// 
/// 
void log_replaced_shader_code(uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it, uint32_t newHash)
{

	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;

		s << "onCreatePipeline, hash to handle = " << std::hex << hash << " ;";
		s << "shader code replaced by :" << to_string(it->second.replace_filename) << ", new hash =" << newHash  <<";";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log render target and depth stencil 
/// </summary>
/// 
/// 
void log_renderTarget_depth(uint32_t count, const resource_view* rtvs, resource_view dsv, command_list* cmd_list)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;

		s << "bind_render_targets_and_depth_stencil():  render target stored ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		s << "    rt count = " << count << ", rtvs = { ";
		for (uint32_t i = 0; i < count; ++i)
			s << (void*)rtvs[i].handle << ", ";
			s << " }, dsv = " << (void*)dsv.handle << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		s << "    shared_data.render_target_view[" << shared_data.count_display << "].created = " << shared_data.render_target_view[shared_data.count_display].created << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());

		
		reshade::api::device* dev = cmd_list->get_device();
		log_texture_view(dev, "render target (rtvs[0])", rtvs[0]);

	}
}

// *******************************************************************************************************
/// <summary>
/// Log render target view creation
/// </summary>
/// 
/// 
void log_create_rendertarget_view(reshade::api::device* dev, reshade::api::resource rendert_res, reshade::api::resource_desc desc)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << "=> on_draw(): resource view for render target effect created, count_display = " << shared_data.count_display;
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		//display infos on resources.
		s << "  renter target resource handle " << (void*)rendert_res.handle << " , type =" << to_string(desc.type) << " , usage =" << " 0x" << std::hex << (uint32_t)desc.usage ;
		s << ", width: " << desc.texture.width << ", height: " << desc.texture.height << ", levels: " << desc.texture.levels << ", format: " << to_string(desc.texture.format);
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		//display infos on resources views
		log_texture_view(dev, "shared_data.render_target_rv_nrgb[shared_data.count_display - 1].texresource_view", shared_data.render_target_view[shared_data.count_display - 1].texresource_view);
	}
}

// *******************************************************************************************************
/// <summary>
/// Log error in render target view creation
/// </summary>
/// 
/// 
void log_error_for_rendertarget()
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << "=> on_draw(): !!!! Error in resource view for render target effect creation, count_display =" << shared_data.count_display << ", resource not matching criteron !!!!";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log render effect engaged in on_draw()
/// </summary>
/// 
/// 
void log_effect(technique_trace tech, command_list* cmd_list, resource_view rv)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{


		std::stringstream s;
		s << "=> on_push_descriptors(): engage render effects for technique = " << tech.name << " / " << tech.eff_name <<";";
		reshade::api::device* dev = cmd_list->get_device();
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		if (shared_data.render_target_view[shared_data.count_display - 1].created)
		{
			log_texture_view(dev, "render target for effect", rv);
		}
		else
		{
			s << "!!! resource view not catched for " << shared_data.count_display - 1 << "!!!";
			reshade::log::message(reshade::log::level::info, s.str().c_str());
		}
		// s << ", resource_view, " << (void*)shared_data.render_target_view[shared_data.count_display - 1].texresource_view.handle;
		
	}
}

// *******************************************************************************************************
/// <summary>
/// Log request render effect in on_bind_pipeline()
/// </summary>
/// 
void log_effect_requested()
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << "=> on_bind_pipeline(): set flag for engaging rendering at next draw, ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

/// *******************************************************************************************************
/// <summary>
/// Log info on a texture view and its associated resource
/// </summary>
/// 
void log_texture_view(reshade::api::device * dev, std::string name, reshade::api::resource_view rview)
{
	
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		//display infos on resources views
		reshade::api::resource_view_desc descv = dev->get_resource_view_desc(rview);
		reshade::api::resource res = dev->get_resource_from_view(rview);
		reshade::api::resource_desc desc_res = dev->get_resource_desc(res);

		std::stringstream s;
		s << "  resource view " << name << ", handle = " << (void*)rview.handle << " , type =" << to_string(descv.type) << " , format =" << (to_string)(descv.format) << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		s << "  associated resource handle " << (void*)res.handle << " , type =" << to_string(desc_res.type) << " , usage =" << " 0x" << std::hex << (uint32_t)desc_res.usage;
		s << ", width: " << desc_res.texture.width << ", height: " << desc_res.texture.height << ", levels: " << desc_res.texture.levels << ", format: " << to_string(desc_res.texture.format);
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

	}
}

/// *******************************************************************************************************
/// <summary>
/// Log info on technique
/// </summary>
/// 
void log_technique_info(effect_runtime* runtime, effect_technique technique, std::string& name, std::string& eff_name, bool technique_status,  int QV_target, bool has_texture)
{
	
	if ((debug_flag) || FORCE_LOG)
	{
		std::stringstream s;
		s << "init of technique in vector, Technique Name: " << name << ", Effect Name: " << eff_name << ", Technique status : " << technique_status << ", QV_target : " << QV_target << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		if (has_texture)
			reshade::log::message(reshade::log::level::info, "   Has DEPTH or STENCIL");
		else
			reshade::log::message(reshade::log::level::info, "   Do not have DEPTH or STENCIL");

	}

}

/// *******************************************************************************************************
/// <summary>
/// Log view used for mirror 
/// </summary>
/// 
void log_mirror_view()
{

	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << "= > on_bind_pipeline() : count_display used for mirror view = " << shared_data.count_display << ", mirror VR = "<< shared_data.mirror_VR << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	}

}

// *******************************************************************************************************
/// <summary>
/// Log export of render target resolution
/// </summary>
/// 
/// 
void log_export_render_targer_res(short int display_to_use)
{
	if ((debug_flag) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_push_descriptors : export pre processor variables, display_to_use = " << display_to_use
			<< ", BUFFER_WIDTH = " << shared_data.render_target_view[display_to_use].width
			<< ", BUFFER_HEIGHT = " << shared_data.render_target_view[display_to_use].height
			<< ", BUFFER_RCP_WIDTH = " << 1.0/shared_data.render_target_view[display_to_use].width
			<< ", BUFFER_RCP_HEIGHT = " << 1.0/shared_data.render_target_view[display_to_use].height
			<< ", MSAAX, value = " << shared_data.cb_inject_values.AAxFactor
			<< ", MSAAY value = " << shared_data.cb_inject_values.AAyFactor
			<< ";";
		reshade::log::message(reshade::log::level::warning, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log wait for compilation
/// </summary>
/// 
/// 
void log_wait()
{
	if ((debug_flag) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_push_descriptors : wait " << shared_data.compil_delay << " ms ;";
		reshade::log::message(reshade::log::level::warning, s.str().c_str());
	}
}


// *******************************************************************************************************
/// <summary>
/// Log export texture for technique
/// </summary>
/// 
/// 
void log_export_texture(short int display_to_use)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_push_descriptors : export pre processor DEPTH and STENCIL texture for display_to_use = " << display_to_use << ";";

		reshade::log::message(reshade::log::level::warning, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log technique loaded from file
/// </summary>
/// 
/// 
/// 
void log_technique_loaded(uint32_t index)
{
	if ((debug_flag) || FORCE_LOG)
	{
		std::stringstream s;
		s << "Enumerate technique loaded into vector, technique handle ";
		if (shared_data.technique_vector[index].technique == 0)
			s << "null, ";
		else
			s << " not null, ";
		s << "effect name = " << shared_data.technique_vector[index].eff_name
		<< ", technique name = " << shared_data.technique_vector[index].name;
		s << ", technique_status = " << shared_data.technique_vector[index].technique_status 
			<< ", QV_target = " << shared_data.technique_vector[index].quad_view_target << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log enumerateTechniques() called
/// </summary>
/// 
/// 
void log_enumerate()
{
	if ((debug_flag) || FORCE_LOG) 
		reshade::log::message(reshade::log::level::info, "enumerate_techniques called");
}

// *******************************************************************************************************
/// <summary>
/// Log VRONLY technique identified
/// </summary>
/// 
void log_VRonly_technique()
{
	if ((debug_flag) || FORCE_LOG) 
		reshade::log::message(reshade::log::level::info, "runtime->enumerate_techniques: VRONLY technique identified");
}

// *******************************************************************************************************
/// <summary>
/// Log technique readed by runtime->enumerate_techniques
/// </summary>
/// 
void log_technique_readed(effect_technique technique, std::string name, std::string eff_name, bool technique_status)
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		std::string status = "not null";
		if (technique == 0)
			status = "null";

		s << "runtime->enumerate_techniques : technique enumerated"
			<< ", technique handle = " << status
			<< ", name = " << name
			<< ", effect = " << eff_name
			<< ", technique status =" << technique_status;
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log info for a constant buffer
/// </summary>
/// 
void log_cbuffer_info(std::string CB_name, reshade::api::buffer_range cbuffer)
{

	if (debug_flag && flag_capture || FORCE_LOG)
	{
		std::stringstream s;
		s << "--> cbuffer " << CB_name << " handle = " << reinterpret_cast<void*>(cbuffer.buffer.handle) << ", size = " << cbuffer.size << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}

}

// *******************************************************************************************************
/// <summary>
/// Log copy of a constant buffer and display its values
/// </summary>
/// 
void log_constant_buffer_copy(std::string CB_name, float* dest_array, int buffer_size)
{
	if (debug_flag && flag_capture || FORCE_LOG)
	{
		std::stringstream s;
		for (int i = 0; i < buffer_size+1; i++)
		{
			s << "--> cbuffer "<< CB_name <<"[" << i << "] = " << dest_array[i] << ";";
			reshade::log::message(reshade::log::level::info, s.str().c_str());
			s.str("");
			s.clear();
		}
	}
}

// *******************************************************************************************************
/// <summary>
/// Log error during mapping a CB
/// </summary>
/// 
void log_constant_buffer_mapping_error(std::string CB_name)
{
	if (debug_flag && flag_capture || FORCE_LOG)
	{
		std::stringstream s;
		s << "**** map_buffer_region KO for " << CB_name << " !!! ***";
		reshade::log::message(reshade::log::level::error, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log shader that will be retained regarding selected options
/// </summary>

void log_filter_shader(std::pair<const uint32_t, Shader_Definition>& entry, bool status)
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		// s << "Shader hash: " << std::hex << entry.first << ", action :" << "0b" << std::bitset<8>(entry.second.action)  << ", feature: " << to_string(entry.second.feature) << ", draw count: " << entry.second.draw_count << ", retained :" << status << ";";
		s << "Shader hash: " << std::hex << entry.first << ", action :" << to_string(entry.second.action) << ", feature: " << to_string(entry.second.feature) << ", draw count: " << entry.second.draw_count << ", retained :" << status << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log preprocessor action
/// </summary>

void log_preprocessor(std::string name, float targetValue, bool update, bool status, float readedValue, bool inFrame, uint16_t step, short int display_to_use)
{
	
	std::string stepName;

	switch(step)
	{
		case 1: 
			stepName = "CREATE";
			break;
		case 2: 
			stepName = "UPDATE";
			break;
		case 3: 
			stepName = "SKIP";
			break;
		default:
			stepName = "UNKNOWN";
			break;
	}
	

	if ( (debug_flag && ( (inFrame && flag_capture) || !inFrame)) || FORCE_LOG)
	{
		std::stringstream s;
		s << stepName << " default_preprocessor, name = '" << name << "', target value = " << targetValue << ", exists = " << status << ", readed value = " << readedValue << " display = " << display_to_use << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}

}

// *******************************************************************************************************
/// <summary>
/// Log preprocessor action
/// 

void log_susperSamping()
{
	if (debug_flag && flag_capture)
	{
		std::stringstream s;
		s << "OnPresent : MSAAxfactor = " << shared_data.MSAAxfactor << ", MSAAxfactor = " << shared_data.MSAAyfactor << ", StencilBufferX =  " << shared_data.renderTargetX << ", render_target_vie width =  " << shared_data.render_target_view[0].width << ", SSfactor =" << shared_data.SSfactor << " ***";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log error when loading shader code
/// 
void log_error_loading_shader_code(std::string message)
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		s << "**** Error when loading shader code : " << message << " !!! ***";
		reshade::log::message(reshade::log::level::error, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// display  pipeline_by_hash content
/// 
void print_pipeline_by_hash(const std::unordered_map<uint32_t, Shader_Definition>& map)
{
	if (debug_flag && flag_capture)
	{
		reshade::log::message(reshade::log::level::info, "pipeline_by_hash:");
		for (const auto& [hash, def] : map)
		{
			std::stringstream s;
			s << "Hash: " << std::hex << hash
				<< ", Action: " << to_string(def.action)
				<< ", Feature: " << to_string(def.feature)
				<< ", Draw Count: " << def.draw_count
				<< ", Replace File: " << to_string(def.replace_filename)
				<< ", Pipeline Handle: " << std::hex << def.substitute_pipeline.handle
				<< ", Hash: " << std::hex << def.hash;

			reshade::log::message(reshade::log::level::error, s.str().c_str());
		}
	}
}

// *******************************************************************************************************
/// <summary>
/// display  pipeline_by_handle content
/// 
void print_pipeline_by_handle(const std::unordered_map<uint64_t, Shader_Definition>& map)
{
	if (debug_flag && flag_capture)
	{
		reshade::log::message(reshade::log::level::info, "pipeline_by_handle:");
		for (const auto& [handle, def] : map)
		{
			std::stringstream s;
			s << "handle: " << std::hex << handle
				<< ", Action: " << to_string(def.action)
				<< ", Feature: " << to_string(def.feature)
				<< ", Draw Count: " << def.draw_count
				<< ", Replace File: " << to_string(def.replace_filename)
				<< ", Cloned Pipeline Handle: " << std::hex << def.substitute_pipeline.handle
				<< ", Hash: " << std::hex << def.hash;

			reshade::log::message(reshade::log::level::error, s.str().c_str());
		}
	}
}