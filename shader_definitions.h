///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// definition of shader to mod
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

#pragma once

#include "reshade.hpp"
#include <unordered_map>


#define MAXNAMECHAR 30
#define CAPTURE_KEY VK_F1

// warning : do not forget to update to_string() if new action or features are added

//mod actions
// 
//replace = the shader will be replaced by a modded one during init
static const uint32_t action_replace = 0b00000001;
// skip : the shader is to be skipped after a count of draw
static const uint32_t action_skip = 0b00000010;
//log : the shader will trigger logging of resources 
static const uint32_t action_log = 0b00000100;
//identify : the shader will be used to identify a configuration of the game (eg VR,..)
static const uint32_t action_identify = 0b00001000;
//inject Texture : the shader need to have textures pushed as additional parameters 
static const uint32_t action_injectText = 0b00010000;
//inject count : the shader will trigger call count 
static const uint32_t action_count = 0b00100000;
//replace_bind = the shader will be replaced by a modded one during bind
static const uint32_t action_replace_bind = 0b01000000;
//inject constant buffer
static const uint32_t action_injectCB = 0b10000000;


// mod features
enum class Feature : uint32_t
{
	//null
	Null = 0,
	// Rotor : disable rotor when in cockpit view
	Rotor = 1,
	// Global : global effects, change color, sharpen, ... for cockpit or outside
	Global = 2,
	// Label : mask labels by cockpit frame
	Label = 3,
	// Get stencil : copy texture t4 from global illum shader
	GetStencil = 4,
	// IHADSS : handle feature for AH64 IHADSS
	IHADSS = 5,
	// define if VRMode
	VRMode = 6,
	// define if view is in welcome screen or map
	mapMode = 7,
	// haze control
	Haze = 8,
	// haze control & flag MSAA
	HazeMSAA2x = 9,
	// remove A10C instrument reflect
	NoReflect = 11,
	// NS430 
	NS430 = 12,
	// NVG
	NVG = 13,
	//GUI 
	GUI = 14,
	// Reshade effects
	Effects = 15,
	// Testing : for testing purpose
	Testing = 20,
	// VS of 2nd global color change PS
	VS_global2 = 21,
	// PS of sky to not modify gAtmInstensity
	Sky = 22

};


// it is assumed that PS, VS and other kind of shaders to be modded in DCS are unique in their unique associated pipeline
struct Shader_Definition {
	uint32_t action; //what is to be done for the pipeline/shader
	Feature feature; // to class pipeline/shader by mod feature
	wchar_t replace_filename[MAXNAMECHAR]; //file name of the modded shader, used only for action "replace"
	uint32_t draw_count; //used only for action "skip"
	reshade::api::pipeline substitute_pipeline; //cloned pipeline/shader with code changed
	uint32_t hash;

	// Constructor
	Shader_Definition(uint32_t act, Feature feat, const wchar_t* filename, uint32_t count)
		: action(act), feature(feat), draw_count(count) {
		wcsncpy_s(replace_filename, filename, MAXNAMECHAR);
		hash = 0;
	}

};

// map of pipeline detected, by using hash from map below
extern std::unordered_map<uint64_t, Shader_Definition> pipeline_by_handle;

// map to detect pipeline to process regarding the features enabled in GUI
extern std::unordered_map<uint32_t, Shader_Definition> pipeline_by_hash;

// map containing all shaders of the mod, filtered in init_mod_features() to build map above
extern std::unordered_map<uint32_t, Shader_Definition> shader_by_hash;

