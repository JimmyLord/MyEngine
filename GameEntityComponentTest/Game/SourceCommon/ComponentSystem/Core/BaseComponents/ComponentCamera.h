//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentCamera_H__
#define __ComponentCamera_H__

#include "../../../Camera/Camera2D.h"
#include "../../../Camera/Camera3D.h"

class ComponentTransform;

class ComponentCamera : public ComponentBase
{
public:
    ComponentTransform* m_pComponentTransform;

    // Needs saving
    bool m_Orthographic;

    bool m_ClearColorBuffer;
    bool m_ClearDepthBuffer;

    float m_DesiredWidth;
    float m_DesiredHeight;

    unsigned int m_LayersToRender;

    // Don't need saving
    unsigned int m_WindowStartX;
    unsigned int m_WindowStartY;
    unsigned int m_WindowWidth;
    unsigned int m_WindowHeight;

    Camera2D m_Camera2D;
    Camera3D m_Camera3D;

#if MYFW_USING_WX
    unsigned int m_FullClearsRequired;
#endif

public:
    ComponentCamera();
    virtual ~ComponentCamera();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentCamera&)*pObject; }
    virtual ComponentCamera& operator=(const ComponentCamera& other);

    void SetDesiredAspectRatio(float width, float height);
    void ComputeProjectionMatrices();

    void Tick(double TimePassed);
    void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
    void OnDrawFrame();

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentCamera*)pObjectPtr)->OnLeftClick( true ); }
    void OnLeftClick(bool clear);
    virtual void FillPropertiesWindow(bool clear);
    static void StaticOnValueChanged(void* pObjectPtr, int id, bool finishedchanging) { ((ComponentCamera*)pObjectPtr)->OnValueChanged( id, finishedchanging ); }
    void OnValueChanged(int id, bool finishedchanging);
#endif //MYFW_USING_WX
};

#endif //__ComponentCamera_H__
