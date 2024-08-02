#include <reshade.hpp>
#include "config.hpp"
#include "crc32_hash.hpp"
#include <fstream>
#include <filesystem>

using namespace reshade::api;

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
	s << "Color shader readed, size = " << (void*)shaderCode.size() << ")";
	reshade::log_message(reshade::log_level::info, s.str().c_str());

	return true;
}