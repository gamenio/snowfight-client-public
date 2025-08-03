const char* DiscardAlpha_frag = R"(

#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;

void main(void)
{
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);

    if ( texColor.a < 1.0 )
        discard;

    gl_FragColor = texColor;
}

)";