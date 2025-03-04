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



// *******************************************************************************************************
/// <summary>
/// save technique status in the ini file
/// </summary>
/// 
void save_technique_status(std::string technique_name, std::string effect_name, bool technique_status, int quad_view_target)
{
    extern CDataFile technique_iniFile;

    /* technique_iniFile.SetValue("technique_name", technique_name, "name of technique for effect", effect_name);
    technique_iniFile.SetBool("technique_status", technique_status, "status of technique", effect_name);
    technique_iniFile.SetInt("quad_view_target", quad_view_target, "Quad view target", effect_name);  
    */
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
/// rebuild the techique vector from technique file (needed if "technique only for VR" option setup)
/// </summary>
/*  no more used
void load_technique_vector(CDataFile technique_iniFile, effect_runtime* runtime)
{
    // Get the list of section names
    technique_iniFile.Load(technique_iniFileName);
    

    for (uint32_t i = 0; i < technique_iniFile.SectionCount(); i++)
    {

        effect_technique technique;
        std::string technique_name, effect_name;
        effect_name = technique_iniFile.m_Sections[i].szName;
        technique_name = technique_iniFile.GetString("technique_name", technique_iniFile.m_Sections[i].szName);
        bool technique_status = technique_iniFile.GetBool("technique_status", technique_iniFile.m_Sections[i].szName);
        int QV_target  = technique_iniFile.GetInt("quad_view_target", technique_iniFile.m_Sections[i].szName);
        uint32_t j = 0;
        if (technique_name != "")
        {
            if (technique_status)
            {
                technique = runtime->find_technique(effect_name.c_str(), technique_name.c_str());
                if (technique == 0)
                {
                    std::stringstream s;
                    s << "!!!! Enumerate technique Error : effect name = " << effect_name << ", technique name = " << technique_name << " !!!";
                    reshade::log::message(reshade::log::level::info, s.str().c_str());
                }
                shared_data.technique_vector.push_back({ technique, technique_name, effect_name , technique_status, QV_target });
                log_technique_loaded(j);
                j++;
            }

        }
        
    }
}
*/

// *******************************************************************************************************
/// <summary>
/// enumerate technique : call a function for all techniques
/// </summary>
void enumerateTechniques(effect_runtime* runtime)
{
    
    extern CDataFile technique_iniFile;

    //purge the technique vector
    shared_data.technique_vector.clear();

    // init flags for texture or uniform injection
    shared_data.uniform_needed = false;
    shared_data.texture_needed = false;
      
    // if (shared_data.VRonly_technique)
    // {      
        // load the technique file, as status of technique are not relevant
        technique_iniFile.Load(technique_iniFileName);       
    // }
    
    if ((debug_flag))
    {
        std::stringstream s;
        s << " ******** enumerate technique called ;";
        reshade::log::message(reshade::log::level::warning, s.str().c_str());
    }

    // Pass the logging function as the callback
    runtime->enumerate_techniques(nullptr, [](effect_runtime* rt, effect_technique technique) {
        
        if ((debug_flag))
        {
            std::stringstream s;
            s << " ******** runtime->enumerate_techniques technique called ;";
            reshade::log::message(reshade::log::level::warning, s.str().c_str());
        }
        // reshade::log::message(reshade::log::level::info, "**** sub Enumerate technique called ****");
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
        // 
        if (shared_data.VRonly_technique)
        {
            technique_status = read_technique_status_from_file(name);

            if ((debug_flag))
            {
                std::stringstream s;
                s << " ******** read status from file for technique " << name;
                reshade::log::message(reshade::log::level::warning, s.str().c_str());
            }

            if (technique_status && shared_data.count_draw == 0)
            {
                std::stringstream s;
                s << " ******** set technique true " << name;
                reshade::log::message(reshade::log::level::warning, s.str().c_str());

                rt->set_technique_state(technique, true);
            }

        }
        else
            technique_status = rt->get_technique_state(technique);

        if ((debug_flag))
        {
            std::stringstream s;
            s << " ******** technique status = " << technique_status;
            reshade::log::message(reshade::log::level::warning, s.str().c_str());
        }

        // add technique in vector if active
        if (technique_status)
        {

            //check if shader is containing a VREM texture (¨DEPTH' or 'STENCIL'
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
        
        if ((debug_flag))
        {
            std::stringstream s;
            s << " ******** saving techniques = " << shared_data.technique_vector.size() << ";";
            reshade::log::message(reshade::log::level::warning, s.str().c_str());
        }
        technique_iniFile.SetFileName(technique_iniFileName);
        technique_iniFile.Save();
    }
    /* else
    {

    }
    */

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
