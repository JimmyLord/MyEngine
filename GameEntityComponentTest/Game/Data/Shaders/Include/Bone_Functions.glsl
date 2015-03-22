#ifdef NO_NORMALS

    #if NUM_INF_BONES > 0

        void ApplyBoneInfluencesToPositionAttribute(vec4 attribpos, out vec4 outpos)
        {
            mat4 bonetransform;
            bonetransform  = u_BoneTransforms[a_BoneIndex[0]] * a_BoneWeight[0];
        #if NUM_INF_BONES > 1
            bonetransform += u_BoneTransforms[a_BoneIndex[1]] * a_BoneWeight[1];
        #elif NUM_INF_BONES > 2
            bonetransform += u_BoneTransforms[a_BoneIndex[2]] * a_BoneWeight[2];
        #elif NUM_INF_BONES > 3
            bonetransform += u_BoneTransforms[a_BoneIndex[3]] * a_BoneWeight[3];
        #endif

            outpos = bonetransform * attribpos;
        }

    #else //NUM_INF_BONES > 0

        void ApplyBoneInfluencesToPositionAttribute(vec4 attribpos, out vec4 outpos)
        {
            outpos = attribpos;
        }

    #endif //NUM_INF_BONES > 0

#else //NO_NORMALS

    #if NUM_INF_BONES > 0

        void ApplyBoneInfluencesToPositionAndNormalAttributes(vec4 attribpos, vec3 attribnormal, out vec4 outpos, out vec3 outnormal)
        {
            mat4 bonetransform;
            bonetransform  = u_BoneTransforms[a_BoneIndex[0]] * a_BoneWeight[0];
        #if NUM_INF_BONES > 1
            bonetransform += u_BoneTransforms[a_BoneIndex[1]] * a_BoneWeight[1];
        #elif NUM_INF_BONES > 2
            bonetransform += u_BoneTransforms[a_BoneIndex[2]] * a_BoneWeight[2];
        #elif NUM_INF_BONES > 3
            bonetransform += u_BoneTransforms[a_BoneIndex[3]] * a_BoneWeight[3];
        #endif

            outpos = bonetransform * attribpos;
            outnormal = ( bonetransform * vec4( attribnormal, 0 ) ).xyz;
        }

    #else //NUM_INF_BONES > 0

        void ApplyBoneInfluencesToPositionAndNormalAttributes(vec4 attribpos, vec3 attribnormal, out vec4 outpos, out vec3 outnormal)
        {
            outpos = attribpos;
            outnormal = attribnormal;
        }

    #endif //NUM_INF_BONES > 0

#endif //NO_NORMALS
