///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// copy texture into a new resource, create the new resource and the associated views
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
#include "functions.h"

using namespace reshade::api;

// *******************************************************************************************************
/// copy_depthStencil()
/// <summary>
///  create needed resource, then copy existing resource into the new one, then create the new associated resource views
/// </summary>
bool copy_depthStencil(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
	// get resource info (depth = t3)
	resource_view src_resource_view_depth = static_cast<const reshade::api::resource_view*>(update.descriptors)[3];
	device* dev = cmd_list->get_device();
	resource scr_resource = dev->get_resource_from_view(src_resource_view_depth);


	// to do once per draw number : if resource and view not created, create them
	if (!shared_data.depthStencil_res[shared_data.count_display].created)
	{
		
		//get additional infos (stencil = t4)
		resource_view src_resource_view_stencil = static_cast<const reshade::api::resource_view*>(update.descriptors)[4];
		resource_desc src_resource_desc = dev->get_resource_desc(scr_resource);
		/*
		resource_desc new_resource_desc;
		new_resource_desc.flags = src_resource_desc.flags;
		new_resource_desc.heap = src_resource_desc.heap;
		new_resource_desc.texture.depth_or_layers = src_resource_desc.texture.depth_or_layers;
		new_resource_desc.texture.format = reshade::api::format::d32_float_s8_uint;
		new_resource_desc.texture.height = src_resource_desc.texture.height;
		new_resource_desc.texture.levels = src_resource_desc.texture.levels;
		new_resource_desc.texture.samples = src_resource_desc.texture.samples;
		new_resource_desc.texture.width = src_resource_desc.texture.width;
		new_resource_desc.type = src_resource_desc.type;
		new_resource_desc.usage = src_resource_desc.usage;
		*/

		//log start of creation process
		log_creation_start("depthStencil");

		// not working : try to optimize things by avoiding texture copy => only &shared_data.stencil_view[shared_data.count_display].texresource_view are updated
		// shared_data.stencil_view[shared_data.count_display].texresource_view = static_cast<const reshade::api::resource_view*>(update.descriptors)[4];		

		
		// create the target resource for texture
		dev->create_resource(src_resource_desc, nullptr, resource_usage::shader_resource, &shared_data.depthStencil_res[shared_data.count_display].texresource, nullptr);
		//dev->create_resource(new_resource_desc, nullptr, resource_usage::shader_resource, &shared_data.depthStencil_res[shared_data.count_display].texresource, nullptr);
		
		
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

		//log infos and check resource view created
		resource_desc check_new_res = dev->get_resource_desc(shared_data.depthStencil_res[shared_data.count_display].texresource);
		log_resource_created("depthStencil", dev, check_new_res);

		// store resolution of resource of first draw to compute protential undersampling/supersampling factor (not for MSAA)
		if (shared_data.count_display == 0)
		{
			shared_data.renderTargetX = check_new_res.texture.width;
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
		shared_data.depthStencil_copy_started = true;

		//flag texture copied to avoid double copy for MSAA because shader is called multiple time for a same "eye rendering" and only first call has good texture
		shared_data.depthStencil_res[shared_data.count_display].copied = true;

		//log copy done
		log_copy_texture("depthStencil");

	}

	return true;
}

// *******************************************************************************************************
/// copy_NS430_text()
/// 
/// TODO : make a generic copy process to avoid duplicate function for each resources
/// <summary>
///  create needed resource, then copy existing resource into the new one, then create the new associated resource views
/// </summary>
/// 
bool copy_NS430_text(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
	// get resource info (t0)
	resource_view src_resource_view_NS430 = static_cast<const reshade::api::resource_view*>(update.descriptors)[0];
	device* dev = cmd_list->get_device();
	resource scr_resource = dev->get_resource_from_view(src_resource_view_NS430);

	// to do once per draw number : if resource and view not created, create them
	if (!shared_data.NS430_res[shared_data.count_display].created)
	{

		//get additional infos
		resource_desc src_resource_desc = dev->get_resource_desc(scr_resource);

		//log start of creation process
		log_creation_start("NS430");

		// create the target resource for texture
		dev->create_resource(src_resource_desc, nullptr, resource_usage::shader_resource, &shared_data.NS430_res[shared_data.count_display].texresource, nullptr);
		// flag creation to avoid to create it again 
		shared_data.NS430_res[shared_data.count_display].created = true;

		// create the resources view
		resource_view_desc resview_NS430 = dev->get_resource_view_desc(src_resource_view_NS430);
		dev->create_resource_view(shared_data.NS430_res[shared_data.count_display].texresource, resource_usage::shader_resource, resview_NS430, &shared_data.NS430_view[shared_data.count_display].texresource_view);
		shared_data.NS430_view[shared_data.count_display].created = true;

		// setup the descriptor table (same for all texture)
		shared_data.update.binding = 0; // t3 as 3 is defined in pipeline_layout
		shared_data.update.count = 1;
		shared_data.update.type = reshade::api::descriptor_type::shader_resource_view;

		//log infos and check resource view created
		resource_desc check_new_res = dev->get_resource_desc(shared_data.NS430_res[shared_data.count_display].texresource);
		log_resource_created("NS430", dev, check_new_res);
	}

	// to do before binding of global illum shader in associated push_descriptor: copy current resource to new resource for later usage in another shader
	// do it once per frame pre draw number
	if (!shared_data.NS430_res[shared_data.count_display].copied)
	{
		//put resources in good usage for copying
		cmd_list->barrier(scr_resource, resource_usage::shader_resource, resource_usage::copy_source);
		cmd_list->barrier(shared_data.NS430_res[shared_data.count_display].texresource, resource_usage::shader_resource, resource_usage::copy_dest);
		// do copy
		cmd_list->copy_resource(scr_resource, shared_data.NS430_res[shared_data.count_display].texresource);
		//restore usage
		cmd_list->barrier(scr_resource, resource_usage::copy_source, resource_usage::shader_resource);
		cmd_list->barrier(shared_data.NS430_res[shared_data.count_display].texresource, resource_usage::copy_dest, resource_usage::shader_resource);

		// flag the first copy of texture to avoid usage of PS before texture copy (eg MFD)
		shared_data.NS430_copy_started = true;

		// flag texture copied to avoid double copy for MSAA because shader is called multiple time for a same "eye rendering" and only first call has good texture
		shared_data.NS430_res[shared_data.count_display].copied = true;

		//log copy done
		log_copy_texture("NS430");

	}

	return true;
}

