//
// Copyright (c) 2014-2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentSprite::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentSprite ); //_VARIABLE_LIST

ComponentSprite::ComponentSprite()
: ComponentRenderable()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_pSprite = 0;
}

ComponentSprite::~ComponentSprite()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    SAFE_RELEASE( m_pSprite );
}

void ComponentSprite::RegisterVariables(CPPListHead* pList, ComponentSprite* pThis) //_VARIABLE_LIST
{
    ComponentRenderable::RegisterVariables( pList, pThis );

    AddVar( pList, "Tint",     ComponentVariableType_ColorByte, MyOffsetOf( pThis, &pThis->m_Tint ),  true,  true, 0, (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged, (CVarFunc_DropTarget)&ComponentSprite::OnDrop, 0 );
    AddVar( pList, "Size",     ComponentVariableType_Vector2,   MyOffsetOf( pThis, &pThis->m_Size ),  true,  true, 0, (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged, (CVarFunc_DropTarget)&ComponentSprite::OnDrop, 0 );
    AddVarPointer( pList, "Material",                                                                 true,  true, 0,
        (CVarFunc_GetPointerValue)&ComponentSprite::GetPointerValue, (CVarFunc_SetPointerValue)&ComponentSprite::SetPointerValue, (CVarFunc_GetPointerDesc)&ComponentSprite::GetPointerDesc, (CVarFunc_SetPointerDesc)&ComponentSprite::SetPointerDesc,
        (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged, (CVarFunc_DropTarget)&ComponentSprite::OnDrop, 0 );
}

ComponentSprite* CastAs_ComponentSprite(ComponentBase* pComponent)
{
    MyAssert( pComponent->IsA( "SpriteComponent" ) );
    return (ComponentSprite*)pComponent;
}

void ComponentSprite::Reset()
{
    ComponentRenderable::Reset();

    if( m_pSprite == 0 )
        m_pSprite = MyNew MySprite( true );

    m_Size.Set( 1.0f, 1.0f );
    m_Tint.Set( 255,255,255,255 );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentSprite::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate ).addFunction( "CastAs_ComponentSprite", CastAs_ComponentSprite );

    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSprite>( "ComponentSprite" )
            //.addData( "localmatrix", &ComponentSprite::m_LocalTransform )
            
            .addFunction( "GetSprite", &ComponentSprite::GetSprite )
        .endClass();
}
#endif //MYFW_USING_LUA

void* ComponentSprite::GetPointerValue(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        if( m_pSprite )
        {
            if( m_pSprite->GetMaterial() )
            {
                if( m_pSprite->GetMaterial()->m_pFile )
                    return m_pSprite->GetMaterial();
            }
        }
    }

    return 0;
}

void ComponentSprite::SetPointerValue(ComponentVariable* pVar, void* newvalue)
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        if( m_pSprite )
            return m_pSprite->SetMaterial( (MaterialDefinition*)newvalue );
    }
}

const char* ComponentSprite::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        MyAssert( m_pSprite );
        MaterialDefinition* pMaterial = m_pSprite->GetMaterial();
        if( pMaterial && pMaterial->m_pFile )
            return pMaterial->m_pFile->m_FullPath;
        else
            return "none";
    }

    return "fix me";
}

void ComponentSprite::SetPointerDesc(ComponentVariable* pVar, const char* newdesc) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        MyAssert( newdesc );
        if( newdesc )
        {
            MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( newdesc );
            if( pMaterial )
                m_pSprite->SetMaterial( pMaterial );
            pMaterial->Release();
        }
    }
}

#if MYFW_USING_WX
void ComponentSprite::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentSprite::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Sprite", ObjectListIcon_Component );
}

void ComponentSprite::OnLeftClick(unsigned int count, bool clear)
{
    ComponentRenderable::OnLeftClick( count, clear );
}

void ComponentSprite::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Sprite", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        if( addcomponentvariables )
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentSprite::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;
        MyAssert( pMaterial );
        MyAssert( m_pSprite );

        oldvalue = m_pSprite->GetMaterial();
        m_pSprite->SetMaterial( pMaterial );

        g_pPanelWatch->m_NeedsRefresh = true;
    }

    return oldvalue;
}

void* ComponentSprite::OnValueChanged(ComponentVariable* pVar, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        MyAssert( pVar->m_ControlID != -1 );

        wxString text = g_pPanelWatch->m_pVariables[pVar->m_ControlID].m_Handle_TextCtrl->GetValue();
        if( text == "" || text == "none" )
        {
            g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

            oldpointer = m_pSprite->GetMaterial();
            m_pSprite->SetMaterial( 0 );
        }
    }

    return oldpointer;
}
#endif //MYFW_USING_WX

cJSON* ComponentSprite::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jComponent = ComponentRenderable::ExportAsJSONObject( savesceneid );

    //ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST

    return jComponent;
}

void ComponentSprite::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jsonobj, sceneid );

    //ImportVariablesFromJSON( jsonobj ); //_VARIABLE_LIST
}

ComponentSprite& ComponentSprite::operator=(const ComponentSprite& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    this->m_Tint = other.m_Tint;
    this->m_Size = other.m_Size;
    this->m_pSprite->SetMaterial( other.m_pSprite->GetMaterial() );

    return *this;
}

void ComponentSprite::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnSurfaceChanged );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnFileRenamed );
    }
}

void ComponentSprite::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentSprite::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    ComponentRenderable::SetMaterial( pMaterial, 0 );

    m_pSprite->SetMaterial( pMaterial );
}

void ComponentSprite::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    ComponentRenderable::Draw( pMatViewProj, pShaderOverride, 0 );

    m_pSprite->SetPosition( &m_pComponentTransform->m_Transform );
    m_pSprite->SetTint( m_Tint );
    m_pSprite->Create( "ComponentSprite", m_Size.x, m_Size.y, 0, 1, 0, 1, Justify_Center, false );
    m_pSprite->Draw( pMatViewProj, pShaderOverride );
}
