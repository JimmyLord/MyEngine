//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentParticleEmitter.h"
#include "ComponentSystem/BaseComponents/ComponentCamera.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "Core/EngineCore.h"
#include "../../../Framework/MyFramework/SourceCommon/RenderGraphs/RenderGraph_Base.h"

#if MYFW_USING_WX
bool ComponentParticleEmitter::m_PanelWatchBlockVisible = true;
#endif

ComponentParticleEmitter::ComponentParticleEmitter(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentRenderable( pEngineCore, pComponentSystemManager )
{
    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_Particles.AllocateObjects( 1000 );
    for( unsigned int i=0; i<m_Particles.Length(); i++ )
    {
        Particle* pObj = MyNew Particle;
        m_Particles.AddInactiveObject( pObj );
    }

    BufferManager* pBufferManager = g_pComponentSystemManager->GetEngineCore()->GetManagers()->GetBufferManager();

#if MYFW_USEINSTANCEDPARTICLES
    {
        m_pParticleRenderer = MyNew ParticleRendererInstanced( false );
        m_pParticleRenderer->AllocateVertices( 1000, "Particle Renderer Instanced", pBufferManager );
    }
#else
    {
        m_pParticleRenderer = MyNew ParticleRenderer( false );
        m_pParticleRenderer->AllocateVertices( 1000, "Particle Renderer", pBufferManager );
    }
#endif

    m_pRenderGraphObject = 0;
    m_pMaterial = 0;
}

ComponentParticleEmitter::~ComponentParticleEmitter()
{
    m_Particles.DeleteAllObjectsInPool();

    SAFE_DELETE( m_pParticleRenderer );

    SAFE_RELEASE( m_pMaterial );

    RemoveFromRenderGraph();
}

void ComponentParticleEmitter::Reset()
{
    ComponentRenderable::Reset();

    m_RunInEditor = false;

    SAFE_RELEASE( m_pMaterial );

    m_ContinuousSpawn = true;
    m_TimeTilNextSpawn = 0;

    m_BurstDuration = 100;

    m_BurstTimeLeft = m_BurstDuration;

    m_BillboardSprites = true;
    
    m_SpawnTime = 1/60.0f;
    m_SpawnTimeVariation = 0.1f;
    m_InitialOffset.Set( 0, 0, 0 );
    m_Center = Vector2( 320.0f, 800.0f );
    m_CenterVariation = Vector2( 1, 1 );
    m_InitialSpeedBoost = 1;
    m_Size = 0.03f; //42;
    m_SizeVariation = 0.03f; //21;
    m_ScaleSpeed = -1.0f;
    m_Color1 = ColorFloat(1,1,1,1);
    m_Color2 = ColorFloat(0,0,0,1);
    m_UseColorsAsOptions = false;
    m_UseColorsAsRange = false;
    m_ColorTransitionDelay = 0;
    m_ColorTransitionSpeed = -1.0f;
    m_Dir = 0;
    m_DirVariation = 0.28f; //75.23f;
    m_TimeToLive = 5.0f; //0.5f;
    m_TimeToLiveVariation = 0.0f;

    m_AlphaModifier = 1.0f;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentParticleEmitter::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentParticleEmitter>( "ComponentParticleEmitter" )
            .addFunction( "CreateBurst", &ComponentParticleEmitter::CreateBurst ) // void ComponentParticleEmitter::CreateBurst(int number, Vector3 offset)
        .endClass();
}
#endif //MYFW_USING_LUA

cJSON* ComponentParticleEmitter::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* component = ComponentRenderable::ExportAsJSONObject( savesceneid, saveid );

    cJSON_AddNumberToObject( component, "RunInEditor", m_RunInEditor );
    cJSON_AddNumberToObject( component, "ContinuousSpawn", m_ContinuousSpawn );
    cJSON_AddNumberToObject( component, "Billboard", m_BillboardSprites );

    cJSONExt_AddFloatArrayToObject( component, "offset", &m_InitialOffset.x, 3 );
    cJSON_AddNumberToObject( component, "size", m_Size );
    cJSON_AddNumberToObject( component, "sizevar", m_SizeVariation );
    cJSON_AddNumberToObject( component, "timetolive", m_TimeToLive );
    cJSONExt_AddFloatArrayToObject( component, "centervar", &m_CenterVariation.x, 2 );
    cJSONExt_AddFloatArrayToObject( component, "dir", &m_Dir.x, 3 );
    cJSONExt_AddFloatArrayToObject( component, "dirvar", &m_DirVariation.x, 3 );

    cJSON_AddNumberToObject( component, "coloroption", m_UseColorsAsOptions );

    cJSONExt_AddFloatArrayToObject( component, "color1", &m_Color1.r, 4 );
    cJSONExt_AddFloatArrayToObject( component, "color2", &m_Color2.r, 4 );

    if( m_pMaterial && m_pMaterial->GetFile() )
        cJSON_AddStringToObject( component, "Material", m_pMaterial->GetMaterialDescription() );

    return component;
}

void ComponentParticleEmitter::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetBool( jsonobj, "RunInEditor", &m_RunInEditor );
    cJSONExt_GetBool( jsonobj, "ContinuousSpawn", &m_ContinuousSpawn );
    cJSONExt_GetBool( jsonobj, "Billboard", &m_BillboardSprites );

    cJSONExt_GetFloatArray( jsonobj, "offset", &m_InitialOffset.x, 3 );
    cJSONExt_GetFloat( jsonobj, "size", &m_Size );
    cJSONExt_GetFloat( jsonobj, "sizevar", &m_SizeVariation );
    cJSONExt_GetFloat( jsonobj, "timetolive", &m_TimeToLive );
    cJSONExt_GetFloatArray( jsonobj, "centervar", &m_CenterVariation.x, 2 );
    cJSONExt_GetFloatArray( jsonobj, "dir", &m_Dir.x, 3 );
    cJSONExt_GetFloatArray( jsonobj, "dirvar", &m_DirVariation.x, 3 );

    cJSONExt_GetBool( jsonobj, "coloroption", &m_UseColorsAsOptions );

    cJSONExt_GetFloatArray( jsonobj, "color1", &m_Color1.r, 4 );
    cJSONExt_GetFloatArray( jsonobj, "color2", &m_Color2.r, 4 );

    cJSON* materialstringobj = cJSON_GetObjectItem( jsonobj, "Material" );
    if( materialstringobj )
    {
        MaterialManager* pMaterialManager = m_pEngineCore->GetManagers()->GetMaterialManager();
        MaterialDefinition* pMaterial = pMaterialManager->LoadMaterial( materialstringobj->valuestring );
        if( pMaterial )
            SetMaterial( pMaterial, 0 );
        pMaterial->Release();
    }
}

ComponentParticleEmitter& ComponentParticleEmitter::operator=(const ComponentParticleEmitter& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    this->m_RunInEditor         = other.m_RunInEditor;
    this->m_ContinuousSpawn     = other.m_ContinuousSpawn;

    this->m_BillboardSprites    = other.m_BillboardSprites;

    this->m_Size                = other.m_Size;
    this->m_SizeVariation       = other.m_SizeVariation;
    this->m_TimeToLive          = other.m_TimeToLive;
    this->m_Dir                 = other.m_Dir;
    this->m_DirVariation        = other.m_DirVariation;

    this->m_UseColorsAsOptions  = other.m_UseColorsAsOptions;

    this->m_Color1              = other.m_Color1;
    this->m_Color2              = other.m_Color2;

    m_pMaterial = other.m_pMaterial;
    if( m_pMaterial )
        m_pMaterial->AddRef();

    return *this;
}

void ComponentParticleEmitter::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentParticleEmitter, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentParticleEmitter, OnSurfaceChanged );
        MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentParticleEmitter, Draw ); //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentParticleEmitter, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentParticleEmitter, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentParticleEmitter, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentParticleEmitter, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentParticleEmitter, OnFileRenamed );
    }
}

void ComponentParticleEmitter::UnregisterCallbacks()
{
    MyAssert( m_EnabledState != EnabledState_Enabled );

    if( m_CallbacksRegistered == true )
    {
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentParticleEmitter::OnLoad()
{
    ComponentBase::OnLoad();

    if( m_pRenderGraphObject == 0 )
        AddToRenderGraph();
}

void ComponentParticleEmitter::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    ComponentRenderable::SetMaterial( pMaterial, submeshindex );

    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterial );
    m_pMaterial = pMaterial;

    m_pParticleRenderer->SetMaterial( m_pMaterial );

    if( m_pRenderGraphObject != 0 )
    {
        m_pRenderGraphObject->SetMaterial( pMaterial, true );
    }
}

void ComponentParticleEmitter::SetVisible(bool visible)
{
    ComponentRenderable::SetVisible( visible );
}

void ComponentParticleEmitter::AddToRenderGraph()
{
    MyAssert( m_pRenderGraphObject == 0 );
    MyAssert( m_pParticleRenderer );

    // Add the particle renderer (submesh) to the main scene graph
    m_pRenderGraphObject = g_pComponentSystemManager->AddSubmeshToRenderGraph( this, m_pParticleRenderer, m_pMaterial, MyRE::PrimitiveType_Triangles, 1, m_LayersThisExistsOn );

    // Force particle submesh to be rendered during transparent pass for now.
    m_pRenderGraphObject->SetFlags( RenderGraphFlag_Transparent );
}

void ComponentParticleEmitter::RemoveFromRenderGraph()
{
    if( m_pRenderGraphObject != 0 )
    {
        g_pComponentSystemManager->RemoveObjectFromRenderGraph( m_pRenderGraphObject );
        m_pRenderGraphObject = 0;
    }
}

void ComponentParticleEmitter::PushChangesToRenderGraphObjects()
{
    //ComponentRenderable::PushChangesToRenderGraphObjects(); // pure virtual

    // Sync RenderGraph object
    if( m_pRenderGraphObject )
    {
        // TODO: Check if opaque or transparent
        // Don't bother updating flags when setting material, it's forced to transparent for now.
        m_pRenderGraphObject->SetMaterial( this->GetMaterial( 0 ), false );
        m_pRenderGraphObject->SetFlags( RenderGraphFlag_Transparent );
        m_pRenderGraphObject->m_Layers = this->m_LayersThisExistsOn;
        m_pRenderGraphObject->m_Visible = this->m_Visible;

        //m_pRenderGraphObject->m_GLPrimitiveType = this->m_GLPrimitiveType;
        //m_pRenderGraphObject->m_PointSize = this->m_PointSize;
    }
}

// Exposed to Lua, change elsewhere if function signature changes.
void ComponentParticleEmitter::CreateBurst(int number, Vector3 offset)
{
    while( number )
    {
        number--;

        Particle* pParticle = m_Particles.MakeObjectActive();
        if( pParticle )
        {
            Vector3 emitterpos = m_pComponentTransform->GetWorldPosition();

            pParticle->pos = emitterpos + m_InitialOffset;
            if( m_CenterVariation.x != 0 )
                pParticle->pos.x += (rand()%(int)(m_CenterVariation.x*10000))/10000.0f - m_CenterVariation.x/2;
            if( m_CenterVariation.y != 0 )
                pParticle->pos.y += (rand()%(int)(m_CenterVariation.y*10000))/10000.0f - m_CenterVariation.y/2;
            pParticle->size = m_Size + (rand()%10000)/10000.0f * m_SizeVariation;

            if( m_UseColorsAsOptions )
            {
                int color = rand()%2;
                if( color == 0 )
                    pParticle->color = m_Color1;
                else //if( color == 1 )
                    pParticle->color = m_Color2;
            }
            else
            {
                pParticle->color = ColorFloat( rand()%255/255.0f, rand()%255/255.0f, rand()%255/255.0f, 255/255.0f );
            }

            pParticle->dir.x = m_Dir.x + m_DirVariation.x * (rand()%10000-5000)/10000.0f;
            pParticle->dir.y = m_Dir.y + m_DirVariation.y * (rand()%10000-5000)/10000.0f;
            pParticle->dir.z = m_Dir.z + m_DirVariation.z * (rand()%10000-5000)/10000.0f;
            pParticle->timealive = 0;
            pParticle->timetolive = m_TimeToLive;
            if( m_TimeToLiveVariation != 0 )
                pParticle->timetolive += (rand()%10000-5000)/10000.0f * m_TimeToLiveVariation;

            //if( m_InitialSpeedBoost )
            //if( 0 )
            //{
            //    pParticle->pos.x += pParticle->dir.x * m_InitialSpeedBoost;
            //    pParticle->pos.y += pParticle->dir.y * m_InitialSpeedBoost;
            //}
        }
    }
}

void ComponentParticleEmitter::TickCallback(float deltaTime)
{
#if MYFW_USING_WX
    if( m_RunInEditor && g_pEngineCore->IsInEditorMode() )
        deltaTime = g_pGameCore->GetTimePassedUnpausedLastFrame();
#endif

    // TODO: if we want to share particle renderers, then don't reset like this.
    m_pParticleRenderer->Reset();

    // rebuild the quad to face the camera each frame. // TODO: don't rebuild if billboarding is disabled.
    if( m_BillboardSprites )
    {
        // Get the camera rotation vector, use the editor camera if in editor mode.
        Vector3 camrot( 0, 0, 0 );
        ComponentCamera* pCamera = g_pComponentSystemManager->GetFirstCamera( true );
        if( pCamera )
            camrot = pCamera->m_pComponentTransform->GetLocalRotation();

        MyMatrix matcamrot;
        matcamrot.CreateSRT( Vector3(1), camrot, Vector3(0) );

        m_pParticleRenderer->RebuildParticleQuad( &matcamrot );
    }
    else
    {
        m_pParticleRenderer->RebuildParticleQuad( 0 );
    }

    //m_TimeAlive += deltaTime;

    for( unsigned int i=0; i<m_Particles.m_ActiveObjects.Count(); i++ )
    {
        Particle* pParticle = m_Particles.m_ActiveObjects[i];
        pParticle->pos += pParticle->dir * deltaTime;
        
        pParticle->timealive += deltaTime;

        float perc = pParticle->timealive / pParticle->timetolive;
        MyClamp( perc, 0.0f, 1.0f );
        //float antiperc = 1 - perc;

        ColorByte color;
        float tempperc = (perc - m_ColorTransitionDelay) / (1 - m_ColorTransitionDelay);
        MyClamp( tempperc, 0.0f, 1.0f );
        color.r = (unsigned char)((pParticle->color.r + pParticle->color.r * m_ColorTransitionSpeed * tempperc) * 255);
        color.g = (unsigned char)((pParticle->color.g + pParticle->color.g * m_ColorTransitionSpeed * tempperc) * 255);
        color.b = (unsigned char)((pParticle->color.b + pParticle->color.b * m_ColorTransitionSpeed * tempperc) * 255);
        color.a = (unsigned char)((pParticle->color.a + pParticle->color.a * m_ColorTransitionSpeed * tempperc) * 255);

        color.a = (unsigned char)(color.a * m_AlphaModifier);

        float size = pParticle->size + pParticle->size * m_ScaleSpeed * perc;

        if( pParticle->timealive > pParticle->timetolive )
        {
            m_Particles.MakeObjectInactiveByIndex( i );
            i--;
        }

        if( size > 0 )
        {
            m_pParticleRenderer->AddPoint( pParticle->pos, 0, color, size );
        }
    }

    if( m_ContinuousSpawn || m_RunInEditor )
        m_TimeTilNextSpawn -= deltaTime;

    if( m_TimeTilNextSpawn < 0 )
    {
        CreateBurst( 1, Vector3(0,0,0) );
        m_TimeTilNextSpawn = m_SpawnTime;
    }
}

void ComponentParticleEmitter::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    // TODO: Particles don't support shader overrides.
    if( pShaderOverride != 0 )
        return;

    ComponentRenderable::Draw( pMatProj, pMatView, pShaderOverride, 0 );

    if( m_pMaterial == 0 )
        return;

    m_pParticleRenderer->SetMaterial( m_pMaterial );

    Vector3 campos( 0, 0, 0 );
    Vector3 camrot( 0, 0, 0 );
    if( pCamera )
    {
        campos = pCamera->m_pComponentTransform->GetLocalPosition();
        camrot = pCamera->m_pComponentTransform->GetLocalRotation();
    }
    else
    {
    }

    m_pParticleRenderer->DrawParticles( campos, camrot, pMatProj, pMatView, pShaderOverride );
}

#if MYFW_EDITOR
ComponentVariable* ComponentParticleEmitter::GetComponentVariableForMaterial(int submeshindex)
{
    return 0; //FindComponentVariableByLabel( &m_ComponentVariableList_ComponentParticleEmitter, "Material" );
}

#if MYFW_USING_WX
void ComponentParticleEmitter::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentParticleEmitter::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Particle Emitter", ObjectListIcon_Component );
}

void ComponentParticleEmitter::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentParticleEmitter::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Particle Emitter", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        if( addcomponentvariables )
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        g_pPanelWatch->AddBool( "Run in editor", &m_RunInEditor, 0, 1 );

        g_pPanelWatch->AddBool( "Spawn on timer", &m_ContinuousSpawn, 0, 1 );

        g_pPanelWatch->AddBool( "Billboard", &m_BillboardSprites, 0, 1 );

        g_pPanelWatch->AddVector3( "Offset", &m_InitialOffset, 0, 0 );
        g_pPanelWatch->AddFloat( "size", &m_Size, 0, 0 );
        g_pPanelWatch->AddFloat( "sizevariation", &m_SizeVariation, 0, 0 );
        g_pPanelWatch->AddFloat( "timetolive", &m_TimeToLive, 0.01f, 5000 );
        g_pPanelWatch->AddVector2( "center variation", &m_CenterVariation, 0, 20 );
        g_pPanelWatch->AddVector3( "dir", &m_Dir, -100, 100 );
        g_pPanelWatch->AddVector3( "dirvariation", &m_DirVariation, -100, 100 );

        g_pPanelWatch->AddBool( "usecolorsasoptions", &m_UseColorsAsOptions, 0, 1 );

        g_pPanelWatch->AddColorFloat( "color1", &m_Color1, 0, 1 );
        g_pPanelWatch->AddColorFloat( "color2", &m_Color2, 0, 1 );

        const char* desc = "no material";
        if( m_pMaterial && m_pMaterial->GetFile() )
            desc = m_pMaterial->GetMaterialShortDescription();
        g_pPanelWatch->AddPointerWithDescription( "Material", 0, desc, this, ComponentParticleEmitter::StaticOnDropMaterial );
    }
}

void ComponentParticleEmitter::OnDropMaterial(int controlid, int x, int y)
{
    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pDropItem->m_Value;
        MyAssert( pMaterial );

        SetMaterial( pMaterial, 0 );

        // update the panel so new Material name shows up.
        if( pMaterial->GetFile() )
            g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.GetControlID() )->m_Description = pMaterial->GetMaterialShortDescription();
    }
}
#endif //MYFW_USING_WX
#endif //MYFW_EDITOR