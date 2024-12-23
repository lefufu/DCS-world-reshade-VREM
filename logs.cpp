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

#include "functions.h"
#include "global_shared.hpp"
#include "mod_injection.h"
#include "to_string.hpp"

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

	if ((debug_flag && flag_capture) || FORCE_LOG)
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
		s << " => on_bind_pipeline : update draw count : " << shared_data.count_display << ";";
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
void log_texture_injected(std::string texture_name)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : "<< texture_name << " textures injected for draw index : ";
		s << shared_data.count_display << ";";
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
void log_init_pipeline(pipeline pipelineHandle, pipeline_layout layout, uint32_t subobjectCount, const pipeline_subobject* subobjects, uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it)
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		// logging infos
		s << "onInitPipeline, pipelineHandle: " << (void*)pipelineHandle.handle << "), ";
		s << "layout =  " << reinterpret_cast<void*>(layout.handle) << " ;";
		s << "subobjectCount =  " << subobjectCount << " ;";
		s << "Type = " << to_string(subobjects[0].type) << " ;";
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
void log_create_CBlayout()
{
	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;
		s << "on_init_pipeline_layout: new pipeline created, hash =" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB.handle) << " ).  DX11 layout created for CB;";
		s << "dx_register_index=" << CBINDEX << ", CBIndex =" << shared_data.CBIndex << "; ";
		reshade::log::message(reshade::log::level::warning, s.str().c_str());
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

void log_error_creating_CBlayout()
{
	std::stringstream s; 
	
	s << "on_init_pipeline_layout(" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB.handle) << " ). !!! Error in creating DX11 layout for CB!!!;";
	reshade::log::message(reshade::log::level::warning, s.str().c_str());
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
	s << "on_init_pipeline_layout(" << reinterpret_cast<void*>(&shared_data.saved_pipeline_layout_CB.handle) << " ). !!! Error in creating DX11 layout for RV !!!;";
	reshade::log::message(reshade::log::level::warning, s.str().c_str());
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
		reshade::log::message(reshade::log::level::warning, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log reset tracking
/// </summary>
/// 
/// 
void log_CB_injected()
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		reshade::log::message(reshade::log::level::info, " -> on_bind_pipeline: CB injected");
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
	reshade::log::message(reshade::log::level::warning, s.str().c_str());
}

// *******************************************************************************************************
/// <summary>
/// Log shader code replacement in create_pipeline
/// </summary>
/// 
/// 
void log_replaced_shader_code(uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it)
{

	if (debug_flag || FORCE_LOG)
	{
		std::stringstream s;

		s << "onCreatePipeline, hash to handle = " << std::hex << hash << " ;";
		s << "shader code replaced by :" << to_string(it->second.replace_filename) << ";";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log render target and depth stencil 
/// </summary>
/// 
/// 
void log_renderTarget_depth(uint32_t count, const resource_view* rtvs, resource_view dsv)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;

		s << "bind_render_targets_and_depth_stencil():  tracking render target ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		s << "    rt logged, count = " << count << ", rtvs = { ";
		for (uint32_t i = 0; i < count; ++i)
			s << (void*)rtvs[i].handle << ", ";
		s << " }, dsv = " << (void*)dsv.handle << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		s << "    shared_data.render_target_view[" << shared_data.count_display << "].created = " << shared_data.render_target_view[shared_data.count_display].created << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());

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

		log_texture_view(dev, "shared_data.render_target_rv_nrgb[shared_data.count_display - 1].texresource_view", shared_data.render_target_rv_nrgb[shared_data.count_display - 1].texresource_view);

		log_texture_view(dev, "shared_data.render_target_rv_rgb[shared_data.count_display - 1].texresource_view", shared_data.render_target_rv_rgb[shared_data.count_display - 1].texresource_view);

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
		s << "=> on_draw(): !!!! Error in resource view for render target effect creation, count_display =" << shared_data.count_display << " !!!!";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

// *******************************************************************************************************
/// <summary>
/// Log render effect engaged in on_draw()
/// </summary>
/// 
/// 
/*
void log_effect(technique_trace tech)
{
	if ((debug_flag && flag_capture) || FORCE_LOG)
	{
		std::stringstream s;
		s << "=> on_push_descriptors(): engage render effects for technique, " << tech.name ;
		s << ", resource_view, " << (void*)shared_data.render_target_view[shared_data.count_display - 1].texresource_view.handle;
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}
*/

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
void log_technique_info(effect_runtime* runtime, effect_technique technique, std::string& name, std::string& eff_name, bool technique_status)
{
	
	if ((debug_flag) || FORCE_LOG)
	{
		// Write to ReShade log
		std::stringstream s;
		s << "init_mod_features(): add technique in vector, Technique Name: " << name << ", Effect Name: " << eff_name << ", Technique status : " << technique_status;
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	}
}