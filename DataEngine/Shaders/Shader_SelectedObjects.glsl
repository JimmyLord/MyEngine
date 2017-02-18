#define BLENDING On

#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

#ifdef VertexShader

attribute vec4 a_Position;

uniform mat4 u_WorldViewProj;
uniform float u_Time;

#define NO_NORMALS 1
#include "Include/Bone_AttribsAndUniforms.glsl"
#include "Include/Bone_Functions.glsl"

void main()
{
    vec4 pos;
    ApplyBoneInfluencesToPositionAttribute( a_Position, pos );

    gl_Position = u_WorldViewProj * pos;
}

#endif

#ifdef FragmentShader

uniform float u_Time;
uniform vec2 u_FBSize;
uniform vec4 u_TextureTintColor;

void main()
{
    // shader is used twice, once for outlines (u_TextureTintColor = 1,1,1,1)
    //                       once for fill (u_TextureTintColor = 0,0,0,0)

    // make 10x10 pixel checkboard, slowly moving
	vec2 offset = floor( (gl_FragCoord.xy + u_Time*5) / vec2(10, 10) );
    float color = mod( offset.x + offset.y, 2.0 );

    // output solid white for outlines, and very transparent checkerboard for fill
	gl_FragColor = vec4( color, color, color, 0.05 ) + u_TextureTintColor;
}

#endif
