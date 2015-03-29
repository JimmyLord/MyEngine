//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentCameraShadow_H__
#define __ComponentCameraShadow_H__

#include "../../Camera/Camera2D.h"
#include "../../Camera/Camera3D.h"

class ComponentTransform;

class ComponentCameraShadow : public ComponentCamera
{
public:
    FBODefinition* m_pDepthFBO;
    MyMatrix m_matViewProj;

public:
    ComponentCameraShadow();
    virtual ~ComponentCameraShadow();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentCameraShadow&)*pObject; }
    virtual ComponentCameraShadow& operator=(const ComponentCameraShadow& other);

    virtual void Tick(double TimePassed);
    virtual void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
    virtual void OnDrawFrame();

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentCameraShadow*)pObjectPtr)->OnLeftClick( true ); }
    void OnLeftClick(bool clear);
    virtual void FillPropertiesWindow(bool clear);
    static void StaticOnValueChanged(void* pObjectPtr, int id, bool finishedchanging) { ((ComponentCameraShadow*)pObjectPtr)->OnValueChanged( id, finishedchanging ); }
    void OnValueChanged(int id, bool finishedchanging);
#endif //MYFW_USING_WX
};

#endif //__ComponentCameraShadow_H__
