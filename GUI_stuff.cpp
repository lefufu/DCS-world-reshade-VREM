///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// All functions to setup GUI
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
#include "mod_injection.h"


// TODO : no more used
static int constant_color = false;

void displaySettings(reshade::api::effect_runtime* runtime)
{


	if (ImGui::CollapsingHeader("General info and help"))
	{
		ImGui::PushTextWrapPos();
		ImGui::TextUnformatted("Provide different enhancement that can be toggled on or off below");
		ImGui::TextUnformatted("See https://forum.dcs.world/topic/207154-3dmigoto-vr-mod-for-dcs-label-masking-color-enhancement-sharpen-fxaa-copypaste-of-radio-msg/ for more info");
		ImGui::PopTextWrapPos();
	}

	ImGui::Separator();

	// debug
	if (ImGui::CollapsingHeader("Debug options", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// verbose logs in reshade.log
		ImGui::Checkbox("Debug messages in log", &debug_flag);
		// define the number of draw to differentiate
		ImGui::SliderFloat("# of draws to differentiate", &shared_data.cb_inject_values.testFlag, 0.0f, 5.0f, "ratio = %.0f");
		// to test global shader
		ImGui::SliderFloat("Test global shader", &shared_data.cb_inject_values.testGlobal, 0.0f, 1.0f, "active: %1.0f");
		//capture a fame 
		static int clicked = 0;
		if (ImGui::Button("Capture frame"))
		{
			shared_data.button_capture = true;
		}
		else
		{
			shared_data.button_capture = false;
		}
	}


	// Helicopters
	if (ImGui::CollapsingHeader("Helicopter parameters", ImGuiTreeNodeFlags_DefaultOpen))
	{
		// define the number of draw to differentiate
		ImGui::SliderFloat("Disable Epileptic flashing rotor", &shared_data.cb_inject_values.rotorFlag, 0.0f, 1.0f, "active: %1.0f");
		//ImGui::Checkbox("Disable Epileptic flashing rotor", &shared_data.cb_inject_values.rotorFlag);
	}

	// Save
	if (ImGui::CollapsingHeader("Save and Load", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static int clicked = 0;
		if (ImGui::Button("Save mod settings"))
			clicked++;
		if (clicked & 1)
		{
			saveShaderTogglerIniFile();
			ImGui::SameLine();
			ImGui::Text("Settings saved");
		}

		static int clicked2 = 0;
		if (ImGui::Button("Reload mod settings"))
			clicked2++;
		if (clicked2 & 1)
		{
			load_setting_IniFile();
			ImGui::SameLine();
			ImGui::Text("Settings reloaded");
		}
	}
}