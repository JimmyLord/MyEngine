#if NUM_INFLUENCE_BONES > 0
void ApplyBoneInfluencesToPositionAndNormalAttributes(inout vec4 pos, inout vec4 normal)
{
    mat4 bonetransform;
    bonetransform  = u_BoneTransforms[a_BoneIndex[0]] * a_BoneWeight[0];
#if NUM_INFLUENCE_BONES > 1
    bonetransform += u_BoneTransforms[a_BoneIndex[1]] * a_BoneWeight[1];
#elif NUM_INFLUENCE_BONES > 2
    bonetransform += u_BoneTransforms[a_BoneIndex[2]] * a_BoneWeight[2];
#elif NUM_INFLUENCE_BONES > 3
    bonetransform += u_BoneTransforms[a_BoneIndex[3]] * a_BoneWeight[3];
#endif

    pos = bonetransform * a_Position;
    normal = bonetransform * vec4( a_Normal.xyz, 0 );
}
#else
void ApplyBoneInfluencesToPositionAndNormalAttributes(inout vec4 pos, inout vec4 normal)
{
    pos = a_Position;
    normal = a_Normal;
}
#endif
