#define EMISSIVE
#define BLENDING On

#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

varying float v_FresnelPerc;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec3 a_Normal;

uniform mat4 u_World;
uniform mat4 u_WorldViewProj;
uniform vec3 u_WSCameraPos;

#define NO_NORMALS 1
#include "Include/Bone_AttribsAndUniforms.glsl"
#include "Include/Bone_Functions.glsl"

void main()
{
    vec4 pos;
    ApplyBoneInfluencesToPositionAttribute( a_Position, pos );

    gl_Position = u_WorldViewProj * pos;

    vec4 WSPos = u_World * pos;
    vec3 WSNormal = a_Normal; // This shader is used by the light component, the sphere won't be rotated.

    vec3 dirToSurface = normalize( WSPos.xyz - u_WSCameraPos );
    v_FresnelPerc = clamp( 0.1 + pow( 1.0 + dot(WSNormal, dirToSurface), 3.0 ), 0.0, 1.0 );
}

#endif

#ifdef FragmentShader

uniform vec4 u_TextureTintColor;

void main()
{
    // Blending is on, alpha will work.
    gl_FragColor = u_TextureTintColor * v_FresnelPerc;
}

#endif
