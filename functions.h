#pragma once


#include <reshade.hpp>
#include <filesystem>

#include "global_shared.hpp"
#include "shader_definitions.h"

using namespace reshade::api;

extern void displaySettings(reshade::api::effect_runtime* runtime);

extern bool load_shader_code(std::vector<std::vector<uint8_t>>& shaderCode, wchar_t filename[]);

extern bool load_shader_code_crosire(device_api device_type, shader_desc& desc, std::vector<std::vector<uint8_t>>& data_to_delete);

extern void clone_pipeline(reshade::api::device* device, reshade::api::pipeline_layout layout, uint32_t subobjectCount, const reshade::api::pipeline_subobject* subobjects, reshade::api::pipeline pipeline, std::vector<std::vector<uint8_t>>& ReplaceshaderCode, Shader_Definition* newShader);

extern void load_setting_IniFile();

extern void init_mod_features();

extern void saveShaderTogglerIniFile();

// extern void save_technique_status(std::string, bool technique_status);

extern void on_present(reshade::api::effect_runtime* runtime);

extern bool copy_depthStencil(reshade::api::command_list* cmd_list, reshade::api::shader_stage stages, reshade::api::pipeline_layout layout, uint32_t param_index, const reshade::api::descriptor_table_update& update);

extern bool copy_NS430_text(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update);

extern void enumerateTechniques(effect_runtime* runtime);

extern bool onReshadeSetTechniqueState(effect_runtime* runtime, effect_technique technique, bool enabled);

extern void disableAllTechnique(bool save_flag);

extern void reEnableAllTechnique(bool save_flag);

extern void log_push_descriptor(reshade::api::shader_stage stages, reshade::api::pipeline_layout layout, uint32_t param_index, const reshade::api::descriptor_table_update& update);

extern void log_resource_created(std::string texture_name, device* dev, resource_desc check_new_res);

extern void log_creation_start(std::string texture_name);

extern void log_copy_texture(std::string texture_name);

extern void log_MSAA();

extern void log_increase_count_display();

extern void log_not_increase_draw_count();

extern void log_start_monitor(std::string texture_name);

extern void log_pipeline_replaced(pipeline pipelineHandle, std::unordered_map<uint64_t, Shader_Definition>::iterator it);

extern void log_texture_injected(std::string texture_name);

extern void log_pipeline_to_process(pipeline pipelineHandle, std::unordered_map<uint64_t, Shader_Definition>::iterator it);

extern void log_ondraw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);

extern void log_on_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);

extern void log_on_drawOrDispatch_indirect(indirect_command type, resource buffer, uint64_t offset, uint32_t draw_count, uint32_t stride);

extern void log_destroy_pipeline(reshade::api::pipeline pipeline, std::unordered_map<uint64_t, Shader_Definition>::iterator it);

extern void log_shader_code_error(pipeline pipelineHandle, uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it);

extern void log_init_pipeline(pipeline pipelineHandle, pipeline_layout layout, uint32_t subobjectCount, const pipeline_subobject* subobjects, uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it);

extern void log_init_pipeline_params(const uint32_t paramCount, const reshade::api::pipeline_layout_param* params, reshade::api::pipeline_layout layout, uint32_t paramIndex, reshade::api::pipeline_layout_param param);

extern void log_create_CBlayout(std::string CBName);

extern void log_error_creating_CBlayout(std::string CBName);

extern void log_reset_tracking();

extern void log_create_RVlayout();

extern void log_error_creating_RVlayout();

extern void log_clear_action_log(std::string varname);

extern void log_CB_injected(std::string CBName);

extern void log_invalid_subobjectCount(pipeline pipelineHandle);

extern void log_shader_code_error_oncreate(uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it);

extern void log_replaced_shader_code(uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it);

extern void log_renderTarget_depth(uint32_t count, const resource_view* rtvs, resource_view dsv, command_list* cmd_list);

extern void log_create_rendertarget_view(reshade::api::device* dev, reshade::api::resource rendert_res, reshade::api::resource_desc desc);

extern void log_error_for_rendertarget();

extern void log_effect(technique_trace tech, command_list* cmd_list, resource_view rv);

extern void log_effect_requested();

extern void log_texture_view(reshade::api::device* dev, std::string name, reshade::api::resource_view rview);

extern void log_technique_info(effect_runtime* runtime, effect_technique technique, std::string& name, std::string& eff_name, bool technique_status, int QV_target, bool has_texture);

extern void log_mirror_view();

extern void log_export_render_targer_res(short int display_to_use);

extern void log_wait();

extern void log_export_texture(short int display_to_use);

extern void log_technique_loaded(uint32_t index);

extern void log_cbuffer_info(std::string CB_name, reshade::api::buffer_range cbuffer);

extern void log_constant_buffer_copy(std::string CB_name, float* dest_array, int buffer_size);

extern void log_constant_buffer_mapping_error(std::string CB_name);

extern bool read_constant_buffer(command_list* cmd_list, const descriptor_table_update& update, std::string CB_name, int descriptors_index, float* dest_array, int buffer_size);

extern void log_filter_shader(std::pair<const uint32_t, Shader_Definition>& entry, bool status);