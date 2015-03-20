//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentAnimationPlayer_H__
#define __ComponentAnimationPlayer_H__

class ComponentTransform;

class ComponentAnimationPlayer : public ComponentUpdateable
{
public:
    ComponentMesh* m_pMeshComponent;

    int m_AnimationIndex;
    float m_AnimationTime;

    int m_LastAnimationIndex;
    float m_LastAnimationTime;

    float m_TransitionTimeLeft;
    float m_TransitionTimeTotal;

public:
    ComponentAnimationPlayer();
    virtual ~ComponentAnimationPlayer();

    static void LuaRegister(lua_State* luastate);

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentAnimationPlayer&)*pObject; }
    virtual ComponentAnimationPlayer& operator=(const ComponentAnimationPlayer& other);

    virtual void Tick(double TimePassed);

    void SetCurrentAnimation(int anim);

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentAnimationPlayer*)pObjectPtr)->OnLeftClick( true ); }
    void OnLeftClick(bool clear);
    virtual void FillPropertiesWindow(bool clear);
#endif //MYFW_USING_WX
};

#endif //__ComponentAnimationPlayer_H__
