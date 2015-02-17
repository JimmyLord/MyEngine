// Shader_TextureVertexColor.glsl

#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

varying lowp vec4 v_Color;
varying lowp vec2 v_UVCoord;

#ifdef VertexShader

uniform mat4 u_WorldViewProj;

attribute vec4 a_Position;
attribute vec4 a_VertexColor;
attribute vec2 a_UVCoord;

void main()
{
    v_Color = a_VertexColor;
    v_UVCoord = a_UVCoord;

    gl_Position = u_WorldViewProj * a_Position;
}

#endif

#ifdef FragmentShader

uniform sampler2D u_TextureColor;

void main()
{
    gl_FragColor = texture2D( u_TextureColor, v_UVCoord ) * v_Color;
}

#endif
