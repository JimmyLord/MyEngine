uniform vec2 u_ViewportSize;
uniform float u_ZNear;
uniform float u_ZFar;
uniform mat4 u_InverseView;
uniform mat4 u_InverseProj;

vec3 WorldPositionFromDepth(float depth)
{
    vec2 xy = (gl_FragCoord.xy / u_ViewportSize) * 2.0 - 1.0;
    float z = depth * 2.0 - 1.0;

    vec4 CSPosition = vec4( xy, z, 1.0 );
    vec4 VSPosition = u_InverseProj * CSPosition;
    VSPosition /= VSPosition.w;
    vec4 WSPosition = u_InverseView * VSPosition;

    return WSPosition.xyz;
}

float ViewSpaceZFromDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    float zVS = 2.0 * u_ZNear * u_ZFar / ( u_ZFar + u_ZNear - z * (u_ZFar - u_ZNear) );

    return zVS;
}
