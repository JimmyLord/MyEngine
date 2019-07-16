#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

varying vec2 v_UVCoord;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec2 a_UVCoord;

uniform mat4 u_WorldViewProj;
uniform vec2 u_UVScale;
uniform vec2 u_UVOffset;

void main()
{
    gl_Position = u_WorldViewProj * a_Position;

    v_UVCoord = (a_UVCoord + u_UVOffset) * u_UVScale;
}

#endif

#ifdef FragmentShader

uniform sampler2D u_TextureColor;

void main()
{
    gl_FragColor = texture2D( u_TextureColor, v_UVCoord );
}

#endif
