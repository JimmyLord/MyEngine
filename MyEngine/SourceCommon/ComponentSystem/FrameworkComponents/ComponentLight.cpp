//
// Copyright (c) 2015-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentLight.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#endif

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentLight );

ComponentLight::ComponentLight(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentData( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR();

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;
    
    m_LightType = LightType_Point;

    m_pLight = nullptr;

#if MYFW_EDITOR
    m_LightSphereRenderTimeRemaining = 0;
#endif
}

ComponentLight::~ComponentLight()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR();

    m_pGameObject->GetTransform()->UnregisterTransformChangedCallbacks( this );

    if( m_pLight )
    {
        LightManager* pLightManager = m_pEngineCore->GetManagers()->GetLightManager();
        pLightManager->DestroyLight( m_pLight );
    }
}

void ComponentLight::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentLight* pThis) //_VARIABLE_LIST
{
    // TODO: Lights don't do inheritance ATM since all vars are indirectly stored in a light object outside the class (m_pLight).

    // Just want to make sure these are the same on all compilers.  They should be since this is a simple class.
    //MyAssert( offsetof( ComponentLight, m_pScriptFile ) == MyOffsetOf( pThis, &pThis->m_pScriptFile ) );

    //AddVariable( pList, "Script", ComponentVariableType::FilePtr, MyOffsetOf( pThis, &pThis->m_pScriptFile ), true, true, nullptr, ComponentLuaScript::StaticOnValueChangedCV, ComponentLuaScript::StaticOnDropCV, nullptr );

    //cJSONExt_AddFloatArrayToObject( jComponent, "Color", &m_pLight->m_Color.r, 4 );
    //cJSONExt_AddFloatArrayToObject( jComponent, "Atten", &m_pLight->m_Attenuation.x, 3 );

    AddVarEnum( pList, "LightType", MyOffsetOf( pThis, &pThis->m_LightType ), true, true, "Type", 3, g_LightTypeStrings,
        (CVarFunc_ValueChanged)&ComponentLight::OnValueChanged, nullptr, nullptr );
}

void ComponentLight::Reset()
{
    ComponentData::Reset();

    if( m_pLight == nullptr )
    {
        LightManager* pLightManager = m_pEngineCore->GetManagers()->GetLightManager();
        m_pLight = pLightManager->CreateLight();
        m_pGameObject->GetTransform()->RegisterTransformChangedCallback( this, StaticOnTransformChanged );
    }

    m_pLight->m_LightType = LightType_Point;
    m_pLight->m_Position.Set( 0, 0, 0 );
    m_pLight->m_SpotDirectionVector.Set( 0, 0, 0 );
    m_pLight->m_Color.Set( 1, 1, 1, 1 );
    m_pLight->m_Attenuation.Set( 0, 0, 0.09f );
}

#if MYFW_EDITOR
#if MYFW_USING_IMGUI
void ComponentLight::AddAllVariablesToWatchPanel(CommandStack* pCommandStack)
{
    ComponentBase::AddAllVariablesToWatchPanel( pCommandStack );

    if( m_pLight )
    {
        if( ImGui::ColorEdit4( "Color", &m_pLight->m_Color.r ) )
        {
            m_LightSphereRenderTimeRemaining = 3.0f;
        }

        // Replacing classic attentuation with a range based version to make deferred lighting "spheres" possible.
        //ImGui::DragFloat3( "Attenuation", &m_pLight->m_Attenuation.x, 0.01f, 0, 20 );
        if( ImGui::DragFloat( "Range", &m_pLight->m_Attenuation.x, 0.1f, 0, 30 ) )
        {
            m_LightSphereRenderTimeRemaining = 3.0f;
        }
    }
}
#endif

void* ComponentLight::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = nullptr;

    //DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    //if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
    //{
    //    oldPointer = old component;
    //}

    return oldPointer;
}

void* ComponentLight::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = nullptr;

    if( pVar->m_Offset == MyOffsetOf( this, &m_LightType ) )
    {
        MyAssert( m_pLight != nullptr );

        m_pLight->m_LightType = m_LightType;
    }

    m_LightSphereRenderTimeRemaining = 3.0f;

    return oldPointer;
}
#endif //MYFW_EDITOR

cJSON* ComponentLight::ExportAsJSONObject(bool saveSceneID, bool saveID)
{
    cJSON* jComponent = ComponentData::ExportAsJSONObject( saveSceneID, saveID );

    cJSONExt_AddFloatArrayToObject( jComponent, "Color", &m_pLight->m_Color.r, 4 );
    cJSONExt_AddFloatArrayToObject( jComponent, "Atten", &m_pLight->m_Attenuation.x, 3 );

    return jComponent;
}

void ComponentLight::ImportFromJSONObject(cJSON* jComponent, SceneID sceneID)
{
    ComponentData::ImportFromJSONObject( jComponent, sceneID );

    cJSONExt_GetFloatArray( jComponent, "Color", &m_pLight->m_Color.r, 4 );
    cJSONExt_GetFloatArray( jComponent, "Atten", &m_pLight->m_Attenuation.x, 3 );

    MyAssert( m_pLight );

    m_pLight->m_LightType = m_LightType;
    m_pLight->m_Position = m_pGameObject->GetTransform()->GetWorldPosition();
    m_pLight->m_SpotDirectionVector = m_pGameObject->GetTransform()->GetWorldTransform()->GetAt();
}

void ComponentLight::OnTransformChanged(const Vector3& newPos, const Vector3& newRot, const Vector3& newScale, bool changedByUserInEditor)
{
    MyAssert( m_pLight );

    m_pLight->m_Position = m_pGameObject->GetTransform()->GetWorldPosition();
    m_pLight->m_SpotDirectionVector = m_pGameObject->GetTransform()->GetWorldTransform()->GetAt();
}

ComponentLight& ComponentLight::operator=(const ComponentLight& other)
{
    MyAssert( &other != this );

    ComponentData::operator=( other );

    this->m_LightType = other.m_LightType;

    this->m_pLight->m_Color = other.m_pLight->m_Color;
    this->m_pLight->m_Attenuation = other.m_pLight->m_Attenuation;

    return *this;
}

void ComponentLight::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnSurfaceChanged );
#if MYFW_EDITOR
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, Tick );
        //MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentLight, Draw );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, Draw );
#endif //MYFW_EDITOR
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLight, OnFileRenamed );
    }
}

void ComponentLight::UnregisterCallbacks()
{
    MyAssert( m_EnabledState != EnabledState_Enabled );

    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
#if MYFW_EDITOR
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
#endif //MYFW_EDITOR
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

    //if( m_pLight && m_EnabledState == EnabledState_Enabled )
    //{
    //    LightManager* pLightManager = m_pEngineCore->GetManagers()->GetLightManager();
    //    pLightManager->SetLightEnabled( m_pLight, true );
    //}
}

void ComponentLight::OnGameObjectDisabled()
{
    ComponentBase::OnGameObjectDisabled();

    //if( m_pLight && m_EnabledState != EnabledState_Enabled )
    //{
    //    LightManager* pLightManager = m_pEngineCore->GetManagers()->GetLightManager();
    //    pLightManager->SetLightEnabled( m_pLight, false );
    //}
}

bool ComponentLight::SetEnabled(bool enableComponent)
{
    if( ComponentBase::SetEnabled( enableComponent ) == false )
        return false;

    if( m_EnabledState == EnabledState_Enabled )
    {
        LightManager* pLightManager = m_pEngineCore->GetManagers()->GetLightManager();
        pLightManager->SetLightEnabled( m_pLight, true );
    }
    else
    {
        LightManager* pLightManager = m_pEngineCore->GetManagers()->GetLightManager();
        pLightManager->SetLightEnabled( m_pLight, false );
    }

    return true;
}

bool ComponentLight::IsVisible()
{
    return true;
}

bool ComponentLight::ExistsOnLayer(unsigned int layerFlags)
{
    if( layerFlags & Layer_Editor )
        return true;
    
    return false;
}

#if MYFW_EDITOR
void ComponentLight::TickCallback(float deltaTime)
{
    m_LightSphereRenderTimeRemaining -= g_pEngineCore->GetTimePassedUnpausedLastFrame();
    
    if( m_LightSphereRenderTimeRemaining < 0 )
    {
        m_LightSphereRenderTimeRemaining = 0;
    }
}

void ComponentLight::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    if( g_pEngineCore->GetEditorPrefs()->Get_View_ShowEditorIcons() == false )
        return;

    MySprite* pSprite = g_pEngineCore->GetEditorState()->m_pEditorIcons[(int)EditorIcon::Light];
    if( pSprite == nullptr )
        return;

    Vector2 size( 1, 1 );

    // Scale and make the lightbulb sprite face the camera.
    MyMatrix* pCameraTransform = pCamera->m_pComponentTransform->GetWorldTransform();

    MyMatrix scale;
    MyMatrix rotAndPos;
    scale.CreateScale( size );
    rotAndPos.CreateLookAtWorld( m_pLight->m_Position, pCameraTransform->GetUp(), pCamera->m_pComponentTransform->GetLocalPosition() );

    MyMatrix transform = rotAndPos * scale;
    //pSprite->SetPosition( &transform );

    // Set the sprite color.
    pSprite->GetMaterial()->m_ColorDiffuse = m_pLight->m_Color.AsColorByte();
    pSprite->GetMaterial()->m_ColorDiffuse.a = 255;
    
    g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Fill );

    pSprite->Draw( pMatProj, pMatView, &transform, pShaderOverride, true );

    // Draw a sphere to visualize light range.
    if( m_LightType == LightType_Point && m_LightSphereRenderTimeRemaining > 0 )
    {
        MyMesh* pMeshBall = g_pEngineCore->GetMesh_MaterialBall();
        if( pMeshBall && pShaderOverride == nullptr )
        {
            MaterialDefinition* pMaterial = g_pEngineCore->GetMaterial_FresnelTint();
            ColorByte lightColor = m_pLight->m_Color.AsColorByte();
            lightColor.a = (unsigned char)MyClamp_Return( m_LightSphereRenderTimeRemaining * 255.0f, 0.0f, 255.0f );
            pMaterial->SetColorDiffuse( lightColor );

            //g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Line );
            //g_pRenderer->SetCullingEnabled( false );
            g_pRenderer->SetDepthWriteEnabled( false );
            pMeshBall->SetMaterial( pMaterial, 0 );

            MyMatrix matWorld;
            matWorld.CreateSRT( m_pLight->m_Attenuation.x, Vector3(0), this->m_pGameObject->GetTransform()->GetWorldPosition() );
            Vector3 camPos = pCamera->m_pComponentTransform->GetWorldPosition();
            Vector3 camRot = pCamera->m_pComponentTransform->GetWorldTransform()->GetAt();
            pMeshBall->Draw( pMatProj, pMatView, &matWorld, &camPos, &camRot, nullptr, 0, nullptr, nullptr, nullptr, nullptr );

            pMeshBall->SetMaterial( nullptr, 0 );
            g_pRenderer->SetDepthWriteEnabled( true );
            //g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Fill );
            //g_pRenderer->SetCullingEnabled( true );
        }
    }

    if( g_pEngineCore->GetDebug_DrawWireframe() )
    {
        g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Line );
    }
}
#endif //MYFW_EDITOR
