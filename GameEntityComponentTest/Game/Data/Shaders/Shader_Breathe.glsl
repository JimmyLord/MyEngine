#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

varying vec4 v_Normal;

uniform float u_Time;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec4 a_Normal;

uniform mat4 u_WorldViewProj;

void main()
{
    gl_Position = u_WorldViewProj * a_Position;

	gl_Position.y *= 1 + sin( u_Time ) * 0.05;

	v_Normal = a_Normal;
}

#endif

#ifdef FragmentShader

uniform sampler2D u_TextureColor;

void main()
{
	gl_FragColor = v_Normal;
}

#endif
