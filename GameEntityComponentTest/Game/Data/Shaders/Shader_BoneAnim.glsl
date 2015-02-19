#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

varying vec4 v_Normal;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec4 a_Normal;
attribute ivec4 a_BoneIndex;
attribute vec4 a_BoneWeight;

uniform mat4 u_WorldViewProj;

void main()
{
    vec4 pos = a_Position;

    gl_Position = u_WorldViewProj * pos;

	v_Normal = a_Normal;
}

#endif

#ifdef FragmentShader

void main()
{
    gl_FragColor = v_Normal;// * 0.5 + 0.5;
}

#endif
