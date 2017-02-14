//
// Copyright (c) 2014-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#include "../../../Framework/MyFramework/SourceCommon/SceneGraphs/SceneGraph_Base.h"
#include "../../../Framework/MyFramework/SourceCommon/SceneGraphs/SceneGraph_Flat.h"

#if MYFW_USING_WX
bool ComponentMesh::m_PanelWatchBlockVisible = true;
#endif

static const char* g_MaterialLabels[] = { "Material1", "Material2", "Material3", "Material4" };

const char* OpenGLPrimitiveTypeStrings[7] =
{
    "Points",
    "Lines",
    "LineLoop",
    "LineStrip",
    "Triangles",
    "TriangleStrip",
    "TriangleFan",
};

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentMesh ); //_VARIABLE_LIST

ComponentMesh::ComponentMesh()
: ComponentRenderable()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_WaitingToAddToSceneGraph = false;

    m_pMesh = 0;
    for( int i=0; i<MAX_SUBMESHES; i++ )
    {
        m_pSceneGraphObjects[i] = 0;
        m_MaterialList[i] = 0;
    }

    m_GLPrimitiveType = GL_TRIANGLES;
    m_PointSize = 1;

    g_pEventManager->RegisterForEvents( Event_ShaderFinishedLoading, this, &ComponentMesh::StaticOnEvent );
}

ComponentMesh::~ComponentMesh()
{
    g_pEventManager->UnregisterForEvents( Event_ShaderFinishedLoading, this, &ComponentMesh::StaticOnEvent );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    SAFE_RELEASE( m_pMesh );
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        if( m_pSceneGraphObjects[i] != 0 )
            g_pComponentSystemManager->RemoveObjectFromSceneGraph( m_pSceneGraphObjects[i] );
        m_pSceneGraphObjects[i] = 0;
        SAFE_RELEASE( m_MaterialList[i] );
    }
}

void ComponentMesh::RegisterVariables(CPPListHead* pList, ComponentMesh* pThis) //_VARIABLE_LIST
{
    ComponentRenderable::RegisterVariables( pList, pThis );

    for( int i=0; i<MAX_SUBMESHES; i++ )
    {
        // materials are not automatically saved/loaded
        MyAssert( MAX_SUBMESHES == 4 );
        ComponentVariable* pVar = AddVar( pList, g_MaterialLabels[i], ComponentVariableType_MaterialPtr,
                                               MyOffsetOf( pThis, &pThis->m_MaterialList[i] ), false, true, 
                                               0, (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, (CVarFunc_DropTarget)&ComponentMesh::OnDropMaterial, 0 );

#if MYFW_USING_WX
        pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentMesh::ShouldVariableBeAddedToWatchPanel) );
#endif
    }

    AddVarEnum( pList, "PrimitiveType", MyOffsetOf( pThis, &pThis->m_GLPrimitiveType ),  true,  true, "Primitive Type", 7, OpenGLPrimitiveTypeStrings, (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, 0, 0 );
    AddVar( pList, "PointSize", ComponentVariableType_Int, MyOffsetOf( pThis, &pThis->m_PointSize ),  true,  true, "Point Size", (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, 0, 0 );
}

void ComponentMesh::Reset()
{
    ComponentRenderable::Reset();

    SAFE_RELEASE( m_pMesh );
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
        SAFE_RELEASE( m_MaterialList[i] );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentMesh::LuaRegister(lua_State* luastate)
{
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void ComponentMesh::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    MyAssert( gameobjectid.IsOk() );
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentMesh::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Mesh", ObjectListIcon_Component );
}

void ComponentMesh::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentMesh::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    //m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Mesh", this, ComponentBase::StaticOnComponentTitleLabelClicked );
    //MyAssert( m_pMesh );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        if( addcomponentvariables )
        {
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
        }
    }
}

bool ComponentMesh::ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar)
{
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        // only show enough material variables for the number of submeshes in the mesh.
        if( pVar->m_Label == g_MaterialLabels[i] )
        {
            if( m_pMesh == 0 || i >= m_pMesh->m_SubmeshList.Count() )
                return false;
        }
    }

    return true;
}

void* ComponentMesh::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( finishedchanging )
    {
        if( strncmp( pVar->m_Label, "Material", strlen("Material") ) == 0 )
        {
            int materialthatchanged = -1;
            for( int i=0; i<MAX_SUBMESHES; i++ )
            {
                if( pVar->m_Label == g_MaterialLabels[i] )
                    materialthatchanged = i;
            }

            MyAssert( pVar->m_ControlID != -1 );

            wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->m_Handle_TextCtrl->GetValue();
            if( text == "" || text == "none" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

                oldpointer = GetMaterial( materialthatchanged );
                SetMaterial( 0, materialthatchanged );
            }
        }

        //if( controlid == m_ControlID_PrimitiveType )
        {
            g_pPanelWatch->SetNeedsRefresh();
        }

        PushChangesToSceneGraphObjects();
    }

    return oldpointer;
}

void* ComponentMesh::OnDropMaterial(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        int materialthatchanged = -1;
        for( int i=0; i<MAX_SUBMESHES; i++ )
        {
            if( pVar->m_Label == g_MaterialLabels[i] )
            {
                materialthatchanged = i;
                break;
            }
        }

        MyAssert( materialthatchanged != -1 );
        if( materialthatchanged != -1 )
        {
            MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;
            MyAssert( pMaterial );

            oldvalue = GetMaterial( materialthatchanged );
            g_pEngineMainFrame->m_pCommandStack->Do( MyNew EditorCommand_ChangeMaterialOnMesh( this, materialthatchanged, pMaterial ) );

            // update the panel so new Material name shows up.
            g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.m_ID )->m_Description = pMaterial->GetName();
        }
    }

    return oldvalue;
}
#endif //MYFW_USING_WX

cJSON* ComponentMesh::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jComponent = ComponentRenderable::ExportAsJSONObject( savesceneid );

    cJSON* jMaterialArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jComponent, "Materials", jMaterialArray );

    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        MyAssert( m_MaterialList[i] == 0 || m_MaterialList[i]->GetFile() ); // new materials should be saved as files before the state is saved.

        cJSON* jMaterial = 0;
        if( m_MaterialList[i] && m_MaterialList[i]->GetFile() )
            jMaterial = cJSON_CreateString( m_MaterialList[i]->GetMaterialDescription() );

        if( jMaterial )
            cJSON_AddItemToArray( jMaterialArray, jMaterial );
    }

    //cJSON_AddNumberToObject( jComponent, "PrimitiveType", m_GLPrimitiveType );
    //cJSON_AddNumberToObject( jComponent, "PointSize", m_PointSize );
    
    // called in ComponentBase::ExportAsJSONObject
    //ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST

    return jComponent;
}

void ComponentMesh::ImportFromJSONObject(cJSON* jComponentMesh, unsigned int sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jComponentMesh, sceneid );

    // TODO: remove this "Material" block, it's for old scenes before I changed to multiple materials.
    cJSON* jMaterial = cJSON_GetObjectItem( jComponentMesh, "Material" );
    if( jMaterial )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMaterial->valuestring );
        if( pMaterial )
        {
            SetMaterial( pMaterial, 0 );
            pMaterial->Release();
        }
    }

    cJSON* jMaterialArray = cJSON_GetObjectItem( jComponentMesh, "Materials" );
    if( jMaterialArray )
    {
        int nummaterials = cJSON_GetArraySize( jMaterialArray );

        //for( int i=0; i<MAX_SUBMESHES; i++ ) { MyAssert( m_MaterialList[i] == 0 ); }

        for( int i=0; i<nummaterials; i++ )
        {
            cJSON* jMaterial = cJSON_GetArrayItem( jMaterialArray, i );
            MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMaterial->valuestring );
            if( pMaterial )
            {
                SetMaterial( pMaterial, i );
                pMaterial->Release();
            }
        }
    }

    //cJSONExt_GetInt( jComponentMesh, "PrimitiveType", &m_GLPrimitiveType );
    //cJSONExt_GetInt( jComponentMesh, "PointSize", &m_PointSize );

    ImportVariablesFromJSON( jComponentMesh ); //_VARIABLE_LIST
}

ComponentMesh& ComponentMesh::operator=(const ComponentMesh& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    //const ComponentMesh* pOther = &other;
    //MyAssert( other.m_MaterialList.Count() == m_MaterialList.Count() );
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        SetMaterial( other.m_MaterialList[i], i );
    }

    m_GLPrimitiveType = other.m_GLPrimitiveType;
    m_PointSize = other.m_PointSize;

    SetMesh( other.m_pMesh );

    return *this;
}

void ComponentMesh::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnSurfaceChanged );
        MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentMesh, Draw ); //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnFileRenamed );
    }
}

void ComponentMesh::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

bool ComponentMesh::OnEvent(MyEvent* pEvent)
{
    // Testing: when any material finishes loading, set the material again to fix the flags of the scenegraphobject
    if( pEvent->GetType() == Event_MaterialFinishedLoading ||
        pEvent->GetType() == Event_ShaderFinishedLoading )
    {
        for( int i=0; i<MAX_SUBMESHES; i++ )
        {
            if( m_MaterialList[i] )
            {
                SetMaterial( m_MaterialList[i], i );
            }
        }
        return false; // keep propagating the event.
    }

    return false;
}

void ComponentMesh::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    ComponentRenderable::SetMaterial( pMaterial, submeshindex );

    MyAssert( submeshindex >= 0 && submeshindex < MAX_SUBMESHES );

    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_MaterialList[submeshindex] );
    m_MaterialList[submeshindex] = pMaterial;

    if( m_pSceneGraphObjects[submeshindex] )
    {
        // TODO: clean this mess
        // clear opaque/transparent flags, set the proper flag based on new material transparency
        SceneGraphFlags flags = m_pSceneGraphObjects[submeshindex]->m_Flags;
        flags = (SceneGraphFlags)(flags & ~(SceneGraphFlag_Opaque | SceneGraphFlag_Transparent));
        if( pMaterial )
        {
            if( m_MaterialList[submeshindex]->IsTransparent() )
                flags = (SceneGraphFlags)(flags | SceneGraphFlag_Transparent);
            else
                flags = (SceneGraphFlags)(flags | SceneGraphFlag_Opaque);
        }

        m_pSceneGraphObjects[submeshindex]->m_pMaterial = pMaterial;
        m_pSceneGraphObjects[submeshindex]->m_Flags = flags;
    }
}

void ComponentMesh::SetVisible(bool visible)
{
    ComponentRenderable::SetVisible( visible );

    for( int i=0; i<MAX_SUBMESHES; i++ )
    {
        if( m_pSceneGraphObjects[i] )
        {
            m_pSceneGraphObjects[i]->m_Visible = visible;
        }
    }
}

//bool ComponentMesh::IsVisible()
//{
//    return ComponentRenderable::IsVisible;
//}

void ComponentMesh::SetMesh(MyMesh* pMesh)
{
    if( m_pMesh == pMesh )
        return;

    if( pMesh )
        pMesh->AddRef();

    if( m_pMesh )
        RemoveFromSceneGraph();

    SAFE_RELEASE( m_pMesh );
    m_pMesh = pMesh;

    if( m_pMesh )
        AddToSceneGraph();
}

bool ComponentMesh::IsMeshReady()
{
    return m_pMesh->m_MeshReady;
}

void ComponentMesh::MeshFinishedLoading()
{
}

void ComponentMesh::AddToSceneGraph()
{
    MyAssert( m_pMesh );

    if( m_pMesh->m_MeshReady )
    {
        MyAssert( m_pMesh->m_SubmeshList.Count() > 0 );
        MyAssert( m_pSceneGraphObjects[0] == 0 );

        // Add the Mesh to the main scene graph
        if( m_pMesh->m_SubmeshList.Count() > 0 )
        {
            SceneGraphFlags flags = SceneGraphFlag_Opaque; // TODO: check if opaque or transparent
            unsigned int layers = m_LayersThisExistsOn;
            g_pComponentSystemManager->AddMeshToSceneGraph( this, m_pMesh, m_MaterialList, m_GLPrimitiveType, m_PointSize, flags, layers, m_pSceneGraphObjects );
        }

        m_WaitingToAddToSceneGraph = false;

        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
    }
    else if( m_WaitingToAddToSceneGraph == false )
    {
        m_WaitingToAddToSceneGraph = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, Tick );
    }
}

void ComponentMesh::RemoveFromSceneGraph()
{
    if( m_pMesh == 0 )
        return;

    if( m_WaitingToAddToSceneGraph )
    {
        m_WaitingToAddToSceneGraph = false;
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        return;
    }

    MyAssert( m_pMesh );
    //MyAssert( m_pMesh->m_SubmeshList.Count() > 0 );

    for( unsigned int i=0; i<m_pMesh->m_SubmeshList.Count(); i++ )
    {
        if( m_pSceneGraphObjects[i] != 0 )
        {
            g_pComponentSystemManager->RemoveObjectFromSceneGraph( m_pSceneGraphObjects[i] );
            m_pSceneGraphObjects[i] = 0;
        }
    }
}

void ComponentMesh::PushChangesToSceneGraphObjects()
{
    //ComponentRenderable::PushChangesToSceneGraphObjects(); // pure virtual

    // Sync scenegraph objects
    for( int i=0; i<MAX_SUBMESHES; i++ )
    {
        if( m_pSceneGraphObjects[i] )
        {
            m_pSceneGraphObjects[i]->m_Flags = SceneGraphFlag_Opaque; // TODO: check if opaque or transparent
            m_pSceneGraphObjects[i]->m_Layers = this->m_LayersThisExistsOn;

            m_pSceneGraphObjects[i]->m_pMaterial = this->GetMaterial( i );
            m_pSceneGraphObjects[i]->m_Visible = this->m_Visible;

            m_pSceneGraphObjects[i]->m_GLPrimitiveType = this->m_GLPrimitiveType;
            m_pSceneGraphObjects[i]->m_PointSize = this->m_PointSize;
        }
    }
}

MyAABounds* ComponentMesh::GetBounds()
{
    if( m_pMesh )
        return m_pMesh->GetBounds();

    return 0;
}

void ComponentMesh::TickCallback(double TimePassed)
{
    // TODO: temp hack, if the gameobject doesn't have a transform (shouldn't happen), then don't try to add to scene graph
    if( m_pGameObject->m_pComponentTransform == 0 )
        return;

    MyAssert( m_pGameObject->m_pComponentTransform );
    MyAssert( m_pMesh );
    MyAssert( m_WaitingToAddToSceneGraph );

    if( IsMeshReady() )
    {
        MeshFinishedLoading();

        // AddToSceneGraph() will stop tick callbacks.
        AddToSceneGraph();
    }
}

void ComponentMesh::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    checkGlError( "start of ComponentMesh::DrawCallback()" );

    ComponentRenderable::Draw( pMatViewProj, pShaderOverride, 0 );

    if( m_pMesh )
    {
        MyMatrix worldtransform = *m_pComponentTransform->GetWorldTransform();

        // simple frustum check
        {
            MyAABounds* bounds = m_pMesh->GetBounds();
            Vector3 center = bounds->GetCenter();
            Vector3 half = bounds->GetHalfSize();

            MyMatrix wvp = *pMatViewProj * worldtransform;

            Vector4 clippos[8];

            // transform AABB extents into clip space.
            clippos[0] = wvp * Vector4(center.x - half.x, center.y - half.y, center.z - half.z, 1);
            clippos[1] = wvp * Vector4(center.x - half.x, center.y - half.y, center.z + half.z, 1);
            clippos[2] = wvp * Vector4(center.x - half.x, center.y + half.y, center.z - half.z, 1);
            clippos[3] = wvp * Vector4(center.x - half.x, center.y + half.y, center.z + half.z, 1);
            clippos[4] = wvp * Vector4(center.x + half.x, center.y - half.y, center.z - half.z, 1);
            clippos[5] = wvp * Vector4(center.x + half.x, center.y - half.y, center.z + half.z, 1);
            clippos[6] = wvp * Vector4(center.x + half.x, center.y + half.y, center.z - half.z, 1);
            clippos[7] = wvp * Vector4(center.x + half.x, center.y + half.y, center.z + half.z, 1);

            // check visibility two planes at a time
            bool visible;
            for( int component=0; component<3; component++ ) // loop through x/y/z
            {
                // check if all 8 points are less than the -w extent of it's axis
                visible = false;
                for( int i=0; i<8; i++ )
                {
                    if( clippos[i][component] >= -clippos[i].w )
                    {
                        visible = true; // this point is on the visible side of the plane, skip to next plane
                        break;
                    }
                }
                if( visible == false ) // all points are on outside of plane, don't draw object
                    break;

                // check if all 8 points are greater than the -w extent of it's axis
                visible = false;
                for( int i=0; i<8; i++ )
                {
                    if( clippos[i][component] <= clippos[i].w )
                    {
                        visible = true; // this point is on the visible side of the plane, skip to next plane
                        break;
                    }
                }
                if( visible == false ) // all points are on outside of plane, don't draw object
                    break;
            }

            // if all points are on outside of frustum, don't draw mesh.
            if( visible == false )
                return;
        }

        for( unsigned int i=0; i<m_pMesh->m_SubmeshList.Count(); i++ )
        {
            m_pMesh->SetMaterial( m_MaterialList[i], i );
            m_pMesh->m_SubmeshList[i]->m_PrimitiveType = m_GLPrimitiveType;
            m_pMesh->m_SubmeshList[i]->m_PointSize = m_PointSize;
        }

        //m_pMesh->SetTransform( worldtransform );

        // Find nearest lights.
        MyLight* lights[4];
        int numlights = g_pLightManager->FindNearestLights( LightType_Point, 4, m_pComponentTransform->GetWorldTransform()->GetTranslation(), lights );

        // Find nearest shadow casting light.
        MyMatrix* pShadowVP = 0;
        TextureDefinition* pShadowTex = 0;
        if( g_ActiveShaderPass == ShaderPass_Main )
        {
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByName( "Shadow Light" );
            if( pObject )
            {
                ComponentBase* pComponent = pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera );
                ComponentCameraShadow* pShadowCam = pComponent->IsA( "CameraShadowComponent" ) ? (ComponentCameraShadow*)pComponent : 0;
                if( pShadowCam )
                {
                    pShadowVP = &pShadowCam->m_matViewProj;
#if 1
                    pShadowTex = pShadowCam->m_pDepthFBO->m_pDepthTexture;
#else
                    pShadowTex = pShadowCam->m_pDepthFBO->m_pColorTexture;
#endif
                }
            }
        }

        Vector3 campos;
        Vector3 camrot;
#if MYFW_USING_WX
        if( g_pEngineCore->m_EditorMode )
        {
            ComponentCamera* pCamera = g_pEngineCore->m_pEditorState->GetEditorCamera();

            campos = pCamera->m_pComponentTransform->GetLocalPosition();
            camrot = pCamera->m_pComponentTransform->GetLocalRotation();
        }
        else
#endif
        {
            ComponentCamera* pCamera = g_pComponentSystemManager->GetFirstCamera();
            if( pCamera )
            {
                campos = pCamera->m_pComponentTransform->GetLocalPosition();
                camrot = pCamera->m_pComponentTransform->GetLocalRotation();
            }
            else
            {
                campos.Set( 0, 0, 0 );
                camrot.Set( 0, 0, 0 );
            }
        }

        m_pMesh->Draw( &worldtransform, pMatViewProj, &campos, &camrot, lights, numlights, pShadowVP, pShadowTex, 0, pShaderOverride );
    }

    checkGlError( "end of ComponentMesh::DrawCallback()" );
}
