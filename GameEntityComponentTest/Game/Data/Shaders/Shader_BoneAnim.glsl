#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

#undef ReceiveShadows
#define ReceiveShadows 0

varying lowp vec3 v_WSPosition;
varying lowp vec4 v_WSNormal;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec4 a_Normal;

uniform mat4 u_World;
uniform mat4 u_WorldViewProj;

#include "Include/Bone_AttribsAndUniforms.glsl"
#include "Include/Bone_Functions.glsl"

void main()
{
    vec4 pos;
    
    ApplyBoneInfluencesToPositionAndNormalAttributes( pos, v_WSNormal );

    v_WSNormal = u_World * vec4( v_WSNormal.xyz, 0 );
    v_WSPosition = (u_World * pos).xyz;

    gl_Position = u_WorldViewProj * pos;
}

#endif

#ifdef FragmentShader

uniform vec3 u_CameraPos;
uniform float u_Shininess;

#include "Include/Light_Uniforms.glsl"
#include "Include/Light_Functions.glsl"

void main()
{
    //gl_FragColor = v_WSNormal;// * 0.5 + 0.5;

    // Calculate the normal vector in world space. normalized in vertex shader
    //   left this here for future where normal might come from a normal map.
    vec3 normalworld = normalize( v_WSNormal.xyz );

    // Accumulate diffuse and specular color for all lights.
    vec4 finaldiffuse = vec4(0,0,0,0);
    vec4 finalspecular = vec4(0,0,0,0);

    // Add in each light, one by one. // finaldiffuse, finalspecular are inout.
#if NUM_LIGHTS > 0
    for( int i=0; i<NUM_LIGHTS; i++ )
        PointLightContribution( i, normalworld, finaldiffuse, finalspecular );
#endif
    //finaldiffuse = vec4( 1,1,1,1 );

    // Mix the texture color with the light color.
    vec4 ambdiff = /*texcolor * v_Color * */( /*finalambient +*/ finaldiffuse );
    vec4 spec = /*u_TextureSpecColor **/ finalspecular;

    // Calculate final color including whether it's in shadow or not.
    gl_FragColor = ( ambdiff );// + spec );// * shadowperc;
    gl_FragColor.a = 1.0;
}

#endif
