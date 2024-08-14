///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// copy Depth Stencil texture into a new resource, create the new resource and the associated views
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

#include <filesystem>

#include <reshade.hpp>

#include "global_shared.hpp"
#include "to_string.hpp"

using namespace reshade::api;

// copy_depthStencil(): create needed resource, then copy existing resource into the new one, then create the new associated resource views
//
bool copy_depthStencil(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
	// get resource info 
	resource_view src_resource_view_depth = static_cast<const reshade::api::resource_view*>(update.descriptors)[3];
	device* dev = cmd_list->get_device();
	resource scr_resource = dev->get_resource_from_view(src_resource_view_depth);

	// to do once per draw number : if resource and view not created, create them
	if (!shared_data.depthStencil_res[shared_data.count_display].created)
	{
		
		//get additional infos
		resource_view src_resource_view_stencil = static_cast<const reshade::api::resource_view*>(update.descriptors)[4];
		resource_desc src_resource_desc = dev->get_resource_desc(scr_resource);
		if (debug_flag)
		{
			std::stringstream s;
			s << " = > copy_depthStencil(" << shared_data.count_display << ") : create resource";
			reshade::log_message(reshade::log_level::info, s.str().c_str());
		}
		// create the target resource for texture
		dev->create_resource(src_resource_desc, nullptr, resource_usage::shader_resource, &shared_data.depthStencil_res[shared_data.count_display].texresource, nullptr);
		// flag creation to avoid to create it again 
		shared_data.depthStencil_res[shared_data.count_display].created = true;

		// create the resources view
		resource_view_desc resview_desc_depth = dev->get_resource_view_desc(src_resource_view_depth);
		dev->create_resource_view(shared_data.depthStencil_res[shared_data.count_display].texresource, resource_usage::shader_resource, resview_desc_depth, &shared_data.depth_view[shared_data.count_display].texresource_view);
		shared_data.depth_view[shared_data.count_display].created = true;
		resource_view_desc resview_desc_stencil = dev->get_resource_view_desc(src_resource_view_stencil);
		dev->create_resource_view(shared_data.depthStencil_res[shared_data.count_display].texresource, resource_usage::shader_resource, resview_desc_stencil, &shared_data.stencil_view[shared_data.count_display].texresource_view);
		shared_data.stencil_view[shared_data.count_display].created = true;

		// setup the descriptor table
		shared_data.update.binding = 0; // t3 as 3 is defined in pipeline_layout
		shared_data.update.count = 1;
		shared_data.update.type = reshade::api::descriptor_type::shader_resource_view;

		//debug
		// if (debug_flag && shared_data.s_do_capture)
		if (debug_flag && flag_capture)
		{
			
			// display resource info
			resource_desc check_new_res = dev->get_resource_desc(shared_data.depthStencil_res[shared_data.count_display].texresource);
			std::stringstream s;
			s << " => copy_depthStencil: for draw " << shared_data.count_display << ": create stencil/depth resource, type: " << to_string(check_new_res.type);

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
			reshade::log_message(reshade::log_level::info, s.str().c_str());
			s.str("");
			s.clear();

			//display resource view infos
			resource_view_desc check_new_res2 = dev->get_resource_view_desc(shared_data.depth_view[shared_data.count_display].texresource_view);
			s << " => copy_depthStencil: create depth resource view , type: " << to_string(check_new_res2.type);

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
			reshade::log_message(reshade::log_level::info, s.str().c_str());
			s.str("");
			s.clear();

			check_new_res2 = dev->get_resource_view_desc(shared_data.stencil_view[shared_data.count_display].texresource_view);
			s << " => copy_depthStencil: create depth resource view , type: " << to_string(check_new_res2.type);

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
			reshade::log_message(reshade::log_level::info, s.str().c_str());
			s.str("");
			s.clear();

		}
	}
	
	// to do before binding of global illum shader in associated push_descriptor: copy current resource to new resource for later usage in another shader
	// do it once per frame
	if (! shared_data.depthStencil_res[shared_data.count_display].copied)
	{
		//put resources in good usage for copying
		cmd_list->barrier(scr_resource, resource_usage::shader_resource, resource_usage::copy_source);
		cmd_list->barrier(shared_data.depthStencil_res[shared_data.count_display].texresource, resource_usage::shader_resource, resource_usage::copy_dest);
		// do copy
		cmd_list->copy_resource(scr_resource, shared_data.depthStencil_res[shared_data.count_display].texresource);
		//restore usage
		cmd_list->barrier(scr_resource, resource_usage::copy_source, resource_usage::shader_resource);
		cmd_list->barrier(shared_data.depthStencil_res[shared_data.count_display].texresource, resource_usage::copy_dest, resource_usage::shader_resource);
		// flag the first copy of texture to avoid usage of PS before texture copy (eg MFD)
		shared_data.texture_copy_started = true;
		//flag
		shared_data.depthStencil_res[shared_data.count_display].copied = true;

		// if (debug_flag && shared_data.s_do_capture)
		if (debug_flag && flag_capture)
		{
			std::stringstream s;
			s << " = > copy_depthStencil: for draw (" << shared_data.count_display << ") : resource DepthStencil copied";
			reshade::log_message(reshade::log_level::info, s.str().c_str());

		}
	}

	return true;
}
