const char* Lightness_frag = R"(

#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;

uniform float u_lightness;

vec3 hsvtorgb(float h, float s, float v)
{
    float C = v*s;
    float hh = h * 6.0;
    float X = C*(1.0-abs(mod(hh,2.0)-1.0));
    float r,g,b;
    r = g = b = 0.0;
    if( hh>=0.0 && hh<1.0 )
    {
        r = C;
        g = X;
    }
    else if( hh>=1.0 && hh<2.0 )
    {
        r = X;
        g = C;
    }
    else if( hh>=2.0 && hh<3.0 )
    {
        g = C;
        b = X;
    }
    else if( hh>=3.0 && hh<4.0 )
    {
        g = X;
        b = C;
    }
    else if( hh>=4.0 && hh<5.0 )
    {
        r = X;
        b = C;
    }
    else
    {
        r = C;
        b = X;
    }
    float m = v-C;
    r += m;
    g += m;
    b += m;
    return vec3(r,g,b);
}

vec3 rgbtohsv(float r, float g, float b)
{
    float M = max(r,max(g,b));
    float m = min(r,min(g,b));
    float C = M-m;
    float h,s,v;
    if( C==0.0 ) h=0.0;
    else if( M==r ) h=mod((g-b)/C, 6.0);
    else if( M==g ) h=(b-r)/C+2.0;
    else h=(r-g)/C+4.0;
    h*=60.0;
    if( h<0.0 ) h+=360.0;
    v = M;
    if( v==0.0 )
        s = 0.0;
    else
        s = C/v;
    h /= 360.0;
    return vec3(h,s,v);
}

void main(void)
{
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);
    if(texColor.a == 0.0)
    {
        gl_FragColor = texColor;
        return;
    }

    if(texColor.a < 1.0)
        texColor.rgb = texColor.rgb / texColor.a;
    vec3 hsv = rgbtohsv(texColor.r, texColor.g, texColor.b);

    float lightness = u_lightness;
    if(lightness > 0.0)
    {
        hsv.y = clamp(hsv.y - hsv.y * lightness, 0.0, 1.0);
        hsv.z = clamp(hsv.z + (1.0 - hsv.z) * lightness, 0.0, 1.0);
    }
    else
    {
        lightness = abs(lightness);
        hsv.z = clamp(hsv.z - hsv.z * lightness, 0.0, 1.0);
    }

    vec3 rgb =  hsvtorgb(hsv.x, hsv.y, hsv.z);
    if(texColor.a < 1.0)
        gl_FragColor.rgb = rgb * texColor.a;
    else
        gl_FragColor.rgb = rgb;
    gl_FragColor.a = texColor.a;

}

)";