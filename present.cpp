///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// present function
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


// *******************************************************************************************************
// Initialize counters
//
void intialize_counters()
{
	// to know the current display : 2D, VR standard: left/right, VR quad view : left eye outer + inner + right eye outer+inner
	shared_data.count_display = 0;

	shared_data.cb_inject_values.mapMode = 1.0;
	shared_data.depthStencil_copy_started = false;
	shared_data.counter_testing = 0;
	do_not_draw = false;
	shared_data.cb_inject_values.GUItodraw = 0.0;

	shared_data.render_effect = false;
	shared_data.track_for_render_target = false;

	// shared_data.render_target_res->copied = false;
	// shared_data.CPerFrame_copied = false;
	shared_data.CB_copied[CPERFRAME_CB_NB] = false;
	// shared_data.track_for_CB[DEF_UNIFORMS_CB_NB] = false;

	shared_data.last_feature = Feature::Null;

	// initialize flags for copy
	for (int i = 0; i < MAXVIEWSPERDRAW; i++)
	{
		shared_data.depthStencil_res[i].copied = false;
		shared_data.NS430_res[i].copied = false;
		shared_data.render_target_view[i].created = false;
	}


	// part to take into account changes during game (!!! will not reload shader list !!!)
	// set if needed to copy rendertarget, needed to compute super/undersampling beside MSAA (usage for technique handled by technique parsing later)
	if (shared_data.cb_inject_values.debandFlag || shared_data.cb_inject_values.sharpenFlag || shared_data.color_feature || shared_data.cb_inject_values.maskLabels || shared_data.cb_inject_values.testGlobal)
		shared_data.texture_needed = true;

	//handle MSAA for resolution
	// no MSAA : possible to have donwsmapling
	//for usage in shaders
	// if (shared_data.render_target_view[0].width > 0) 
	// 	shared_data.SSfactor = shared_data.renderTargetX / shared_data.render_target_view[0].width;

	shared_data.cb_inject_values.AAxFactor = shared_data.SSfactor;
	shared_data.cb_inject_values.AAyFactor = shared_data.SSfactor;
	//for usage in techniques
	shared_data.MSAAxfactor = 1.0;
	shared_data.MSAAyfactor = 1.0;

	if (shared_data.MSAA_factor == 1)
	{
		//for usage in shaders
		shared_data.cb_inject_values.AAxFactor = 2.0;
		shared_data.cb_inject_values.AAyFactor = 1.0;
		//for usage in techniques
		shared_data.MSAAxfactor = shared_data.cb_inject_values.AAxFactor;
		shared_data.MSAAyfactor = shared_data.cb_inject_values.AAyFactor;
	}
	else
		if (shared_data.MSAA_factor == 2)
		{
			//for usage in shaders
			shared_data.cb_inject_values.AAxFactor = 2.0;
			shared_data.cb_inject_values.AAyFactor = 2.0;
			//for usage in techniques
			shared_data.MSAAxfactor = shared_data.cb_inject_values.AAxFactor;
			shared_data.MSAAyfactor = shared_data.cb_inject_values.AAyFactor;
		}
	log_susperSamping();
}

// *******************************************************************************************************
// handle all key press outside imgui shortcut
//
void handle_keypress(effect_runtime* runtime)
{

	/*
	//example on handling "hold" feature => effect is triggered only when key is pressed
	if (runtime->is_key_down(VK_F1) && runtime->is_key_down(VK_SHIFT))
	{
		shared_data.cb_inject_values.testGlobal = 1.0;
	}
	else
	{
		shared_data.cb_inject_values.testGlobal = 0.0;
	}
	*/

	// obsolete : default CTRL+I toggle on/off video in IHADSS
	// default ALT+F6 toggle on/off video in IHADSS
	if (runtime->is_key_pressed(shared_data.vk_TADS_video) && runtime->is_key_down(shared_data.vk_TADS_video_mod))
	{
		// Toggle the value of disable_video_IHADSS between 0.0 and 1.0
		if (shared_data.cb_inject_values.disable_video_IHADSS == 1.0)
		{
			shared_data.cb_inject_values.disable_video_IHADSS = 0.0;
		}
		else if (shared_data.cb_inject_values.disable_video_IHADSS == 0.0)
		{
			shared_data.cb_inject_values.disable_video_IHADSS = 1.0;
		}
	}

	// obsolete :  default SHIFT+I toggle on/off boresight convergence of IHADSS
	// default  ALT+F8 toggle on/off boresight convergence of IHADSS
	if (runtime->is_key_pressed(shared_data.vk_IHADSS_boresight) && runtime->is_key_down(shared_data.vk_IHADSS_boresight_mod))
	{
		// Toggle the value of disable_video_IHADSS between 0.0 and 1.0
		if (shared_data.cb_inject_values.IHADSSBoresight == 1.0)
		{
			shared_data.cb_inject_values.IHADSSBoresight = 0.0;
		}
		else if (shared_data.cb_inject_values.IHADSSBoresight == 0.0)
		{
			shared_data.cb_inject_values.IHADSSBoresight = 1.0;
		}
	}

	// obsolete :  default ALT+I toggle on/off lest eye of IHADSS
	// default ALT+F10 toggle on/off lest eye of IHADSS
	if (runtime->is_key_pressed(shared_data.vk_IHADSSNoLeft) && runtime->is_key_down(shared_data.vk_IHADSSNoLeft_mod))
	{
		// Toggle the value of disable_video_IHADSS between 0.0 and 1.0
		if (shared_data.cb_inject_values.IHADSSNoLeft == 1.0)
		{
			shared_data.cb_inject_values.IHADSSNoLeft = 0.0;
		}
		else if (shared_data.cb_inject_values.IHADSSNoLeft == 0.0)
		{
			shared_data.cb_inject_values.IHADSSNoLeft = 1.0;
		}
	}

	// obsolete :  default ALT+V toggle NS430 hiding and screen texture display in GUI
	// default ALT+F7 toggle NS430 hiding and screen texture display in GUI
	if (runtime->is_key_pressed(shared_data.vk_NS430) && runtime->is_key_down(shared_data.vk_NS430_mod))
	{
		// Toggle the value of disable_video_IHADSS between 0.0 and 1.0
		if (shared_data.cb_inject_values.NS430Flag == 1.0)
		{
			shared_data.cb_inject_values.NS430Flag = 0.0;
		}
		else if (shared_data.cb_inject_values.NS430Flag == 0.0)
		{
			shared_data.cb_inject_values.NS430Flag = 1.0;
		}
	}

	// obsolete : default SHIFT+1 toggle ON limiter
	// default SHIFT+F5 toggle ON limiter
	if (runtime->is_key_pressed(shared_data.vk_fps) && runtime->is_key_down(shared_data.vk_fps_on_mod) )
	{
		shared_data.fps_enabled = true;
		shared_data.fps_started = false;

	}

	// obsolete : default CTRL+1 toggle OFF limiter
	// default CTRL+F5 toggle OFF limiter
	if (runtime->is_key_pressed(shared_data.vk_fps) && runtime->is_key_down(shared_data.vk_fps_off_mod))
	{
		// Toggle the value of disable_video_IHADSS between 0.0 and 1.0
		shared_data.fps_enabled = false;

	}

	// default ALT+F5 toggle ON/OFF technique
	if (runtime->is_key_pressed((shared_data.vk_technique)) && runtime->is_key_down(shared_data.vk_technique_mod))
	{
		if (shared_data.effects_feature)
		{
			shared_data.effects_feature = false;
		}
		else if (!shared_data.effects_feature)
		{
			shared_data.effects_feature = true;
		}

	}

}

// *******************************************************************************************************
// This registers a callback for the 'present' event, which occurs every time a new frame is presented to the screen.
// used to 
//		Monitor keypress and set variable to inject
//      Initialize counter for each frame
//		Launch debugging of a frame
//
void on_present(effect_runtime* runtime)
{

	// initialize counter to identfiy what to do when in the next frame
	intialize_counters();

	// store runtime for effect in on_draw()
	shared_data.runtime = runtime;

	// frame counter 
	if (shared_data.frame_counter != 0x7FFFFFFF) shared_data.frame_counter += 1;
	else shared_data.frame_counter = 0;
	//need to improve
	shared_data.cb_inject_values.frame_counter = shared_data.frame_counter;

	// handle key press to toggle features not managed by imgui (eg TADS picture removed)
	handle_keypress(runtime);

	// frame capture by button on GUI
	if (flag_capture)
	{
		if (debug_flag)
		{
			reshade::log::message(reshade::log::level::info, "present()");
			reshade::log::message(reshade::log::level::info, "--- End Frame ---");
		}
		flag_capture = false;
	}
	else
	{
		// The keyboard shortcut to trigger logging
		// if (runtime->is_key_pressed(VK_F10))
		if (shared_data.button_capture)
		{
			flag_capture = true;
			if (debug_flag)
			{
				reshade::log::message(reshade::log::level::info, "--- Frame ---");

				//display pipeline_by_hash
				print_pipeline_by_hash(pipeline_by_hash);

				//display shader by hash
				print_pipeline_by_handle(pipeline_by_handle);
			}

		}
	}


	//load activated techniques list
	if (shared_data.button_technique  || shared_data.technique_init == 1)
	{
		shared_data.button_technique = false;
		shared_data.technique_init = 0;
		enumerateTechniques(runtime);
		// flag_capture = true;
	}
	
	//fps limiter
	if (shared_data.fps_enabled && shared_data.fps_feature)
	{

		if (!shared_data.fps_started)
		{
			shared_data.fps_started = true;
			shared_data.fps_last_point = std::chrono::high_resolution_clock::now();
		}

		// const auto time_per_frame = std::chrono::high_resolution_clock::duration(std::chrono::seconds(1)) / shared_data.fps_limit;
		const auto next_point = shared_data.fps_last_point + shared_data.time_per_frame;

		while (next_point > std::chrono::high_resolution_clock::now())
			std::this_thread::sleep_for(std::chrono::high_resolution_clock::duration(std::chrono::milliseconds(1)));

		shared_data.fps_last_point = next_point;
	}

	
	// disable technique if VR only
	// shared_data.count_draw set to number of viewports only after 1st rendering of 3D 
	if (shared_data.count_draw > 0)
	{
		if (!shared_data.flag_re_enabled && shared_data.technique_vector.size() > 0 && shared_data.VRonly_technique && shared_data.render_target_view[shared_data.count_draw - 1].compiled == true)
		{
			shared_data.flag_re_enabled = true;
			//std::this_thread::sleep_for(std::chrono::high_resolution_clock::duration(std::chrono::milliseconds(1000)));

			disableAllTechnique(false);
		}
	}
	


	// to debug crash at launch
	// flag_capture = true;

}
