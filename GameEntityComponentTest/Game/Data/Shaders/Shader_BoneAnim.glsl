#ifdef WIN32
#define lowp
#define mediump
#else
precision mediump float;
#endif

#ifdef PassMain

    #include "Include/WSVaryings.glsl"

#if ReceiveShadows
    varying lowp vec4 v_ShadowPos;

    uniform mat4 u_ShadowLightWVPT;
    uniform sampler2D u_ShadowTexture;
#endif //ReceiveShadows

#ifdef VertexShader

    attribute vec4 a_Position;
    attribute vec3 a_Normal;

    uniform mat4 u_World;
    uniform mat4 u_WorldViewProj;

    #include "Include/Bone_AttribsAndUniforms.glsl"
    #include "Include/Bone_Functions.glsl"
    #include "Include/WSVaryings_Functions.glsl"

    void main()
    {
        vec4 pos;
        vec3 normal;
    
        ApplyBoneInfluencesToPositionAndNormalAttributes( a_Position, a_Normal, pos, normal );
        SetWSPositionAndNormalVaryings( u_World, pos, normal );

        gl_Position = u_WorldViewProj * pos;
#if ReceiveShadows
        v_ShadowPos = u_ShadowLightWVPT * pos;
#endif //ReceiveShadows
    }

#endif //VertexShader

#ifdef FragmentShader

    uniform vec3 u_WSCameraPos;
    uniform float u_Shininess;

    #include "Include/Light_Uniforms.glsl"
    #include "Include/Light_Functions.glsl"

    void main()
    {
        // Calculate the normal vector in local space. normalized again since interpolation can/will distort it.
        //   TODO: handle normal maps.
        vec3 WSnormal = normalize( v_WSNormal );

        // Whether fragment is in shadow or not, return 0.5 if it is, 1.0 if not.
        float shadowperc = CalculateShadowPercentage();

        // Hardcoded ambient
        vec4 finalambient = vec4(0.2, 0.2, 0.2, 1.0);

        // Accumulate diffuse and specular color for all lights.
        vec4 finaldiffuse = vec4(0,0,0,0);
        vec4 finalspecular = vec4(0,0,0,0);

        // Add in each light, one by one. // finaldiffuse, finalspecular are inout.
    #if NUM_LIGHTS > 0
        for( int i=0; i<NUM_LIGHTS; i++ )
            PointLightContribution( i, v_WSPosition.xyz, u_WSCameraPos, WSnormal, u_Shininess, finaldiffuse, finalspecular );
    #endif

        // Mix the texture color with the light color.
        vec4 ambdiff = /*texcolor * v_Color * */( finalambient + finaldiffuse );
        vec4 spec = /*u_TextureSpecColor **/ finalspecular;

        // Calculate final color including whether it's in shadow or not.
        gl_FragColor = ( ambdiff + spec ) * shadowperc;
        gl_FragColor.a = 1.0;
    }

#endif

#endif //PassMain

#ifdef PassShadowCastRGB

    #include "Include/WSVaryings.glsl"

#ifdef VertexShader

    attribute vec4 a_Position;
    attribute vec3 a_Normal;

    uniform mat4 u_World;
    uniform mat4 u_WorldViewProj;

    #include "Include/Bone_AttribsAndUniforms.glsl"
    #include "Include/Bone_Functions.glsl"
    #include "Include/WSVaryings_Functions.glsl"

    void main()
    {
        vec4 pos;
        vec3 normal;
    
        ApplyBoneInfluencesToPositionAndNormalAttributes( a_Position, a_Normal, pos, normal );
        SetWSPositionAndNormalVaryings( u_World, pos, normal );

        gl_Position = u_WorldViewProj * pos;
    }

#endif //VertexShader

#ifdef FragmentShader

    void main()
    {
#if 1
        gl_FragColor = vec4(1,1,1,1);
#else
        float value = gl_FragCoord.z;

        // Pack depth float value into RGBA, for ES 2.0 where depth textures don't exist.
        const vec4 bitSh = vec4( 256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0 );
        const vec4 bitMsk = vec4( 0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0 );
        vec4 res = fract( value * bitSh );
        res -= res.xxyz * bitMsk;

        gl_FragColor = res;
#endif
    }

#endif //Fragment Shader

#endif //PassShadowCastRGB
