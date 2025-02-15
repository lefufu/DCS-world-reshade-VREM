//****************************************
// VREM Uniform variable handled by VREM

//stencil texture 
// 40 = cockpit
// 0 = sky
// 16 <= water < 29
texture StencilBufferTex : STENCIL;
sampler<uint4> StencilBuffer { Texture = StencilBufferTex; };

bool is_cockpit(float2 coord)
{
    bool check = false;
	
	uint sampledData = tex2Dlod(StencilBuffer, float4(coord, 0, 0)).g;
	
	if (sampledData == 40)
        check = true;
	
	return check;
}


// depth texture
texture DepthBufferTex : DEPTH;
sampler DepthBuffer { Texture = DepthBufferTex; };

/* no choice for SMAA
// target for quad view => 0 : all, 1 Outer, 2 Innner
uniform int VREMQuadViewTarget <
    ui_category = "VREM Settings";
	ui_items = 
    "All views\0"
	"QV Outer\0"
    "QV Inner\0"
	;
    ui_label = "Quad view targets";
    ui_tooltip = "Define if technique will be rendered on all views, or only on outer views or only in Inner views. Works only if HMD is quad view";
    ui_type = "combo";
> = 0;
*/

float get_depth(float2 texcoord )
{
	float depth = tex2Dlod(DepthBuffer, float4(texcoord.x, texcoord.y, 0.0, 0.0) ).x;

	
	#if RESHADE_DEPTH_INPUT_IS_LOGARITHMIC
		const float C = 0.01;
		depth = (exp(depth * log(C + 1.0)) - 1.0) / C;
	#endif
	
	#if RESHADE_DEPTH_INPUT_IS_REVERSED
		depth = 1.0 - depth;
	#endif
	
	const float N = 1.0;
	depth /= RESHADE_DEPTH_LINEARIZATION_FAR_PLANE - depth * (RESHADE_DEPTH_LINEARIZATION_FAR_PLANE - N);
	
	return depth;
}

