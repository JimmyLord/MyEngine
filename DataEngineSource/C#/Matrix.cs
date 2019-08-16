//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace MyEngine
{ 
    [StructLayout(LayoutKind.Sequential)]
    public class mat4
    {
        // Values are stored column major.
        // m11 m21 m31 m41       Sx  0  0 Tx
        // m12 m22 m32 m42  --\   0 Sy  0 Ty
        // m13 m23 m33 m43  --/   0  0 Sz Tz
        // m14 m24 m34 m44        0  0  0  1
        public float m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44;

        public mat4()
        {
            m11 = 11;
            m12 = 12;
            m13 = 13;
            m14 = 14;
        }

        public void SetIdentity()
        {
            //GCHandle h = GCHandle.Alloc(this, GCHandleType.Pinned); SetIdentity( h.AddrOfPinnedObject() ); h.Free();

            // Test time to call C++ SetIdentity 1,000,000 times. // Result: 0.0548 seconds
            {
                DateTime startTime = DateTime.Now;
                for( int i=0; i<1000000; i++ )
                {
                    SetIdentity( this );
                }
                DateTime endTime = DateTime.Now;
                MyEngine.Log.Info( "SetIdentity C++ time: " + endTime.Subtract( startTime ).ToString() );                
            }

            // Test time to set identity in C# 1,000,000 times. // Result: 0.0278 seconds
            {
                DateTime startTime = DateTime.Now;
                for( int i=0; i<1000000; i++ )
                {
                    m12 = m13 = m14 = m21 = m23 = m24 = m31 = m32 = m34 = m41 = m42 = m43 = 0;
                    m11 = m22 = m33 = m44 = 1;
                }
                DateTime endTime = DateTime.Now;
                MyEngine.Log.Info( "SetIdentity C# time : " + endTime.Subtract( startTime ).ToString() );
            }
        }

        public static mat4 operator*(mat4 t, mat4 o)
        {
            mat4 newmat = new mat4();

            newmat.m11 = t.m11 * o.m11 + t.m21 * o.m12 + t.m31 * o.m13 + t.m41 * o.m14;
            newmat.m12 = t.m12 * o.m11 + t.m22 * o.m12 + t.m32 * o.m13 + t.m42 * o.m14;
            newmat.m13 = t.m13 * o.m11 + t.m23 * o.m12 + t.m33 * o.m13 + t.m43 * o.m14;
            newmat.m14 = t.m14 * o.m11 + t.m24 * o.m12 + t.m34 * o.m13 + t.m44 * o.m14;
            newmat.m21 = t.m11 * o.m21 + t.m21 * o.m22 + t.m31 * o.m23 + t.m41 * o.m24;
            newmat.m22 = t.m12 * o.m21 + t.m22 * o.m22 + t.m32 * o.m23 + t.m42 * o.m24;
            newmat.m23 = t.m13 * o.m21 + t.m23 * o.m22 + t.m33 * o.m23 + t.m43 * o.m24;
            newmat.m24 = t.m14 * o.m21 + t.m24 * o.m22 + t.m34 * o.m23 + t.m44 * o.m24;
            newmat.m31 = t.m11 * o.m31 + t.m21 * o.m32 + t.m31 * o.m33 + t.m41 * o.m34;
            newmat.m32 = t.m12 * o.m31 + t.m22 * o.m32 + t.m32 * o.m33 + t.m42 * o.m34;
            newmat.m33 = t.m13 * o.m31 + t.m23 * o.m32 + t.m33 * o.m33 + t.m43 * o.m34;
            newmat.m34 = t.m14 * o.m31 + t.m24 * o.m32 + t.m34 * o.m33 + t.m44 * o.m34;
            newmat.m41 = t.m11 * o.m41 + t.m21 * o.m42 + t.m31 * o.m43 + t.m41 * o.m44;
            newmat.m42 = t.m12 * o.m41 + t.m22 * o.m42 + t.m32 * o.m43 + t.m42 * o.m44;
            newmat.m43 = t.m13 * o.m41 + t.m23 * o.m42 + t.m33 * o.m43 + t.m43 * o.m44;
            newmat.m44 = t.m14 * o.m41 + t.m24 * o.m42 + t.m34 * o.m43 + t.m44 * o.m44;

            return newmat;
        }

        public void SetIdentityWorking()
        {
            m12 = m13 = m14 = m21 = m23 = m24 = m31 = m32 = m34 = m41 = m42 = m43 = 0;
            m11 = m22 = m33 = m44 = 1;
        }

        public void Scale(vec3 scale)
        {
            m11 *= scale.x; m21 *= scale.x; m31 *= scale.x; m41 *= scale.x;
            m12 *= scale.y; m22 *= scale.y; m32 *= scale.y; m42 *= scale.y;
            m13 *= scale.z; m32 *= scale.z; m33 *= scale.z; m43 *= scale.z;
        }

        public void Rotate(float angle, float x, float y, float z)
        {
            float sinAngle, cosAngle;
            float mag = (float)System.Math.Sqrt( x * x + y * y + z * z );
      
            sinAngle = (float)System.Math.Sin( angle * System.Math.PI / 180.0f );
            cosAngle = (float)System.Math.Cos( angle * System.Math.PI / 180.0f );
            if( mag > 0.0f )
            {
                float xx, yy, zz, xy, yz, zx, xs, ys, zs;
                float oneMinusCos;
   
                x /= mag;
                y /= mag;
                z /= mag;

                xx = x * x;
                yy = y * y;
                zz = z * z;
                xy = x * y;
                yz = y * z;
                zx = z * x;
                xs = x * sinAngle;
                ys = y * sinAngle;
                zs = z * sinAngle;
                oneMinusCos = 1.0f - cosAngle;

                mat4 rotMat = new mat4();
                rotMat.m11 = (oneMinusCos * xx) + cosAngle;
                rotMat.m12 = (oneMinusCos * xy) - zs;
                rotMat.m13 = (oneMinusCos * zx) + ys;
                rotMat.m14 = 0.0f; 

                rotMat.m21 = (oneMinusCos * xy) + zs;
                rotMat.m22 = (oneMinusCos * yy) + cosAngle;
                rotMat.m23 = (oneMinusCos * yz) - xs;
                rotMat.m24 = 0.0f;

                rotMat.m31 = (oneMinusCos * zx) - ys;
                rotMat.m32 = (oneMinusCos * yz) + xs;
                rotMat.m33 = (oneMinusCos * zz) + cosAngle;
                rotMat.m34 = 0.0f; 

                rotMat.m41 = 0.0f;
                rotMat.m42 = 0.0f;
                rotMat.m43 = 0.0f;
                rotMat.m44 = 1.0f;

                mat4 temp = rotMat * this;
                m11 = temp.m11;
                m12 = temp.m12;
                m13 = temp.m13;
                m14 = temp.m14;
                m21 = temp.m21;
                m22 = temp.m22;
                m23 = temp.m23;
                m24 = temp.m24;
                m31 = temp.m31;
                m32 = temp.m32;
                m33 = temp.m33;
                m34 = temp.m34;
                m41 = temp.m41;
                m42 = temp.m42;
                m43 = temp.m43;
                m44 = temp.m44;
            }
        }

        public void Translate(float x, float y, float z)
        {
            m41 += x;
            m42 += y;
            m43 += z;
        }

        public void CreateSRTWorking(vec3 scale, vec3 rot, vec3 pos)
        {
            SetIdentityWorking();
            Scale( scale );
            Rotate( rot.z, 0, 0, 1 ); // roll
            Rotate( rot.x, 1, 0, 0 ); // pitch
            Rotate( rot.y, 0, 1, 0 ); // yaw
            Translate( pos.x, pos.y, pos.z );
        }

        public void CreateSRT(vec3 scale, vec3 rot, vec3 pos)
        {
            // Test time to call C++ CreateSRT as pinned gc handles 1,000,000 times. // Result: 1.35 seconds
            {
                DateTime startTime = DateTime.Now;
                for( int i=0; i<1000000; i++ )
                {
                    GCHandle hThis = GCHandle.Alloc(this, GCHandleType.Pinned);
                    GCHandle hScale = GCHandle.Alloc(scale, GCHandleType.Pinned);
                    GCHandle hRot = GCHandle.Alloc(rot, GCHandleType.Pinned);
                    GCHandle hPos = GCHandle.Alloc(pos, GCHandleType.Pinned);
                    CreateSRTPinned( hThis.AddrOfPinnedObject(), hScale.AddrOfPinnedObject(), hRot.AddrOfPinnedObject(), hPos.AddrOfPinnedObject() );
                    hThis.Free();
                    hScale.Free();
                    hRot.Free();
                    hPos.Free();
                }
                DateTime endTime = DateTime.Now;
                MyEngine.Log.Info( "CreateSRT C++ Pinned time : " + endTime.Subtract( startTime ).ToString() );                
            }

            // Test time to call C++ CreateSRT directly with hacked ptrs in C++ 1,000,000 times. // Result: 0.81 seconds
            {
                DateTime startTime = DateTime.Now;
                for( int i=0; i<1000000; i++ )
                {
                    CreateSRT( this, scale, rot, pos );
                }
                DateTime endTime = DateTime.Now;
                MyEngine.Log.Info( "CreateSRT C++ Hack time   : " + endTime.Subtract( startTime ).ToString() );
            }

            // Test time for one call to C++ which calls CreateSRT 1,000,000 times. // Result: 0.77 seconds
            {
                DateTime startTime = DateTime.Now;
                CreateSRTMillion( this, scale, rot, pos );
                DateTime endTime = DateTime.Now;
                MyEngine.Log.Info( "CreateSRT C++ Million time: " + endTime.Subtract( startTime ).ToString() );
            }

            // Test time to call CreateSRT in C# 1,000,000 times. // Result: 0.46 seconds
            {
                DateTime startTime = DateTime.Now;
                for( int i=0; i<1000000; i++ )
                {
                    CreateSRTWorking( scale, rot, pos );
                }
                DateTime endTime = DateTime.Now;
                MyEngine.Log.Info( "CreateSRT C# time         : " + endTime.Subtract( startTime ).ToString() );
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void SetIdentity(mat4 pThis);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void CreateSRTPinned(IntPtr pThis, IntPtr scale, IntPtr rot, IntPtr pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void CreateSRT(mat4 pThis, vec3 scale, vec3 rot, vec3 pos);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void CreateSRTMillion(mat4 pThis, vec3 scale, vec3 rot, vec3 pos);
    }
}
