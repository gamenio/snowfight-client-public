const char* SilhouettableNodeEffect_frag = R"(

#ifdef GL_ES
precision mediump float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform vec2 u_screenPixelSize;
uniform sampler2D u_screenTexture;

uniform int u_premultipliedAlpha;
uniform float u_alphaValue;
uniform vec3 u_sourceMaskColor;

void main(void)
{
	 vec4 texColor = texture2D(CC_Texture0, v_texCoord);
	 vec2 screenTexCoord = gl_FragCoord.xy * u_screenPixelSize;
	 vec4 screenColor = texture2D(u_screenTexture, screenTexCoord);

	 if (screenColor.rgb == u_sourceMaskColor)
		discard;
	 else
	 {
		if(u_premultipliedAlpha != 0)
			gl_FragColor = texColor * v_fragmentColor * u_alphaValue;
		else
		{
			gl_FragColor.rgb = texColor.rgb * v_fragmentColor.rgb;
			gl_FragColor.a = texColor.a * v_fragmentColor.a * u_alphaValue;
		}
	 }
}
)";
