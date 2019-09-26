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

namespace MyEngine
{
    public class ComponentTransform : ComponentBase
    {
        public vec3 GetLocalPosition() { return ComponentTransform.GetLocalPosition( m_pNativeObject ); }
        public void SetLocalPosition(float x, float y, float z) { ComponentTransform.SetLocalPosition( m_pNativeObject, x, y, z ); }
        public void SetLocalPosition(vec3 pos) { ComponentTransform.SetLocalPosition( m_pNativeObject, pos ); }
        public void SetLocalTransform(ref mat4 transform) { ComponentTransform.SetLocalTransform( m_pNativeObject, ref transform ); }

        [MethodImpl(MethodImplOptions.InternalCall)] private extern static vec3 GetLocalPosition(IntPtr pNativeObject);
        [MethodImpl(MethodImplOptions.InternalCall)] private extern static void SetLocalPosition(IntPtr pNativeObject, float x, float y, float z);
        [MethodImpl(MethodImplOptions.InternalCall)] private extern static void SetLocalPosition(IntPtr pNativeObject, vec3 pos);
        [MethodImpl(MethodImplOptions.InternalCall)] private extern static void SetLocalTransform(IntPtr pNativeObject, ref mat4 transform);
    }
}
