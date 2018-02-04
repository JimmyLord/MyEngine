//
// Copyright (c) 2015-2017 Jimmy Lord http://www.flatheadgames.com
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
protected:
    FBODefinition* m_pDepthFBO;

    MyLight* m_pLight;

public:
    FBODefinition* GetFBO() { return m_pDepthFBO; }
    MyMatrix* GetViewProjMatrix();

public:
    ComponentCameraShadow();
    virtual ~ComponentCameraShadow();
    SetClassnameBase( "CameraShadowComponent" ); // only first 8 character count.

    virtual cJSON* ExportAsJSONObject(bool savesceneid, bool saveid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentCameraShadow&)*pObject; }
    ComponentCameraShadow& operator=(const ComponentCameraShadow& other);

    virtual void RegisterCallbacks(); // TODO: change this component to use callbacks.
    virtual void UnregisterCallbacks(); // TODO: change this component to use callbacks.

    virtual void OnGameObjectEnabled();
    virtual void OnGameObjectDisabled();

    static void StaticOnTransformChanged(void* pObjectPtr, Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor) { ((ComponentCameraShadow*)pObjectPtr)->OnTransformChanged( newpos, newrot, newscale, changedbyuserineditor ); }
    void OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor);

    virtual void Tick(double TimePassed);
    virtual void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
    virtual void OnDrawFrame();

public:
#if MYFW_EDITOR
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentCameraShadow*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false, bool ignoreblockvisibleflag = false);
#endif //MYFW_USING_WX
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool directlychanged, bool finishedchanging, double oldvalue, bool valuewaschangedbydragging) { ((ComponentCameraShadow*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);
#endif //MYFW_EDITOR
};

#endif //__ComponentCameraShadow_H__
