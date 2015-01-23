#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

#ifdef VertexShader

attribute vec4 a_Position;

uniform mat4 u_WorldViewProj;
uniform float u_Time;

void main()
{
    gl_Position = u_WorldViewProj * a_Position;

	gl_Position.y += sin( u_Time + a_Position.x ) * 5;
}

#endif

#ifdef FragmentShader

uniform vec4 u_TextureTintColor;

void main()
{
    gl_FragColor = u_TextureTintColor;
}

#endif
