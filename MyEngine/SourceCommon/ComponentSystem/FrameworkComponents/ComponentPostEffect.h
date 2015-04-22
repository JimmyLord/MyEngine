//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentPostEffect_H__
#define __ComponentPostEffect_H__

#include "../../Camera/Camera2D.h"
#include "../../Camera/Camera3D.h"

class ComponentTransform;

class ComponentPostEffect : public ComponentData
{
public:
    MySprite* m_pFullScreenQuad;
    ShaderGroup* m_pShaderGroup;

public:
    ComponentPostEffect();
    virtual ~ComponentPostEffect();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentPostEffect&)*pObject; }
    virtual ComponentPostEffect& operator=(const ComponentPostEffect& other);

    void SetShader(ShaderGroup* pShader);
    void Render(FBODefinition* pFBO);

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticOnLeftClick(void* pObjectPtr) { ((ComponentPostEffect*)pObjectPtr)->OnLeftClick( true ); }
    void OnLeftClick(bool clear);
    virtual void FillPropertiesWindow(bool clear);
    static void StaticOnDropShader(void* pObjectPtr) { ((ComponentPostEffect*)pObjectPtr)->OnDropShader(); }
    void OnDropShader();
    static void StaticOnValueChanged(void* pObjectPtr, int controlid, bool finishedchanging) { ((ComponentPostEffect*)pObjectPtr)->OnValueChanged( controlid, finishedchanging ); }
    void OnValueChanged(int controlid, bool finishedchanging);
#endif //MYFW_USING_WX
};

#endif //__ComponentPostEffect_H__
