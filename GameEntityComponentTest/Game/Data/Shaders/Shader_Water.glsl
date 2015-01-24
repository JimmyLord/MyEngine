#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

uniform float u_Time;

varying vec2 v_UVCoord;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec2 a_UVCoord;

uniform mat4 u_WorldViewProj;

void main()
{
    gl_Position = u_WorldViewProj * a_Position;

	gl_Position.y += sin( a_Position.x / 5.0 + u_Time ) * 1.0;
	gl_Position.y -= sin( a_Position.z / 40.0 + u_Time ) * 2.0;

	v_UVCoord = a_UVCoord;
}

#endif

#ifdef FragmentShader

uniform sampler2D u_TextureColor;

void main()
{
	vec2 newUV = v_UVCoord;
	newUV.x += sin( u_Time + v_UVCoord.y * 10 ) * 0.04;
	newUV.y -= sin( u_Time + v_UVCoord.x * 15 ) * 0.06;

	vec4 color1 = texture2D( u_TextureColor, newUV + u_Time / 20.0 );
    vec4 color2 = texture2D( u_TextureColor, newUV + vec2( u_Time / 21.0, 0.0 ) );

	color1 += vec4( 15/255.0, 103/255.0, 227/255.0, 0.0 );

	vec4 color = color1 + color2 * -0.13;

	gl_FragColor = color;
}

#endif
