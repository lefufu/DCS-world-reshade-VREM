/**
 * HDR
 * by Christian Cann Schuldt Jensen ~ CeeJay.dk
 *
 * Not actual HDR - It just tries to mimic an HDR look (relatively high performance cost)
 *
 * modified by lefuneste to add VREM support
 */

#include "ReShadeUI.fxh"
//change for VREM here 
#include "VREM.fxh"


uniform int VREMtarget_area <
    ui_category = "VREM Settings";
	ui_items = "All\0cockpit only\0External only\0";
	// end of change
    ui_label = "Target region";
    ui_tooltip = "Choose the area on whihc technique will change output";
    ui_type = "combo";
> = 0;
// end of change

uniform float HDRPower < __UNIFORM_SLIDER_FLOAT1
	//change for VREM:  add UI category
	ui_category = "HDR";
	ui_min = 0.0; ui_max = 8.0;
	ui_label = "Power";
> = 1.30;
uniform float radius1 < __UNIFORM_SLIDER_FLOAT1
	//change for VREM:  add UI category
	ui_category = "HDR";
	ui_min = 0.0; ui_max = 8.0;
	ui_label = "Radius 1";
> = 0.793;
uniform float radius2 < __UNIFORM_SLIDER_FLOAT1
	//change for VREM:  add UI category
	ui_category = "HDR";
	ui_min = 0.0; ui_max = 8.0;
	ui_label = "Radius 2";
	ui_tooltip = "Raising this seems to make the effect stronger and also brighter.";
> = 0.87;

#include "ReShade.fxh"

float3 HDRPass(float4 vpos : SV_Position, float2 texcoord : TexCoord) : SV_Target
{
	float3 color = tex2D(ReShade::BackBuffer, texcoord).rgb;
	
		// change for VREM 
	if ( (is_cockpit(texcoord) && VREMtarget_area == 1) ||  (!is_cockpit(texcoord) && VREMtarget_area == 2)  || VREMtarget_area ==0)
	{
		// end of change

		float3 bloom_sum1 = tex2D(ReShade::BackBuffer, texcoord + float2(1.5, -1.5) * radius1 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum1 += tex2D(ReShade::BackBuffer, texcoord + float2(-1.5, -1.5) * radius1 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum1 += tex2D(ReShade::BackBuffer, texcoord + float2( 1.5,  1.5) * radius1 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum1 += tex2D(ReShade::BackBuffer, texcoord + float2(-1.5,  1.5) * radius1 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum1 += tex2D(ReShade::BackBuffer, texcoord + float2( 0.0, -2.5) * radius1 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum1 += tex2D(ReShade::BackBuffer, texcoord + float2( 0.0,  2.5) * radius1 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum1 += tex2D(ReShade::BackBuffer, texcoord + float2(-2.5,  0.0) * radius1 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum1 += tex2D(ReShade::BackBuffer, texcoord + float2( 2.5,  0.0) * radius1 * BUFFER_PIXEL_SIZE).rgb;

		bloom_sum1 *= 0.005;

		float3 bloom_sum2 = tex2D(ReShade::BackBuffer, texcoord + float2(1.5, -1.5) * radius2 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum2 += tex2D(ReShade::BackBuffer, texcoord + float2(-1.5, -1.5) * radius2 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum2 += tex2D(ReShade::BackBuffer, texcoord + float2( 1.5,  1.5) * radius2 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum2 += tex2D(ReShade::BackBuffer, texcoord + float2(-1.5,  1.5) * radius2 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum2 += tex2D(ReShade::BackBuffer, texcoord + float2( 0.0, -2.5) * radius2 * BUFFER_PIXEL_SIZE).rgb;	
		bloom_sum2 += tex2D(ReShade::BackBuffer, texcoord + float2( 0.0,  2.5) * radius2 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum2 += tex2D(ReShade::BackBuffer, texcoord + float2(-2.5,  0.0) * radius2 * BUFFER_PIXEL_SIZE).rgb;
		bloom_sum2 += tex2D(ReShade::BackBuffer, texcoord + float2( 2.5,  0.0) * radius2 * BUFFER_PIXEL_SIZE).rgb;

		bloom_sum2 *= 0.010;

		float dist = radius2 - radius1;
		float3 HDR = (color + (bloom_sum2 - bloom_sum1)) * dist;
		float3 blend = HDR + color;
		color = pow(abs(blend), abs(HDRPower)) + HDR; // pow - don't use fractions for HDRpower
		
		return saturate(color);
	//change for VREM
	} else
		return color;
	//end of change
}

technique HDR_for_VREM
{
	pass
	{
		VertexShader = PostProcessVS;
		PixelShader = HDRPass;
	}
}
