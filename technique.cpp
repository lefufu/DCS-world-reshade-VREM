///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// technique functions : not working as reshade technique can not handle mutliple render resolution !
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

#include <cmath>
#include <imgui.h>
#include <reshade.hpp>
#include "functions.h"
#include "global_shared.hpp"
#include "to_string.hpp"
#include "shader_definitions.h"

#include "CDataFile.h"

static size_t g_charBufferSize;
static char g_charBuffer[CHAR_BUFFER_SIZE];
static bool technique_status;



//*******************************************************************************************************
/// <summary>
// roundToDecimal() : to avoid re writing preprocessor variable because rouding of value as string in reshade
/// </summary>

float roundToDecimal(float value, int decimals) {
    float factor = std::pow(10.0f, decimals);
    return std::round(value * factor) / factor;
}

//*******************************************************************************************************
/// <summary>
// default_preprocessor() : create preprocessor definition if different of value
/// </summary>
int default_preprocessor(effect_runtime* runtime, std::string name, float defaultValue, bool update, short int display_to_use)
{
    bool exists;
    char value[30];
    size_t  size = sizeof(value);
    float parsed = 0.0;

    int result = 0;

    exists = runtime->get_preprocessor_definition(name.c_str(), value, &size);



    if (exists)
    {
        // update preprocessor definition value only if requested (not needed to check existence)
        float parsed = std::strtof(value, nullptr);
        float rounded = roundToDecimal(defaultValue, 6);

        if ((parsed != rounded) && update)
        {
            runtime->set_preprocessor_definition(name.c_str(), std::to_string(rounded).c_str());
            result = 1;

            log_preprocessor(name, defaultValue, update, exists, parsed, false, 2, display_to_use);

        }
        else
        {
            log_preprocessor(name, defaultValue, update, exists, parsed, false, 3, display_to_use);

        }

    }
    else
    {
        // create preprocessor definition
        runtime->set_preprocessor_definition(name.c_str(), std::to_string(defaultValue).c_str());
        result = 1;

        log_preprocessor(name, defaultValue, update, exists, parsed, false, 1, display_to_use);

    }

    return result;

}

//*******************************************************************************************************
/// <summary>
// default_preprocessor() : initialize preprocess variable to avoid compil error when DCS launched for first time after mod install, not working as get_preprocessor_definition() does always return false...
/// </summary>

void init_preprocess(effect_runtime* runtime)
{
    // if (!shared_data.init_preprocessor && shared_data.effects_feature)
    if (!shared_data.init_preprocessor)
    {

        shared_data.init_preprocessor = true;
        int check = 0;

        check += default_preprocessor(runtime, "MSAAX", 1.0, false, -1);
        check += default_preprocessor(runtime, "MSAAY", 1.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_WIDTH", 1920.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_HEIGHT", 1080.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_RCP_WIDTH", 1.0/1920, false, -1);
        check += default_preprocessor(runtime, "BUFFER_RCP_HEIGHT", 1.0 / 1080, false, -1);
        /*
        check += default_preprocessor(runtime, "BUFFER_WIDTH_QVIN", 1920.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_HEIGHT_QVIN", 1080.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_RCP_WIDTH_QVIN", 1.0 / 1920, false, -1);
        check += default_preprocessor(runtime, "BUFFER_RCP_HEIGHT_QVIN", 1.0 / 1080, false, -1);
        */

        // set one technique to activate pre process


        //save check done
        saveShaderTogglerIniFile();
    }
}


// *******************************************************************************************************
/// <summary>
/// save technique status in the ini file
/// </summary>
/// 
void save_technique_status(std::string technique_name, std::string effect_name, bool technique_status, int quad_view_target)
{
    extern CDataFile technique_iniFile;

    technique_iniFile.SetBool(technique_name, technique_status, "", "technique");
}


// *******************************************************************************************************
/// <summary>
/// read the technique status in the .ini file
/// </summary>
bool read_technique_status_from_file(std::string name)
{
    extern CDataFile technique_iniFile;
    bool status;

    if (name == VR_ONLY_EFFECT)
        // do not use this effect for VR
        status = false;
    else
        status = technique_iniFile.GetBool(name, "technique");

    return status;

}

// *******************************************************************************************************
/// <summary>
/// enumerate technique : call a function for all techniques
/// </summary>
void enumerateTechniques(effect_runtime* runtime)
{
    
    if (shared_data.effects_feature)
    {
        extern CDataFile technique_iniFile;

        //purge the technique vector
        shared_data.technique_vector.clear();

        // init flags for texture or uniform injection
        shared_data.uniform_needed = false;
        //shared_data.texture_needed = false;

        // load the technique file, as status of technique are not relevant
        technique_iniFile.Load(technique_iniFileName);


        // Pass the logging function as the callback
        runtime->enumerate_techniques(nullptr, [](effect_runtime* rt, effect_technique technique) {

            // Buffer size definition
            g_charBufferSize = CHAR_BUFFER_SIZE;

            // Get technique name
            rt->get_technique_name(technique, g_charBuffer, &g_charBufferSize);
            std::string name(g_charBuffer);

            // Get effect name
            g_charBufferSize = CHAR_BUFFER_SIZE;
            rt->get_technique_effect_name(technique, g_charBuffer, &g_charBufferSize);
            std::string eff_name(g_charBuffer);

            // get the "VREM "VR only" empty technique (used to ensure to have at least 1 active technique and make runtime->enumerate_techniques called
            if (name == VR_ONLY_NAME)
            {
                shared_data.VR_only_technique_handle = technique;
                //disable technique if not VR only
                if (!shared_data.VRonly_technique)
                    rt->set_technique_state(shared_data.VR_only_technique_handle, false);
            }

            // if shared_data.VRonly_technique is set, the technique state will not be used, instead the status will be read from the technique .ini file
            if (shared_data.VRonly_technique)
            {
                technique_status = read_technique_status_from_file(name);
                // activate the technique, as it may be keep off from previous game session and it need to be activated before the 3D window at least once per session
                if (technique_status && shared_data.count_draw == 0)
                {
                    rt->set_technique_state(technique, true);
                }

            }
            else
                technique_status = rt->get_technique_state(technique);

            // add technique in vector if active
            if (technique_status)
            {

                //check if shader is containing a VREM texture (¨DEPTH' or 'STENCIL') or other options in GUI that need stencil
                bool has_depth_or_stencil = false;
                if (rt->find_texture_variable(g_charBuffer, DEPTH_NAME) != 0 || rt->find_texture_variable(g_charBuffer, STENCIL_NAME) != 0)
                {
                    has_depth_or_stencil = true;
                    shared_data.texture_needed = true;
                }

                //check if shader is containing VREM uniform
                bool has_uniform = false;
                reshade::api::effect_uniform_variable unif = rt->find_uniform_variable(g_charBuffer, QV_TARGET_NAME);
                int QV_target = shared_data.effect_target_QV;
                if (unif != 0) rt->get_uniform_value_int(unif, &QV_target, 1);


                // add the technique in the vector
                shared_data.technique_vector.push_back({ technique, name, eff_name , technique_status, QV_target });

                //log 
                log_technique_info(rt, technique, name, eff_name, technique_status, QV_target, has_depth_or_stencil);

            }
            // log_technique_info(rt, technique, name, eff_name, technique_status, -1, false);
            // save changes only if VROnly is not set
            if (!shared_data.VRonly_technique)
            {
                save_technique_status(name, eff_name, technique_status, shared_data.effect_target_QV);
            }

            });
        //save technique list
        if (!shared_data.VRonly_technique)
        {

            technique_iniFile.SetFileName(technique_iniFileName);
            technique_iniFile.Save();
        }

        //create post process if not existing
        // init_preprocess(runtime);
    }
}

// *******************************************************************************************************
/// <summary>
/// Called for every technique change => set refresh of technique
/// </summary>
/// 
bool onReshadeSetTechniqueState(effect_runtime* runtime, effect_technique technique, bool enabled) {

    // request update of shader if not VR only
    if (!shared_data.VRonly_technique)
        shared_data.button_technique = true;

    // let things as requested
    return false;
}

// *******************************************************************************************************
/// <summary>
/// Disable all techniques
/// </summary>
/// 
void disableAllTechnique(bool save_flag) {

    // disable all active techniques
    for (int i = 0; i < shared_data.technique_vector.size(); ++i)
        shared_data.runtime->set_technique_state(shared_data.technique_vector[i].technique, false);

    //enable VR only technique
    if (shared_data.VR_only_technique_handle != 0)
        shared_data.runtime->set_technique_state(shared_data.VR_only_technique_handle, true);
    
}

// *******************************************************************************************************
/// <summary>
/// Re enable all techniques
/// </summary>
/// 
void reEnableAllTechnique(bool save_flag) {

    // enable all active techniques
    for (int i = 0; i < shared_data.technique_vector.size(); ++i)
        shared_data.runtime->set_technique_state(shared_data.technique_vector[i].technique, true);

    //disable VR only technique
    if (shared_data.VR_only_technique_handle != 0 )
        shared_data.runtime->set_technique_state(shared_data.VR_only_technique_handle, false);

}


