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

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("General info and help"))
	{
		ImGui::PushTextWrapPos();
		ImGui::TextUnformatted("Provide different enhancement that can be toggled on or off below");
		ImGui::TextUnformatted("See DCS world forums (Home / English / Digital Combat Simulator / DCS World Topics / DCS Modding / Utility/Program Mods for DCS World / Reshade VREM) for more info");
		ImGui::PopTextWrapPos();
	}
	ImGui::Separator();

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("Reshade Techniques in VR displays"))
	{
		//disable option if 2D
		/*
		if (!shared_data.cb_inject_values.VRMode)
		{
			ImGui::Text("2D mode : Technique injection disabled");
			ImGui::BeginDisabled();
		}
		*/
			// enable/disable Reshade effects changes
			ImGui::Checkbox("Activate reshade Techniques in VR", &shared_data.effects_feature);
			if (shared_data.effects_feature != shared_data.init_effects_feature)
			{
				ImGui::SameLine();
				ImGui::Text("clik on button Refresh Techniques");
				saveShaderTogglerIniFile();

			}
			
			ImGui::Text("Use %s+%s in game to enable/disable technique in VR ", shared_data.key_technique_mod, shared_data.key_technique);
			// if (!shared_data.effects_feature || shared_data.count_draw <= 2)
			if (!shared_data.effects_feature )
			{
				ImGui::BeginDisabled();
			}

			// define technique QV render targets
			ImGui::Text("Target area for standard reshade Techniques if Quad View (no VREM Techniques):");
			ImGui::RadioButton("All", &shared_data.effect_target_QV, 0); ImGui::SameLine();
			ImGui::RadioButton("Outer only", &shared_data.effect_target_QV, 1); ImGui::SameLine();
			ImGui::RadioButton("Inner only", &shared_data.effect_target_QV, 2);

			
			// enable/disable technique displayed only in VR (only if the VREM "fake" VR only technique is found)
			/*if (shared_data.VR_only_technique_handle == 0)
			{
				ImGui::BeginDisabled();
			}*/

			
			if (ImGui::Checkbox("Techniques only for VR views", &shared_data.VRonly_technique))
			{
				if (!shared_data.VRonly_technique)
				{
					// select techniques saved in techniqe.ini file
					reEnableAllTechnique();
				}
				if (shared_data.VRonly_technique)
				{
					// deselect techniques
					disableAllTechnique();
				}
			}
		
			if (shared_data.VRonly_technique)
			{
				ImGui::Text("!!! Technique are still active for VR, even if no more selected in Reshade !!!");
				ImGui::Text("!!! Untick option to add or remove a new technique for VR, otherwise change will not be taken into account !!!");
				ImGui::Text(" Technique(s) activated in VR: ");
				ImGui::Text(" ----------------------------- ");

				std::stringstream s;
				for (int i = 0; i < shared_data.technique_vector.size(); ++i)
				{
					ImGui::Text(" > %s", shared_data.technique_vector[i].name.c_str());
				}

			}
			/*
			if (shared_data.VR_only_technique_handle == 0)
			{
				ImGui::EndDisabled();
			}
			*/

			// if (!shared_data.effects_feature || shared_data.count_draw <= 2)

			// refresh techniques 
			if (ImGui::Button("Refresh Techniques"))
			{
				shared_data.button_technique = true;
			}

			if (!shared_data.effects_feature)
			{
				ImGui::EndDisabled();
			}
		/*
		if (!shared_data.cb_inject_values.VRMode)
		{
			ImGui::EndDisabled();
		}
		*/
	}

	ImGui::Separator();

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("Color change"))
	{
		//-----------------------------------------------------------------------------------------------------------------
		// enable/disable color changes
		ImGui::Checkbox("Activate Color feature", &shared_data.color_feature);
		if (shared_data.color_feature != shared_data.init_color_feature)
		{
			ImGui::SameLine();
			ImGui::Text("Game restart needed (option saved)");
			saveShaderTogglerIniFile();

		}

		if (!shared_data.color_feature)
		{
			ImGui::BeginDisabled();
		}
		ImGui::SliderFloat("Enable color changes", &shared_data.cb_inject_values.colorFlag, 0.0f, 1.0f, "Active: %1.0f");
		ImGui::Separator();
		if (!shared_data.cb_inject_values.colorFlag)
		{
			ImGui::BeginDisabled();
		}

		// set color for cockpit
		ImGui::SliderFloat("Cockpit color common addition", &shared_data.cb_inject_values.cockpitAdd, 0.0f, 1.0f, "Add: %.2f");
		ImGui::SliderFloat("Cockpit color common multiplication", &shared_data.cb_inject_values.cockpitMul, 0.0f, 5.0f, "Mul: %.2f");
		ImGui::SliderFloat("Cockpit color saturation", &shared_data.cb_inject_values.cockpitSat, -5.0f, 5.0f, "Sat: %.2f");
		ImGui::Separator();
		// set color for external
		ImGui::SliderFloat("External color common addition", &shared_data.cb_inject_values.extAdd, 0.0f, 1.0f, "Add : %.2f");
		ImGui::SliderFloat("External color common multiplication", &shared_data.cb_inject_values.extMul, 0.0f, 5.0f, "Mul: %.2f");
		ImGui::SliderFloat("External color saturation", &shared_data.cb_inject_values.extSat, -5.0f, 5.0f, "Sat: %.2f");

		if (!shared_data.cb_inject_values.colorFlag)
		{
			ImGui::EndDisabled();
		}


		if (!shared_data.color_feature)
		{
			ImGui::EndDisabled();
		}
	}
	ImGui::Separator();

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("Cockpit sharpen, sky and sea deband"))
	{
		
		ImGui::Checkbox("Activate sharpen/deband", &shared_data.sharpenDeband_feature);
		if (shared_data.sharpenDeband_feature != shared_data.init_sharpenDeband_feature)
		{
			ImGui::SameLine();
			ImGui::Text("Game restart needed (option saved)");
			saveShaderTogglerIniFile();

		}
		if (!shared_data.sharpenDeband_feature)
		{
			ImGui::BeginDisabled();
		}
		//-----------------------------------------------------------------------------------------------------------------
		// enable/disable cockpit sharpen
		ImGui::SliderFloat("Enable cockpit sharpen", &shared_data.cb_inject_values.sharpenFlag, 0.0f, 1.0f, "Active: %1.0f");
		ImGui::Separator();
		if (!shared_data.cb_inject_values.sharpenFlag)
		{
			ImGui::BeginDisabled();
		}
		// set sharpen options
		ImGui::SliderFloat("Sharpen intensity", &shared_data.cb_inject_values.fSharpenIntensity, 0.0f, 10.0f, "Sharpen: %.2f");
		ImGui::SliderFloat("Sharpen luma", &shared_data.cb_inject_values.lumaFactor, 0.0f, 10.0f, "Luma: %.2f");

		if (!shared_data.cb_inject_values.sharpenFlag)
		{
			ImGui::EndDisabled();
		}
		ImGui::Separator();
		//-----------------------------------------------------------------------------------------------------------------
		// enable/disable sky and sea deband
		ImGui::SliderFloat("Enable sky and sea deband", &shared_data.cb_inject_values.debandFlag, 0.0f, 1.0f, "Active: %1.0f");
		ImGui::Separator();
		if (!shared_data.cb_inject_values.debandFlag)
		{
			ImGui::BeginDisabled();
		}
		// set deband options
		ImGui::SliderFloat("NoiseStrength", &shared_data.cb_inject_values.Threshold, 0.0f, 4096.0f, "Noise: %1.0f");
		ImGui::SliderFloat("DitherStrength", &shared_data.cb_inject_values.Range, 1.0f, 64.0f, "Range: %1.0f");
		ImGui::SliderFloat("Grain", &shared_data.cb_inject_values.Grain, 0.0f, 4096.0f, "Grain: %1.0f");
		if (!shared_data.cb_inject_values.debandFlag)
		{
			ImGui::EndDisabled();
		}

		if (!shared_data.sharpenDeband_feature)
		{
			ImGui::EndDisabled();
		}
		
	}
	ImGui::Separator();

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("Mics: Haze control, A10C instrument reflection, NVG size, label masking"))
	{
		ImGui::Checkbox("Activate Haze, instrument reflection, NVG features", &shared_data.misc_feature);
		if (shared_data.misc_feature != shared_data.init_misc_feature)
		{
			ImGui::SameLine();
			ImGui::Text("Game restart needed (option saved)");
			saveShaderTogglerIniFile();

		}
		if (!shared_data.misc_feature)
		{
			ImGui::BeginDisabled();
		}

		// set Haze factor
		ImGui::SliderFloat("Haze strenght", &shared_data.cb_inject_values.hazeReduction, 0.0f, 1.0f, "Strenght: %.2f");

		// enable/disable reflection removal for A10C
		ImGui::SliderFloat("Remove A10C instr. reflect", &shared_data.cb_inject_values.noReflect, 0.0f, 1.0f, "Active: %1.0f");

		// set NVG size
		ImGui::SliderFloat("NVG size", &shared_data.cb_inject_values.NVGSize, 0.9f, 3.0f, "Scale: %.2f");

		// set NVG YPOS
		ImGui::SliderFloat("NVG vert. Pos", &shared_data.cb_inject_values.NVGYPos, -0.5f, 1.0f, "YPos: %.2f");

		// enable/disable label fix
		ImGui::SliderFloat("Labels hidden by cockpit frame", &shared_data.cb_inject_values.maskLabels, 0.0f, 1.0f, "Active: %1.0f");

		if (!shared_data.misc_feature)
		{
			ImGui::EndDisabled();
		}

	}


	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("Helicopter"))
	{
		
		ImGui::Checkbox("Activate Helicopter features", &shared_data.helo_feature);
		if (shared_data.helo_feature != shared_data.init_helo_feature)
		{
			ImGui::SameLine();
			ImGui::Text("Game restart needed (option saved)");
			saveShaderTogglerIniFile();

		}
		if (!shared_data.helo_feature)
		{
			ImGui::BeginDisabled();
		}
		// enable/disable rotor fix
		ImGui::SliderFloat("Disable Epileptic flashing rotor", &shared_data.cb_inject_values.rotorFlag, 0.0f, 1.0f, "active: %1.0f");

		// enable/disable vido in TADS (just to show the status)
		ImGui::SliderFloat("Disable TADS or PNVS in IHADSS", &shared_data.cb_inject_values.disable_video_IHADSS, 0.0f, 1.0f, "Active: %1.0f");
		if (!shared_data.cb_inject_values.disable_video_IHADSS)
		{
			ImGui::BeginDisabled();
		}
		ImGui::Text("Use %s+%s in game to change", shared_data.key_TADS_video_mod, shared_data.key_TADS_video);
		ImGui::SliderFloat("Trigger value for Day", &shared_data.cb_inject_values.TADSDay, 0.0f, 1.0f, "Active: %0.2f");
		ImGui::SliderFloat("Trigger Value for Night", &shared_data.cb_inject_values.TADSNight, 0.0f, 1.0f, "Active: %0.2f");
		if (!shared_data.cb_inject_values.disable_video_IHADSS)
		{
			ImGui::EndDisabled();
		}
		// set convergence for boresight
		ImGui::SliderFloat("Boresight convergence for IHASSS", &shared_data.cb_inject_values.IHADSSBoresight, 0.0f, 1.0f, "Enabled: %1.0f");
		if (!shared_data.cb_inject_values.IHADSSBoresight)
		{
			ImGui::BeginDisabled();
		}
		ImGui::SliderFloat("Boresight convergence offset", &shared_data.cb_inject_values.IHADSSxOffset, -0.25f, 0.25f, "Offset: %.3f");
		ImGui::Text("Use %s+%s in game to enable/disable boresight convergence for IHADSS : ", shared_data.key_IHADSS_boresight_mod, shared_data.key_IHADSS_boresight);
		if (!shared_data.cb_inject_values.IHADSSBoresight)
		{
			ImGui::EndDisabled();
		}
		// enable/disable left eye in IHADSS (just to show the status)
		ImGui::SliderFloat("Disable left eye in IHADSS", &shared_data.cb_inject_values.IHADSSNoLeft, 0.0f, 1.0f, "Active: %1.0f");
		if (!shared_data.cb_inject_values.IHADSSNoLeft)
		{
			ImGui::BeginDisabled();
		}
		ImGui::Text("Use %s+%s in game to change", shared_data.key_IHADSSNoLeft_mod, shared_data.key_IHADSSNoLeft);
		if (!shared_data.cb_inject_values.IHADSSNoLeft)
		{
			ImGui::EndDisabled();
		}

		if (!shared_data.helo_feature)
		{
			ImGui::EndDisabled();
		}
	}
	ImGui::Separator();

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("NS430"))
	{
		
		ImGui::Checkbox("Activate NS430 features", &shared_data.NS430_feature);
		if (shared_data.NS430_feature != shared_data.init_NS430_feature)
		{
			ImGui::SameLine();
			ImGui::Text("Game restart needed (option saved)");
			saveShaderTogglerIniFile();

		}
		if (!shared_data.NS430_feature)
		{
			ImGui::BeginDisabled();
		}
		
		// enable/disable label fix
		ImGui::SliderFloat("Hide NS430", &shared_data.cb_inject_values.NS430Flag, 0.0f, 1.0f, "Active: %1.0f");
		if (!shared_data.cb_inject_values.NS430Flag)
		{
			ImGui::BeginDisabled();
		}
		ImGui::Text("Use %s+%s in game to change", shared_data.key_NS430_mod, shared_data.key_NS430);
		ImGui::SliderFloat("NS430 Scale", &shared_data.cb_inject_values.NS430Scale, 3.0f, 10.0f, "Scale: %.2f");
		ImGui::SliderFloat("NS430 Xposition ", &shared_data.cb_inject_values.NS430Xpos, 0.0f, 1.0f, "Xpos: %.2f");
		ImGui::SliderFloat("NS430 Yposition ", &shared_data.cb_inject_values.NS430Ypos, 0.0f, 1.0f, "YPos: %.2f");
		ImGui::SliderFloat("NS430 Offset", &shared_data.cb_inject_values.NS430Convergence, 0.5f, 1.5f, "Offset: %.3f");
		ImGui::SliderFloat("NS430 Area YScale ", &shared_data.cb_inject_values.GUIYScale, 1.0f, 2.0f, "YScale: %.2f");
		if (!shared_data.cb_inject_values.NS430Flag)
		{
			ImGui::EndDisabled();
		}
		if (!shared_data.NS430_feature)
		{
			ImGui::EndDisabled();
		}
	}
	ImGui::Separator();

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("Fps limiter"))
	{

		ImGui::Checkbox("Activate fps limiter features", &shared_data.fps_feature);
		if (!shared_data.fps_feature)
		{
			ImGui::BeginDisabled();
		}

			if (ImGui::Checkbox("Enabled", &shared_data.fps_enabled))
				shared_data.fps_started = false;

			ImGui::Text("Use %s+%s in game to enable, %S+%s to disable", shared_data.key_fps_on_mod, shared_data.key_fps, shared_data.key_fps_off_mod, shared_data.key_fps);

			if (ImGui::SliderInt("Target FPS", reinterpret_cast<int*>(&shared_data.fps_limit), 30, 120))
			{
				shared_data.fps_started = false;
				shared_data.time_per_frame = std::chrono::high_resolution_clock::duration(std::chrono::seconds(1)) / shared_data.fps_limit;
			}
		if (!shared_data.fps_feature)
		{
			ImGui::EndDisabled();
		}

	}
	ImGui::Separator();

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("Save and Load"))
	{
		// static int clicked = 0;
		if (ImGui::Button("Save mod settings"))
		{
			saveShaderTogglerIniFile();
			ImGui::SameLine();
			ImGui::Text("Settings saved");
		}

		// static int clicked2 = 0;
		if (ImGui::Button("Reload mod settings"))
		{
			load_setting_IniFile();
			ImGui::SameLine();
			ImGui::Text("Settings reloaded");
		}

	}
	ImGui::Separator();

	// *******************************************************************************************************
	if (ImGui::CollapsingHeader("Debug options"))
	{
		
		// delay for compilation
		ImGui::SliderInt("Delay for compilation", &shared_data.compil_delay, 0, 500, "%d ms");
		
		// Disable optimization
		ImGui::Checkbox("Disable optimization", &shared_data.disable_optimisation);

		// verbose logs in reshade.log
		ImGui::Checkbox("Debug messages in log / enable display mask", &debug_flag);
		if (debug_flag != shared_data.init_debug_feature)
		{
			ImGui::SameLine();
			ImGui::Text("Game restart needed (option saved)");
			saveShaderTogglerIniFile();

		}

		// define the number of draw to differentiate
		// ImGui::SliderFloat("# of draws to differentiate", &shared_data.cb_inject_values.testFlag, 0.0f, 5.0f, "ratio = %.0f");
		// to test global shader
		ImGui::SliderFloat("Display stencil mask", &shared_data.cb_inject_values.testGlobal, 0.0f, 1.0f, "active: %1.0f");
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
}