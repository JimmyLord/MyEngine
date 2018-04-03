#define BLENDING Off

#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

#ifdef VertexShader

attribute vec4 a_Position;

void main()
{
    gl_Position = a_Position;
	gl_Position.z = 0;
}

#endif

#ifdef FragmentShader

void main()
{
    gl_FragColor = vec4( 0, 0, 1, 1 );
}

#endif
