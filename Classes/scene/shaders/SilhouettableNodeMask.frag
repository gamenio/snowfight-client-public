const char* SilhouettableNodeMask_frag = R"(

#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;

uniform vec3 u_maskColor;

void main(void)
{
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);
	if(texColor.a > 0.0)
	{
		gl_FragColor.rgb = u_maskColor;
		gl_FragColor.a = 1.0;
	}
	else
		discard;
}

)";