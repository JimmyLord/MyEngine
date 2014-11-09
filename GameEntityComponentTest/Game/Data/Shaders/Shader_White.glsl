// Shader_VertexColor.glsl

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

void main()
{
    gl_FragColor = vec4(1,1,1,1);
}

#endif
