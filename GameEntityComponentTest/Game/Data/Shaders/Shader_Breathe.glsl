#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

varying vec4 v_Normal;

uniform float u_Time;

#ifdef VertexShader

attribute vec4 a_Position;
attribute vec4 a_Normal;

uniform mat4 u_World;
uniform mat4 u_ViewProj;
uniform float u_PointSize;

void main()
{
	gl_PointSize = u_PointSize;

    //gl_Position = u_WorldViewProj * a_Position;

	//gl_Position.y *= 1 + sin( u_Time ) * 0.05;

	vec4 worldpos = u_World * a_Position;

    //worldpos.y += sin( u_Time + worldpos.x * 6 );

    vec4 wrappedpos = worldpos;
	wrappedpos.y = sin( worldpos.x ) * (1.0 - worldpos.y * 0.2);
	wrappedpos.x = cos( worldpos.x ) * (1.0 - worldpos.y * 0.2);

    gl_Position = u_ViewProj * wrappedpos;

	v_Normal = a_Normal;
}

#endif

#ifdef FragmentShader

uniform sampler2D u_TextureColor;

void main()
{
	gl_FragColor = v_Normal;
}

#endif
