#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

#ifdef VertexShader

uniform mat4 u_WorldViewProj;
attribute vec4 a_Position;

void main()
{
    gl_Position = u_WorldViewProj * a_Position;
}

#endif

#ifdef FragmentShader

uniform vec4 u_TextureTintColor;

void main()
{
    gl_FragColor = u_TextureTintColor;
}

#endif
