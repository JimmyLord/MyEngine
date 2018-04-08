#define BLENDING Off

#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

varying vec2 v_UVCoord;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec2 a_UVCoord;

void main()
{
    gl_Position = a_Position;

	v_UVCoord = a_UVCoord;
}

#endif

#ifdef FragmentShader

uniform sampler2D u_TextureAlbedoShine;
uniform sampler2D u_TexturePosition;
uniform sampler2D u_TextureNormal;

uniform vec3 u_WSCameraPos;

#define NUM_DIR_LIGHTS 0
#include <Include/Light_Uniforms.glsl>
#include <Include/Light_Functions.glsl>

void main()
{
    vec4 albedoShine = texture2D( u_TextureAlbedoShine, v_UVCoord );
	vec3 WSPosition = texture2D( u_TexturePosition, v_UVCoord ).xyz; // * 100.0 - vec3(15,0,0);
	vec3 WSNormal = texture2D( u_TextureNormal, v_UVCoord ).xyz; //(texture2D( u_TextureNormal, v_UVCoord ).xyz - 0.5) * 2;

    // Accumulate ambient, diffuse and specular color for all lights.
    vec3 finalAmbient = vec3(0,0,0);
    vec3 finalDiffuse = vec3(0,0,0);
    vec3 finalSpecular = vec3(0,0,0);

	//// Add in directional light.
    //DirLightContribution( WSPosition.xyz, u_WSCameraPos, WSNormal, albedoShine.w, finalAmbient, finalDiffuse, finalSpecular );
    //finalDiffuse *= shadowperc;

    // Add in each light, one by one. // finalDiffuse, finalSpecular are inout.
#if NUM_LIGHTS > 0
    for( int i=0; i<NUM_LIGHTS; i++ )
        PointLightContribution( u_LightPos[i], u_LightColor[i], u_LightAttenuation[i], WSPosition.xyz, u_WSCameraPos, WSNormal, albedoShine.w, finalAmbient, finalDiffuse, finalSpecular );
#endif

    // Mix the texture color with the light color.
    vec3 ambDiff = albedoShine.rgb * ( finalAmbient + finalDiffuse );
    vec3 spec = finalSpecular;

    // Calculate final color.
    gl_FragColor.rgb = ambDiff + spec;
    gl_FragColor.a = 1;

    //gl_FragColor = vec4( albedoShine.rgb + position + normal, 1 );
	//gl_FragColor.xyz = WSNormal;

	//gl_FragColor.xyz = WSPosition;
}

#endif
