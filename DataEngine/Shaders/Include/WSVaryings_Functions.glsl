void SetWSPositionAndNormalVaryings(mat4 matworld, vec4 attribpos, vec3 attribnormal)
{
    v_WSPosition = matworld * attribpos;
    v_WSNormal = ( matworld * vec4( attribnormal, 0 ) ).xyz;
}
