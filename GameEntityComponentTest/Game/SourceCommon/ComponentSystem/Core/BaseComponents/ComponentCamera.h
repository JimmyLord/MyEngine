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

#ifndef __ComponentCamera_H__
#define __ComponentCamera_H__

#include "../../../Camera/Camera2D.h"
#include "../../../Camera/Camera3D.h"

class ComponentTransform;

class ComponentCamera : public ComponentBase
{
public:
    ComponentTransform* m_pComponentTransform;

    bool m_Orthographic;

    float m_DesiredWidth;
    float m_DesiredHeight;

    Camera2D m_Camera2D;
    Camera3D m_Camera3D;

    unsigned int m_LayersToRender;

public:
    ComponentCamera();
    ComponentCamera(GameObject* owner);
    virtual ~ComponentCamera();

    virtual void Reset();

    void SetDesiredAspectRatio(float width, float height);

    void Tick(double TimePassed);
    void OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight);
    void OnDrawFrame();

public:
#if MYFW_USING_WX
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticFillPropertiesWindow(void* pObjectPtr) { ((ComponentCamera*)pObjectPtr)->FillPropertiesWindow(true); }
    void FillPropertiesWindow(bool clear);
#endif //MYFW_USING_WX
};

#endif //__ComponentCamera_H__
