//
// Copyright (c) 2014 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentCollisionObject_H__
#define __ComponentCollisionObject_H__

class ComponentCollisionObject : public ComponentUpdateable
{
public:
    ComponentTransform* m_pComponentTransform;

    btRigidBody* m_pBody;

public:
    ComponentCollisionObject();
    ComponentCollisionObject(GameObject* owner);
    virtual ~ComponentCollisionObject();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj);

    virtual void Reset();

    virtual void OnPlay();
    virtual void OnStop();
    virtual void Tick(double TimePassed);

    void SyncRigidBodyToTransform();

public:
#if MYFW_USING_WX
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticFillPropertiesWindow(void* pObjectPtr) { ((ComponentCollisionObject*)pObjectPtr)->FillPropertiesWindow(true); }
    void FillPropertiesWindow(bool clear);
#endif //MYFW_USING_WX
};

#endif //__ComponentCollisionObject_H__
