#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

uniform float u_Time;

varying vec2 v_UVCoord;
varying vec4 v_Worldpos;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec2 a_UVCoord;

uniform mat4 u_World;
uniform mat4 u_ViewProj;
uniform float u_PointSize;

void main()
{
	gl_PointSize = u_PointSize;

	v_Worldpos = u_World * a_Position;

    //v_Worldpos.y += sin( u_Time + v_Worldpos.x * 12 );

	vec4 wrappedpos = v_Worldpos;
	wrappedpos.y = sin( v_Worldpos.x ) * (1.0 - v_Worldpos.y * 0.2);
	wrappedpos.x = cos( v_Worldpos.x ) * (1.0 - v_Worldpos.y * 0.2);

    gl_Position = u_ViewProj * wrappedpos;

	v_UVCoord = a_UVCoord;
}

#endif

#ifdef FragmentShader

uniform sampler2D u_TextureColor;

void main()
{
	vec2 newUV = v_UVCoord;
	newUV.x += sin( u_Time + gl_FragCoord.y / 40.0 ) * 0.04;
	newUV.y -= sin( u_Time + gl_FragCoord.x / 44.0 ) * 0.06;

	vec4 color1 = texture2D( u_TextureColor, newUV + u_Time / 20.0 - vec2( 0, u_Time * 0.2 ) );
	vec4 color2 = texture2D( u_TextureColor, newUV - vec2( u_Time / 21.0, 0.0 ) - vec2( 0, u_Time * 0.2 ) );

	color1 *= 0.2;
	color1 += vec4(
		sin( v_Worldpos.z / 50.0 + u_Time * 0.2 ) / 2.0 + 0.5,
		sin( -v_Worldpos.z / 100.0 + u_Time * 0.2 ) / 2.0 + 0.5,
		sin( v_Worldpos.z / 75.0 + u_Time * 0.2 ) / 2.0 + 0.5,
		1
	);

	vec4 color = color1 + color2 * -0.33;

    //if( length( color.xyz ) < 1 )
    //    color.xyz = normalize( color.xyz );

    //vec4 color = texture2D( u_TextureColor, v_UVCoord );

	gl_FragColor = color;
}

#endif
