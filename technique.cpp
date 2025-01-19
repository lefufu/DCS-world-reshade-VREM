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

static size_t g_charBufferSize;
static char g_charBuffer[CHAR_BUFFER_SIZE];
static bool technique_status;

void enumerateTechniques(effect_runtime* runtime)
{
    
    //purge the technique vector
    shared_data.technique_vector.clear();

    // init flags for texture or uniform injection
    shared_data.uniform_needed = false;
    shared_data.texture_needed = false;
    
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

        technique_status = rt->get_technique_state(technique);

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

        });
}

