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

	vec4 newpos = u_World * a_Position;
	newpos.y = sin( newpos.x );
	newpos.x = cos( newpos.x );

    gl_Position = u_ViewProj * newpos;

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
