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

uniform mat4 u_WorldViewProj;
uniform mat3 u_UVTransform;

void main()
{
    gl_Position = u_WorldViewProj * a_Position;

    v_UVCoord = ( u_UVTransform * vec3(a_UVCoord, 1) ).xy;
}

#endif

#ifdef FragmentShader

uniform sampler2D u_TextureColor;
uniform vec4 u_TextureTintColor;

void main()
{
    //gl_FragColor = vec4( v_UVCoord, 0, 1 );
    gl_FragColor = texture2D( u_TextureColor, v_UVCoord ) * u_TextureTintColor;

    if( gl_FragColor.a < 1 )
        discard;
}

#endif
