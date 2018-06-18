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

varying vec3 v_WSDirToFragment;

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
    v_WSDirToFragment = WSPosition.xyz - u_WSCameraPos;
}

#endif

#ifdef FragmentShader

uniform vec2 u_TextureSize;
uniform vec3 u_CamAt;
uniform sampler2D u_TextureAlbedo;
uniform sampler2D u_TexturePositionShine;
uniform sampler2D u_TextureNormal;
uniform sampler2D u_TextureDepth;
uniform vec3 u_WSCameraPos;

#define NUM_DIR_LIGHTS 0
#include <Include/Light_Uniforms.glsl>
#include <Include/Light_Functions.glsl>
#include <Include/DepthBuffer_Functions.glsl>

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
    {
        //float depth = texture2D( u_TextureDepth, UVCoord ).x;
        //WSPosition = WorldPositionFromDepth( depth );
    }

    // Another alternate method to compute world position using distance from camera from gbuffer.
    {
        //float distanceFromCamera = specularShine; // Other objects need to store distance in gbuffer.
        //vec3 normalizedDirToFragment = normalize( v_WSDirToFragment );
        //WSPosition = u_WSCameraPos + normalizedDirToFragment * distanceFromCamera;
    }

    // Yet another alternative: Convert depth from depth buffer to world position.
    {
        //float depth = texture2D( u_TextureDepth, UVCoord ).x;
        //float viewSpaceZ = ViewSpaceZFromDepth( depth );

        //vec3 normalizedDirToFragment = normalize( v_WSDirToFragment );

        //float distanceFromCamera = viewSpaceZ / dot( normalizedDirToFragment, u_CamAt );
        //WSPosition = u_WSCameraPos + normalizedDirToFragment * distanceFromCamera;
    }

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
