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

const int MAX_BONES = 100;
uniform mat4 u_BoneTransforms[MAX_BONES];

void main()
{
    mat4 bonetransform;
    bonetransform  = u_BoneTransforms[a_BoneIndex[0]] * a_BoneWeight[0];
    bonetransform += u_BoneTransforms[a_BoneIndex[1]] * a_BoneWeight[1];
    bonetransform += u_BoneTransforms[a_BoneIndex[2]] * a_BoneWeight[2];
    bonetransform += u_BoneTransforms[a_BoneIndex[3]] * a_BoneWeight[3];

    vec4 pos = bonetransform * a_Position;

    gl_Position = u_WorldViewProj * pos;

	v_Normal = bonetransform * vec4( a_Normal.xyz, 0 );
}

#endif

#ifdef FragmentShader

void main()
{
    gl_FragColor = v_Normal;// * 0.5 + 0.5;
}

#endif
