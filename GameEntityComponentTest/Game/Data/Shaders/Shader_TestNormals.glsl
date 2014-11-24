#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

varying vec4 v_Normal;

#ifdef VertexShader

uniform mat4 u_WorldViewProj;
attribute vec4 a_Position;
attribute vec4 a_Normal;

void main()
{
    gl_Position = u_WorldViewProj * a_Position;

	v_Normal = a_Normal;
}

#endif

#ifdef FragmentShader

void main()
{
    gl_FragColor = v_Normal;// * 0.5 + 0.5;
}

#endif
