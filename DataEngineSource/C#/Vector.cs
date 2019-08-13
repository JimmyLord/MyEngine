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
    public class vec3
    {
        public float x, y, z;

        public vec3(float nx, float ny, float nz)
        {
            x = nx;
            y = ny;
            z = nz;
        }

        public void Length()        { GCHandle h = GCHandle.Alloc(this, GCHandleType.Pinned); Length( h.AddrOfPinnedObject() );        h.Free(); }
        public void LengthSquared() { GCHandle h = GCHandle.Alloc(this, GCHandleType.Pinned); LengthSquared( h.AddrOfPinnedObject() ); h.Free(); }
        public void Normalize()     { GCHandle h = GCHandle.Alloc(this, GCHandleType.Pinned); Normalize( h.AddrOfPinnedObject() );     h.Free(); }

        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void Length(IntPtr pThis);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void LengthSquared(IntPtr pThis);
        [MethodImpl(MethodImplOptions.InternalCall)] public extern static void Normalize(IntPtr pThis);

        //private IntPtr m_pNativeObject = (IntPtr)null;

        //public float x { get { return GetX( this.m_pNativeObject ); } }
        //public float y { get { return GetY( this.m_pNativeObject ); } }
        //public float z { get { return GetZ( this.m_pNativeObject ); } }
        //[MethodImpl(MethodImplOptions.InternalCall)] private extern static float GetX(IntPtr pNativeObject);
        //[MethodImpl(MethodImplOptions.InternalCall)] private extern static float GetY(IntPtr pNativeObject);
        //[MethodImpl(MethodImplOptions.InternalCall)] private extern static float GetZ(IntPtr pNativeObject);

        //[MethodImpl(MethodImplOptions.InternalCall)] public extern static vec3 GetVector();
    }
}
