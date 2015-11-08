//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentLight::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentLight );

ComponentLight::ComponentLight()
: ComponentData()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR();

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pLight = 0;
}

ComponentLight::~ComponentLight()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR();

    if( m_pLight )
        g_pLightManager->DestroyLight( m_pLight );
}

void ComponentLight::RegisterVariables(CPPListHead* pList, ComponentLight* pThis) //_VARIABLE_LIST
{
    // TODO: lights don't do inheritance ATM since all vars are indirectly stored in a light object outside the class (m_pLight).

    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
    //MyAssert( offsetof( ComponentLight, m_pScriptFile ) == MyOffsetOf( pThis, &pThis->m_pScriptFile ) );

    //AddVariable( pList, "Script", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pScriptFile ), true, true, 0, ComponentLuaScript::StaticOnValueChangedCV, ComponentLuaScript::StaticOnDropCV, 0 );

    //cJSONExt_AddFloatArrayToObject( jComponent, "Color", &m_pLight->m_Color.r, 4 );
    //cJSONExt_AddFloatArrayToObject( jComponent, "Atten", &m_pLight->m_Attenuation.x, 3 );
}

void ComponentLight::Reset()
{
    ComponentData::Reset();

    if( m_pLight == 0 )
    {
        m_pLight = g_pLightManager->CreateLight();
        m_pGameObject->m_pComponentTransform->RegisterPositionChangedCallback( this, StaticOnTransformPositionChanged );
    }

    m_pLight->m_Position = m_pGameObject->m_pComponentTransform->GetPosition();
    m_pLight->m_Color.Set( 1, 1, 1, 1 );
    m_pLight->m_Attenuation.Set( 0, 0, 0.09f );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_WX
void ComponentLight::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentLight::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Light" );
}

void ComponentLight::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentLight::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Light", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentData::FillPropertiesWindow( clear );

        if( addcomponentvariables )
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        if( m_pLight )
        {
            g_pPanelWatch->AddColorFloat( "color", &m_pLight->m_Color, 0, 1 );
            g_pPanelWatch->AddVector3( "atten", &m_pLight->m_Attenuation, 0, 1 );
        }
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentLight::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jComponent = ComponentData::ExportAsJSONObject( savesceneid );

    //ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST

    cJSONExt_AddFloatArrayToObject( jComponent, "Color", &m_pLight->m_Color.r, 4 );
    cJSONExt_AddFloatArrayToObject( jComponent, "Atten", &m_pLight->m_Attenuation.x, 3 );

    return jComponent;
}

void ComponentLight::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentData::ImportFromJSONObject( jsonobj, sceneid );

    //ImportVariablesFromJSON( jsonobj ); //_VARIABLE_LIST

    cJSONExt_GetFloatArray( jsonobj, "Color", &m_pLight->m_Color.r, 4 );
    cJSONExt_GetFloatArray( jsonobj, "Atten", &m_pLight->m_Attenuation.x, 3 );
}

void ComponentLight::OnTransformPositionChanged(Vector3& newpos, bool changedbyeditor)
{
    MyAssert( m_pLight );

    m_pLight->m_Position = newpos;
}

ComponentLight& ComponentLight::operator=(const ComponentLight& other)
{
    MyAssert( &other != this );

    ComponentData::operator=( other );

    this->m_pLight->m_Color = other.m_pLight->m_Color;
    this->m_pLight->m_Attenuation = other.m_pLight->m_Attenuation;

    return *this;
}

void ComponentLight::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnSurfaceChanged );
#if MYFW_USING_WX
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, Draw );
#endif //MYFW_USING_WX
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnFileRenamed );
    }
}

void ComponentLight::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
#if MYFW_USING_WX
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
#endif //MYFW_USING_WX
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentLight::OnGameObjectEnabled()
{
    ComponentBase::OnGameObjectEnabled();

    if( m_pLight )
        g_pLightManager->SetLightEnabled( m_pLight, true );
}

void ComponentLight::OnGameObjectDisabled()
{
    ComponentBase::OnGameObjectDisabled();

    if( m_pLight )
        g_pLightManager->SetLightEnabled( m_pLight, false );
}


bool ComponentLight::IsVisible()
{
    return true;
}

bool ComponentLight::ExistsOnLayer(unsigned int layerflags)
{
    if( layerflags & Layer_EditorFG )
        return true;
    
    return false;
}

#if MYFW_USING_WX
void ComponentLight::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    MySprite* pSprite = g_pEngineCore->m_pEditorState->m_pEditorIcons[EditorIcon_Light];
    if( pSprite == 0 )
        return;

    Vector2 size( 1, 1 );

    // Scale and make the lightbulb sprite face the camera.
    MyMatrix* pCameraTransform = pCamera->m_pComponentTransform->GetLocalTransform();

    MyMatrix scale;
    MyMatrix rotpos;
    scale.CreateScale( size );
    rotpos.CreateLookAtWorld( m_pLight->m_Position, pCameraTransform->GetUp(), pCamera->m_pComponentTransform->GetPosition() );

    MyMatrix transform = rotpos * scale;
    pSprite->SetPosition( &transform );

    // Set the sprite color
    pSprite->GetMaterial()->m_ColorDiffuse = m_pLight->m_Color.AsColorByte();
    
    pSprite->Draw( pMatViewProj, pShaderOverride );
}
#endif
