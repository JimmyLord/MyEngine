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
    public struct vec3
    {
        public float x, y, z;

        public vec3(float nx, float ny, float nz)
        {
            x = nx;
            y = ny;
            z = nz;
        }

        public override string ToString() { return "(" + x + "," + y + "," + z + ")"; }

        public float Length()                           { return Length( this ); }
        public float LengthSquared()                    { return LengthSquared( this ); }
        public void Normalize()                         { Normalize( this ); }
        public static vec3 operator+(vec3 t, vec3 o)    { return new vec3( t.x+o.x, t.y+o.y, t.z+o.z ); }
        public static vec3 operator-(vec3 t, vec3 o)    { return OperatorMinus( t, o ); }
        public static vec3 operator*(vec3 t, float o)   { return new vec3( t.x*o, t.y*o, t.z*o ); }

        [MethodImpl(MethodImplOptions.InternalCall)] private extern static float Length(vec3 pThis);
        [MethodImpl(MethodImplOptions.InternalCall)] private extern static float LengthSquared(vec3 pThis);
        [MethodImpl(MethodImplOptions.InternalCall)] private extern static void Normalize(vec3 pThis);
        [MethodImpl(MethodImplOptions.InternalCall)] private extern static vec3 OperatorMinus(vec3 pThis, vec3 pOther);
    }
}
