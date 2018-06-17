#define BLENDING On
#define BLENDFUNC One One

#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

#undef ReceiveShadows
#define ReceiveShadows 0

varying vec3 v_ViewRay;

#ifdef VertexShader

attribute vec4 a_Position;

uniform mat4 u_WorldViewProj;
uniform mat4 u_WorldView;
uniform mat4 u_World;
uniform vec3 u_WSCameraPos;

void main()
{
    gl_Position = u_WorldViewProj * a_Position;

    vec4 WSPosition = u_World * a_Position;
    v_ViewRay = WSPosition.xyz - u_WSCameraPos;
}

#endif

#ifdef FragmentShader

uniform vec2 u_TextureSize;
uniform vec2 u_ViewportSize;
uniform float u_ZNear;
uniform float u_ZFar;
uniform sampler2D u_TextureAlbedo;
uniform sampler2D u_TexturePositionShine;
uniform sampler2D u_TextureNormal;
uniform sampler2D u_TextureDepth;

uniform mat4 u_InverseView;
uniform mat4 u_InverseProj;

uniform vec3 u_WSCameraPos;

#define NUM_DIR_LIGHTS 0
#include <Include/Light_Uniforms.glsl>
#include <Include/Light_Functions.glsl>

vec3 WorldPositionFromDepth(float depth)
{
    vec2 xy = (gl_FragCoord.xy / u_ViewportSize) * 2.0 - 1.0;
    float z = depth * 2.0 - 1.0;

    vec4 CSPosition = vec4( xy, z, 1.0 );
    vec4 VSPosition = u_InverseProj * CSPosition;
    VSPosition /= VSPosition.w;
    vec4 WSPosition = u_InverseView * VSPosition;

    return WSPosition.xyz;
}

float ViewSpaceZFromDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    float zVS = 2.0 * u_ZNear * u_ZFar / ( u_ZFar + u_ZNear - z * (u_ZFar - u_ZNear) );

    return zVS;
}

void main()
{
    vec2 UVCoord = gl_FragCoord.xy / u_TextureSize;

    vec4 albedoColor = texture2D( u_TextureAlbedo, UVCoord );
	vec4 WSPositionShine = texture2D( u_TexturePositionShine, UVCoord );
    vec3 WSPosition = WSPositionShine.xyz;
    float specularShine = WSPositionShine.w;
	vec3 WSNormal = texture2D( u_TextureNormal, UVCoord ).xyz;

    // World position is currently rendered into "u_TexturePositionShine".
    // Alternatively, it can be computed from depth.
    //float depth = texture2D( u_TextureDepth, UVCoord ).x;
    //WSPosition = WorldPositionFromDepth( depth );

    // Another alternate method to compute world position using distance from camera from gbuffer.
    //float distanceFromCamera = specularShine; // Other objects need to store distance in gbuffer.
    //vec3 normalizedViewRay = normalize( v_ViewRay );
    //WSPosition = u_WSCameraPos + normalizedViewRay * distanceFromCamera;

    // Accumulate ambient, diffuse and specular color for all lights.
    vec3 finalAmbient = vec3( 0.0, 0.0, 0.0 ); // not used.
    vec3 finalDiffuse = vec3( 0.0, 0.0, 0.0 );
    vec3 finalSpecular = vec3( 0.0, 0.0, 0.0 );

    // Add in each light, one by one. // finalDiffuse, finalSpecular are inout.
#if NUM_LIGHTS > 0
    for( int i=0; i<NUM_LIGHTS; i++ )
        PointLightContribution( u_LightPos[i], u_LightColor[i], u_LightAttenuation[i], WSPosition, u_WSCameraPos, WSNormal, specularShine, finalAmbient, finalDiffuse, finalSpecular );
#endif

    // Mix the texture color with the light color.
    vec3 ambDiff = albedoColor.rgb * ( finalDiffuse );
    vec3 spec = finalSpecular;

    // Calculate final color.
    gl_FragColor.rgb = ambDiff + spec;
    gl_FragColor.a = 1;

    //gl_FragColor.rgb = clamp( gl_FragColor.rgb, 0.0, 1.0 );
    //gl_FragColor.xyz = WSNormal;

    //gl_FragColor.xyz = WSPosition;
    //gl_FragColor.xyz = vec3(1);
    //gl_FragColor.xyz = vec3(1-depth);
}

#endif
