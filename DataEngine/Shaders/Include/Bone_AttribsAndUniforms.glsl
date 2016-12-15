#if NUM_INF_BONES > 0
    const int MAX_BONES = 50;

    attribute vec4 a_BoneIndex;
    attribute vec4 a_BoneWeight;

    uniform mat4 u_BoneTransforms[MAX_BONES];
#endif
