#pragma once


#include <reshade.hpp>
#include <filesystem>

#include "shader_definitions.h"

extern void displaySettings(reshade::api::effect_runtime* runtime);

extern bool load_shader_code(std::vector<std::vector<uint8_t>>& shaderCode, wchar_t filename[]);

extern void clone_pipeline(reshade::api::device* device, reshade::api::pipeline_layout layout, uint32_t subobjectCount, const reshade::api::pipeline_subobject* subobjects, reshade::api::pipeline pipeline, std::vector<std::vector<uint8_t>>& ReplaceshaderCode, Shader_Definition* newShader);

extern void load_setting_IniFile();

extern void saveShaderTogglerIniFile();

extern void on_present(reshade::api::effect_runtime* runtime);

extern bool copy_depthStencil(reshade::api::command_list* cmd_list, reshade::api::shader_stage stages, reshade::api::pipeline_layout layout, uint32_t param_index, const reshade::api::descriptor_table_update& update);
