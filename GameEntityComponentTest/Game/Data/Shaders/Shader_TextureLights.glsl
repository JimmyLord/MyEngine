#ifdef WIN32
#version 120
#define lowp
#define mediump
#else
precision mediump float;
#endif

#ifdef PassMain

    varying lowp vec3 v_WSPosition;
    varying lowp vec2 v_UVCoord;
    varying lowp vec3 v_WSNormal;
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

    uniform vec3 u_CameraPos;

    #include "Include/Light_Uniforms.glsl"

    #ifdef VertexShader

        attribute vec4 a_Position;
        attribute vec2 a_UVCoord;
        attribute vec3 a_Normal;
        //attribute vec4 a_VertexColor;

        void main()
        {
            vec4 LSPosition = a_Position;
                
            //LSPosition.y += sin( a_Position.x );

            gl_Position = u_WorldViewProj * LSPosition;
#if ReceiveShadows
            v_ShadowPos = u_ShadowLightWVP * LSPosition;
#endif //ReceiveShadows

            v_WSPosition = (u_World * LSPosition).xyz;
            mat3 normalmatrix = mat3( u_World );
            v_WSNormal = normalize( normalmatrix * a_Normal );
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

            // Calculate the normal vector in world space. normalized in vertex shader
            //   left this here for future where normal might come from a normal map.
            vec3 normalworld = v_WSNormal;

            //normalworld = normalize( vec3( -cos( v_WSPosition.x ), 1, 0 ) );

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
                PointLightContribution( i, normalworld, finaldiffuse, finalspecular );
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
