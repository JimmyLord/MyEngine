//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentSprite_H__
#define __ComponentSprite_H__

class ComponentTransform;

class ComponentSprite : public ComponentRenderable
{
private:
    // Component Variable List
    MYFW_COMPONENT_DECLARE_VARIABLE_LIST( ComponentSprite ); //_VARIABLE_LIST

public:
    MySprite* m_pSprite;
    ColorByte m_Tint;
    Vector2 m_Size;

public:
    ComponentSprite();
    virtual ~ComponentSprite();
    SetClassnameBase( "SpriteComponent" ); // only first 8 character count.

    static void LuaRegister(lua_State* luastate);

    virtual cJSON* ExportAsJSONObject(bool savesceneid);
    virtual void ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid);

    virtual void Reset();
    virtual void CopyFromSameType_Dangerous(ComponentBase* pObject) { *this = (ComponentSprite&)*pObject; }
    ComponentSprite& operator=(const ComponentSprite& other);

    virtual void RegisterCallbacks();
    virtual void UnregisterCallbacks();

    virtual MaterialDefinition* GetMaterial(int submeshindex) { if( m_pSprite ) return m_pSprite->GetMaterial(); return 0; }
    virtual void SetMaterial(MaterialDefinition* pMaterial, int submeshindex);
    virtual void Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride = 0, int drawcount = 0);

public:
    MySprite* GetSprite() { return m_pSprite; }

    // Runtime component variable callbacks. //_VARIABLE_LIST
    static void* StaticGetPointerValue(void* pObjectPtr, ComponentVariable* pVar) { return ((ComponentSprite*)pObjectPtr)->GetPointerValue(pVar); }
    void* GetPointerValue(ComponentVariable* pVar);

    static const char* StaticGetPointerDesc(void* pObjectPtr, ComponentVariable* pVar) { return ((ComponentSprite*)pObjectPtr)->GetPointerDesc( pVar ); }
    const char* GetPointerDesc(ComponentVariable* pVar);

    static void StaticSetPointerDesc(void* pObjectPtr, ComponentVariable* pVar, const char* newdesc) { return ((ComponentSprite*)pObjectPtr)->SetPointerDesc( pVar, newdesc ); }
    void SetPointerDesc(ComponentVariable* pVar, const char* newdesc);

public:
#if MYFW_USING_WX
    static bool m_PanelWatchBlockVisible;

    virtual void AddToObjectsPanel(wxTreeItemId gameobjectid);

    // Object panel callbacks.
    static void StaticOnLeftClick(void* pObjectPtr, wxTreeItemId id, unsigned int count) { ((ComponentSprite*)pObjectPtr)->OnLeftClick( count, true ); }
    void OnLeftClick(unsigned int count, bool clear);
    virtual void FillPropertiesWindow(bool clear, bool addcomponentvariables = false);

    // Component variable callbacks. //_VARIABLE_LIST
    static void* StaticOnDrop(void* pObjectPtr, ComponentVariable* pVar, wxCoord x, wxCoord y) { return ((ComponentSprite*)pObjectPtr)->OnDrop(pVar, x, y); }
    void* OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y);

    static void* StaticOnValueChanged(void* pObjectPtr, ComponentVariable* pVar, bool finishedchanging, double oldvalue) { return ((ComponentSprite*)pObjectPtr)->OnValueChanged( pVar, finishedchanging ); }
    void* OnValueChanged(ComponentVariable* pVar, bool finishedchanging);
#endif //MYFW_USING_WX
};

#endif //__ComponentSprite_H__
