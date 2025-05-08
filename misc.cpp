///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// Miscellaneous functions
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
#include "crc32_hash.hpp"
#include "CDataFile.h"
#include "global_shared.hpp"
#include "mod_injection.h"
#include "shader_definitions.h"

#include "functions.h"
#include "to_string.hpp"

using namespace reshade::api;

// *******************************************************************************************************
/// <summary>
/// Calculates a crc32 hash from the passed in shader bytecode. The hash is used to identity the shader in future runs.
/// </summary>
/// <param name="shaderData"></param>
/// <returns></returns>

uint32_t calculateShaderHash(void* shaderData)
{
	if (nullptr == shaderData)
	{
		return 0;
	}

	const auto shaderDesc = *static_cast<shader_desc*>(shaderData);
	return compute_crc32(static_cast<const uint8_t*>(shaderDesc.code), shaderDesc.code_size);
}

// *******************************************************************************************************
/// <summary>
/// Load one setting with default value
/// </summary>
static float read_float_with_defaut(CDataFile iniFile, bool fileExist, std::string var_to_read, std::string category, float default_value)
{
	float result;

	if (!fileExist)
		result = default_value;
	else
	{
		result = iniFile.GetFloat(var_to_read, category);
		if (result == FLT_MIN) result = default_value;
	}
	return result;
}

static float read_int_with_defaut(CDataFile iniFile, bool fileExist, std::string var_to_read, std::string category, int default_value)
{
	int result;
	if (!fileExist)
		result = default_value;
	else
	{
		result = iniFile.GetInt(var_to_read, category);
		if (result == INT_MIN) result = default_value;
	}
	return result;
}


static std::string read_string_with_defaut(CDataFile iniFile, bool fileExist, std::string var_to_read, std::string category, std::string default_value)
{
	std::string result;
	if (!fileExist)
		result = default_value;
	else
	{
		result = iniFile.GetString(var_to_read, category);
		if (result == "") result = default_value;
	}
	return result;
}

// *******************************************************************************************************
/// <summary>
///convert a VK* string into a uint32_t
/// </summary>

std::unordered_map<std::string, int> vk_map = 
{
		// Function Keys
		{"VK_F1",  0x70}, {"VK_F2",  0x71}, {"VK_F3",  0x72}, {"VK_F4",  0x73},
		{"VK_F5",  0x74}, {"VK_F6",  0x75}, {"VK_F7",  0x76}, {"VK_F8",  0x77},
		{"VK_F9",  0x78}, {"VK_F10", 0x79}, {"VK_F11", 0x7A}, {"VK_F12", 0x7B},
		{"VK_F13", 0x7C}, {"VK_F14", 0x7D}, {"VK_F15", 0x7E}, {"VK_F16", 0x7F},
		{"VK_F17", 0x80}, {"VK_F18", 0x81}, {"VK_F19", 0x82}, {"VK_F20", 0x83},
		{"VK_F21", 0x84}, {"VK_F22", 0x85}, {"VK_F23", 0x86}, {"VK_F24", 0x87},

		// Alphanumeric Keys (A-Z, 0-9)
		{"A", 0x41}, {"B", 0x42}, {"C", 0x43}, {"D", 0x44}, {"E", 0x45},
		{"F", 0x46}, {"G", 0x47}, {"H", 0x48}, {"I", 0x49}, {"J", 0x4A},
		{"K", 0x4B}, {"L", 0x4C}, {"M", 0x4D}, {"N", 0x4E}, {"O", 0x4F},
		{"P", 0x50}, {"Q", 0x51}, {"R", 0x52}, {"S", 0x53}, {"T", 0x54},
		{"U", 0x55}, {"V", 0x56}, {"W", 0x57}, {"X", 0x58}, {"Y", 0x59},
		{"Z", 0x5A},

		{"0", 0x30}, {"1", 0x31}, {"2", 0x32}, {"3", 0x33}, {"4", 0x34},
		{"5", 0x35}, {"6", 0x36}, {"7", 0x37}, {"8", 0x38}, {"9", 0x39},

		// Control Keys
		{"VK_ESCAPE", 0x1B}, {"VK_RETURN", 0x0D}, {"VK_BACK", 0x08}, {"VK_TAB", 0x09},
		{"VK_SPACE", 0x20}, {"VK_SHIFT", 0x10}, {"VK_CONTROL", 0x11}, {"VK_MENU", 0x12}, // Alt key
		{"VK_CAPITAL", 0x14}, {"VK_LSHIFT", 0xA0}, {"VK_RSHIFT", 0xA1},
		{"VK_LCONTROL", 0xA2}, {"VK_RCONTROL", 0xA3}, {"VK_LMENU", 0xA4}, {"VK_RMENU", 0xA5},

		// Arrow Keys
		{"VK_LEFT",  0x25}, {"VK_UP",    0x26}, {"VK_RIGHT", 0x27}, {"VK_DOWN",  0x28},

		// Numpad Keys
		{"VK_NUMPAD0", 0x60}, {"VK_NUMPAD1", 0x61}, {"VK_NUMPAD2", 0x62}, {"VK_NUMPAD3", 0x63},
		{"VK_NUMPAD4", 0x64}, {"VK_NUMPAD5", 0x65}, {"VK_NUMPAD6", 0x66}, {"VK_NUMPAD7", 0x67},
		{"VK_NUMPAD8", 0x68}, {"VK_NUMPAD9", 0x69}, {"VK_MULTIPLY", 0x6A}, {"VK_ADD", 0x6B},
		{"VK_SEPARATOR", 0x6C}, {"VK_SUBTRACT", 0x6D}, {"VK_DECIMAL", 0x6E}, {"VK_DIVIDE", 0x6F},

		// Miscellaneous Keys
		{"VK_INSERT", 0x2D}, {"VK_DELETE", 0x2E}, {"VK_HOME", 0x24}, {"VK_END", 0x23},
		{"VK_PRIOR", 0x21}, {"VK_NEXT", 0x22},  // Page Up, Page Down
		{"VK_SNAPSHOT", 0x2C}, {"VK_SCROLL", 0x91}, {"VK_PAUSE", 0x13},

		// OEM Keys (Symbols and Punctuation)
		{"VK_OEM_1", 0xBA}, {"VK_OEM_PLUS", 0xBB}, {"VK_OEM_COMMA", 0xBC}, {"VK_OEM_MINUS", 0xBD},
		{"VK_OEM_PERIOD", 0xBE}, {"VK_OEM_2", 0xBF}, {"VK_OEM_3", 0xC0}, {"VK_OEM_4", 0xDB},
		{"VK_OEM_5", 0xDC}, {"VK_OEM_6", 0xDD}, {"VK_OEM_7", 0xDE}, {"VK_OEM_8", 0xDF}
	};

uint32_t VK_to_key(std::string key_name)
{
	auto it = vk_map.find(key_name);
	return (it != vk_map.end()) ? it->second : 0; // Return 0 if not found
}



// *******************************************************************************************************
/// <summary>
/// Loads the mod settings from the DCSVREM ini file.
/// </summary>
void load_setting_IniFile()
{
	// Will assume it's started at the start of the application and therefore no entries are presents

	bool fileExist = true;

	CDataFile iniFile;
	if (!iniFile.Load(settings_iniFileName))
		fileExist = false;

	// set default values if file is existing but do not have the variable
	// load mod settings
	debug_flag = iniFile.GetBool("debug_flag", "Debug");

	shared_data.cb_inject_values.testFlag = read_float_with_defaut(iniFile, fileExist, "testFlag", "Debug", 0.0);
	shared_data.cb_inject_values.testFlag = read_float_with_defaut(iniFile, fileExist, "disable_optimisation", "Debug", 0.0);
	
	// reshade effects
	shared_data.effects_feature = iniFile.GetBool("effects_feature", "Effects");
	shared_data.effect_target_QV = read_int_with_defaut(iniFile, fileExist, "QV_render_target", "Effects", 0);

	//keybind
	shared_data.key_TADS_video = read_string_with_defaut(iniFile, fileExist, "key_TADS_video", "KeyBind", "VK_F6");
	shared_data.key_TADS_video_mod = read_string_with_defaut(iniFile, fileExist, "key_TADS_video_mod", "KeyBind", "VK_MENU");
	shared_data.vk_TADS_video = VK_to_key(shared_data.key_TADS_video);
	shared_data.vk_TADS_video_mod = VK_to_key(shared_data.key_TADS_video_mod);

	shared_data.key_IHADSS_boresight = read_string_with_defaut(iniFile, fileExist, "key_IHADSS_boresight", "KeyBind", "VK_F8");
	shared_data.key_IHADSS_boresight_mod = read_string_with_defaut(iniFile, fileExist, "key_IHADSS_boresight_mod", "KeyBind", "VK_MENU");
	shared_data.vk_IHADSS_boresight = VK_to_key(shared_data.key_IHADSS_boresight);
	shared_data.vk_IHADSS_boresight_mod = VK_to_key(shared_data.key_IHADSS_boresight_mod);

	shared_data.key_IHADSSNoLeft = read_string_with_defaut(iniFile, fileExist, "key_IHADSSNoLeft", "KeyBind", "VK_F10");
	shared_data.key_IHADSSNoLeft_mod = read_string_with_defaut(iniFile, fileExist, "key_IHADSSNoLeft_mod", "KeyBind", "VK_MENU");
	shared_data.vk_IHADSSNoLeft = VK_to_key(shared_data.key_IHADSSNoLeft);
	shared_data.vk_IHADSSNoLeft_mod = VK_to_key(shared_data.key_IHADSSNoLeft_mod);


	shared_data.key_NS430 = read_string_with_defaut(iniFile, fileExist, "key_NS430", "KeyBind", "VK_F7");
	shared_data.key_NS430_mod = read_string_with_defaut(iniFile, fileExist, "key_NS430_mod", "KeyBind", "VK_MENU");
	shared_data.vk_NS430 = VK_to_key(shared_data.key_NS430);
	shared_data.vk_NS430_mod = VK_to_key(shared_data.key_NS430_mod);

	shared_data.key_fps = read_string_with_defaut(iniFile, fileExist, "key_fps", "KeyBind", "VK_F5");
	shared_data.key_fps_on_mod = read_string_with_defaut(iniFile, fileExist, "key_fps_on_mod", "KeyBind", "VK_SHIFT");
	shared_data.key_fps_off_mod = read_string_with_defaut(iniFile, fileExist, "key_fps_off_mod", "KeyBind", "VK_CONTROL");
	shared_data.vk_fps = VK_to_key(shared_data.key_fps);
	shared_data.vk_fps_on_mod = VK_to_key(shared_data.key_fps_on_mod);
	shared_data.vk_fps_off_mod = VK_to_key(shared_data.key_fps_off_mod);

	shared_data.key_technique = read_string_with_defaut(iniFile, fileExist, "key_technique", "KeyBind", "VK_F5");
	shared_data.key_technique_mod = read_string_with_defaut(iniFile, fileExist, "key_technique_mod", "KeyBind", "VK_MENU");
	shared_data.vk_technique = VK_to_key(shared_data.key_technique);
	shared_data.vk_technique_mod = VK_to_key(shared_data.key_technique_mod);


	// helicopter
	shared_data.helo_feature = iniFile.GetBool("helo_feature", "Helo");
	shared_data.cb_inject_values.rotorFlag = read_float_with_defaut(iniFile, fileExist, "rotorFlag", "Helo", 0.0);
	shared_data.cb_inject_values.disable_video_IHADSS = read_float_with_defaut(iniFile, fileExist, "disable_video_IHADSS", "Helo", 0.0);
	shared_data.cb_inject_values.IHADSSBoresight = read_float_with_defaut(iniFile, fileExist, "IHADSSBoresight", "Helo", 0.0);
	shared_data.cb_inject_values.IHADSSxOffset = read_float_with_defaut(iniFile, fileExist, "IHADSSxOffset", "Helo", 0.0);
	shared_data.cb_inject_values.TADSDay = read_float_with_defaut(iniFile, fileExist, "TADSDayValue", "Helo", 0.28);
	shared_data.cb_inject_values.TADSNight = read_float_with_defaut(iniFile, fileExist, "TADSnightValue", "Helo", 0.02);

	//misc
	shared_data.misc_feature = iniFile.GetBool("misc_feature", "Misc");
	shared_data.cb_inject_values.maskLabels = read_float_with_defaut(iniFile, fileExist, "maskLabels", "Misc", 0.0);
	shared_data.cb_inject_values.hazeReduction = read_float_with_defaut(iniFile, fileExist, "hazeReduction", "Misc", 0.0);
	shared_data.cb_inject_values.noReflect = read_float_with_defaut(iniFile, fileExist, "noReflect", "Misc", 0.0);
	shared_data.cb_inject_values.NVGSize = read_float_with_defaut(iniFile, fileExist, "NVGSize", "Misc", 1.0);
	shared_data.cb_inject_values.NVGYPos = read_float_with_defaut(iniFile, fileExist, "NVGYPos", "Misc", 0.0);

	// color
	shared_data.color_feature = iniFile.GetBool("color_feature", "Color");
	shared_data.cb_inject_values.colorFlag = read_float_with_defaut(iniFile, fileExist, "colorFlag", "Color", 0.0);
	shared_data.cb_inject_values.cockpitAdd = read_float_with_defaut(iniFile, fileExist, "cockpitAdd", "Color", 0.0);
	shared_data.cb_inject_values.cockpitMul = read_float_with_defaut(iniFile, fileExist, "cockpitMul", "Color", 1.0);
	shared_data.cb_inject_values.cockpitSat = read_float_with_defaut(iniFile, fileExist, "cockpitSat", "Color", 0.0);
	shared_data.cb_inject_values.extAdd = read_float_with_defaut(iniFile, fileExist, "extAdd", "Color", 0.0);
	shared_data.cb_inject_values.extMul = read_float_with_defaut(iniFile, fileExist, "extMul", "Color", 1.0);
	shared_data.cb_inject_values.extSat = read_float_with_defaut(iniFile, fileExist, "extSat", "Color", 0.0);

	//sharpen
	shared_data.sharpenDeband_feature = iniFile.GetBool("sharpenDeband_feature", "Sharpen");
	shared_data.cb_inject_values.sharpenFlag = read_float_with_defaut(iniFile, fileExist, "sharpenFlag", "Sharpen", 0.0);
	shared_data.cb_inject_values.fSharpenIntensity = read_float_with_defaut(iniFile, fileExist, "sharpenIntensity", "Sharpen", 1.0);
	shared_data.cb_inject_values.lumaFactor = read_float_with_defaut(iniFile, fileExist, "lumaFactor", "Sharpen", 1.0);

	//deband
	shared_data.cb_inject_values.debandFlag = read_float_with_defaut(iniFile, fileExist, "debandFlag", "Sharpen", 0.0);
	shared_data.cb_inject_values.Threshold = read_float_with_defaut(iniFile, fileExist, "Threshold", "Deband", 128.0);
	shared_data.cb_inject_values.Range = read_float_with_defaut(iniFile, fileExist, "Range", "Deband", 32.0);
	shared_data.cb_inject_values.Iterations = read_float_with_defaut(iniFile, fileExist, "Iterations", "Deband", 2.0);
	shared_data.cb_inject_values.Grain = read_float_with_defaut(iniFile, fileExist, "Grain", "Deband", 48.0);

	//NS430
	shared_data.NS430_feature = iniFile.GetBool("NS430_feature", "NS430");
	shared_data.cb_inject_values.NS430Scale = read_float_with_defaut(iniFile, fileExist, "Scale", "NS430", 1.0);
	shared_data.cb_inject_values.NS430Xpos = read_float_with_defaut(iniFile, fileExist, "Xpos", "NS430", 0.7);
	shared_data.cb_inject_values.NS430Ypos = read_float_with_defaut(iniFile, fileExist, "Ypos", "NS430", 0.7);
	shared_data.cb_inject_values.NS430Convergence = read_float_with_defaut(iniFile, fileExist, "Convergence", "NS430", 1.0);
	shared_data.cb_inject_values.GUIYScale = read_float_with_defaut(iniFile, fileExist, "GUIYScale", "NS430", 1.0);

	// reshade effects
	shared_data.effects_feature = iniFile.GetBool("effects_feature", "Effects");
	shared_data.effect_target_QV = read_int_with_defaut(iniFile, fileExist, "QV_render_target", "Effects", 0);
	shared_data.VRonly_technique = iniFile.GetBool("VRonly_technique", "Effects");

	// fps limiter
	shared_data.fps_limit = read_int_with_defaut(iniFile, fileExist, "fps_limit", "fps", 120);
	shared_data.fps_feature = iniFile.GetBool("fps_feature", "fps");

	//delay for compilation
	shared_data.compil_delay = read_int_with_defaut(iniFile, fileExist, "compil_delay", "debug", 100);

	// init global variables not saved in file
	shared_data.cb_inject_values.disable_video_IHADSS = 0.0;
	shared_data.cb_inject_values.AAxFactor = 1.0;
	shared_data.cb_inject_values.AAyFactor = 1.0;
	shared_data.cb_inject_values.IHADSSNoLeft = 0.0;
	shared_data.cb_inject_values.NS430Flag = 0.0;

	// save initial value to display relaunch game messages when changed
	shared_data.init_color_feature = shared_data.color_feature;
	shared_data.init_sharpenDeband_feature = shared_data.sharpenDeband_feature;
	shared_data.init_misc_feature = shared_data.misc_feature;
	shared_data.init_helo_feature = shared_data.helo_feature;
	shared_data.init_NS430_feature = shared_data.NS430_feature;
	shared_data.init_debug_feature = debug_flag;
	shared_data.init_VRonly_technique = shared_data.VRonly_technique;

}

// *******************************************************************************************************
/// <summary>
/// Saves the mod settings to the DCSVREM ini file.
/// </summary>
void saveShaderTogglerIniFile()
{
	// format: first section with # of groups, then per group a section with pixel and vertex shaders, as well as their name and key value.
	// groups are stored with "Group" + group counter, starting with 0.
	CDataFile iniFile;

	//keybind
	iniFile.SetValue("key_TADS_video", shared_data.key_TADS_video, "key used to disable TADS mirror in IHADSS features", "KeyBind");
	iniFile.SetValue("key_TADS_video_mod", shared_data.key_TADS_video_mod, "modifier key used to disable TADS mirror in IHADSS  features", "KeyBind");

	iniFile.SetValue("key_IHADSS_boresight", shared_data.key_IHADSS_boresight, "key used for IHADSS boresight features", "KeyBind");
	iniFile.SetValue("key_IHADSS_boresight_mod", shared_data.key_IHADSS_boresight_mod, "modifier key used for IHADSS boresight features", "KeyBind");

	iniFile.SetValue("key_IHADSS_boresight", shared_data.key_IHADSS_boresight, "key used for IHADSS boresight features", "KeyBind");
	iniFile.SetValue("key_IHADSS_boresight_mod", shared_data.key_IHADSS_boresight_mod, "modifier key used for IHADSS boresight features", "KeyBind");

	iniFile.SetValue("key_IHADSSNoLeft", shared_data.key_IHADSSNoLeft, "key used to disable IHADSS left features", "KeyBind");
	iniFile.SetValue("key_IHADSSNoLeft_mod", shared_data.key_IHADSSNoLeft_mod, "modifier key key used to disable IHADSS left features", "KeyBind");
	
	
	iniFile.SetValue("key_NS430", shared_data.key_NS430, "key used for NS430 features", "KeyBind");
	iniFile.SetValue("key_NS430_mod", shared_data.key_NS430_mod, "modifier key used for NS430 features", "KeyBind");

	iniFile.SetValue("key_fps", shared_data.key_fps, "key used for fps limiter", "KeyBind");
	iniFile.SetValue("key_fps_on_mod", shared_data.key_fps_on_mod, "modifier key used to set fps limiter on", "KeyBind");
	iniFile.SetValue("key_fps_off_mod", shared_data.key_fps_off_mod, "modifier key used to set fps limiter off", "KeyBind");
	
	// helicopter
	iniFile.SetBool("helo_feature", shared_data.helo_feature, "Helicopter features", "Helo");
	iniFile.SetFloat("rotorFlag", shared_data.cb_inject_values.rotorFlag, "Flag to hide helicopters rotor", "Helo");
	iniFile.SetFloat("disable_video_IHADSS", shared_data.cb_inject_values.disable_video_IHADSS, "Flag to enable/disable video in TADS", "Helo");
	iniFile.SetFloat("IHADSSBoresight", shared_data.cb_inject_values.IHADSSBoresight, "Flag to enable boresight IHADSS convergence", "Helo");
	iniFile.SetFloat("IHADSSxOffset", shared_data.cb_inject_values.IHADSSxOffset, "convergence offset value for boresighting IHADSS", "Helo");
	iniFile.SetFloat("TADSDayValue", shared_data.cb_inject_values.TADSDay, "trigger value to disable TADS video during day", "Helo");
	iniFile.SetFloat("TADSNightValue", shared_data.cb_inject_values.TADSNight, "trigger value to disable TADS video during night", "Helo");

	// reshade effects
	iniFile.SetBool("effects_feature", shared_data.effects_feature, "Enable Reshade effects", "Effects");
	iniFile.SetInt("QV_render_target", shared_data.effect_target_QV, "QV render target", "Effects");
	iniFile.SetBool("VRonly_technique", shared_data.VRonly_technique, "Reshade technique applied only on VR views", "Effects");

	// debug
	iniFile.SetBool("debug_feature", debug_flag, "Enable debug features", "Debug");
	iniFile.SetFloat("testFlag", shared_data.cb_inject_values.testFlag, "for debugging purpose", "Debug");
	iniFile.SetBool("disable_optimisation", shared_data.disable_optimisation, "Disable Optimisation ", "Debug");
	iniFile.SetBool("debug_flag", debug_flag, "Disable Optimisation ", "Debug");
	iniFile.SetFloat("compil_delay", shared_data.compil_delay, "compilation delay", "Debug");

	//misc
	iniFile.SetBool("misc_feature", shared_data.misc_feature, "Enable Misc features", "Misc");
	iniFile.SetFloat("maskLabels", shared_data.cb_inject_values.maskLabels, "for hiding labels", "Misc");
	iniFile.SetFloat("hazeReduction", shared_data.cb_inject_values.hazeReduction, "for haze control", "Misc");
	iniFile.SetFloat("noReflect", shared_data.cb_inject_values.noReflect, "remove A10C instrument reflexion", "Misc");
	iniFile.SetFloat("NVGSize", shared_data.cb_inject_values.NVGSize, "Scale NVG", "Misc");
	iniFile.SetFloat("NVGYPos", shared_data.cb_inject_values.NVGYPos, "height of NVG", "Misc");
	// color
	iniFile.SetBool("color_feature", shared_data.color_feature, "Enable Color features", "Color");
	iniFile.SetFloat("colorFlag", shared_data.cb_inject_values.colorFlag, "Activate color change ", "Color");
	iniFile.SetFloat("cockpitAdd", shared_data.cb_inject_values.cockpitAdd, "add to all cockpit color component ", "Color");
	iniFile.SetFloat("cockpitMul", shared_data.cb_inject_values.cockpitMul, "multiply of all cockpit color component ", "Color");
	iniFile.SetFloat("cockpitSat", shared_data.cb_inject_values.cockpitSat, "change saturation on all cockpit color component ", "Color");
	iniFile.SetFloat("extAdd", shared_data.cb_inject_values.extAdd, "add to all external color component ", "Color");
	iniFile.SetFloat("extMul", shared_data.cb_inject_values.extMul, "multiply on all external color component ", "Color");
	iniFile.SetFloat("extSat", shared_data.cb_inject_values.extSat, "change saturation of all external color component ", "Color");
	// sharpen
	iniFile.SetBool("sharpenDeband_feature", shared_data.sharpenDeband_feature, "Enable Sharpen and deband features", "Sharpen");
	iniFile.SetFloat("sharpenFlag", shared_data.cb_inject_values.sharpenFlag, "Enable sharpen", "Sharpen");
	iniFile.SetFloat("fSharpenIntensity", shared_data.cb_inject_values.fSharpenIntensity, "Sharpen Intensity", "Sharpen");
	iniFile.SetFloat("lumaFactor", shared_data.cb_inject_values.lumaFactor, "Luma", "Sharpen");
	//deband
	iniFile.SetFloat("debandFlag", shared_data.cb_inject_values.debandFlag, "Enable deband", "Sharpen");
	iniFile.SetFloat("Threshold", shared_data.cb_inject_values.Threshold, "Deband Threshold", "Deband");
	iniFile.SetFloat("Range", shared_data.cb_inject_values.Range, "Deband Range ", "Deband");
	iniFile.SetFloat("Iterations", shared_data.cb_inject_values.Iterations, "Deband Iterations (not used)", "Deband");
	iniFile.SetFloat("Grain", shared_data.cb_inject_values.Grain, "Deband Grain", "Deband");

	//NS430
	iniFile.SetBool("NS430_feature", shared_data.NS430_feature, "Enable NS430 features", "NS430");
	iniFile.SetFloat("Scale", shared_data.cb_inject_values.NS430Scale, "Size of NS430 in VR GUI", "NS430");
	iniFile.SetFloat("Xpos", shared_data.cb_inject_values.NS430Xpos, "Size of NS430 in VR GUI", "NS430");
	iniFile.SetFloat("Ypos", shared_data.cb_inject_values.NS430Ypos, "Size of NS430 in VR GUI", "NS430");
	iniFile.SetFloat("Convergence", shared_data.cb_inject_values.NS430Convergence, "convergence of NS430 in VR GUI", "NS430");
	iniFile.SetFloat("GUIYScale", shared_data.cb_inject_values.GUIYScale, "Y size of VR GUI", "NS430");
	
	// fps limiter
	iniFile.SetInt("fps_limit", shared_data.fps_limit, "fps_limiter", "fps");
	iniFile.SetBool("fps_feature", shared_data.fps_feature, "Enable fps limiter feature", "fps");

	iniFile.SetFileName(settings_iniFileName);
	iniFile.Save();
}


// *******************************************************************************************************
/// <summary>
/// set pipelines in pipeline_by_hash regarding features activated in GUI
/// </summary>
void init_mod_features()
{
	
	// build the pipeline to use regarding options set
	//for (auto& entry : shader_by_hash)
	for (std::pair<const uint32_t, Shader_Definition> &entry : shader_by_hash)
	{
		
		bool add_line = false;

		// add misc entries
		if (shared_data.misc_feature && (entry.second.feature == Feature::Label || entry.second.feature == Feature::NoReflect || entry.second.feature == Feature::NVG || entry.second.feature == Feature::Haze))
		{
			add_line = true;
		}

		// add Helo entries
		if (shared_data.helo_feature && (entry.second.feature == Feature::IHADSS || entry.second.feature == Feature::Rotor))
		{
			add_line = true;
		}

		// add NS430 entries
		if (shared_data.NS430_feature && (entry.second.feature == Feature::NS430  || entry.second.feature == Feature::GUI))
		{
			add_line = true;
		}

		// add entries for depthstencil,  for color, sharpen, label
		if  ((shared_data.color_feature || shared_data.sharpenDeband_feature || shared_data.cb_inject_values.maskLabels || shared_data.effects_feature) && entry.second.feature == Feature::GetStencil)
		{
			add_line = true;
		}

		// safety : to ensure effects whatever settings
		if ((entry.second.feature == Feature::Global || entry.second.feature == Feature::Haze || entry.second.feature == Feature::HazeMSAA2x || entry.second.feature == Feature::mapMode
				|| entry.second.feature == Feature::VRMode))
		{
			add_line = true;
		}

		// optimization: do not load global shader if no color changes
		if (entry.second.feature == Feature::Global && !shared_data.color_feature && !shared_data.sharpenDeband_feature && !shared_data.init_debug_feature)
		{
			entry.second.action = action_injectText | action_log;
		}

		// add effect entries
		if (entry.second.feature == Feature::Effects )
		{
			add_line = true;
		}

		log_filter_shader(entry, add_line);

		/*
		std::stringstream s;
		s << "add shader : " << std::hex << entry.first << ", " << entry.second.action << ", " << to_string(entry.second.feature) << ", " << entry.second.draw_count << " add_line =" << add_line  <<  ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		*/
		

		if (add_line)
		{
			pipeline_by_hash.emplace(
				entry.first,
				Shader_Definition(entry.second.action,
					entry.second.feature,
					entry.second.replace_filename,
					entry.second.draw_count)
			);


		}

	}

	shared_data.time_per_frame = std::chrono::high_resolution_clock::duration(std::chrono::seconds(1)) / shared_data.fps_limit;

	// init flag to load technique list
	// shared_data.technique_init == -1;
}