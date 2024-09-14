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
/// Loads the mod settings from the DCSVREM ini file.
/// </summary>
void load_setting_IniFile()
{
	// Will assume it's started at the start of the application and therefore no entries are presents

	CDataFile iniFile;
	if (!iniFile.Load(settings_iniFileName))
	{
		
		// file not existing : set default values
		debug_flag = false;
		shared_data.cb_inject_values.rotorFlag = 0.0;
		shared_data.cb_inject_values.disable_video_IHADSS = 0.0;
		shared_data.cb_inject_values.IHADSSBoresight = 0.0;
		shared_data.cb_inject_values.IHADSSxOffset = 0.0;

		shared_data.cb_inject_values.maskLabels = 0.0;
		shared_data.cb_inject_values.hazeReduction = 1.0;
		shared_data.cb_inject_values.noReflect = 0.0;
		shared_data.cb_inject_values.NVGSize = 1.0;
		shared_data.cb_inject_values.NVGYPos = 0.0;

		shared_data.cb_inject_values.cockpitAdd = 0.0;
		shared_data.cb_inject_values.cockpitMul = 1.0;
		shared_data.cb_inject_values.cockpitSat = 0.0;
		shared_data.cb_inject_values.extAdd = 0.0;
		shared_data.cb_inject_values.extMul = 1.0;
		shared_data.cb_inject_values.extSat = 0.0;

		shared_data.cb_inject_values.fSharpenIntensity = 1.0;
		shared_data.cb_inject_values.lumaFactor = 1.0;
		shared_data.cb_inject_values.Threshold = 128.0;
		shared_data.cb_inject_values.Range = 32.0;
		shared_data.cb_inject_values.Iterations = 2.0;
		shared_data.cb_inject_values.Grain = 48.0;

		shared_data.cb_inject_values.NS430Flag = 0.0;
		shared_data.cb_inject_values.NS430Scale = 4.0;
		shared_data.cb_inject_values.NS430Xpos = 0.7;
		shared_data.cb_inject_values.NS430Ypos = 0.7;
		shared_data.cb_inject_values.NS430Convergence = 1.0;
		shared_data.cb_inject_values.GUIYScale = 1.0;

		return;
	}

	// set default values if file is existing but do not have the variable
	// debug
	debug_flag = iniFile.GetBool("debug", "Debug");
	shared_data.cb_inject_values.testFlag = iniFile.GetFloat("testFlag", "Debug");

	// helicopter
	shared_data.cb_inject_values.rotorFlag = iniFile.GetFloat("rotorFlag", "Helo");
	if (shared_data.cb_inject_values.rotorFlag == FLT_MIN) shared_data.cb_inject_values.rotorFlag = 0.0;
	shared_data.cb_inject_values.disable_video_IHADSS = iniFile.GetFloat("disable_video_IHADSS", "Helo");
	if (shared_data.cb_inject_values.disable_video_IHADSS == FLT_MIN) shared_data.cb_inject_values.disable_video_IHADSS = 0.0;
	shared_data.cb_inject_values.IHADSSBoresight = iniFile.GetFloat("IHADSSBoresight", "Helo");
	if (shared_data.cb_inject_values.IHADSSBoresight == FLT_MIN) shared_data.cb_inject_values.IHADSSBoresight = 0.0;
	shared_data.cb_inject_values.IHADSSxOffset = iniFile.GetFloat("IHADSSxOffset", "Helo");
	if (shared_data.cb_inject_values.IHADSSxOffset == FLT_MIN) shared_data.cb_inject_values.IHADSSxOffset = 0.0;
	//misc
	shared_data.cb_inject_values.maskLabels = iniFile.GetFloat("maskLabels", "Misc");
	if (shared_data.cb_inject_values.maskLabels == FLT_MIN) shared_data.cb_inject_values.maskLabels = 0.0;
	shared_data.cb_inject_values.hazeReduction = iniFile.GetFloat("hazeReduction", "Misc");
	if (shared_data.cb_inject_values.hazeReduction == FLT_MIN) shared_data.cb_inject_values.hazeReduction = 1.0;
	shared_data.cb_inject_values.noReflect = iniFile.GetFloat("noReflect", "Misc");
	if (shared_data.cb_inject_values.noReflect == FLT_MIN) shared_data.cb_inject_values.noReflect = 0.0;
	shared_data.cb_inject_values.NVGSize = iniFile.GetFloat("NVGSize", "Misc");
	if (shared_data.cb_inject_values.NVGSize == FLT_MIN) shared_data.cb_inject_values.NVGSize = 1.0;
	shared_data.cb_inject_values.NVGYPos = iniFile.GetFloat("NVGYPos", "Misc");
	if (shared_data.cb_inject_values.NVGYPos == FLT_MIN) shared_data.cb_inject_values.NVGYPos = 0.0;
	// color
	shared_data.cb_inject_values.cockpitAdd = iniFile.GetFloat("cockpitAdd", "Color");
	if (shared_data.cb_inject_values.cockpitAdd == FLT_MIN) shared_data.cb_inject_values.cockpitAdd = 0.0;
	shared_data.cb_inject_values.cockpitMul = iniFile.GetFloat("cockpitMul", "Color");
	if (shared_data.cb_inject_values.cockpitMul == FLT_MIN) shared_data.cb_inject_values.cockpitMul = 1.0;
	shared_data.cb_inject_values.cockpitSat = iniFile.GetFloat("cockpitSat", "Color");
	if (shared_data.cb_inject_values.cockpitSat == FLT_MIN) shared_data.cb_inject_values.cockpitSat = 0.0;
	shared_data.cb_inject_values.extAdd = iniFile.GetFloat("extAdd", "Color");
	if (shared_data.cb_inject_values.extAdd == FLT_MIN) shared_data.cb_inject_values.extAdd = 0.0;
	shared_data.cb_inject_values.extMul = iniFile.GetFloat("extMul", "Color");
	if (shared_data.cb_inject_values.extMul == FLT_MIN) shared_data.cb_inject_values.extMul = 1.0;
	shared_data.cb_inject_values.extSat = iniFile.GetFloat("extSat", "Color");
	if (shared_data.cb_inject_values.extSat == FLT_MIN) shared_data.cb_inject_values.extSat = 0.0;
	//sharpen
	shared_data.cb_inject_values.fSharpenIntensity = iniFile.GetFloat("sharpenIntensity", "Sharpen");
	if (shared_data.cb_inject_values.fSharpenIntensity == FLT_MIN) shared_data.cb_inject_values.fSharpenIntensity = 1.0;
	shared_data.cb_inject_values.lumaFactor = iniFile.GetFloat("lumaFactor", "Sharpen");
	if (shared_data.cb_inject_values.lumaFactor == FLT_MIN) shared_data.cb_inject_values.lumaFactor = 1.0;
	//deband
	shared_data.cb_inject_values.Threshold = iniFile.GetFloat("Threshold", "Deband");
	if (shared_data.cb_inject_values.Threshold == FLT_MIN) shared_data.cb_inject_values.Threshold = 128.0;
	shared_data.cb_inject_values.Range = iniFile.GetFloat("Range", "Deband");
	if (shared_data.cb_inject_values.Range == FLT_MIN) shared_data.cb_inject_values.Range = 32.0;
	shared_data.cb_inject_values.Iterations = iniFile.GetFloat("Iterations", "Deband");
	if (shared_data.cb_inject_values.Iterations == FLT_MIN) shared_data.cb_inject_values.Iterations = 2.0;
	shared_data.cb_inject_values.Grain = iniFile.GetFloat("Grain", "Deband");
	if (shared_data.cb_inject_values.Grain == FLT_MIN) shared_data.cb_inject_values.Grain = 48.0;
	//NS430
	shared_data.cb_inject_values.NS430Scale = iniFile.GetFloat("Scale", "NS430");
	if (shared_data.cb_inject_values.NS430Scale == FLT_MIN) shared_data.cb_inject_values.NS430Scale = 4.0;
	shared_data.cb_inject_values.NS430Xpos = iniFile.GetFloat("Xpos", "NS430");
	if (shared_data.cb_inject_values.NS430Xpos == FLT_MIN) shared_data.cb_inject_values.NS430Xpos = 0.7;
	shared_data.cb_inject_values.NS430Ypos = iniFile.GetFloat("Ypos", "NS430");
	if (shared_data.cb_inject_values.NS430Ypos == FLT_MIN) shared_data.cb_inject_values.NS430Ypos = 0.7;
	shared_data.cb_inject_values.NS430Convergence = iniFile.GetFloat("Convergence", "NS430");
	if (shared_data.cb_inject_values.NS430Convergence == FLT_MIN) shared_data.cb_inject_values.NS430Convergence = 1.0;
	shared_data.cb_inject_values.GUIYScale = iniFile.GetFloat("GUIYScale", "NS430");
	if (shared_data.cb_inject_values.GUIYScale == FLT_MIN) shared_data.cb_inject_values.GUIYScale = 1.0;

	

	// init global variables not saved in file
	shared_data.cb_inject_values.disable_video_IHADSS = 0.0;
	shared_data.cb_inject_values.AAxFactor = 1.0;
	shared_data.cb_inject_values.AAyFactor = 1.0;
	shared_data.cb_inject_values.IHADSSNoLeft = 0.0;
	shared_data.cb_inject_values.NS430Flag = 0.0;
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
	// helicopter
	iniFile.SetFloat("rotorFlag", shared_data.cb_inject_values.rotorFlag, "Flag to hide helicopters rotor", "Helo");
	iniFile.SetFloat("disable_video_IHADSS", shared_data.cb_inject_values.disable_video_IHADSS, "Flag to enable/disable video in TADS", "Helo");
	iniFile.SetFloat("IHADSSBoresight", shared_data.cb_inject_values.IHADSSBoresight, "Flag to enable boresight IHADSS convergence", "Helo");
	iniFile.SetFloat("IHADSSxOffset", shared_data.cb_inject_values.IHADSSxOffset, "convergence offset value for boresighting IHADSS", "Helo");


	// debug
	iniFile.SetBool("debug", debug_flag, "Enable debug features", "Debug");
	iniFile.SetFloat("testFlag", shared_data.cb_inject_values.testFlag, "for debugging purpose", "Debug");
	//misc
	iniFile.SetFloat("maskLabels", shared_data.cb_inject_values.maskLabels, "for hiding labels", "Misc");
	iniFile.SetFloat("hazeReduction", shared_data.cb_inject_values.hazeReduction, "for haze control", "Misc");
	iniFile.SetFloat("noReflect", shared_data.cb_inject_values.noReflect, "remove A10C instrument reflexion", "Misc");
	iniFile.SetFloat("NVGSize", shared_data.cb_inject_values.NVGSize, "Scale NVG", "Misc");
	iniFile.SetFloat("NVGYPos", shared_data.cb_inject_values.NVGYPos, "height of NVG", "Misc");
	// color
	iniFile.SetFloat("cockpitAdd", shared_data.cb_inject_values.cockpitAdd, "add to all cockpit color component ", "Color");
	iniFile.SetFloat("cockpitMul", shared_data.cb_inject_values.cockpitMul, "multiply of all cockpit color component ", "Color");
	iniFile.SetFloat("cockpitSat", shared_data.cb_inject_values.cockpitSat, "change saturation on all cockpit color component ", "Color");
	iniFile.SetFloat("extAdd", shared_data.cb_inject_values.extAdd, "add to all external color component ", "Color");
	iniFile.SetFloat("extMul", shared_data.cb_inject_values.extMul, "multiply on all external color component ", "Color");
	iniFile.SetFloat("extSat", shared_data.cb_inject_values.extSat, "change saturation of all external color component ", "Color");
	// sharpen
	iniFile.SetFloat("fSharpenIntensity", shared_data.cb_inject_values.fSharpenIntensity, "Sharpen Intensity", "Sharpen");
	iniFile.SetFloat("lumaFactor", shared_data.cb_inject_values.lumaFactor, "Luma", "Sharpen");
	//deband
	iniFile.SetFloat("Threshold", shared_data.cb_inject_values.Threshold, "Deband Threshold", "Deband");
	iniFile.SetFloat("Range", shared_data.cb_inject_values.Range, "Deband Range ", "Deband");
	iniFile.SetFloat("Iterations", shared_data.cb_inject_values.Iterations, "Deband Iterations (not used)", "Deband");
	iniFile.SetFloat("Grain", shared_data.cb_inject_values.Grain, "Deband Grain", "Deband");

	//NS430
	iniFile.SetFloat("Scale", shared_data.cb_inject_values.NS430Scale, "Size of NS430 in VR GUI", "NS430");
	iniFile.SetFloat("Xpos", shared_data.cb_inject_values.NS430Xpos, "Size of NS430 in VR GUI", "NS430");
	iniFile.SetFloat("Ypos", shared_data.cb_inject_values.NS430Ypos, "Size of NS430 in VR GUI", "NS430");
	iniFile.SetFloat("Convergence", shared_data.cb_inject_values.NS430Convergence, "convergence of NS430 in VR GUI", "NS430");
	iniFile.SetFloat("GUIYScale", shared_data.cb_inject_values.GUIYScale, "Y size of VR GUI", "NS430");
	

	iniFile.SetFileName(settings_iniFileName);
	iniFile.Save();
}
