#if NUM_LIGHTS > 0
    uniform vec3 u_LightPos[NUM_LIGHTS];
    //uniform vec3 u_LightDir[NUM_LIGHTS]; // for spot lights.
    uniform vec4 u_LightColor[NUM_LIGHTS];
    uniform vec3 u_LightAttenuation[NUM_LIGHTS];
#endif
