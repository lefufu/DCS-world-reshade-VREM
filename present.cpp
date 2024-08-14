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
	// to know the current display : 2D, VR standard: left/right, VR quad view : left/right + eye left/right

	shared_data.count_display = 0;
	shared_data.cb_inject_values.mapMode = 1.0;
	shared_data.texture_copy_started = false;

	// initialize flags for copy
	for (int i = 0; i < MAXVIEWSPERDRAW; i++)
	{
		shared_data.depthStencil_res[i].copied = false;
	}

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

	// CTRL+I toggle on/off video in IHADSS
	if (runtime->is_key_pressed('I') && runtime->is_key_down(VK_CONTROL))
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

	// initialize counter to identfiy what to do when in the frame
	intialize_counters();

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
			reshade::log_message(reshade::log_level::info, "present()");
			reshade::log_message(reshade::log_level::info, "--- End Frame ---");
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
				reshade::log_message(reshade::log_level::info, "--- Frame ---");
			}

		}
	}
}
