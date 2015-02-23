#if NUM_LIGHTS > 0
void PointLightContribution(int index, vec3 normalworld, inout vec4 finaldiffuse, inout vec4 finalspecular)
{
    // Vertex and camera positions in world space.
    vec3 vertposworld = v_WSPosition;
    vec3 camposworld = u_CameraPos;

    // Material properties
    float shininess = u_Shininess;

    // Light properties
    vec3 lightpos = u_LightPos[index];
    vec4 lightcolor = u_LightColor[index];
    vec3 lightatten = u_LightAttenuation[index];

    vec3 unnormalizedlightdirvector = lightpos - vertposworld;
    vec3 lightdirvector = normalize( unnormalizedlightdirvector );

    // attenuation
    float dist = length( unnormalizedlightdirvector );
    float attenuation = 1.0 / (lightatten.x + dist*lightatten.y + dist*dist*lightatten.z);

    // diffuse
    float diffperc = max( dot( normalworld, lightdirvector ), 0.0 );
    finaldiffuse += lightcolor * diffperc * attenuation;

    // specular
    vec3 viewvector = normalize( camposworld - vertposworld );
    vec3 halfvector = normalize( viewvector + lightdirvector );
    float specperc = max( dot( normalworld, halfvector ), 0.0 );
    specperc = pow( specperc, shininess );
    finalspecular += lightcolor * specperc * attenuation;
}
#endif

float CalculateShadowPercentage()
{
#if ReceiveShadows
    vec2 shadowcoord = (v_ShadowPos.xy / v_ShadowPos.w) * 0.5 + 0.5;
    vec4 shadowtex = texture2D( u_ShadowTexture, shadowcoord );

    // Unpack depth float value from rgba.
    const vec4 bitSh = vec4(1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0);
    float texzdepth = dot( shadowtex, bitSh );

    float bias = 0.01;
    float projzdepth = ((v_ShadowPos.z-bias)/v_ShadowPos.w) * 0.5 + 0.5;

    if( texzdepth < projzdepth )
        return 0.5;
#endif //ReceiveShadows

    return 1.0;
}

