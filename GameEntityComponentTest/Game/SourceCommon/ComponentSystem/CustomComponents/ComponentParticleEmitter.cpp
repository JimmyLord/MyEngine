//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "GameCommonHeader.h"

bool ComponentParticleEmitter::m_PanelWatchBlockVisible = true;

ComponentParticleEmitter::ComponentParticleEmitter()
: ComponentRenderable()
{
    m_BaseType = BaseComponentType_Renderable;

    m_Particles.AllocateObjects( 1000 );
    for( unsigned int i=0; i<m_Particles.Length(); i++ )
    {
        Particle* pObj = MyNew Particle;
        m_Particles.AddInactiveObject( pObj );
    }

    m_pParticleRenderer = MyNew ParticleRenderer();
    m_pParticleRenderer->AllocateVertices( 1000, "Particle Renderer" );

    g_pComponentSystemManager->RegisterComponentTickCallback( &StaticTick, this );

    m_pShaderGroup = 0;
    m_pTexture = 0;
}

ComponentParticleEmitter::~ComponentParticleEmitter()
{
    m_Particles.DeleteAllObjectsInPool();

    SAFE_DELETE( m_pParticleRenderer );

    SAFE_RELEASE( m_pShaderGroup );
    SAFE_RELEASE( m_pTexture );

    g_pComponentSystemManager->UnregisterComponentTickCallback( &StaticTick, this );
}

void ComponentParticleEmitter::Reset()
{
    ComponentRenderable::Reset();

    SAFE_RELEASE( m_pShaderGroup );
    SAFE_RELEASE( m_pTexture );

    m_TimeTilNextSpawn = 0;

    m_BurstDuration = 100;

    m_BurstTimeLeft = m_BurstDuration;
    
    m_SpawnTime = 0.1f;
    m_SpawnTimeVariation = 0.1f;
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

#if MYFW_USING_WX
void ComponentParticleEmitter::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    wxTreeItemId id = g_pPanelObjectList->AddObject( this, ComponentParticleEmitter::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Particle Emitter" );
}

void ComponentParticleEmitter::OnLeftClick(bool clear)
{
    ComponentBase::OnLeftClick( clear );
}

void ComponentParticleEmitter::FillPropertiesWindow(bool clear)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Particle Emitter", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        g_pPanelWatch->AddFloat( "size", &m_Size, 0, 1 );
        g_pPanelWatch->AddFloat( "sizevariation", &m_SizeVariation, 0, 1 );
        g_pPanelWatch->AddFloat( "timetolive", &m_TimeToLive, 0.01f, 5 );
        g_pPanelWatch->AddVector3( "dir", &m_Dir, -1, 1 );
        g_pPanelWatch->AddVector3( "dirvariation", &m_DirVariation, 0, 1 );

        g_pPanelWatch->AddBool( "usecolorsasoptions", &m_UseColorsAsOptions, 0, 1 );

        g_pPanelWatch->AddColorFloat( "color1", &m_Color1, 0, 1 );
        g_pPanelWatch->AddColorFloat( "color2", &m_Color2, 0, 1 );

        const char* desc = "no shader";
        if( m_pShaderGroup && m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile )
            desc = m_pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
        g_pPanelWatch->AddPointerWithDescription( "Shader", 0, desc, this, ComponentParticleEmitter::StaticOnDropShader );

        desc = "no texture";
        if( m_pTexture )
            desc = m_pTexture->m_Filename;
        g_pPanelWatch->AddPointerWithDescription( "Texture", 0, desc, this, ComponentParticleEmitter::StaticOnDropTexture );
    }
}

void ComponentParticleEmitter::OnDropShader()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_ShaderGroupPointer )
    {
        ShaderGroup* pShaderGroup = (ShaderGroup*)g_DragAndDropStruct.m_Value;
        assert( pShaderGroup );

        SetShader( pShaderGroup );

        // update the panel so new Shader name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pShaderGroup->GetShader( ShaderPass_Main )->m_pFile->m_FilenameWithoutExtension;
    }
}

void ComponentParticleEmitter::OnDropTexture()
{
    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        assert( pFile );

        int len = strlen( pFile->m_FullPath );
        const char* filenameext = &pFile->m_FullPath[len-4];

        if( strcmp( filenameext, ".png" ) == 0 )
        {
            TextureDefinition* pOldTexture = m_pTexture;
            m_pTexture = g_pTextureManager->FindTexture( pFile->m_FullPath );
            SAFE_RELEASE( pOldTexture );
        }

        // update the panel so new Shader name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pTexture->m_Filename;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_TextureDefinitionPointer )
    {
        TextureDefinition* pOldTexture = m_pTexture;
        m_pTexture = (TextureDefinition*)g_DragAndDropStruct.m_Value;
        SAFE_RELEASE( pOldTexture );

        // update the panel so new Shader name shows up.
        g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = m_pTexture->m_Filename;
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentParticleEmitter::ExportAsJSONObject()
{
    cJSON* component = ComponentRenderable::ExportAsJSONObject();

    cJSON_AddNumberToObject( component, "size", m_Size );
    cJSON_AddNumberToObject( component, "sizevar", m_SizeVariation );
    cJSON_AddNumberToObject( component, "timetolive", m_TimeToLive );
    cJSONExt_AddFloatArrayToObject( component, "dir", &m_Dir.x, 3 );
    cJSONExt_AddFloatArrayToObject( component, "dirvar", &m_DirVariation.x, 3 );

    cJSON_AddNumberToObject( component, "coloroption", m_UseColorsAsOptions );

    cJSONExt_AddFloatArrayToObject( component, "color1", &m_Color1.r, 4 );
    cJSONExt_AddFloatArrayToObject( component, "color2", &m_Color2.r, 4 );

    if( m_pShaderGroup )
        cJSON_AddStringToObject( component, "Shader", m_pShaderGroup->GetName() );
    if( m_pTexture )
        cJSON_AddStringToObject( component, "Texture", m_pTexture->m_Filename );

    return component;
}

void ComponentParticleEmitter::ImportFromJSONObject(cJSON* jsonobj, unsigned int sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jsonobj, sceneid );

    cJSONExt_GetFloat( jsonobj, "size", &m_Size );
    cJSONExt_GetFloat( jsonobj, "sizevar", &m_SizeVariation );
    cJSONExt_GetFloat( jsonobj, "timetolive", &m_TimeToLive );
    cJSONExt_GetFloatArray( jsonobj, "dir", &m_Dir.x, 3 );
    cJSONExt_GetFloatArray( jsonobj, "dirvar", &m_DirVariation.x, 3 );

    cJSONExt_GetBool( jsonobj, "coloroption", &m_UseColorsAsOptions );

    cJSONExt_GetFloatArray( jsonobj, "color1", &m_Color1.r, 4 );
    cJSONExt_GetFloatArray( jsonobj, "color2", &m_Color2.r, 4 );

    cJSON* shaderstringobj = cJSON_GetObjectItem( jsonobj, "Shader" );
    if( shaderstringobj )
    {
        ShaderGroup* pShaderGroup = g_pShaderGroupManager->FindShaderGroupByName( shaderstringobj->valuestring );
        if( pShaderGroup )
        {
            pShaderGroup->AddRef();
            SAFE_RELEASE( m_pShaderGroup );
            m_pShaderGroup = pShaderGroup;
        }
    }

    cJSON* texturestringobj = cJSON_GetObjectItem( jsonobj, "Texture" );
    if( texturestringobj )
    {
        TextureDefinition* pTexture = g_pTextureManager->FindTexture( texturestringobj->valuestring );
        if( pTexture )
        {
            pTexture->AddRef();
            SAFE_RELEASE( m_pTexture );
            m_pTexture = pTexture;
        }
    }
}

ComponentParticleEmitter& ComponentParticleEmitter::operator=(const ComponentParticleEmitter& other)
{
    assert( &other != this );

    ComponentRenderable::operator=( other );

    this->m_Size                = other.m_Size;
    this->m_SizeVariation       = other.m_SizeVariation;
    this->m_TimeToLive          = other.m_TimeToLive;
    this->m_Dir                 = other.m_Dir;
    this->m_DirVariation        = other.m_DirVariation;

    this->m_UseColorsAsOptions  = other.m_UseColorsAsOptions;

    this->m_Color1              = other.m_Color1;
    this->m_Color2              = other.m_Color2;

    m_pShaderGroup = other.m_pShaderGroup;
    if( m_pShaderGroup )
        m_pShaderGroup->AddRef();

    m_pTexture = other.m_pTexture;
    if( m_pTexture )
        m_pTexture->AddRef();

    return *this;
}

void ComponentParticleEmitter::SetShader(ShaderGroup* pShader)
{
    ComponentRenderable::SetShader( pShader );

    pShader->AddRef();
    SAFE_RELEASE( m_pShaderGroup );
    m_pShaderGroup = pShader;

    m_pParticleRenderer->SetShaderAndTexture( m_pShaderGroup, m_pTexture );
}

void ComponentParticleEmitter::CreateBurst(int number, Vector3 pos)
{
    while( number )
    {
        number--;

        Particle* pParticle = m_Particles.MakeObjectActive();
        if( pParticle )
        {
            pParticle->pos = pos;
            if( m_CenterVariation.x != 0 )
                pParticle->pos.x += (rand()%(int)m_CenterVariation.x - m_CenterVariation.x/2);
            if( m_CenterVariation.y != 0 )
                pParticle->pos.y += (rand()%(int)m_CenterVariation.y - m_CenterVariation.y/2);
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
            {
                pParticle->pos.x += pParticle->dir.x * m_InitialSpeedBoost;
                pParticle->pos.y += pParticle->dir.y * m_InitialSpeedBoost;
            }
        }
    }
}

void ComponentParticleEmitter::Tick(double TimePassed)
{
    // TODO: if we want to share particle renderers, then don't reset like this.
    m_pParticleRenderer->Reset();

    //m_TimeAlive += TimePassed;

    for( unsigned int i=0; i<m_Particles.m_ActiveObjects.Count(); i++ )
    {
        Particle* pParticle = m_Particles.m_ActiveObjects[i];
        pParticle->pos += pParticle->dir * (float)TimePassed;
        
        pParticle->timealive += (float)TimePassed;

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
            m_pParticleRenderer->AddPoint( pParticle->pos, 0, color, size );
    }
}

void ComponentParticleEmitter::Draw(MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride, int drawcount)
{
    ComponentRenderable::Draw( pMatViewProj, pShaderOverride, drawcount );

    Vector3 pos = m_pComponentTransform->GetPosition();
    CreateBurst( 1, pos );
    m_pParticleRenderer->SetShaderAndTexture( m_pShaderGroup, m_pTexture );
    m_pParticleRenderer->Draw( pMatViewProj );
}
