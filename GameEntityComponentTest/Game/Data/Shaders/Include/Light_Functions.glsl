#if NUM_LIGHTS > 0
void PointLightContribution(int index, vec3 vertpos, vec3 campos, vec3 normal, float shininess, inout vec4 finaldiffuse, inout vec4 finalspecular)
{
    // vert, cam, normal and light positions are in local space.

    // Light properties
    vec3 lightpos = u_LightPos[index];
    vec4 lightcolor = u_LightColor[index];
    vec3 lightatten = u_LightAttenuation[index];

    vec3 unnormalizedlightdirvector = lightpos - vertpos;
    vec3 lightdirvector = normalize( unnormalizedlightdirvector );

    // attenuation
    float dist = length( unnormalizedlightdirvector );
    float attenuation = 1.0 / (lightatten.x + dist*lightatten.y + dist*dist*lightatten.z);

    // diffuse
    float diffperc = max( dot( normal, lightdirvector ), 0.0 );
    finaldiffuse += lightcolor * diffperc * attenuation;

    // specular
    vec3 viewvector = normalize( campos - vertpos );
    vec3 halfvector = normalize( viewvector + lightdirvector );
    float specperc = max( dot( normal, halfvector ), 0.0 );
    specperc = pow( specperc, shininess );
    finalspecular += lightcolor * specperc * attenuation;
}
#endif

float CalculateShadowPercentage()
{
#if ReceiveShadows
    vec2 shadowcoord = v_ShadowPos.xy / v_ShadowPos.w;
    vec4 shadowtex = texture2D( u_ShadowTexture, shadowcoord );
    
    float bias = 0.002;
    float projzdepth = (v_ShadowPos.z-bias) / v_ShadowPos.w;

#if 1
    float texzdepth = shadowtex.r;
#else
    // Unpack depth float value from rgba.
    const vec4 bitSh = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float texzdepth = dot( shadowtex, bitSh );
#endif
        
    if( texzdepth < projzdepth )
        return 0.5;
#endif //ReceiveShadows

    return 1.0;
}

