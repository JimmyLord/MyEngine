//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentCamera::m_PanelWatchBlockVisible = true;
#endif

// 32 layers strings since ComponentBase::ExportVariablesToJSON -> case ComponentVariableType_Flags is looking for 32
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

ComponentCamera::ComponentCamera()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Camera;

    m_pComponentTransform = 0;
}

ComponentCamera::~ComponentCamera()
{
    SAFE_RELEASE( m_pPostEffectFBOs[0] );
    SAFE_RELEASE( m_pPostEffectFBOs[1] );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentCamera::RegisterVariables(CPPListHead* pList, ComponentCamera* pThis) //_VARIABLE_LIST
{
    AddVarFlags( pList, "Layers", MyOffsetOf( pThis, &pThis->m_LayersToRender ), true, true, 0,
                 g_NumberOfVisibilityLayers, g_pVisibilityLayerStrings,
                 (CVarFunc_ValueChanged)&ComponentCamera::OnValueChanged, 0, 0 );
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

        g_pPanelWatch->AddBool( "Ortho", &m_Orthographic, 0, 1, this, ComponentCamera::StaticOnValueChanged );

        g_pPanelWatch->AddFloat( "Desired width", &m_DesiredWidth, 640, 960, this, ComponentCamera::StaticOnValueChanged );
        g_pPanelWatch->AddFloat( "Desired height", &m_DesiredHeight, 640, 960, this, ComponentCamera::StaticOnValueChanged );

        g_pPanelWatch->AddBool( "ColorBit", &m_ClearColorBuffer, 0, 1, this, ComponentCamera::StaticOnValueChanged );
        g_pPanelWatch->AddBool( "DepthBit", &m_ClearDepthBuffer, 0, 1, this, ComponentCamera::StaticOnValueChanged );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentCamera::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue, void* newpointer)
{
    void* oldpointer = 0;

    ComputeProjectionMatrices();
    m_FullClearsRequired = 2;

    return oldpointer;
}

void ComponentCamera::OtherOnValueChanged(int controlid, bool finishedchanging)
{
    ComputeProjectionMatrices();
    m_FullClearsRequired = 2;
}
#endif //MYFW_USING_WX

cJSON* ComponentCamera::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid, saveid );

    cJSON_AddNumberToObject( jComponent, "Ortho", m_Orthographic );

    cJSON_AddNumberToObject( jComponent, "DWidth", m_DesiredWidth );
    cJSON_AddNumberToObject( jComponent, "DHeight", m_DesiredHeight );

    cJSON_AddNumberToObject( jComponent, "ColorBit", m_ClearColorBuffer );
    cJSON_AddNumberToObject( jComponent, "DepthBit", m_ClearDepthBuffer );

    //cJSON_AddNumberToObject( jComponent, "Layers", m_LayersToRender );

    return jComponent;
}

void ComponentCamera::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
{
    // if we're loading a camera, start the layers flags at 0, so we set the flags when loading.
    m_LayersToRender = 0;

    ComponentBase::ImportFromJSONObject( jComponent, sceneid );

    cJSONExt_GetBool( jComponent, "Ortho", &m_Orthographic );

    cJSONExt_GetFloat( jComponent, "DWidth", &m_DesiredWidth );
    cJSONExt_GetFloat( jComponent, "DHeight", &m_DesiredHeight );

    cJSONExt_GetBool( jComponent, "ColorBit", &m_ClearColorBuffer );
    cJSONExt_GetBool( jComponent, "DepthBit", &m_ClearDepthBuffer );

    //cJSONExt_GetUnsignedInt( jComponent, "Layers", &m_LayersToRender );
}

void ComponentCamera::Reset()
{
    ComponentBase::Reset();

    m_pComponentTransform = m_pGameObject->m_pComponentTransform;

    m_Orthographic = false;

    m_ClearColorBuffer = true;
    m_ClearDepthBuffer = true;
    
    m_DesiredWidth = 0;
    m_DesiredHeight = 0;
    
    m_LayersToRender = 0x00FF;

    m_WindowStartX = 0;
    m_WindowStartY = 0;
    m_WindowWidth = 0;
    m_WindowHeight = 0;

    m_pPostEffectFBOs[0] = 0;
    m_pPostEffectFBOs[1] = 0;

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
    
    this->m_DesiredWidth = other.m_DesiredWidth;
    this->m_DesiredHeight = other.m_DesiredHeight;

    this->m_LayersToRender = other.m_LayersToRender;

    return *this;
}

void ComponentCamera::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnSurfaceChanged );
#if MYFW_USING_WX
        //MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentCamera, Draw );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, Draw );
#endif //MYFW_USING_WX
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentCamera, OnFileRenamed );
    }
}

void ComponentCamera::UnregisterCallbacks()
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

void ComponentCamera::SetDesiredAspectRatio(float width, float height)
{
    m_DesiredWidth = width;
    m_DesiredHeight = height;

    ComputeProjectionMatrices();
}

void ComponentCamera::ComputeProjectionMatrices()
{
    if( m_WindowHeight == 0 )
        return;

    float deviceratio = (float)m_WindowWidth / (float)m_WindowHeight;
    float gameratio = m_DesiredWidth / m_DesiredHeight;

    m_Camera3D.SetupProjection( deviceratio, gameratio, 45 );
    m_Camera2D.Setup( (float)m_WindowWidth, (float)m_WindowHeight, m_DesiredWidth, m_DesiredHeight );
}

void ComponentCamera::Tick(double TimePassed)
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

void ComponentCamera::OnSurfaceChanged(unsigned int startx, unsigned int starty, unsigned int width, unsigned int height, unsigned int desiredaspectwidth, unsigned int desiredaspectheight)
{
    m_DesiredWidth = (float)desiredaspectwidth;
    m_DesiredHeight = (float)desiredaspectheight;

    m_WindowStartX = startx;
    m_WindowStartY = starty;
    m_WindowWidth = width;
    m_WindowHeight = height;

    ComputeProjectionMatrices();

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
    // TODO: Clean up, make func other than tick to this.
    // Update camera view/proj before drawing.
    Tick( 0 );

    //int currentframebuffer;
    //glGetIntegerv( GL_FRAMEBUFFER_BINDING, &currentframebuffer );
 
    //MyBindFramebuffer( GL_FRAMEBUFFER, 0, m_WindowWidth, m_WindowHeight );
    g_GLStats.m_CurrentFramebufferWidth = m_WindowWidth;
    g_GLStats.m_CurrentFramebufferHeight = m_WindowHeight;

    //int fbo;
    //glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fbo );

    //if( fbo != 0 )
    //    int bp = 1;
    ////MyBindFramebuffer( GL_FRAMEBUFFER, 0 );

#if MYFW_USING_WX
    // if we resize the window and we're in a wx build, clear the entire backbuffer for 2 frames.
    // this is required since we're potentially GL_SCISSOR_TEST'ing an uncleared area.
    if( m_FullClearsRequired > 0 )
    {
        glDisable( GL_SCISSOR_TEST );
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        m_FullClearsRequired--;
    }
#endif

    // if there are any post effect components, render to a texture and pass that into the effect component.

    // Find the first actual ComponentPostEffect.
    ComponentPostEffect* pPostEffect = GetNextPostEffect( 0 );

    // render the main scene... into a texture if a ComponentPostEffect was found.
    {
        if( pPostEffect )
        {
            // if a post effect was found, render to a texture.
            if( m_pPostEffectFBOs[0] == 0 )
            {
                m_pPostEffectFBOs[0] = g_pTextureManager->CreateFBO( m_WindowWidth, m_WindowHeight, GL_NEAREST, GL_NEAREST, true, 32, false );
            }
            else
            {
                g_pTextureManager->ReSetupFBO( m_pPostEffectFBOs[0], m_WindowWidth, m_WindowHeight, GL_NEAREST, GL_NEAREST, true, 32, false );
            }
            m_pPostEffectFBOs[0]->Bind( false );
            glDisable( GL_SCISSOR_TEST );
            glViewport( 0, 0, m_WindowWidth, m_WindowHeight );
        }
        else
        {
            // only draw to part of the window, using scissor test and glViewport.
            if( m_WindowStartX != 0 || m_WindowStartY != 0 )
            {
                // scissor test is really only needed for the glClear call.
                glEnable( GL_SCISSOR_TEST );
                glScissor( m_WindowStartX, m_WindowStartY, m_WindowWidth, m_WindowHeight );
            }
            else
            {
                glDisable( GL_SCISSOR_TEST );
                glScissor( m_WindowStartX, m_WindowStartY, m_WindowWidth, m_WindowHeight );
            }

            glViewport( m_WindowStartX, m_WindowStartY, m_WindowWidth, m_WindowHeight );
        }

        glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );

        if( m_ClearColorBuffer && m_ClearDepthBuffer )
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        else if( m_ClearColorBuffer )
            glClear( GL_COLOR_BUFFER_BIT );
        else if( m_ClearDepthBuffer )
            glClear( GL_DEPTH_BUFFER_BIT );

        if( m_Orthographic )
        {   
            g_pComponentSystemManager->OnDrawFrame( this, &m_Camera2D.m_matViewProj, 0 );
        }
        else
        {
            g_pComponentSystemManager->OnDrawFrame( this, &m_Camera3D.m_matViewProj, 0 );
        }
    }

    // ping pong between 2 fbos for the remaining post effects, render the final one to screen.
    int fboindex = 0;
    while( pPostEffect )
    {
        ComponentPostEffect* pNextPostEffect = GetNextPostEffect( pPostEffect );

        if( pNextPostEffect )
        {
            // if there is a next effect, render into the next unused fbo.
            if( m_pPostEffectFBOs[!fboindex] == 0 )
            {
                m_pPostEffectFBOs[!fboindex] = g_pTextureManager->CreateFBO( m_WindowWidth, m_WindowHeight, GL_NEAREST, GL_NEAREST, true, 32, false );
            }
            else
            {
                g_pTextureManager->ReSetupFBO( m_pPostEffectFBOs[!fboindex], m_WindowWidth, m_WindowHeight, GL_NEAREST, GL_NEAREST, true, 32, false );
            }
            m_pPostEffectFBOs[!fboindex]->Bind( false );

            glDisable( GL_SCISSOR_TEST );
            glViewport( 0, 0, m_WindowWidth, m_WindowHeight );

            glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }
        else
        {
            // if there isn't another post effect, render to the screen.
            MyBindFramebuffer( GL_FRAMEBUFFER, 0, m_WindowWidth, m_WindowHeight );

            // only draw to part of the window, using scissor test and glViewport.
            if( m_WindowStartX != 0 || m_WindowStartY != 0 )
            {
                // scissor test is really only needed for the glClear call.
                glEnable( GL_SCISSOR_TEST );
                glScissor( m_WindowStartX, m_WindowStartY, m_WindowWidth, m_WindowHeight );
            }
            else
            {
                glDisable( GL_SCISSOR_TEST );
            }

            glViewport( m_WindowStartX, m_WindowStartY, m_WindowWidth, m_WindowHeight );
            glClearColor( 0.0f, 0.0f, 0.2f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        }

        pPostEffect->Render( m_pPostEffectFBOs[fboindex] );

        fboindex = !fboindex;
        pPostEffect = pNextPostEffect;
    }

    MyBindFramebuffer( GL_FRAMEBUFFER, 0, m_WindowWidth, m_WindowHeight );
}

bool ComponentCamera::IsVisible()
{
    return true;
}

bool ComponentCamera::ExistsOnLayer(unsigned int layerflags)
{
    if( layerflags & Layer_EditorFG )
        return true;
    
    return false;
}

#if MYFW_USING_WX
void ComponentCamera::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    if( g_pEngineMainFrame->m_ShowEditorIcons == false )
        return;

    MySprite* pSprite = g_pEngineCore->m_pEditorState->m_pEditorIcons[EditorIcon_Camera];
    if( pSprite == 0 )
        return;

    // Set the sprite color
    pSprite->GetMaterial()->m_ColorDiffuse = ColorByte( 255, 255, 255, 255 );

    // make the sprite face the same direction as the camera.
    MyMatrix rot90;
    rot90.SetIdentity();
    rot90.Rotate( -90, 0, 1, 0 );

    MyMatrix transform = *m_pComponentTransform->GetWorldTransform() * rot90;
    //pSprite->SetPosition( &transform );
    
    // disable culling, so we see the camera from both sides.
    glDisable( GL_CULL_FACE );
    pSprite->Draw( &transform, pMatViewProj, pShaderOverride );
    glEnable( GL_CULL_FACE );
}
#endif
