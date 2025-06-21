///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// read a constant buffer into a new resource
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


#include <reshade.hpp>
#include "global_shared.hpp"
#include "functions.h"

using namespace reshade::api;


// *******************************************************************************************************
/// create_CB_layout()
/// <summary>
///  create a CB layout for CB created or modified by VREM 
/// </summary>

void create_modified_CB_layout(reshade::api::device* device, int cbindex, std::string CB_name, int layout_number)
{
	// create a new pipeline_layout for VREM constant buffer to be updated by push_constant(), cb number defined in CBINDEX (in mod_injection.h)
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
	newParams.push_constants.dx_register_index = cbindex;
	newParams.push_constants.dx_register_space = 0;
	newParams.push_constants.visibility = reshade::api::shader_stage::all;

	//put the VREM parameters in CB
	bool  result = device->create_pipeline_layout(1, &newParams, &shared_data.saved_pipeline_layout_CB[layout_number]);
	//logs
	if (result) log_create_CBlayout(CB_name, layout_number);
	else log_error_creating_CBlayout(CB_name, layout_number);
}

// *******************************************************************************************************
/// read_Constant_buffer()
/// <summary>
///  read value of a constant buffer and put it in a destination float array
/// </summary>
bool read_constant_buffer(command_list* cmd_list, const descriptor_table_update& update, std::string CB_name, int descriptors_index, float *dest_array, int buffer_size)
{
	
	bool error = false;
	
	// select in descriptors table the needed constant buffer to copy
	auto cbuffer = static_cast<const reshade::api::buffer_range*>(update.descriptors)[descriptors_index];
	log_cbuffer_info(CB_name, cbuffer);

	// create a new buffer resource to copy source buffer
	device* dev = cmd_list->get_device();

	// set elements for src and dest.
	resource_desc src_buffer_desc = dev->get_resource_desc(cbuffer.buffer);
	resource_usage src_usage = src_buffer_desc.usage;
	memory_heap src_heap = src_buffer_desc.heap;

	resource dest_buffer;
	resource_desc dest_buffer_desc;

	dest_buffer_desc.type = resource_type::buffer;
	dest_buffer_desc.buffer.size = src_buffer_desc.buffer.size;
	dest_buffer_desc.buffer.stride = src_buffer_desc.buffer.stride;
	//this should not be changed !
	dest_buffer_desc.usage = resource_usage::copy_dest;
	dest_buffer_desc.heap = memory_heap::gpu_to_cpu;

	// create the new resource
	dev->create_resource(dest_buffer_desc, nullptr, resource_usage::constant_buffer, &dest_buffer, nullptr);

	// copy old to new
	cmd_list->barrier(cbuffer.buffer, src_usage, resource_usage::copy_source);
	// do copy
	cmd_list->copy_resource(cbuffer.buffer, dest_buffer);
	//restore usage
	cmd_list->barrier(cbuffer.buffer, resource_usage::copy_source, src_usage);

	// map new cb to the destination variable and free resources
	void* data = nullptr;
	if (cmd_list->get_device()->map_buffer_region(dest_buffer, 0, -1, map_access::read_only, &data))
	{
		memcpy(dest_array, data, buffer_size*4); // float = 4 bytes
		cmd_list->get_device()->unmap_buffer_region(dest_buffer);

		//delete resource created for copy
		dev->destroy_resource(dest_buffer);

		//log operation and dump CB
		log_constant_buffer_copy(CB_name, dest_array, buffer_size);
	}
	else
	{
		log_constant_buffer_mapping_error(CB_name);
		error = true;
	}

	return error;
}