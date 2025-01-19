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