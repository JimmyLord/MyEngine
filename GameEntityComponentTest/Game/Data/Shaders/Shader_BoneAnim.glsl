#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

#undef ReceiveShadows
#define ReceiveShadows 0

#include "Include/WSVaryings.glsl"

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec3 a_Normal;

uniform mat4 u_World;
uniform mat4 u_WorldViewProj;

#include "Include/Bone_AttribsAndUniforms.glsl"
#include "Include/Bone_Functions.glsl"
#include "Include/WSVaryings_Functions.glsl"

void main()
{
    vec4 pos;
    vec3 normal;
    
    ApplyBoneInfluencesToPositionAndNormalAttributes( a_Position, a_Normal, pos, normal );
    SetWSPositionAndNormalVaryings( u_World, pos, normal );

    gl_Position = u_WorldViewProj * pos;
}

#endif

#ifdef FragmentShader

uniform vec3 u_WSCameraPos;
uniform float u_Shininess;

#include "Include/Light_Uniforms.glsl"
#include "Include/Light_Functions.glsl"

void main()
{
    // Calculate the normal vector in local space. normalized again since interpolation can/will distort it.
    //   TODO: handle normal maps.
    vec3 WSnormal = normalize( v_WSNormal );

    // Hardcoded ambient
    vec4 finalambient = vec4(0.2, 0.2, 0.2, 1.0);

    // Accumulate diffuse and specular color for all lights.
    vec4 finaldiffuse = vec4(0,0,0,0);
    vec4 finalspecular = vec4(0,0,0,0);

    // Add in each light, one by one. // finaldiffuse, finalspecular are inout.
#if NUM_LIGHTS > 0
    for( int i=0; i<NUM_LIGHTS; i++ )
        PointLightContribution( i, v_WSPosition.xyz, u_WSCameraPos, WSnormal, u_Shininess, finaldiffuse, finalspecular );
#endif

    // Mix the texture color with the light color.
    vec4 ambdiff = /*texcolor * v_Color * */( finalambient + finaldiffuse );
    vec4 spec = /*u_TextureSpecColor **/ finalspecular;

    // Calculate final color including whether it's in shadow or not.
    gl_FragColor = ( ambdiff + spec );// * shadowperc;
    gl_FragColor.a = 1.0;
}

#endif
