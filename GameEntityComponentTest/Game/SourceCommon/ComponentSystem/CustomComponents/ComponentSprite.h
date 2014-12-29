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

#ifndef __ComponentSprite_H__
#define __ComponentSprite_H__

class ComponentTransform;

class ComponentSprite : public ComponentRenderable
{
public:
    MySprite* m_pSprite;
    ColorByte m_Tint;
    Vector2 m_Size;

public:
    ComponentSprite();
    virtual ~ComponentSprite();

    virtual cJSON* ExportAsJSONObject();
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentSprite&)*pObject; }
    virtual ComponentSprite& operator=(const ComponentSprite& other);

    virtual void SetShader(ShaderGroup* pShader);
    virtual void Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride = 0, int drawcount = 0);

public:
#if MYFW_USING_WX
    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);
    static void StaticFillPropertiesWindow(void* pObjectPtr) { ((ComponentSprite*)pObjectPtr)->FillPropertiesWindow(true); }
    void FillPropertiesWindow(bool clear);
    static void StaticOnDropShaderGroup(void* pObjectPtr) { ((ComponentSprite*)pObjectPtr)->OnDropShaderGroup(); }
    void OnDropShaderGroup();
#endif //MYFW_USING_WX
};

#endif //__ComponentSprite_H__
