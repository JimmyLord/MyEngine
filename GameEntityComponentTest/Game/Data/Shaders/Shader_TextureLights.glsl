#ifdef WIN32
#version 120
#define lowp
#define mediump
#else
precision mediump float;
#endif

#ifdef PassMain

    #include "Include/WSVaryings.glsl"

    varying lowp vec2 v_UVCoord;
    varying lowp vec4 v_Color;
#if ReceiveShadows
    varying lowp vec4 v_ShadowPos;
#endif //ReceiveShadows

    uniform mat4 u_World;
    uniform mat4 u_WorldViewProj;

#if ReceiveShadows
    uniform mat4 u_ShadowLightWVP;
    uniform sampler2D u_ShadowTexture;
#endif //ReceiveShadows

    uniform sampler2D u_TextureColor;
    uniform vec4 u_TextureTintColor;
    uniform vec4 u_TextureSpecColor;
    uniform float u_Shininess;

    uniform vec3 u_WSCameraPos;

    #include "Include/Light_Uniforms.glsl"

    #ifdef VertexShader

        attribute vec4 a_Position;
        attribute vec2 a_UVCoord;
        attribute vec3 a_Normal;
        //attribute vec4 a_VertexColor;

        #include "Include/WSVaryings_Functions.glsl"

        void main()
        {
            gl_Position = u_WorldViewProj * a_Position;
#if ReceiveShadows
            v_ShadowPos = u_ShadowLightWVP * a_Position;
#endif //ReceiveShadows

            SetWSPositionAndNormalVaryings( u_World, a_Position, a_Normal );
            v_UVCoord = a_UVCoord;
            v_Color = u_TextureTintColor;
        }

    #endif //VertexShader

    #ifdef FragmentShader

        #include "Include/Light_Functions.glsl"

        void main()
        {
            // Get the textures color.
            vec4 texcolor = texture2D( u_TextureColor, v_UVCoord );

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
            vec4 ambdiff = texcolor * v_Color * ( finalambient + finaldiffuse );
            vec4 spec = u_TextureSpecColor * finalspecular;

            // Calculate final color including whether it's in shadow or not.
            gl_FragColor = ( ambdiff + spec ) * shadowperc;
            gl_FragColor.a = 1.0;
        }

    #endif //Fragment Shader

#endif //PassMain

#ifdef PassShadowCastRGB

    #ifdef VertexShader

        uniform mat4 u_WorldViewProj;

        attribute vec4 a_Position;

        void main()
        {
            gl_Position = u_WorldViewProj * a_Position;
        }

    #endif //VertexShader

    #ifdef FragmentShader

        void main()
        {
            float value = gl_FragCoord.z;

            // Pack depth float value into RGBA, for ES 2.0 where depth textures don't exist.
            const vec4 bitSh = vec4( 256.0*256.0*256.0, 256.0*256.0, 256.0, 1.0 );
            const vec4 bitMsk = vec4( 0.0, 1.0/256.0, 1.0/256.0, 1.0/256.0 );
            vec4 res = fract( value * bitSh );
            res -= res.xxyz * bitMsk;

            gl_FragColor = res;
        }

    #endif //Fragment Shader

#endif //PassShadowCastRGB
