#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 0
#endif

#if NUM_DIR_LIGHTS > 0
void DirLightContribution(vec3 vertPos, vec3 cameraPos, vec3 normal, float shininess, inout vec3 finalAmbient, inout vec3 finalDiffuse, inout vec3 finalSpecular)
{
    // Light properties
    vec3 lightDir = u_DirLightDir;
    vec3 lightColor = u_DirLightColor;

    vec3 unnormalizedLightDirVector = lightDir;
    vec3 lightDirVector = normalize( unnormalizedLightDirVector ) * -1.0;

    // Ambient.
    finalAmbient += lightColor * 0.1;

    // Diffuse.
    float diffPerc = max( 0.0, dot( normal, lightDirVector ) );
    finalDiffuse += lightColor * diffPerc;

    // Specular.
    vec3 viewVector = normalize( cameraPos - vertPos );
    vec3 halfVector = normalize( viewVector + lightDirVector );
    float specPerc = max( 0.0, dot( normal, halfVector ) );
    specPerc = pow( specPerc, shininess );
    finalSpecular += lightColor * specPerc;
}
#endif

#if NUM_LIGHTS > 0
void PointLightContribution(vec3 lightPos, vec3 lightColor, vec3 lightAtten, vec3 vertPos, vec3 cameraPos, vec3 normal, float shininess, inout vec3 finalAmbient, inout vec3 finalDiffuse, inout vec3 finalSpecular)
{
    vec3 unnormalizedLightDirVector = lightPos - vertPos;
    vec3 lightDirVector = normalize( unnormalizedLightDirVector );

    // Attenuation.
    //float dist = length( unnormalizedLightDirVector );
    //float attenuation = 1.0 / (0.00001 + lightAtten.x + dist*lightAtten.y + dist*dist*lightAtten.z);
    float lightRadius = lightAtten.x;
    vec3 lightDirShortened = unnormalizedLightDirVector / lightRadius;
    float attenuation = max( 0.0, 1.0 - dot( lightDirShortened, lightDirShortened ) );

    // Ambient.
    finalAmbient += lightColor * 0.0;

    // Diffuse.
    float diffPerc = max( 0.0, dot( normal, lightDirVector ) );
    finalDiffuse += lightColor * diffPerc * attenuation;

    // Specular.
    vec3 camDirVector = normalize( cameraPos - vertPos );
    vec3 halfVector = normalize( camDirVector + lightDirVector );
    float specPerc = max( 0.0, dot( normal, halfVector ) );
    specPerc = pow( specPerc, shininess );
    finalSpecular += lightColor * specPerc * attenuation;
}
#endif

float CalculateShadowPercentage(vec4 shadowPos)
{
#if ReceiveShadows

    vec2 shadowCoord = shadowPos.xy / shadowPos.w;
    vec4 shadowTex = texture2D( u_ShadowTexture, shadowCoord );

    float bias = 0.0002;
    float projZDepth = (shadowPos.z - bias) / shadowPos.w;

#if 1
    float texZDepth = shadowTex.r;
#else
    // Unpack depth float value from rgba.
    const vec4 bitSh = vec4( 1.0/(256.0*256.0*256.0), 1.0/(256.0*256.0), 1.0/256.0, 1.0 );
    float texZDepth = dot( shadowTex, bitSh );
#endif

    if( texZDepth < projZDepth )
        return 0.0;

#endif //ReceiveShadows

    return 1.0;
}

