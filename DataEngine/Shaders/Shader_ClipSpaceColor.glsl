#define BLENDING On
#define BLENDFUNC One One

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
}

#endif

#ifdef FragmentShader

uniform vec4 u_TextureTintColor;

void main()
{
    gl_FragColor = u_TextureTintColor;
}

#endif
