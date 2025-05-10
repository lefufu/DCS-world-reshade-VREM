//****************************************
// VREM Uniform variable handled by VREM

//stencil texture 
// 40 = cockpit
// 0 = sky
// 16 <= water < 29
texture StencilBufferTex : STENCIL;
sampler<uint4> StencilBuffer { Texture = StencilBufferTex; };

// uniform int VREM_MSAAx < hidden = true; > = MSAAX;
// uniform int VREM_MSAAy < hidden = true; > = MSAAY;
// uniform int MSAAX < ui_type = "drag"; ui_label = "BUFFER Width"; ui_min = 0; ui_max = 2; > ;
// uniform int MSAAY < ui_type = "drag"; ui_label = "BUFFER Width"; ui_min = 0; ui_max = 2; > ;

uniform int superSX < ui_type = "drag"; ui_label = "totox"; ui_min = 0; ui_max = 2; > = MSAAX;
uniform int superSY < ui_type = "drag"; ui_label = "totoy"; ui_min = 0; ui_max = 2; > = MSAAY;

bool is_cockpit(float2 coord)
{
    bool check = false;
	
	float2 coord2;
	coord2.x = MSAAX * coord.x;
	coord2.y = MSAAY * coord.y;
	
	uint sampledData = tex2Dlod(StencilBuffer, float4(coord2, 0, 0)).g;
	
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

