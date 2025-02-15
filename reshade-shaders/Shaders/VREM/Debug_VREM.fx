//
// debug shader for VREM : display depth or stencil buffer
//
// 2025/01 							Lefuneste.
//
// https://forum.dcs.world/topic/356128-reshade-vr-enhancer-mod-vrem/#comment-5503216

#include "ReShade.fxh"
#include "ReShadeUI.fxh"

#include "VREM.fxh"

//****************************************
// GUI

uniform int display_mode <
    ui_type = "combo";
    ui_label = "Mode";
    ui_tooltip = "Choose a texture to display";
    ui_category = "Debug Options";
    ui_items = 
    "Nothing\0"
	"Depth\0"
    "Stencil\0"
	"QV Resolution\0"
    ;
> = 0;

uniform int bufferWidth < ui_type = "drag"; ui_label = "BUFFER Width"; ui_min = 0; ui_max = 4096; > = BUFFER_WIDTH;
uniform int bufferHeight < ui_type = "drag"; ui_label = "BUFFER Height"; ui_min = 0; ui_max = 4096; > = BUFFER_HEIGHT;

uniform int QVWidth < ui_type = "drag"; ui_label = "QV Width"; ui_min = 1076; ui_max = 1660; > = 1076;
uniform int QVHeight < ui_type = "drag"; ui_label = "QV Height"; ui_min = 1076; ui_max = 1420; > = 1076;

//****************************************
// code

// pixel shader
float3 Ps_VREM_Test(float4 position : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
	float3 ori = tex2Dlod(ReShade::BackBuffer, float4(texcoord, 0.0, 0.0)).rgb;
	
	float3 output ;
	
	if (!display_mode)
	{
		return ori;
	}
	
	float depth = tex2Dlod(DepthBuffer, float4(texcoord, 0, 0)).x;

	
	#if RESHADE_DEPTH_INPUT_IS_LOGARITHMIC
		const float C = 0.01;
		depth = (exp(depth * log(C + 1.0)) - 1.0) / C;
	#endif
	
	#if RESHADE_DEPTH_INPUT_IS_REVERSED
		depth = 1.0 - depth;
	#endif
	
	const float N = 1.0;
	depth /= RESHADE_DEPTH_LINEARIZATION_FAR_PLANE - depth * (RESHADE_DEPTH_LINEARIZATION_FAR_PLANE - N);
	
	if (display_mode == 1)
		output = float3(depth, 0, 0);
	else 
		if (display_mode == 2)
		{
		
			uint sampledData = tex2Dlod(StencilBuffer, float4(texcoord, 0, 0)).g;
			
			output = float3(0, 1, 0);
			
			if (sampledData == 40)  
				output = float3(1, 0, 0);
			
			if (sampledData == 0)  
				output = float3(0, 0, 0.5);
			
			if (sampledData >=16 &&  sampledData  < 29)
				output = float3(0, 0, 1.0);
				
		}
		else 
		{
			float2 size = tex2Dsize(StencilBuffer);
			if (size.x == QVWidth) 
			ori.x += 1;
			output = ori;
		}
	
	
	return output;
	
}


technique VREM_Test_masks
{
    pass
    {
		VertexShader = PostProcessVS;
        PixelShader = Ps_VREM_Test; 
    }
}
