//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

#include "ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/EngineFileManager.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentCameraShadow.h"
#include "ComponentSystem/FrameworkComponents/ComponentPostEffect.h"
#include "Core/EngineCore.h"

#if MYFW_EDITOR
#include "../SourceEditor/EditorState.h"
#include "../SourceEditor/Prefs/EditorPrefs.h"
#include "../SourceEditor/Prefs/EditorKeyBindings.h"
#include "../SourceEditor/TransformGizmo.h"
#endif

#if MYFW_USING_WX
bool ComponentCamera::m_PanelWatchBlockVisible = true;
#endif

// 32 layers strings since ComponentBase::ExportVariablesToJSON -> case ComponentVariableType::Flags is looking for 32
const char* g_pVisibilityLayerStrings[32] = //g_NumberOfVisibilityLayers] =
{
    "Main view",
    "HUD",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15",
    "16",
    "17",
    "18",
    "19",
    "20",
    "21",
    "22",
    "23",
    "24",
    "25",
    "26",
    "27",
    "28",
    "29",
    "30",
    "31",
    "32",
};

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentCamera ); //_VARIABLE_LIST

ComponentCamera::ComponentCamera(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentBase( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Camera;

    m_pComponentTransform = 0;
    m_DeferredGBufferVisible = false;
}

ComponentCamera::~ComponentCamera()
{
    SAFE_RELEASE( m_pPostEffectFBOs[0] );
    SAFE_RELEASE( m_pPostEffectFBOs[1] );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    if( m_pComponentSystemManager )
    {
        FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();

        if( m_pDeferredShaderFile_AmbientDirectional )
            pFileManager->FreeFile( m_pDeferredShaderFile_AmbientDirectional );
        if( m_pDeferredShaderFile_PointLight )
            pFileManager->FreeFile( m_pDeferredShaderFile_PointLight );
    }
    else
    {
        MyAssert( m_pDeferredShaderFile_AmbientDirectional == nullptr );
        MyAssert( m_pDeferredShaderFile_PointLight == nullptr );
    }

    SAFE_RELEASE( m_pDeferredShader_AmbientDirectional );
    SAFE_RELEASE( m_pDeferredShader_PointLight );
    SAFE_RELEASE( m_pDeferredMaterial_AmbientDirectional );
    SAFE_RELEASE( m_pDeferredMaterial_PointLight );

    SAFE_RELEASE( m_pDeferredQuadMesh );
    SAFE_RELEASE( m_pDeferredSphereMeshFile );
    SAFE_RELEASE( m_pDeferredSphereMesh );
}

void ComponentCamera::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentCamera* pThis) //_VARIABLE_LIST
{
    ComponentVariable* pVar;

    AddVarFlags( pList, "Layers", MyOffsetOf( pThis, &pThis->m_LayersToRender ), true, true, 0,
                 g_NumberOfVisibilityLayers, g_pVisibilityLayerStrings,
                 (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );

    pVar = AddVar( pList, "Deferred", ComponentVariableType::Bool, MyOffsetOf( pThis, &pThis->m_Deferred ), true, true, 0, (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );

    pVar = AddVar( pList, "Ortho", ComponentVariableType::Bool, MyOffsetOf( pThis, &pThis->m_Orthographic ), true, true, 0, (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );

    pVar = AddVar( pList, "DesiredWidth", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_DesiredWidth ), true, true, 0, (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
#if MYFW_USING_WX
    pVar->SetEditorLimits( 640, 960 );
    pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentCamera::ShouldVariableBeAddedToWatchPanel) );
#endif

    pVar = AddVar( pList, "DesiredHeight", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_DesiredHeight ), true, true, 0, (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
#if MYFW_USING_WX
    pVar->SetEditorLimits( 640, 960 );
    pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentCamera::ShouldVariableBeAddedToWatchPanel) );
#endif

    pVar = AddVar( pList, "OrthoNearZ", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_OrthoNearZ ), true, true, "Near Z", (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
#if MYFW_USING_WX
    pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentCamera::ShouldVariableBeAddedToWatchPanel) );
#endif

    pVar = AddVar( pList, "OrthoFarZ", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_OrthoFarZ ), true, true, "Far Z", (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
#if MYFW_USING_WX
    pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentCamera::ShouldVariableBeAddedToWatchPanel) );
#endif

    pVar = AddVar( pList, "FoV", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_FieldOfView ), true, true, 0, (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
#if MYFW_USING_WX
    pVar->SetEditorLimits( 1, 179 );
    pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentCamera::ShouldVariableBeAddedToWatchPanel) );
#endif
    
    pVar = AddVar( pList, "PerspectiveNearZ", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_PerspectiveNearZ ), true, true, "Near Z", (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
#if MYFW_USING_WX
    pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentCamera::ShouldVariableBeAddedToWatchPanel) );
#endif

    pVar = AddVar( pList, "PerspectiveFarZ", ComponentVariableType::Float, MyOffsetOf( pThis, &pThis->m_PerspectiveFarZ ), true, true, "Far Z", (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
#if MYFW_USING_WX
    pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentCamera::ShouldVariableBeAddedToWatchPanel) );
#endif

    pVar = AddVar( pList, "ColorBit", ComponentVariableType::Bool, MyOffsetOf( pThis, &pThis->m_ClearColorBuffer ), true, true, 0, (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
    pVar = AddVar( pList, "DepthBit", ComponentVariableType::Bool, MyOffsetOf( pThis, &pThis->m_ClearDepthBuffer ), true, true, 0, (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
}

#if MYFW_USING_WX
void ComponentCamera::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentCamera::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Camera", ObjectListIcon_Component );
}

void ComponentCamera::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentCamera::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Camera", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}
#endif //MYFW_USING_WX

#if MYFW_EDITOR
bool ComponentCamera::ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar)
{
    if( m_Orthographic == true )
    {
        if( strcmp( pVar->m_Label, "DesiredWidth" ) == 0 )       return true;
        if( strcmp( pVar->m_Label, "DesiredHeight" ) == 0 )      return true;
        if( strcmp( pVar->m_Label, "OrthoNearZ" ) == 0 )         return true;
        if( strcmp( pVar->m_Label, "OrthoFarZ" ) == 0 )          return true;
    }
    else //if( m_Orthographic == false )
    {
        if( strcmp( pVar->m_Label, "FoV" ) == 0 )                return true;
        if( strcmp( pVar->m_Label, "PerspectiveNearZ" ) == 0 )   return true;
        if( strcmp( pVar->m_Label, "PerspectiveFarZ" ) == 0 )    return true;
    }

    return false;
}

void* ComponentCamera::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    ComputeProjectionMatrices();

    if( finishedChanging )
    {
        m_FullClearsRequired = 1;

#if MYFW_USING_WX
        m_FullClearsRequired = 2;
        g_pPanelWatch->SetNeedsRefresh();
#endif //MYFW_USING_WX
    }

    return oldpointer;
}
#endif //MYFW_EDITOR

cJSON* ComponentCamera::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid, saveid );

    return jComponent;
}

void ComponentCamera::ImportFromJSONObject(cJSON* jComponent, SceneID sceneid)
{
    // if we're loading a camera, start the layers flags at 0, so we set the flags when loading.
    m_LayersToRender = 0;

    ComponentBase::ImportFromJSONObject( jComponent, sceneid );
}

void ComponentCamera::Reset()
{
    ComponentBase::Reset();

    m_pComponentTransform = nullptr;
    if( m_pGameObject )
    {
        m_pComponentTransform = m_pGameObject->GetTransform();
    }

    m_Orthographic = false;

    m_ClearColorBuffer = true;
    m_ClearDepthBuffer = true;

    // Deferred lighting variables.
    m_Deferred = false;
    m_pGBuffer = 0;

    m_pDeferredShaderFile_AmbientDirectional = 0;
    m_pDeferredShaderFile_PointLight = 0;
    m_pDeferredShader_AmbientDirectional = 0;
    m_pDeferredShader_PointLight = 0;
    m_pDeferredMaterial_AmbientDirectional = 0;
    m_pDeferredMaterial_PointLight = 0;

    m_pDeferredQuadMesh = 0;
    m_pDeferredSphereMeshFile = 0;
    m_pDeferredSphereMesh = 0;

    // Ortho matrix variables.
    m_DesiredWidth = 640;
    m_DesiredHeight = 960;
    m_OrthoNearZ = 0;
    m_OrthoFarZ = 1000;

    // Perspective matrix variables.
    m_FieldOfView = 45;
    m_PerspectiveNearZ = 1;
    m_PerspectiveFarZ = 10000;
    
    m_LayersToRender = 0x00FF;

    m_Viewport.Set( 0, 0, 0, 0 );

    m_pPostEffectFBOs[0] = 0;
    m_pPostEffectFBOs[1] = 0;

#if MYFW_EDITOR
    m_FullClearsRequired = 1;
#endif //MYFW_EDITOR

#if MYFW_USING_WX
    m_FullClearsRequired = 2;
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif
}

ComponentCamera& ComponentCamera::operator=(const ComponentCamera& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    this->m_Orthographic = other.m_Orthographic;
    this->m_Deferred = other.m_Deferred;

    this->m_ClearColorBuffer = other.m_ClearColorBuffer;
    this->m_ClearDepthBuffer = other.m_ClearDepthBuffer;
    
    this->m_DesiredWidth = other.m_DesiredWidth;
    this->m_DesiredHeight = other.m_DesiredHeight;
    this->m_OrthoNearZ = other.m_OrthoNearZ;
    this->m_OrthoFarZ = other.m_OrthoFarZ;

    this->m_FieldOfView = other.m_FieldOfView;
    this->m_PerspectiveNearZ = other.m_PerspectiveNearZ;
    this->m_PerspectiveFarZ = other.m_PerspectiveFarZ;

    this->m_LayersToRender = other.m_LayersToRender;

    return *this;
}

void ComponentCamera::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnSurfaceChanged );
#if MYFW_EDITOR
        //MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentCamera, Draw );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, Draw );
#endif //MYFW_EDITOR
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnFileRenamed );
    }
}

void ComponentCamera::UnregisterCallbacks()
{
    MyAssert( m_EnabledState != EnabledState_Enabled );

    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
#if MYFW_EDITOR
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
#endif //MYFW_EDITOR
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentCamera::SetDesiredAspectRatio(float width, float height)
{
    m_DesiredWidth = width;
    m_DesiredHeight = height;

    ComputeProjectionMatrices();
}

void ComponentCamera::ComputeProjectionMatrices()
{
    if( m_Viewport.GetHeight() == 0 )
    {
        float ratio = m_DesiredWidth / m_DesiredHeight;

        MyClamp( m_FieldOfView, 1.0f, 179.0f );
        if( m_PerspectiveNearZ < 0.01f )
            m_PerspectiveNearZ = 0.01f;

        m_Camera3D.SetupProjection( ratio, ratio, m_FieldOfView, m_PerspectiveNearZ, m_PerspectiveFarZ );
        m_Camera2D.SetupDirect( -m_DesiredWidth/2, m_DesiredWidth/2, -m_DesiredHeight/2, m_DesiredHeight/2, m_OrthoNearZ, m_OrthoFarZ );
    }
    else
    {
        float deviceratio = (float)m_Viewport.GetWidth() / (float)m_Viewport.GetHeight();
        float gameratio = m_DesiredWidth / m_DesiredHeight;

        MyClamp( m_FieldOfView, 1.0f, 179.0f );
        if( m_PerspectiveNearZ < 0.01f )
            m_PerspectiveNearZ = 0.01f;

        m_Camera3D.SetupProjection( deviceratio, gameratio, m_FieldOfView, m_PerspectiveNearZ, m_PerspectiveFarZ );
        m_Camera2D.Setup( (float)m_Viewport.GetWidth(), (float)m_Viewport.GetHeight(), m_DesiredWidth, m_DesiredHeight, m_OrthoNearZ, m_OrthoFarZ );
    }
}

void ComponentCamera::Tick(float deltaTime)
{
    m_pComponentTransform->UpdateTransform();
    MyMatrix matView = *m_pComponentTransform->GetWorldTransform();
    matView.Inverse();

    if( m_Orthographic )
    {
        m_Camera2D.m_matView = matView;
        m_Camera2D.UpdateMatrices();
    }
    else
    {
        m_Camera3D.m_matView = matView;
        m_Camera3D.UpdateMatrices();
    }
}

void ComponentCamera::OnSurfaceChanged(uint32 x, uint32 y, uint32 width, uint32 height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    if( m_Viewport.GetX() == x && m_Viewport.GetY() == y && m_Viewport.GetWidth() == width && m_Viewport.GetHeight() == height )
        return;

    //m_DesiredWidth = (float)desiredaspectwidth;
    //m_DesiredHeight = (float)desiredaspectheight;

    m_Viewport.Set( x, y, width, height );

    ComputeProjectionMatrices();

    // Check if viewport is bigger than the FBO and resize if it is.
    if( m_pGBuffer != 0 )
    {
        const int numcolorformats = 3;
        FBODefinition::FBOColorFormat colorformats[numcolorformats];
        colorformats[0] = FBODefinition::FBOColorFormat_RGBA_UByte;  // Albedo (RGB)
        colorformats[1] = FBODefinition::FBOColorFormat_RGBA_Float16; // Positions (RGB) / Specular Shine/Power (A)
        colorformats[2] = FBODefinition::FBOColorFormat_RGB_Float16; // Normals (RGB)

        TextureManager* pTextureManager = m_pEngineCore->GetManagers()->GetTextureManager();
        pTextureManager->ReSetupFBO( m_pGBuffer, m_Viewport.GetWidth(), m_Viewport.GetHeight(), MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, colorformats, numcolorformats, 32, true );
    }

#if MYFW_EDITOR
    m_FullClearsRequired = 1;
#endif //MYFW_EDITOR

#if MYFW_USING_WX
    m_FullClearsRequired = 2;
#endif
}

ComponentPostEffect* ComponentCamera::GetNextPostEffect(ComponentPostEffect* pLastEffect) // pass in 0 to get first effect.
{
    ComponentBase* pComponent;

    if( pLastEffect == 0 )
        pComponent = m_pGameObject->GetFirstComponentOfBaseType( BaseComponentType_Data );
    else
        pComponent = m_pGameObject->GetNextComponentOfBaseType( pLastEffect );

    ComponentPostEffect* pPostEffect = 0;

    while( pComponent != 0 )
    {
        pPostEffect = pComponent->IsA( "PostEffectComponent" ) ? (ComponentPostEffect*)pComponent : 0;
        
        if( pPostEffect && pPostEffect->m_pMaterial != 0 && pPostEffect->IsEnabled() )
            return pPostEffect; // if we found a valid initialized post effect, return it.
        else
            pComponent = m_pGameObject->GetNextComponentOfBaseType( pComponent );
    }

    return 0;
}

void ComponentCamera::OnDrawFrame()
{
    // Store the current FBO, we will use it as the final render target.
    unsigned int startingFBO = g_GLStats.m_CurrentFramebuffer;

    // TODO: Clean up, make func other than tick do this.
    // Update camera view/proj before drawing.
    Tick( 0 );

    g_GLStats.m_CurrentFramebufferWidth = m_Viewport.GetWidth();
    g_GLStats.m_CurrentFramebufferHeight = m_Viewport.GetHeight();

#if MYFW_EDITOR
    // If we resize the window and we're in an editor build, clear the backbuffer.
    // This is required since we're potentially GL_SCISSOR_TEST'ing an uncleared area.
    if( m_ClearColorBuffer && m_FullClearsRequired > 0 )
    {
        g_pRenderer->ClearScissorRegion();
        g_pRenderer->SetClearColor( ColorFloat( 0.0f, 0.0f, 0.0f, 1.0f ) );
        g_pRenderer->ClearBuffers( true, true, false );

        m_FullClearsRequired--;
    }
#endif //MYFW_EDITOR

    // If there are any post effect components, render to a texture and pass that into the effect component.

    // Find the first actual ComponentPostEffect.
    ComponentPostEffect* pPostEffect = GetNextPostEffect( 0 );

    // First pass: Render the main scene... into a post effect FBO if a ComponentPostEffect was found.
    {
        if( pPostEffect )
        {
            TextureManager* pTextureManager = m_pEngineCore->GetManagers()->GetTextureManager();

            // If a post effect was found, render to an FBO.
            if( m_pPostEffectFBOs[0] == 0 )
            {
                m_pPostEffectFBOs[0] = pTextureManager->CreateFBO( m_Viewport.GetWidth(), m_Viewport.GetHeight(), MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, false );
#if MYFW_EDITOR
                m_pPostEffectFBOs[0]->MemoryPanel_Hide();
#endif
            }
            else
            {
                pTextureManager->ReSetupFBO( m_pPostEffectFBOs[0], m_Viewport.GetWidth(), m_Viewport.GetHeight(), MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, false );
            }

            m_pPostEffectFBOs[0]->Bind( false );

            MyViewport viewport( 0, 0, m_Viewport.GetWidth(), m_Viewport.GetHeight() );
            g_pRenderer->EnableViewport( &viewport, true );
        }
        else
        {
            // Enable viewport and enable/disable scissor region if needed.
            g_pRenderer->EnableViewport( &m_Viewport, true );
        }

        DrawScene();
    }

    // Other passes: Ping pong between 2 FBOs for the remaining post effects, render the final one to screen.
    int fboindex = 0;
    while( pPostEffect )
    {
        ComponentPostEffect* pNextPostEffect = GetNextPostEffect( pPostEffect );

        if( pNextPostEffect )
        {
            TextureManager* pTextureManager = m_pEngineCore->GetManagers()->GetTextureManager();

            // If there is a next effect, render into the next unused FBO.
            if( m_pPostEffectFBOs[!fboindex] == 0 )
            {
                m_pPostEffectFBOs[!fboindex] = pTextureManager->CreateFBO( m_Viewport.GetWidth(), m_Viewport.GetHeight(), MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, false );
#if MYFW_EDITOR
                m_pPostEffectFBOs[!fboindex]->MemoryPanel_Hide();
#endif
            }
            else
            {
                pTextureManager->ReSetupFBO( m_pPostEffectFBOs[!fboindex], m_Viewport.GetWidth(), m_Viewport.GetHeight(), MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, FBODefinition::FBOColorFormat_RGBA_UByte, 32, false );
            }
            m_pPostEffectFBOs[!fboindex]->Bind( false );

            MyViewport viewport( 0, 0, m_Viewport.GetWidth(), m_Viewport.GetHeight() );
            g_pRenderer->EnableViewport( &viewport, true );
        }
        else
        {
            // If there isn't another post effect, render back to the FBO that was set before this function.
            g_pRenderer->BindFramebuffer( startingFBO );
            //MyBindFramebuffer( GL_FRAMEBUFFER, startingFBO, m_Viewport.GetWidth(), m_Viewport.GetHeight() );

            // Enable viewport and enable/disable scissor region if needed.
            g_pRenderer->EnableViewport( &m_Viewport, true );
        }

        g_pRenderer->SetClearColor( ColorFloat( 0.0f, 0.0f, 0.2f, 1.0f ) );
        g_pRenderer->ClearBuffers( true, true, false );

        pPostEffect->Render( m_pPostEffectFBOs[fboindex] );

        fboindex = !fboindex;
        pPostEffect = pNextPostEffect;
    }

    // Restore the FBO to what was set when we entered this method.
    if( startingFBO != g_GLStats.m_CurrentFramebuffer )
    {
        // The FBO should already be set, either we didn't change it, or the final pass was sent to this FBO.
        MyAssert( false );
        g_pRenderer->BindFramebuffer( startingFBO );
        //MyBindFramebuffer( GL_FRAMEBUFFER, startingFBO, m_Viewport.GetWidth(), m_Viewport.GetHeight() );
    }
}

// DrawScene() is an internal method.
void ComponentCamera::DrawScene()
{
    // Store the current FBO, we will use it as the final render target.
    unsigned int startingFBO = g_GLStats.m_CurrentFramebuffer;

    MyMatrix* pMatProj;
    MyMatrix* pMatView;
    if( m_Orthographic )
    {
        pMatProj = &m_Camera2D.m_matProj;
        pMatView = &m_Camera2D.m_matView;
    }
    else
    {
        pMatProj = &m_Camera3D.m_matProj;
        pMatView = &m_Camera3D.m_matView;
    }

    bool renderedADeferredPass = false;

#if !MYFW_OPENGLES2
    // Start a deferred render pass, creating and binding the G-Buffer.
    if( m_Deferred && g_ActiveShaderPass == ShaderPass_Main )
    {
        // Create gbuffer and deferred shader if they don't exist.
        if( m_pGBuffer == 0 )
        {
            TextureManager* pTextureManager = m_pEngineCore->GetManagers()->GetTextureManager();
            TextureDefinition* pErrorTexture = pTextureManager->GetErrorTexture();
            MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
            MeshManager* pMeshManager = m_pEngineCore->GetManagers()->GetMeshManager();
            ShaderGroupManager* pShaderGroupManager = m_pEngineCore->GetManagers()->GetShaderGroupManager();
            FileManager* pFileManager = m_pEngineCore->GetManagers()->GetFileManager();
            EngineFileManager* pEngineFileManager = static_cast<EngineFileManager*>( pFileManager );

            const int numcolorformats = 3;
            FBODefinition::FBOColorFormat colorformats[numcolorformats];
            colorformats[0] = FBODefinition::FBOColorFormat_RGBA_UByte;  // Albedo (RGB)
            colorformats[1] = FBODefinition::FBOColorFormat_RGBA_Float16; // Positions (RGB) / Specular Shine/Power (A)
            colorformats[2] = FBODefinition::FBOColorFormat_RGB_Float16; // Normals (RGB)

            m_pGBuffer = pTextureManager->CreateFBO( m_Viewport.GetWidth(), m_Viewport.GetHeight(), MyRE::MinFilter_Nearest, MyRE::MagFilter_Nearest, colorformats, numcolorformats, 32, true );
#if MYFW_EDITOR
            m_pGBuffer->MemoryPanel_Hide();
#endif

            MyAssert( m_pDeferredShaderFile_AmbientDirectional == 0 );
            MyAssert( m_pDeferredShaderFile_PointLight == 0 );
            MyAssert( m_pDeferredShader_AmbientDirectional == 0 );
            MyAssert( m_pDeferredShader_PointLight == 0 );
            MyAssert( m_pDeferredMaterial_AmbientDirectional == 0 );
            MyAssert( m_pDeferredMaterial_PointLight == 0 );

            m_pDeferredShaderFile_AmbientDirectional = pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_Deferred_AmbientDirectional.glsl" );
            m_pDeferredShaderFile_PointLight = pEngineFileManager->RequestFile_UntrackedByScene( "Data/DataEngine/Shaders/Shader_Deferred_PointLight.glsl" );
#if MYFW_EDITOR
            m_pDeferredShaderFile_AmbientDirectional->MemoryPanel_Hide();
            m_pDeferredShaderFile_PointLight->MemoryPanel_Hide();
#endif
            m_pDeferredShader_AmbientDirectional = MyNew ShaderGroup( m_pEngineCore, m_pDeferredShaderFile_AmbientDirectional, pErrorTexture );
            m_pDeferredShader_PointLight = MyNew ShaderGroup( m_pEngineCore, m_pDeferredShaderFile_PointLight, pErrorTexture );

            m_pDeferredMaterial_AmbientDirectional = new MaterialDefinition( pMaterialManager, m_pDeferredShader_AmbientDirectional );
            m_pDeferredMaterial_PointLight = new MaterialDefinition( pMaterialManager, m_pDeferredShader_PointLight );

            m_pDeferredQuadMesh = new MyMesh( m_pEngineCore );
            m_pDeferredQuadMesh->CreateClipSpaceQuad( Vector2( 0, 0 ), Vector2( m_pGBuffer->GetWidth()/(float)m_pGBuffer->GetTextureWidth(), m_pGBuffer->GetHeight()/(float)m_pGBuffer->GetTextureHeight() ) );
            m_pDeferredQuadMesh->SetMaterial( m_pDeferredMaterial_AmbientDirectional, 0 );
            m_pDeferredQuadMesh->RegisterSetupCustomUniformsCallback( this, StaticSetupCustomUniformsCallback );

            MyAssert( m_pDeferredSphereMesh == 0 );
            MyAssert( m_pDeferredSphereMeshFile == 0 );
            m_pDeferredSphereMesh = MyNew MyMesh( m_pEngineCore );
            m_pDeferredSphereMeshFile = pFileManager->RequestFile( "Data/DataEngine/Meshes/sphere.obj.mymesh" );
#if MYFW_EDITOR
            m_pDeferredSphereMeshFile->MemoryPanel_Hide();
#endif

            m_pDeferredSphereMesh->SetSourceFile( m_pDeferredSphereMeshFile );
            m_pDeferredSphereMesh->RegisterSetupCustomUniformsCallback( this, StaticSetupCustomUniformsCallback );
        }

        if( m_pGBuffer->IsFullyLoaded() )
        {
            // Set the global render pass to deferred, so each object will render with the correct shader.
            g_ActiveShaderPass = ShaderPass_MainDeferred;
            renderedADeferredPass = true;

            m_pGBuffer->Bind( false );
        }
    }
#endif //!MYFW_OPENGLES2

    // Clear the buffer and render the scene.
    {
        if( renderedADeferredPass )
            g_pRenderer->SetClearColor( ColorFloat( 0.0f, 0.0f, 0.0f, 1.0f ) );
        else
            g_pRenderer->SetClearColor( ColorFloat( 0.0f, 0.0f, 0.2f, 1.0f ) );

        g_pRenderer->ClearBuffers( m_ClearColorBuffer, m_ClearDepthBuffer, false );

        // Generally, draw everything in one pass, deferred renderer will break it into multiple passes below.
        // Internally, g_pComponentSystemManager->DrawFrame will separate Opaques from Transparents.
        bool drawOpaques = true;
        bool drawTransparents = true;
        EmissiveDrawOptions emissiveDrawOption = EmissiveDrawOption_EitherEmissiveOrNot;
        bool drawOverlays = true;

        // For deferred, only render opaques here, all others will be rendered in forward shading pass below.
        if( renderedADeferredPass )
        {
            drawTransparents = false;
            emissiveDrawOption = EmissiveDrawOption_NoEmissives;
            drawOverlays = false;
        }

        // Draw the selected objects.
        g_pComponentSystemManager->DrawFrame( this, pMatProj, pMatView, 0, drawOpaques, drawTransparents, emissiveDrawOption, drawOverlays );

#if MYFW_EDITOR
        if( m_pGBuffer && renderedADeferredPass && m_DeferredGBufferVisible )
        {
            if( ImGui::Begin( "Deferred G-Buffer", &m_DeferredGBufferVisible, ImVec2(150, 150), 1 ) )
            {
                // Create a context menu only available from the title bar.
                if( ImGui::BeginPopupContextItem() )
                {
                    if( ImGui::MenuItem( "Close" ) )
                        m_DeferredGBufferVisible = false;

                    ImGui::EndPopup();
                }

                ImGui::Image( (ImTextureID)m_pGBuffer->GetColorTexture(0), ImVec2(128,128), ImVec2(0,1), ImVec2(1,0) );
                ImGui::SameLine();
                ImGui::Image( (ImTextureID)m_pGBuffer->GetColorTexture(1), ImVec2(128,128), ImVec2(0,1), ImVec2(1,0) );
                ImGui::SameLine();
                ImGui::Image( (ImTextureID)m_pGBuffer->GetColorTexture(2), ImVec2(128,128), ImVec2(0,1), ImVec2(1,0) );
            }
            ImGui::End();
        }
#endif //MYFW_EDITOR
    }

    // Restore the FBO to what was set when we entered this method.
    if( startingFBO != g_GLStats.m_CurrentFramebuffer )
    {
        // The FBO should already be set, either we didn't change it, or the final pass was sent to this FBO.
        //MyAssert( false );
        g_pRenderer->BindFramebuffer( startingFBO );
        //MyBindFramebuffer( GL_FRAMEBUFFER, startingFBO, m_Viewport.GetWidth(), m_Viewport.GetHeight() );
    }

    // Finish our deferred render if we started it.
    // Render a single image for all opaque objects combining all 3 deferred textures.  One pass per light.
    // TODO: Only render opaque objects, render transparents after with forward shaders.
    if( renderedADeferredPass )
    {
        g_ActiveShaderPass = ShaderPass_Main;

        // Clear the buffer.
        g_pRenderer->SetClearColor( ColorFloat( 0.0f, 0.0f, 0.0f, 1.0f ) );
        g_pRenderer->ClearBuffers( true, true, false );

        // Find nearest shadow casting light. TODO: handle this better.
        MyMatrix* pShadowVP = 0;
        TextureDefinition* pShadowTex = 0;
        if( g_ActiveShaderPass == ShaderPass_Main )
        {
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByName( "Shadow Light" );
            if( pObject )
            {
                ComponentBase* pComponent = pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera );
                if( pComponent )
                {
                    ComponentCameraShadow* pShadowCam = pComponent->IsA( "CameraShadowComponent" ) ? (ComponentCameraShadow*)pComponent : 0;
                    if( pShadowCam )
                    {
                        pShadowVP = pShadowCam->GetViewProjMatrix();
#if 1
                        pShadowTex = pShadowCam->GetFBO()->GetDepthTexture();
#else
                        pShadowTex = pShadowCam->GetFBO()->GetColorTexture( 0 );
#endif
                    }
                }
            }
        }

        // Render one dir light and loop through all point lights.
        if( m_pDeferredSphereMesh->IsReady() )
        {
            LightManager* pLightManager = m_pEngineCore->GetManagers()->GetLightManager();

            // TODO: This only needs to be done once, but since the mesh file isn't loaded above,
            //       the submesh isn't created and material can't be set.
            m_pDeferredSphereMesh->SetMaterial( m_pDeferredMaterial_PointLight, 0 );

            // Handle clear color, ambient light, scene depth values and a single directional light.
            // Leave depth test and depth write on for this quad since it writes in our depth values.
            {
                // Render a full screen quad to combine the 3 textures from the G-Buffer.
                // Blending should be disabled, which it is by default.
                // Textures are set below in SetupCustomUniformsCallback().
                MyLight* pLight;
                pLightManager->FindNearestLights( LightType_Directional, 1, Vector3(0,0,0), &pLight );

                Vector3 worldPosition = m_pComponentTransform->GetWorldPosition();

                if( pLight )
                    m_pDeferredQuadMesh->Draw( 0, 0, 0, &worldPosition, 0, &pLight, 1, pShadowVP, pShadowTex, 0, 0 );
                else
                    m_pDeferredQuadMesh->Draw( 0, 0, 0, &worldPosition, 0, 0, 0, pShadowVP, pShadowTex, 0, 0 );
            }

            // Disable depth write and depth test for point light spheres.
            g_pRenderer->SetDepthWriteEnabled( false );
            g_pRenderer->SetDepthTestEnabled( false );

            // Draw all the point lights in the scene, using a sphere for each.
            {
                // Swap culling to draw backs of spheres for the rest of the lights.
                g_pRenderer->SetCullMode( MyRE::CullMode_Front );

                for( CPPListNode* pNode = pLightManager->GetLightList()->GetHead(); pNode; pNode = pNode->GetNext() )
                {
                    MyLight* pLight = static_cast<MyLight*>( pNode );

                    if( pLight->m_LightType == LightType_Point )
                    {
                        // Scale sphere.
                        float radius = pLight->m_Attenuation.x;
                        MyMatrix matWorld;
                        matWorld.CreateSRT( radius, 0, pLight->m_Position );

                        MyMatrix* pMatViewProj;
                        if( m_Orthographic )
                            pMatViewProj = &m_Camera2D.m_matViewProj;
                        else
                            pMatViewProj = &m_Camera3D.m_matViewProj;

                        // Render a sphere to combine the 3 textures from the G-Buffer.
                        // Point light shader should set blending to additive.
                        // Textures are set below in SetupCustomUniformsCallback().
                        Vector3 worldPosition = m_pComponentTransform->GetWorldPosition();
                        m_pDeferredSphereMesh->Draw( pMatProj, pMatView, &matWorld, &worldPosition, 0, &pLight, 1, 0, 0, 0, 0 );
                    }
                }
            }

            // Restore the old gl state.
            g_pRenderer->SetDepthWriteEnabled( true );
            g_pRenderer->SetDepthTestEnabled( true );
            g_pRenderer->SetCullMode( MyRE::CullMode_Back );
            g_pRenderer->SetBlendFunc( MyRE::BlendFactor_SrcAlpha, MyRE::BlendFactor_OneMinusSrcAlpha );
        }

        // Render opaque/emissive objects with normal forward render pass.
        {
            bool drawOpaques = true;
            bool drawTransparents = false;
            EmissiveDrawOptions emissiveDrawOption = EmissiveDrawOption_OnlyEmissives;
            bool drawOverlays = false;

            m_pComponentSystemManager->DrawFrame( this, pMatProj, pMatView, 0, drawOpaques, drawTransparents, emissiveDrawOption, drawOverlays );
        }

        // Render transparent objects with normal forward render pass.
        {
            bool drawOpaques = false;
            bool drawTransparents = true;
            EmissiveDrawOptions emissiveDrawOption = EmissiveDrawOption_EitherEmissiveOrNot;
            bool drawOverlays = true;

            m_pComponentSystemManager->DrawFrame( this, pMatProj, pMatView, 0, drawOpaques, drawTransparents, emissiveDrawOption, drawOverlays );
        }
    }
}

void ComponentCamera::SetupCustomUniformsCallback(Shader_Base* pShader) // StaticSetupCustomUniformsCallback
{
    // TODO: Not this...
    pShader->ProgramDeferredRenderingUniforms( m_pGBuffer, m_PerspectiveNearZ, m_PerspectiveFarZ, m_pGameObject->GetTransform()->GetWorldTransform(), ColorFloat( 0, 0, 0.2f, 1 ) );
}

bool ComponentCamera::IsVisible()
{
    return true;
}

bool ComponentCamera::ExistsOnLayer(unsigned int layerflags)
{
    if( layerflags & Layer_Editor )
        return true;
    
    return false;
}

#if MYFW_EDITOR
void ComponentCamera::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    if( m_pEngineCore->GetEditorPrefs()->Get_View_ShowEditorIcons() == false )
        return;

    MySprite* pSprite = g_pEngineCore->GetEditorState()->m_pEditorIcons[EditorIcon_Camera];
    if( pSprite == 0 )
        return;

    // Set the sprite color.
    pSprite->GetMaterial()->m_ColorDiffuse = ColorByte( 255, 255, 255, 255 );

    // Make the sprite face the same direction as the camera.
    MyMatrix rot90;
    rot90.SetIdentity();
    rot90.Rotate( -90, 0, 1, 0 );

    MyMatrix transform = *m_pComponentTransform->GetWorldTransform() * rot90;
    //pSprite->SetPosition( &transform );
    
    // Disable culling, so we see the camera from both sides.
    g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Fill );
    g_pRenderer->SetCullingEnabled( false );
    pSprite->Draw( pMatProj, pMatView, &transform, pShaderOverride, true );
    g_pRenderer->SetCullingEnabled( true );
    if( g_pEngineCore->GetDebug_DrawWireframe() )
        g_pRenderer->SetPolygonMode( MyRE::PolygonDrawMode_Line );
}

bool ComponentCamera::HandleInputForEditorCamera(int keyAction, int keyCode, int mouseAction, int id, float x, float y, float pressure)
{
    EngineCore* pEngineCore = g_pEngineCore;
    EditorState* pEditorState = pEngineCore->GetEditorState();
    ComponentCamera* pCamera = this;

    MyMatrix startCamTransform = *pCamera->m_pComponentTransform->GetLocalTransform( false );

    // If mouse message. down, up, dragging or wheel.
    if( mouseAction != -1 )
    {
        unsigned int mods = pEditorState->m_ModifierKeyStates;

        // Get the editor camera's local transform.
        MyMatrix* matCamera = pCamera->m_pComponentTransform->GetLocalTransform( true );

        // Move camera in/out if mousewheel spinning.
        if( mouseAction == GCBA_Wheel )
        {
            // Pressure is also mouse wheel movement rate in editor configurations.
#if MYFW_RIGHTHANDED
            Vector3 dir = Vector3( 0, 0, 1 ) * -(pressure/fabsf(pressure));
#else
            Vector3 dir = Vector3( 0, 0, 1 ) * (pressure/fabsf(pressure));
#endif
            float speed = 100.0f;
            if( pEditorState->m_ModifierKeyStates & MODIFIERKEY_Shift )
                speed *= 5;

            if( dir.LengthSquared() > 0 )
                matCamera->TranslatePreRotScale( dir * speed * 1/60.0f); //* pEngineCore->GetTimePassedUnpausedLastFrame() );
        }

        // If left mouse down, reset the transform gizmo tool.
        if( mouseAction == GCBA_Down && id == 0 )
        {
            pEditorState->m_pTransformGizmo->m_LastIntersectResultIsValid = false;
        }

        // Enter/Exit RotatingEditorCamera camera state on right-click.
        {
            // If the right mouse button was clicked, switch to rotating editor camera state.
            if( mouseAction == GCBA_Down && id == 1 )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_RotatingEditorCamera;
            }

            // If we're in EDITORACTIONSTATE_RotatingEditorCamera and the right mouse goes up.
            if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_RotatingEditorCamera &&
                mouseAction == GCBA_Up && id == 1 )
            {
                pEditorState->m_EditorActionState = EDITORACTIONSTATE_None;
            }
        }

        // If space is held, left button will pan the camera around. or just middle button.
        if( mouseAction == GCBA_Held || mouseAction == GCBA_RelativeMovement )
        {
            if( ( (mods & MODIFIERKEY_LeftMouse) && (mods & MODIFIERKEY_Space) ) || (mods & MODIFIERKEY_MiddleMouse) )
            {
    #if MYFW_OSX
                // TODO: Fix OSX to support locked mouse cursor.
                Vector3 dir = pEditorState->m_LastMousePosition - pEditorState->m_CurrentMousePosition;
    #else //MYFW_OSX
                // Try to lock the editor mouse cursor so we can move camera with raw mouse input.
                if( IsMouseLocked() == false )
                {
                    //LOGInfo( LOGTag, "Request mouse lock\n" );
                    SetMouseLock( true );
                }

                Vector2 dir( 0, 0 );
                if( mouseAction == GCBA_RelativeMovement )
                {
                    //LOGInfo( LOGTag, "relative movement.\n" );
                    dir.Set( -x, -y );
                }
    #endif //MYFW_OSX

                //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );
                if( dir.LengthSquared() > 0 )
                    matCamera->TranslatePreRotScale( dir * 0.05f );
            }
            else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_GroupSelectingObjects &&
                (mods & MODIFIERKEY_LeftMouse) )
            {
            }
            // If right mouse is down, rotate the camera around selected object or around it's current position.
            else if( pEditorState->m_EditorActionState == EDITORACTIONSTATE_RotatingEditorCamera &&
                (mods & MODIFIERKEY_RightMouse) )
            {
    #if MYFW_OSX
                // TODO: fix OSX to support locked mouse cursor.
                Vector3 dir = (pEditorState->m_LastMousePosition - pEditorState->m_CurrentMousePosition) * -1;
    #else //MYFW_OSX
                // Try to lock the editor mouse cursor so we can move camera with raw mouse input.
                if( IsMouseLocked() == false )
                {
                    //LOGInfo( LOGTag, "Request mouse lock\n" );
                    SetMouseLock( true );
                }

                Vector2 dir( 0, 0 );
                if( mouseAction == GCBA_RelativeMovement )
                {
                    //LOGInfo( LOGTag, "Relative Movement\n" );
                    dir.Set( x, y );
                }
    #endif //MYFW_OSX

                if( dir.LengthSquared() > 0 )
                {
                    Vector3 pivot;
                    float distancefrompivot;

                    // Different pivot and distance from pivot depending if Alt is held.
                    if( mods & MODIFIERKEY_Alt && pEditorState->m_pSelectedObjects.size() > 0 && pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0] )
                    {
                        // pivot around the transform gizmo
                        pivot = pEditorState->m_pTransformGizmo->m_pTranslate1Axis[0]->GetTransform()->GetWorldTransform()->GetTranslation();
                        distancefrompivot = (matCamera->GetTranslation() - pivot).Length();
                    }
                    else
                    {
                        // Pivot on the camera, just change rotation.
                        pivot = matCamera->GetTranslation();
                        distancefrompivot = 0;
                    }

                    //LOGInfo( LOGTag, "dir (%0.2f, %0.2f)\n", dir.x, dir.y );

                    Vector3 angle = pCamera->m_pComponentTransform->GetLocalRotation();

                    // TODO: Make this degrees per inch.
                    float degreesperpixel = 0.125f;

    #if MYFW_RIGHTHANDED
                    angle.y += dir.x * degreesperpixel;
                    angle.x -= dir.y * degreesperpixel;
    #else
                    angle.y -= dir.x * degreesperpixel;
                    angle.x += dir.y * degreesperpixel;
    #endif
                    MyClamp( angle.x, -90.0f, 90.0f );

                    // Create a new local transform.
                    matCamera->SetIdentity();
    #if MYFW_RIGHTHANDED
                    matCamera->Translate( 0, 0, distancefrompivot );
    #else
                    matCamera->Translate( 0, 0, -distancefrompivot );
    #endif
                    matCamera->Rotate( angle.x, 1, 0, 0 );
                    matCamera->Rotate( angle.y, 0, 1, 0 );
                    matCamera->Translate( pivot );

                    // Update the local scale/rotation/translation from the local transform.
                    pCamera->m_pComponentTransform->UpdateLocalSRT();
                }
            }
        }

        // Pull the scale/pos/rot from the local matrix and update the values in the watch window.
        pCamera->m_pComponentTransform->UpdateLocalSRT();
        pCamera->m_pComponentTransform->UpdateTransform();
    }

    // Handle editor keys.
    if( keyAction == GCBA_Held )
    {
        EditorKeyBindings* pKeys = pEngineCore->GetEditorPrefs()->GetKeyBindings();
        uint8 modifiers = static_cast<uint8>( pEditorState->m_ModifierKeyStates );

        // Get the editor camera's local transform.
        MyMatrix* matCamera = pCamera->m_pComponentTransform->GetLocalTransform( true );

        // Focus camera on selected objects.
        if( pKeys->KeyMatches( HotkeyAction::Camera_Focus, modifiers, keyCode, HotkeyContext::Global ) )
        {
            // Make sure there is an object actually selected.
            if( pEditorState->m_pSelectedObjects.size() > 0 )
            {
                Vector3 targetPos = pEditorState->m_pTransformGizmo->m_GizmoWorldTransform.GetTranslation();
                Vector3 lookAt = matCamera->GetAt();

                // Magic const number, maybe should be somewhere special?
                float cameraSnapDist = 5.0f;
                Vector3 offset = -1.0f * lookAt * cameraSnapDist;
                Vector3 finalPosition = targetPos + offset;

                matCamera->SetTranslation( finalPosition );
            }
        }

        // Handle editor camera movement.
        {
            Vector3 dir( 0, 0, 0 );

            uint8 modifiersWithoutShift = modifiers & ~MODIFIERKEY_Shift;

            if( pKeys->KeyMatches( HotkeyAction::Camera_Forward, modifiersWithoutShift, keyCode ) ) dir.z +=  1;
            if( pKeys->KeyMatches( HotkeyAction::Camera_Left,    modifiersWithoutShift, keyCode ) ) dir.x += -1;
            if( pKeys->KeyMatches( HotkeyAction::Camera_Back,    modifiersWithoutShift, keyCode ) ) dir.z += -1;
            if( pKeys->KeyMatches( HotkeyAction::Camera_Right,   modifiersWithoutShift, keyCode ) ) dir.x +=  1;
            if( pKeys->KeyMatches( HotkeyAction::Camera_Up,      modifiersWithoutShift, keyCode ) ) dir.y +=  1;
            if( pKeys->KeyMatches( HotkeyAction::Camera_Down,    modifiersWithoutShift, keyCode ) ) dir.y += -1;

            float speed = 7.0f;
            if( modifiers & MODIFIERKEY_Shift )
                speed *= 5;

            if( dir.LengthSquared() > 0 )
                matCamera->TranslatePreRotScale( dir * speed * pEngineCore->GetTimePassedUnpausedLastFrame() );
        }

        pCamera->m_pComponentTransform->UpdateLocalSRT();
        pCamera->m_pComponentTransform->UpdateTransform();
    }

    // If the camera is locked to an object,
    //     apply any changes we made to the editor camera to the offset from the locked object.
    if( pEditorState->m_CameraState == EditorCameraState_LockedToObject )
    {
        MyMatrix endCamTransform = *pCamera->m_pComponentTransform->GetLocalTransform( false );
        MyMatrix startCamInverseTransform = startCamTransform.GetInverse();

        MyMatrix changeInCamTransform = startCamInverseTransform * endCamTransform;
        MyMatrix newTransform = pEditorState->m_OffsetFromObject * changeInCamTransform;

        pEditorState->m_OffsetFromObject = newTransform;
    }

    return false;
}
#endif //MYFW_EDITOR
