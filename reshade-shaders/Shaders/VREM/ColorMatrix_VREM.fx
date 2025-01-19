/**
 * Color Matrix version 1.0
 * by Christian Cann Schuldt Jensen ~ CeeJay.dk
 *
 * ColorMatrix allow the user to transform the colors using a color matrix
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


uniform float3 ColorMatrix_Red < __UNIFORM_SLIDER_FLOAT3
	//change for VREM:  add UI category
	ui_category = "ColorMatrix";
	ui_min = 0.0; ui_max = 1.0;
	ui_label = "Matrix Red";
	ui_tooltip = "How much of a red, green and blue tint the new red value should contain. Should sum to 1.0 if you don't wish to change the brightness.";
> = float3(0.817, 0.183, 0.000);
uniform float3 ColorMatrix_Green < __UNIFORM_SLIDER_FLOAT3
	//change for VREM:  add UI category
	ui_category = "ColorMatrix";
	ui_min = 0.0; ui_max = 1.0;
	ui_label = "Matrix Green";
	ui_tooltip = "How much of a red, green and blue tint the new green value should contain. Should sum to 1.0 if you don't wish to change the brightness.";
> = float3(0.333, 0.667, 0.000);
uniform float3 ColorMatrix_Blue < __UNIFORM_SLIDER_FLOAT3
	//change for VREM:  add UI category
	ui_category = "ColorMatrix";
	ui_min = 0.0; ui_max = 1.0;
	ui_label = "Matrix Blue";
	ui_tooltip = "How much of a red, green and blue tint the new blue value should contain. Should sum to 1.0 if you don't wish to change the brightness.";
> = float3(0.000, 0.125, 0.875);

uniform float Strength < __UNIFORM_SLIDER_FLOAT1
	//change for VREM:  add UI category
	ui_category = "ColorMatrix";
	ui_min = 0.0; ui_max = 1.0;
	ui_tooltip = "Adjust the strength of the effect.";
> = 1.0;

#include "ReShade.fxh"

float3 ColorMatrixPass(float4 position : SV_Position, float2 texcoord : TexCoord) : SV_Target
{
	
	
	float3 color = tex2D(ReShade::BackBuffer, texcoord).rgb;
	
	// change for VREM 
	if ( (is_cockpit(texcoord) && VREMtarget_area == 1) ||  (!is_cockpit(texcoord) && VREMtarget_area == 2)  || VREMtarget_area ==0)
	{
		// end of change
		const float3x3 ColorMatrix = float3x3(ColorMatrix_Red, ColorMatrix_Green, ColorMatrix_Blue);
		color = lerp(color, mul(ColorMatrix, color), Strength);
		return saturate(color);
	//change for VREM
	} else
		return color;
	//end of change
}

technique ColorMatrix_for_VREM
{
	pass
	{
		VertexShader = PostProcessVS;
		PixelShader = ColorMatrixPass;
	}
}
