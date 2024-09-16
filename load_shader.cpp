///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// function to load and reload shaders
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
#include "config.hpp"
#include "crc32_hash.hpp"
#include "shader_definitions.h"
#include <fstream>
#include <filesystem>
#include <unordered_map>

#include "functions.h"

using namespace reshade::api;

// *******************************************************************************************************
// load_shader_code() : load shader code from cso file
// 
bool load_shader_code(std::vector<std::vector<uint8_t>>& shaderCode, wchar_t filename[])
{

	// Prepend executable file name to image files
	wchar_t file_prefix[MAX_PATH] = L"";
	GetModuleFileNameW(nullptr, file_prefix, ARRAYSIZE(file_prefix));

	std::filesystem::path replace_path = file_prefix;
	replace_path = replace_path.parent_path();
	replace_path /= RESHADE_ADDON_SHADER_LOAD_DIR;

	replace_path /= filename;

	// Check if a replacement file for this shader hash exists and if so, overwrite the shader code with its contents
	if (!std::filesystem::exists(replace_path))
		return false;

	std::ifstream file(replace_path, std::ios::binary);
	file.seekg(0, std::ios::end);
	std::vector<uint8_t> shader_code(static_cast<size_t>(file.tellg()));
	file.seekg(0, std::ios::beg).read(reinterpret_cast<char*>(shader_code.data()), shader_code.size());

	// Keep the shader code memory alive after returning from this 'create_pipeline' event callback
	// It may only be freed after the 'init_pipeline' event was called for this pipeline
	shaderCode.push_back(std::move(shader_code));

	// log info
	std::stringstream s;
	s << "Shader readed, size = " << (void*)shaderCode.size() << ")";
	reshade::log_message(reshade::log_level::info, s.str().c_str());

	return true;
}

// *******************************************************************************************************
// load_shader_code_crosire() : identify and load shader code from cso file, as done by crosire for usage in create_pipeline
// update pipeline_by_hash with new hash
// 
bool load_shader_code_crosire(device_api device_type, shader_desc& desc, std::vector<std::vector<uint8_t>>& data_to_delete)
{
	if (desc.code_size == 0)
		return false;

	uint32_t shader_hash = compute_crc32(static_cast<const uint8_t*>(desc.code), desc.code_size);
	std::unordered_map<uint32_t, Shader_Definition>::iterator it = pipeline_by_hash.find(shader_hash);

	//check if hash is to be processed
	if (it != pipeline_by_hash.end()) {

		if (it->second.action & action_replace)
		{
			// Prepend executable file name to image files
			wchar_t file_prefix[MAX_PATH] = L"";
			GetModuleFileNameW(nullptr, file_prefix, ARRAYSIZE(file_prefix));

			std::filesystem::path replace_path = file_prefix;
			replace_path = replace_path.parent_path();
			replace_path /= RESHADE_ADDON_SHADER_LOAD_DIR;

			replace_path /= it->second.replace_filename;

			// Check if a replacement file for this shader hash exists and if so, overwrite the shader code with its contents
			if (!std::filesystem::exists(replace_path))
				return false;

			std::ifstream file(replace_path, std::ios::binary);
			file.seekg(0, std::ios::end);
			std::vector<uint8_t> shader_code(static_cast<size_t>(file.tellg()));
			file.seekg(0, std::ios::beg).read(reinterpret_cast<char*>(shader_code.data()), shader_code.size());

			// Keep the shader code memory alive after returning from this 'create_pipeline' event callback
			// It may only be freed after the 'init_pipeline' event was called for this pipeline
			data_to_delete.push_back(std::move(shader_code));

			desc.code = data_to_delete.back().data();
			desc.code_size = data_to_delete.back().size();

			//update hash and pipeline_by_hash
			uint32_t new_shader_hash = compute_crc32(static_cast<const uint8_t*>(desc.code), desc.code_size);
			pipeline_by_hash.emplace(
				new_shader_hash,
				Shader_Definition(it->second.action,
					it->second.feature,
					it->second.replace_filename,
					it->second.draw_count)
			);
			pipeline_by_hash.erase(shader_hash);

			log_replaced_shader_code(shader_hash, it);
			return true;
		}
		else return false;
	} else return false;

}
