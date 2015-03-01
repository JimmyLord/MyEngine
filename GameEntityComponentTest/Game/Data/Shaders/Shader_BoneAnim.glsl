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

uniform mat4 u_WorldViewProj;

#include "Include/Bone_AttribsAndUniforms.glsl"
#include "Include/Bone_Functions.glsl"

void main()
{
    vec4 pos;
    ApplyBoneInfluencesToPositionAndNormalAttributes( pos, v_Normal );

    gl_Position = u_WorldViewProj * pos;
}

#endif

#ifdef FragmentShader

void main()
{
    gl_FragColor = v_Normal;// * 0.5 + 0.5;
}

#endif
