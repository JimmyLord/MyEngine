//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

namespace MyEngine
{
    public class MyScriptInterface
    {
        public MyEngine.GameObject m_GameObject;

        public virtual void OnLoad() { }
        public virtual void OnPlay() { }
        public virtual void OnStop() { }
        public virtual bool OnTouch(int action, int id, float x, float y, float pressure, float size) { return false; }
        public virtual bool OnButtons(int action, int id) { return false; }
        public virtual void Update(float deltaTime) { }
    }
}
