//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentLight_H__
#define __ComponentLight_H__

class ComponentLight : public ComponentData
{
public:
    MyLight* m_pLight;

public:
    ComponentLight();
    virtual ~ComponentLight();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentLight&)*pObject; }
    virtual ComponentLight& operator=(const ComponentLight& other);

    static void StaticOnTransformPositionChanged(void* pObjectPtr, Vector3& newpos) { ((ComponentLight*)pObjectPtr)->OnTransformPositionChanged( newpos ); }
    void OnTransformPositionChanged(Vector3& newpos);

public:
#if MYFW_USING_WX
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentLight*)pObjectPtr)->OnLeftClick( true ); }
    void OnLeftClick(bool clear);
    virtual void FillPropertiesWindow(bool clear);
#endif //MYFW_USING_WX
};

#endif //__ComponentLight_H__