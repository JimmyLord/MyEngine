//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentCameraShadow::m_PanelWatchBlockVisible = true;
#endif

ComponentCameraShadow::ComponentCameraShadow()
: ComponentCamera()
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Camera;

    m_pComponentTransform = 0;
    m_pDepthFBO = 0;

    m_pLight = 0;
}

ComponentCameraShadow::~ComponentCameraShadow()
{
    SAFE_RELEASE( m_pDepthFBO );

    m_pGameObject->GetTransform()->UnregisterTransformChangedCallbacks( this );

    if( m_pLight )
        g_pLightManager->DestroyLight( m_pLight );
}

#if MYFW_USING_WX
void ComponentCameraShadow::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentCameraShadow::StaticOnLeftClick, ComponentCamera::StaticOnRightClick, gameobjectid, "Camera", ObjectListIcon_Component );
}

void ComponentCameraShadow::OnLeftClick(unsigned int count, bool clear)
{
    ComponentCamera::OnLeftClick( count, clear );
}

void ComponentCameraShadow::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "CameraShadow", this, ComponentCamera::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentCamera::FillPropertiesWindow( clear );

        if( m_pLight )
        {
            g_pPanelWatch->AddColorFloat( "Color", &m_pLight->m_Color, 0, 1 );
            g_pPanelWatch->AddVector3( "Attenuation", &m_pLight->m_Attenuation, 0, 1 );
        }
    }
}

void ComponentCameraShadow::OnValueChanged(int controlid, bool finishedchanging)
{
}
#endif //MYFW_USING_WX

cJSON* ComponentCameraShadow::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentCamera::ExportAsJSONObject( savesceneid, saveid );

    //cJSON_AddNumberToObject( jComponent, "Ortho", m_Orthographic );

    //cJSON_AddNumberToObject( jComponent, "DWidth", m_DesiredWidth );
    //cJSON_AddNumberToObject( jComponent, "DHeight", m_DesiredHeight );

    //cJSON_AddNumberToObject( jComponent, "ColorBit", m_ClearColorBuffer );
    //cJSON_AddNumberToObject( jComponent, "DepthBit", m_ClearDepthBuffer );

    //cJSON_AddNumberToObject( jComponent, "Layers", m_LayersToRender );

    cJSONExt_AddFloatArrayToObject( jComponent, "Color", &m_pLight->m_Color.r, 4 );
    cJSONExt_AddFloatArrayToObject( jComponent, "Atten", &m_pLight->m_Attenuation.x, 3 );

    return jComponent;
}

void ComponentCameraShadow::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    ComponentCamera::ImportFromJSONObject( jsonobj, sceneid );

    //cJSONExt_GetBool( jsonobj, "Ortho", &m_Orthographic );

    //cJSONExt_GetFloat( jsonobj, "DWidth", &m_DesiredWidth );
    //cJSONExt_GetFloat( jsonobj, "DHeight", &m_DesiredHeight );

    //cJSONExt_GetBool( jsonobj, "ColorBit", &m_ClearColorBuffer );
    //cJSONExt_GetBool( jsonobj, "DepthBit", &m_ClearDepthBuffer );

    //cJSONExt_GetUnsignedInt( jsonobj, "Layers", &m_LayersToRender );

    cJSONExt_GetFloatArray( jsonobj, "Color", &m_pLight->m_Color.r, 4 );
    cJSONExt_GetFloatArray( jsonobj, "Atten", &m_pLight->m_Attenuation.x, 3 );

    MyAssert( m_pLight );

    m_pLight->m_LightType = LightType_Directional;
    m_pLight->m_Position = m_pGameObject->GetTransform()->GetWorldPosition();
    m_pLight->m_SpotDirectionVector = m_pGameObject->GetTransform()->GetWorldTransform()->GetAt();

    ComputeProjectionMatrices();
}

void ComponentCameraShadow::Reset()
{
    ComponentCamera::Reset();

    SAFE_RELEASE( m_pDepthFBO );

    int texres = 2048;
#if MYFW_OPENGLES2
    m_pDepthFBO = g_pTextureManager->CreateFBO( texres, texres, MyRE::MinFilter_Linear, MyRE::MagFilter_Linear, FBODefinition::FBOColorFormat_RGBA_UByte, 0, false );
#else
    m_pDepthFBO = g_pTextureManager->CreateFBO( texres, texres, MyRE::MinFilter_Linear, MyRE::MagFilter_Linear, FBODefinition::FBOColorFormat_RGBA_UByte, 32, true );
#endif

    if( m_pLight == 0 )
    {
        m_pLight = g_pLightManager->CreateLight();
        m_pGameObject->GetTransform()->RegisterTransformChangedCallback( this, StaticOnTransformChanged );
    }

    m_pLight->m_LightType = LightType_Point;
    m_pLight->m_Position.Set( 0, 0, 0 );
    m_pLight->m_SpotDirectionVector.Set( 0, 0, 0 );
    m_pLight->m_Color.Set( 1, 1, 1, 1 );
    m_pLight->m_Attenuation.Set( 0, 0, 0.09f );
}

ComponentCameraShadow& ComponentCameraShadow::operator=(const ComponentCameraShadow& other)
{
    MyAssert( &other != this );

    ComponentCamera::operator=( other );

    this->m_pLight->m_Color = other.m_pLight->m_Color;
    this->m_pLight->m_Attenuation = other.m_pLight->m_Attenuation;

    return *this;
}

void ComponentCameraShadow::RegisterCallbacks()
{
    // TODO: change this component to use callbacks.
    // for now register ComponentCamera's callbacks, mainly draw so the editor icon will appear.
    ComponentCamera::RegisterCallbacks();

//    if( m_Enabled && m_CallbacksRegistered == false )
//    {
//        m_CallbacksRegistered = true;
//
//        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCameraShadow, Tick );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCameraShadow, OnSurfaceChanged );
//#if MYFW_USING_WX
//        //MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentCameraShadow, Draw );
//        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCameraShadow, Draw );
//#endif //MYFW_USING_WX
//        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCameraShadow, OnTouch );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCameraShadow, OnButtons );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCameraShadow, OnKeys );
//        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCameraShadow, OnFileRenamed );
//    }
}

void ComponentCameraShadow::UnregisterCallbacks()
{
    // TODO: change this component to use callbacks.
    // for now register ComponentCamera's callbacks, mainly draw so the editor icon will appear.
    ComponentCamera::UnregisterCallbacks();

//    if( m_CallbacksRegistered == true )
//    {
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
//#if MYFW_USING_WX
//        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
//#endif //MYFW_USING_WX
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
//        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );
//
//        m_CallbacksRegistered = false;
//    }
}

void ComponentCameraShadow::OnGameObjectEnabled()
{
    ComponentBase::OnGameObjectEnabled();

    if( m_pLight )
        g_pLightManager->SetLightEnabled( m_pLight, true );
}

void ComponentCameraShadow::OnGameObjectDisabled()
{
    ComponentBase::OnGameObjectDisabled();

    if( m_pLight )
        g_pLightManager->SetLightEnabled( m_pLight, false );
}

void ComponentCameraShadow::OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor)
{
    MyAssert( m_pLight );

    m_pLight->m_Position = m_pGameObject->GetTransform()->GetWorldPosition();
    m_pLight->m_SpotDirectionVector = m_pGameObject->GetTransform()->GetWorldTransform()->GetAt();
}

void ComponentCameraShadow::Tick(float deltaTime)
{
    ComponentCamera::Tick( deltaTime );
}

void ComponentCameraShadow::OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    // TODO: this gets called if the window is resized, we might want to upres the fbo here.

    //ComponentCamera::OnSurfaceChanged( x, y, width, height, desiredaspectwidth, desiredaspectheight );
}

void ComponentCameraShadow::OnDrawFrame()
{
    checkGlError( "start of ComponentCameraShadow::OnDrawFrame()" );

    //glCullFace( GL_FRONT );

    g_ActiveShaderPass = ShaderPass_ShadowCastRGBA;

    m_pDepthFBO->Bind( false );
    checkGlError( "m_pDepthFBO->Bind()" );

    MyViewport viewport( 0, 0, m_pDepthFBO->GetWidth(), m_pDepthFBO->GetHeight() );
    g_pRenderer->EnableViewport( &viewport, true );

#if MYFW_OPENGLES2
    g_pRenderer->SetClearColor( ColorFloat( 1, 1, 1, 1 ) );
    g_pRenderer->ClearBuffers( true, false, false );
#else
    g_pRenderer->SetClearColor( ColorFloat( 1, 1, 1, 1 ) );
    g_pRenderer->SetClearDepth( 1 );
    g_pRenderer->ClearBuffers( true, true, false );
#endif

    if( m_Orthographic )
    {
        g_pComponentSystemManager->DrawFrame( this, &m_Camera2D.m_matProj, &m_Camera2D.m_matView, 0, true, false, EmissiveDrawOption_EitherEmissiveOrNot, false );
    }
    else
    {
        g_pComponentSystemManager->DrawFrame( this, &m_Camera3D.m_matProj, &m_Camera3D.m_matView, 0, true, false, EmissiveDrawOption_EitherEmissiveOrNot, false );
    }

    m_pDepthFBO->Unbind( false );
    g_ActiveShaderPass = ShaderPass_Main;
    //glCullFace( GL_BACK );

    checkGlError( "end of ComponentCameraShadow::OnDrawFrame()" );
}

MyMatrix* ComponentCameraShadow::GetViewProjMatrix()
{
    if( m_Orthographic )
    {
        return &m_Camera2D.m_matViewProj;
    }
    else
    {
        return &m_Camera3D.m_matViewProj;
    }
}
