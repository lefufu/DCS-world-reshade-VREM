///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// clone a pipleline
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
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>

#include "global_shared.hpp"
#include "shader_definitions.h"


using namespace reshade::api;

void clone_pipeline(
    device* device,
    reshade::api::pipeline_layout layout,
    uint32_t subobjectCount,
    const reshade::api::pipeline_subobject* subobjects,
    reshade::api::pipeline pipeline,
    std::vector<std::vector<uint8_t>>& ReplaceshaderCode,
    Shader_Definition *newShader)
{

    struct CachedShader {
        void* data = nullptr;
        size_t size = 0;
    };
    std::stringstream s;

    if (debug_flag)
    {
        //log beginning of copy
        s << "CLONING PIPELINE("
            << reinterpret_cast<void*>(pipeline.handle)
            << ") Layout : " << reinterpret_cast<void*>(layout.handle)
            << ", Hash = " << std::hex << newShader->hash << ";"
            << ", subobjects counts: " << (subobjectCount)
            << " )";
        reshade::log_message(reshade::log_level::info, s.str().c_str());
        s.str("");
        s.clear();
    }
    // clone subobjects
    reshade::api::pipeline_subobject* newSubobjects = new reshade::api::pipeline_subobject[subobjectCount];
    memcpy(newSubobjects, subobjects, sizeof(reshade::api::pipeline_subobject) * subobjectCount);

    // clone the desc and change code source
    for (uint32_t i = 0; i < subobjectCount; ++i) {
        auto clonedSubObject = &newSubobjects[i];
        CachedShader* cache;

        //original Desc
        shader_desc desc = *static_cast<shader_desc*>(subobjects[i].data);

        // Clone desc
        reshade::api::shader_desc clonedDesc;
        memcpy(&clonedDesc, &desc, sizeof(reshade::api::shader_desc));

        // Point to cloned desc
        clonedSubObject->data = &clonedDesc;

        // change code source to use the new one 
        // clone ReplaceshaderCode
        cache = new CachedShader{
            malloc(ReplaceshaderCode.back().size()),
            ReplaceshaderCode.back().size()
        };
        memcpy(cache->data, ReplaceshaderCode.back().data(), cache->size);
        clonedDesc.code = cache->data;
        clonedDesc.code_size = cache->size;

        // create cloned pipeline
        reshade::api::pipeline pipelineClone;

        bool builtPipelineOK = device->create_pipeline(
            layout,
            subobjectCount,
            &newSubobjects[0],
            &pipelineClone
        );

        if (builtPipelineOK) {
            // Add cloned Pipeline to the new_shader object
            newShader->substitute_pipeline = pipelineClone;

            if (debug_flag)
            {
                s << "pipeline  cloned  ("
                    << ", orig pipeline handle: " << reinterpret_cast<void*>(pipeline.handle)
                    << ", cloned pipeline handle: " << reinterpret_cast<void*>(pipelineClone.handle)
                    << ")";
                reshade::log_message(reshade::log_level::info, s.str().c_str());
                s.str("");
                s.clear();
            }
        }
        else
        {
            // log error
            s << "!!! Error in cloning pipeline !!! ("
                << ", orig pipeline: " << reinterpret_cast<void*>(pipeline.handle)
                << ")";
            reshade::log_message(reshade::log_level::info, s.str().c_str());
        }

        // free allocated memory as the shader is created or failed
        free(cache->data);
    }
}

