/**
 *                  _______  ___  ___       ___           ___
 *                 /       ||   \/   |     /   \         /   \
 *                |   (---- |  \  /  |    /  ^  \       /  ^  \
 *                 \   \    |  |\/|  |   /  /_\  \     /  /_\  \
 *              ----)   |   |  |  |  |  /  _____  \   /  _____  \
 *             |_______/    |__|  |__| /__/     \__\ /__/     \__\
 *
 *                               E N H A N C E D
 *       S U B P I X E L   M O R P H O L O G I C A L   A N T I A L I A S I N G
 *
 *                               for ReShade 3.0+
 *
 *  modified by lefuneste to add VREM support
 */

//------------------- Preprocessor Settings -------------------

#if !defined(SMAA_PRESET_LOW) && !defined(SMAA_PRESET_MEDIUM) && !defined(SMAA_PRESET_HIGH) && !defined(SMAA_PRESET_ULTRA)
#define SMAA_PRESET_CUSTOM // Do not use a quality preset by default
#endif

// VREM modification here
#include "SMAA_VREM.fxh"

// target for quad view => 0 : all, 1 Outer, 2 Innner
uniform int VREMQuadViewTarget < hidden = true; > = 1;

uniform int VREMtarget_area <
    ui_category = "VREM Settings";
	ui_items = "All\0cockpit only\0External only\0VREM_hybrid\0";
	// end of change
    ui_label = "Target region";
    ui_tooltip = "Choose the area on which technique will change output";
    ui_type = "combo";
> = 0;


// end of change
//----------------------- UI Variables ------------------------

#include "ReShadeUI.fxh"

uniform int EdgeDetectionType < __UNIFORM_COMBO_INT1
	ui_items = "Luminance edge detection\0Color edge detection\0Depth edge detection\0";
	ui_label = "Edge Detection Type";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = 1;

#ifdef SMAA_PRESET_CUSTOM
uniform float EdgeDetectionThreshold < __UNIFORM_DRAG_FLOAT1
	ui_min = 0.05; ui_max = 0.20; ui_step = 0.001;
	ui_tooltip = "Edge detection threshold. If SMAA misses some edges try lowering this slightly.";
	ui_label = "Edge Detection Threshold";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = 0.10;

uniform float DepthEdgeDetectionThreshold < __UNIFORM_DRAG_FLOAT1
	ui_min = 0.001; ui_max = 0.10; ui_step = 0.001;
	ui_tooltip = "Depth Edge detection threshold. If SMAA misses some edges try lowering this slightly.";
	ui_label = "Depth Edge Detection Threshold";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = 0.01;

uniform int MaxSearchSteps < __UNIFORM_SLIDER_INT1
	ui_min = 0; ui_max = 112;
	ui_label = "Max Search Steps";
	ui_tooltip = "Determines the radius SMAA will search for aliased edges.";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = 32;

uniform int MaxSearchStepsDiagonal < __UNIFORM_SLIDER_INT1
	ui_min = 0; ui_max = 20;
	ui_label = "Max Search Steps Diagonal";
	ui_tooltip = "Determines the radius SMAA will search for diagonal aliased edges";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = 16;

uniform int CornerRounding < __UNIFORM_SLIDER_INT1
	ui_min = 0; ui_max = 100;
	ui_label = "Corner Rounding";
	ui_tooltip = "Determines the percent of anti-aliasing to apply to corners.";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = 25;

uniform bool PredicationEnabled < __UNIFORM_INPUT_BOOL1
	ui_label = "Enable Predicated Thresholding";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = false;

uniform float PredicationThreshold < __UNIFORM_DRAG_FLOAT1
	ui_min = 0.005; ui_max = 1.00; ui_step = 0.01;
	ui_tooltip = "Threshold to be used in the additional predication buffer.";
	ui_label = "Predication Threshold";
		//change for VREM:  add UI category
	ui_category = "SMAA";
> = 0.01;

uniform float PredicationScale < __UNIFORM_SLIDER_FLOAT1
	ui_min = 1; ui_max = 8;
	ui_tooltip = "How much to scale the global threshold used for luma or color edge.";
	ui_label = "Predication Scale";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = 2.0;

uniform float PredicationStrength < __UNIFORM_SLIDER_FLOAT1
	ui_min = 0; ui_max = 4;
	ui_tooltip = "How much to locally decrease the threshold.";
	ui_label = "Predication Strength";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = 0.4;
#endif

uniform int DebugOutput < __UNIFORM_COMBO_INT1
	ui_items = "None\0View edges\0View weights\0";
	ui_label = "Debug Output";
	//change for VREM:  add UI category
	ui_category = "SMAA";
> = false;

#ifdef SMAA_PRESET_CUSTOM
	#define SMAA_PREDICATION PredicationEnabled
	#define SMAA_THRESHOLD EdgeDetectionThreshold
	#define SMAA_DEPTH_THRESHOLD DepthEdgeDetectionThreshold
	#define SMAA_MAX_SEARCH_STEPS MaxSearchSteps
	#define SMAA_MAX_SEARCH_STEPS_DIAG MaxSearchStepsDiagonal
	#define SMAA_CORNER_ROUNDING CornerRounding
	#define SMAA_PREDICATION_THRESHOLD PredicationThreshold
	#define SMAA_PREDICATION_SCALE PredicationScale
	#define SMAA_PREDICATION_STRENGTH PredicationStrength
#endif

#define SMAA_RT_METRICS float4(BUFFER_RCP_WIDTH, BUFFER_RCP_HEIGHT, BUFFER_WIDTH, BUFFER_HEIGHT)
#define SMAA_CUSTOM_SL 1

#define SMAATexture2D(tex) sampler tex
#define SMAATexturePass2D(tex) tex
#define SMAASampleLevelZero(tex, coord) tex2Dlod(tex, float4(coord, coord))
#define SMAASampleLevelZeroPoint(tex, coord) SMAASampleLevelZero(tex, coord)
#define SMAASampleLevelZeroOffset(tex, coord, offset) tex2Dlodoffset(tex, float4(coord, coord), offset)
#define SMAASample(tex, coord) tex2D(tex, coord)
#define SMAASamplePoint(tex, coord) SMAASample(tex, coord)
#define SMAASampleOffset(tex, coord, offset) tex2Doffset(tex, coord, offset)
#define SMAA_BRANCH [branch]
#define SMAA_FLATTEN [flatten]

#if (__RENDERER__ == 0xb000 || __RENDERER__ == 0xb100)
	#define SMAAGather(tex, coord) tex2Dgather(tex, coord, 0)
#endif

#include "SMAA.fxh"
#include "ReShade.fxh"

// Textures
// change for VREM => replace texture name

texture depthTex_QVO < pooled = true; >
{ 
	Width = BUFFER_WIDTH;   
	Height = BUFFER_HEIGHT;   
	Format = R16F;  
};


texture edgesTex_QVO < pooled = true; >
{
	Width = BUFFER_WIDTH;   
	Height = BUFFER_HEIGHT; 
	Format = RG8;
};
texture blendTex_QVO < pooled = true; >
{
	Width = BUFFER_WIDTH;   
	Height = BUFFER_HEIGHT; 
	Format = RGBA8;
};

texture areaTex < source = "AreaTex.png"; >
{
	Width = 160;
	Height = 560;
	Format = RG8;
};
texture searchTex < source = "SearchTex.png"; >
{
	Width = 64;
	Height = 16;
	Format = R8;
};

// Samplers

sampler depthLinearSampler
{
	Texture = depthTex_QVO;
};


sampler colorGammaSampler
{
	Texture = ReShade::BackBufferTex;
	AddressU = Clamp; AddressV = Clamp;
	MipFilter = Point; MinFilter = Linear; MagFilter = Linear;
	SRGBTexture = false;
};
sampler colorLinearSampler
{
	Texture = ReShade::BackBufferTex;
	AddressU = Clamp; AddressV = Clamp;
	MipFilter = Point; MinFilter = Linear; MagFilter = Linear;
	SRGBTexture = false;
};
sampler edgesSampler
{
	Texture = edgesTex_QVO;
	AddressU = Clamp; AddressV = Clamp;
	MipFilter = Linear; MinFilter = Linear; MagFilter = Linear;
	SRGBTexture = false;
};
sampler blendSampler
{
	Texture = blendTex_QVO;
	AddressU = Clamp; AddressV = Clamp;
	MipFilter = Linear; MinFilter = Linear; MagFilter = Linear;
	SRGBTexture = false;
};
sampler areaSampler
{
	Texture = areaTex;
	AddressU = Clamp; AddressV = Clamp; AddressW = Clamp;
	MipFilter = Linear; MinFilter = Linear; MagFilter = Linear;
	SRGBTexture = false;
};
sampler searchSampler
{
	Texture = searchTex;
	AddressU = Clamp; AddressV = Clamp; AddressW = Clamp;
	MipFilter = Point; MinFilter = Point; MagFilter = Point;
	SRGBTexture = false;
};

// Vertex shaders

void SMAAEdgeDetectionWrapVS(
	in uint id : SV_VertexID,
	out float4 position : SV_Position,
	out float2 texcoord : TEXCOORD0,
	out float4 offset[3] : TEXCOORD1)
{
	PostProcessVS(id, position, texcoord);
	SMAAEdgeDetectionVS(texcoord, offset);
}
void SMAABlendingWeightCalculationWrapVS(
	in uint id : SV_VertexID,
	out float4 position : SV_Position,
	out float2 texcoord : TEXCOORD0,
	out float2 pixcoord : TEXCOORD1,
	out float4 offset[3] : TEXCOORD2)
{
	PostProcessVS(id, position, texcoord);
	SMAABlendingWeightCalculationVS(texcoord, pixcoord, offset);
}
void SMAANeighborhoodBlendingWrapVS(
	in uint id : SV_VertexID,
	out float4 position : SV_Position,
	out float2 texcoord : TEXCOORD0,
	out float4 offset : TEXCOORD1)
{
	PostProcessVS(id, position, texcoord);
	SMAANeighborhoodBlendingVS(texcoord, offset);
}

// Pixel shaders

float SMAADepthLinearizationPS(
	float4 position : SV_Position,
	float2 texcoord : TEXCOORD) : SV_Target
{
	float3 color = tex2D(ReShade::BackBuffer, texcoord).rgb;
	bool iscockpit = is_cockpit(texcoord);
	if ( (iscockpit && (VREMtarget_area == 1 || VREMtarget_area == 3)) ||  (!iscockpit && VREMtarget_area == 2)  || VREMtarget_area == 0)
		return ReShade::GetLinearizedDepth(texcoord);
	else
		return 0;
}

float2 SMAAEdgeDetectionWrapPS(
	float4 position : SV_Position,
	float2 texcoord : TEXCOORD0,
	float4 offset[3] : TEXCOORD1) : SV_Target
{
	float3 color = tex2D(ReShade::BackBuffer, texcoord).rgb;
	bool iscockpit = is_cockpit(texcoord);
	
	if ( (iscockpit && VREMtarget_area == 1) ||  (!iscockpit && VREMtarget_area == 2)  || VREMtarget_area == 0 || VREMtarget_area == 3)
	{
		
		// handle hybrid : external = luma, cockpit = edge
		if (VREMtarget_area == 3)
		{
			if (! iscockpit)
			{
				if (SMAA_PREDICATION == true) 
					return SMAALumaEdgePredicationDetectionPS(texcoord, offset, colorGammaSampler, depthLinearSampler);
				else 
					return SMAALumaEdgeDetectionPS(texcoord, offset, colorGammaSampler);
			} else
			{
				return SMAADepthEdgeDetectionPS(texcoord, offset, depthLinearSampler);
			}				
		} else
		{
		
			if (EdgeDetectionType == 0 && SMAA_PREDICATION == true)
				return SMAALumaEdgePredicationDetectionPS(texcoord, offset, colorGammaSampler, depthLinearSampler);
			else if (EdgeDetectionType == 0)
				return SMAALumaEdgeDetectionPS(texcoord, offset, colorGammaSampler);

			if (EdgeDetectionType == 2)
				return SMAADepthEdgeDetectionPS(texcoord, offset, depthLinearSampler);

			if (SMAA_PREDICATION)
				return SMAAColorEdgePredicationDetectionPS(texcoord, offset, colorGammaSampler, depthLinearSampler);
			else
				return SMAAColorEdgeDetectionPS(texcoord, offset, colorGammaSampler);
		}
	}
	else
		return 0;
}

float4 SMAABlendingWeightCalculationWrapPS(
	float4 position : SV_Position,
	float2 texcoord : TEXCOORD0,
	float2 pixcoord : TEXCOORD1,
	float4 offset[3] : TEXCOORD2) : SV_Target
{
	return SMAABlendingWeightCalculationPS(texcoord, pixcoord, offset, edgesSampler, areaSampler, searchSampler, 0.0);
}

float3 SMAANeighborhoodBlendingWrapPS(
	float4 position : SV_Position,
	float2 texcoord : TEXCOORD0,
	float4 offset : TEXCOORD1) : SV_Target
{
	if (DebugOutput == 1)
		return tex2D(edgesSampler, texcoord).rgb;
	if (DebugOutput == 2)
		return tex2D(blendSampler, texcoord).rgb;

	return SMAANeighborhoodBlendingPS(texcoord, offset, colorLinearSampler, blendSampler).rgb;
}

// Rendering passes

technique SMAA_no_quad_view_or_QVOuter
{
	pass LinearizeDepthPass
	{
		VertexShader = PostProcessVS;
		PixelShader = SMAADepthLinearizationPS;
		RenderTarget = depthTex_QVO;
	}

	pass EdgeDetectionPass
	{
		VertexShader = SMAAEdgeDetectionWrapVS;
		PixelShader = SMAAEdgeDetectionWrapPS;
		RenderTarget = edgesTex_QVO;
		ClearRenderTargets = true;
		StencilEnable = true;
		StencilPass = REPLACE;
		StencilRef = 1;
	}
	pass BlendWeightCalculationPass
	{
		VertexShader = SMAABlendingWeightCalculationWrapVS;
		PixelShader = SMAABlendingWeightCalculationWrapPS;
		RenderTarget = blendTex_QVO;
		ClearRenderTargets = true;
		StencilEnable = true;
		StencilPass = KEEP;
		StencilFunc = EQUAL;
		StencilRef = 1;
	}
	pass NeighborhoodBlendingPass
	{
		VertexShader = SMAANeighborhoodBlendingWrapVS;
		PixelShader = SMAANeighborhoodBlendingWrapPS;
		StencilEnable = false;
		SRGBWriteEnable = true;
	}
}
