///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM addon. VR Enhancer Mod for DCS using reshade
// 
// mod paramters to share with shaders
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

// CB number to be injected in the shaders
static const int CBINDEX = 13;
static const int CPERFRAME_INDEX = 6;
static const int DEF_UNIFORMS_INDEX = 0; 

// to use for all tables related to CB (pipeline_layout,...)
static const int MOD_CB_NB = 0;
static const int CPERFRAME_CB_NB = 1;
// too many calls for VS of instrument=> cancelled 
// static const int DEF_UNIFORMS_CB_NB = 2;
static const int NUMBER_OF_MODIFIED_CB = 2;
static const int MAX_OF_MODIFIED_VALUES = 3;

// index of values to change in CB
// index of gAtmIntensity in the float array mapped for cPerFrame (cb6) 
#define FOG_INDEX 11 // c2.w => 2*4 + 3 = 11
// #define OPACITY_INDEX 55 // 13.z => 13*4+3 = 55
#define GCOCKPITIBL_INDEX_X 30*4
#define GCOCKPITIBL_INDEX_Y 30*4+1

// size of CB
#define CPERFRAME_SIZE 152 //in float
// #define DEF_UNIFORM_SIZE 68 //in float

// only one value to save for CPerFrame
static const int  GATMINTENSITY_SAVE = 0;
static const int  GCOCKPITIBL_X_SAVE = 1;
static const int  GCOCKPITIBL_Y_SAVE = 1;

// only one value to save for def_uniform
// static const int  OPACITY_SAVE = 0;

// RV number to be injected in the shaders
static const int RVINDEX = 3;

// size of the constant buffer containing all mod parameters, to be injected in shaders
static const int CBSIZE = 44;

// maximum size of all CB
static const int MAX_CBSIZE = 152;

// Must be 32bit aligned
struct ShaderInjectData {
	float testFlag; //0.x
	float rotorFlag; //0.y
	float testGlobal; //0.z
	float disable_video_IHADSS; //0.w
	float count_display; //1.x
	float mapMode; //1.y
	float VRMode; //1.z
	float maskLabels; //1.w
	float hazeReduction; //2.x => used in asm !
	float noReflect; //2.y
	float cockpitSat; //2.z
	float cockpitMul; //2.w
	float cockpitAdd; //3.x
	float extSat; //3.y
	float extMul; //3.z
	float extAdd; //3.w
	float colorFlag; //4.x
	float fSharpenIntensity; //4.y
	float lumaFactor; //4.z
	float sharpenFlag; //4.w
	float debandFlag; //5.x
	float Threshold; //5.y
	float Range; //5.z
	float Iterations; //5.w
	float Grain; //6.x
	float frame_counter; //6.y
	float AAxFactor; //6.z => to be injected in shaders, contain all super and under sampling values, at the opposite of MSAAxfactor only for technique and only for MSAA
	float AAyFactor; //6.w => to be injected in shaders, at the opposite of MSAAyfactor only for technique, at the opposite of MSAAxfactor only for technique and only for MSAA
	float IHADSSxOffset; //7.x
	float IHADSSBoresight; //7.y
	float IHADSSNoLeft; //7.z
	float NS430Flag; //7.w
	float NS430Xpos; //8.x
	float NS430Ypos; //8.y
	float NS430Scale; //8.z
	float NS430Convergence; //8.w
	float NVGSize; //9.x
	float GUIYScale; //9.y
	float GUItodraw; //9.z
	float NVGYPos; //9.w
	float TADSNight; //10.x
	float TADSDay; //10.y
	float gCockpitIBL; //10.z
	float dunmmy2; //10.w
};


// NS430 screen size
static const int NS430_textSizeX = 240;
static const int NS430_textSizeY = 128;

static const int NS430_renderSizeX = 720;
static const int NS430_renderSizeY = 384;

// VR menu windows size 
static const int VRGUI_sizeX = 1680;
static const int VRGUI_sizeY = 1050;

/*
#ifndef __cplusplus
cbuffer cb13 : register(b13) {
	ShaderInjectData injectedData : packoffset(c0);
}
#endif */
