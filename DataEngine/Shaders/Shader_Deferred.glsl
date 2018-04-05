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

void main()
{
    vec4 albedoShine = texture2D( u_TextureAlbedoShine, v_UVCoord );
	vec3 position = texture2D( u_TexturePosition, v_UVCoord ).xyz;
	vec3 normal = texture2D( u_TextureNormal, v_UVCoord ).xyz;

    gl_FragColor = vec4( albedoShine.rgb + position + normal, 1 );
}

#endif
