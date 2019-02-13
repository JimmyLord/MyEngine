//
// Copyright (c) 2014-2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentCameraShadow.h"
#include "ComponentLuaScript.h"
#include "ComponentMesh.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "../../../Framework/MyFramework/SourceCommon/SceneGraphs/SceneGraph_Base.h"
#include "../../../Framework/MyFramework/SourceCommon/SceneGraphs/SceneGraph_Flat.h"
#include "../../../Framework/MyFramework/SourceCommon/Renderers/BaseClasses/Shader_Base.h"

#if MYFW_EDITOR
#include "../SourceEditor/Editor_ImGui/EditorMainFrame_ImGui.h"
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

// Component Variable List.
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentMesh ); //_VARIABLE_LIST

ComponentMesh::ComponentMesh()
: ComponentRenderable()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_WaitingToAddToSceneGraph = false;

    m_pMesh = nullptr;
    for( int i=0; i<MAX_SUBMESHES; i++ )
    {
        m_pSceneGraphObjects[i] = nullptr;
        m_pMaterials[i] = nullptr;
    }

    m_GLPrimitiveType = MyRE::PrimitiveType_Triangles;
    m_PointSize = 1;

    m_pComponentLuaScript = nullptr;

    g_pEventManager->RegisterForEvents( Event_ShaderFinishedLoading, this, &ComponentMesh::StaticOnEvent );
}

ComponentMesh::~ComponentMesh()
{
    g_pEventManager->UnregisterForEvents( Event_ShaderFinishedLoading, this, &ComponentMesh::StaticOnEvent );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    m_pGameObject->GetTransform()->UnregisterTransformChangedCallbacks( this );

    SAFE_RELEASE( m_pMesh );
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        if( m_pSceneGraphObjects[i] != nullptr )
            g_pComponentSystemManager->RemoveObjectFromSceneGraph( m_pSceneGraphObjects[i] );
        m_pSceneGraphObjects[i] = nullptr;
        SAFE_RELEASE( m_pMaterials[i] );
    }

    MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Tick );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnSurfaceChanged );
    MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Draw );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnTouch );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnButtons );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnKeys );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnFileRenamed );
}

void ComponentMesh::RegisterVariables(CPPListHead* pList, ComponentMesh* pThis) //_VARIABLE_LIST
{
    ComponentRenderable::RegisterVariables( pList, pThis );

    for( int i=0; i<MAX_SUBMESHES; i++ )
    {
        // Materials are not automatically saved/loaded.
        MyAssert( MAX_SUBMESHES == 4 );
        ComponentVariable* pVar = AddVar( pList, g_MaterialLabels[i], ComponentVariableType_MaterialPtr,
                                          MyOffsetOf( pThis, &pThis->m_pMaterials[i] ), false, true, 
                                          nullptr, (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, (CVarFunc_DropTarget)&ComponentMesh::OnDropMaterial, nullptr );

#if MYFW_EDITOR
        pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentMesh::ShouldVariableBeAddedToWatchPanel) );
        pVar->AddCallback_VariableAddedToInterface( (CVarFunc_VariableAddedToInterface)(&ComponentMesh::VariableAddedToWatchPanel) );
#endif
    }

    AddVarEnum( pList, "PrimitiveType", MyOffsetOf( pThis, &pThis->m_GLPrimitiveType ),  true,  true, "Primitive Type", 7, OpenGLPrimitiveTypeStrings, (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, nullptr, nullptr );
    AddVar( pList, "PointSize", ComponentVariableType_Int, MyOffsetOf( pThis, &pThis->m_PointSize ),  true,  true, "Point Size", (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, nullptr, nullptr );
}

void ComponentMesh::Reset()
{
    ComponentRenderable::Reset();

    SAFE_RELEASE( m_pMesh );
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
        SAFE_RELEASE( m_pMaterials[i] );

    m_pGameObject->GetTransform()->RegisterTransformChangedCallback( this, StaticOnTransformChanged );

    m_pComponentLuaScript = nullptr;
}

#if MYFW_USING_LUA
void ComponentMesh::LuaRegister(lua_State* luaState)
{
}
#endif //MYFW_USING_LUA

#if MYFW_EDITOR
ComponentVariable* ComponentMesh::GetComponentVariableForMaterial(int submeshIndex)
{
    char temp[20];
    sprintf_s( temp, 20, "Material%d", submeshIndex+1 );

    return FindComponentVariableByLabel( &m_ComponentVariableList_ComponentMesh, temp );
}

bool ComponentMesh::ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar)
{
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        // Only show enough material variables for the number of submeshes in the mesh.
        if( pVar->m_Label == g_MaterialLabels[i] )
        {
            if( m_pMesh == nullptr || i >= m_pMesh->GetSubmeshListCount() )
                return false;
        }
    }

    return true;
}

void ComponentMesh::VariableAddedToWatchPanel(ComponentVariable* pVar)
{
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        if( m_pMaterials[i] == nullptr )
            continue;

#if MYFW_USING_IMGUI

        if( pVar->m_Label == g_MaterialLabels[i] )
        {
            if( ImGui::IsItemHovered() )
            {
                ImGui::BeginTooltip();
                ImGui::Text( "%s", m_pMaterials[i]->GetName() );
                g_pEngineCore->GetEditorMainFrame_ImGui()->AddMaterialPreview( m_pMaterials[i], false, ImVec2( 100, 100 ), ImVec4( 1, 1, 1, 1 ) );
                ImGui::EndTooltip();
            }

#if _DEBUG && MYFW_WINDOWS
            if( ImGui::Button( "Trigger Breakpoint on Next Draw" ) )
            {
                TriggerBreakpointOnNextDraw( i );
            }
#endif //_DEBUG && MYFW_WINDOWS

            if( m_pMaterials[i]->m_UniformValues[0].m_Type != ExposedUniformType_NotSet )
            {
                ImGui::SameLine();
                if( ImGui::CollapsingHeader( g_MaterialLabels[i] ) )
                {
                    ImGui::Indent( 20 );
                    g_pEngineCore->GetEditorMainFrame_ImGui()->AddInlineMaterial( m_pMaterials[i] );
                    ImGui::Unindent( 20 );
                }
            }
        }
#endif //MYFW_USING_IMGUI
    }
}

void* ComponentMesh::OnDropMaterial(ComponentVariable* pVar, int x, int y)
{
    void* oldPointer = nullptr;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_MaterialDefinitionPointer )
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
            MaterialDefinition* pMaterial = (MaterialDefinition*)pDropItem->m_Value;

            oldPointer = GetMaterial( materialthatchanged );
            SetMaterial( pMaterial, materialthatchanged );
            //g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeMaterialOnMesh( this, pVar, materialthatchanged, pMaterial ) );
        }
    }

    return oldPointer;
}

void* ComponentMesh::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldPointer = nullptr;

    if( finishedChanging )
    {
        if( strncmp( pVar->m_Label, "Material", strlen("Material") ) == 0 )
        {
            int materialThatChanged = -1;
            for( int i=0; i<MAX_SUBMESHES; i++ )
            {
                if( pVar->m_Label == g_MaterialLabels[i] )
                {
                    materialThatChanged = i;
                    break;
                }
            }
            MyAssert( materialThatChanged != -1 );

            if( changedByInterface )
            {
            }
            else
            {
                oldPointer = GetMaterial( materialThatChanged );
                MaterialDefinition* pNewMaterial = pNewValue ? pNewValue->GetMaterialPtr() : nullptr;
                SetMaterial( pNewMaterial, materialThatChanged );
            }
        }

        PushChangesToSceneGraphObjects();
    }

    return oldPointer;
}

#if _DEBUG && MYFW_WINDOWS
void ComponentMesh::TriggerBreakpointOnNextDraw(int submeshIndex)
{
    if( m_pMesh )
    {
        m_pMesh->GetSubmesh( submeshIndex )->TriggerBreakpointOnNextDraw();
    }
}
#endif //_DEBUG && MYFW_WINDOWS

#endif //MYFW_EDITOR

cJSON* ComponentMesh::ExportAsJSONObject(bool saveSceneID, bool saveID)
{
    cJSON* jComponent = ComponentRenderable::ExportAsJSONObject( saveSceneID, saveID );

    cJSON* jMaterialArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jComponent, "Materials", jMaterialArray );

    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        MyAssert( m_pMaterials[i] == nullptr || m_pMaterials[i]->GetFile() ); // New materials should be saved as files before the state is saved.

        cJSON* jMaterial = nullptr;
        if( m_pMaterials[i] && m_pMaterials[i]->GetFile() )
            jMaterial = cJSON_CreateString( m_pMaterials[i]->GetMaterialDescription() );

        if( jMaterial )
            cJSON_AddItemToArray( jMaterialArray, jMaterial );
    }

    //cJSON_AddNumberToObject( jComponent, "PrimitiveType", m_GLPrimitiveType );
    //cJSON_AddNumberToObject( jComponent, "PointSize", m_PointSize );
    
    return jComponent;
}

void ComponentMesh::ImportFromJSONObject(cJSON* jComponent, SceneID sceneID)
{
    ComponentRenderable::ImportFromJSONObject( jComponent, sceneID );

    // TODO: Remove this "Material" block, it's for old scenes before I changed to multiple materials.
    cJSON* jMaterial = cJSON_GetObjectItem( jComponent, "Material" );
    if( jMaterial )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMaterial->valuestring );
        if( pMaterial )
        {
            SetMaterial( pMaterial, 0 );
            pMaterial->Release();
        }
    }

    cJSON* jMaterialArray = cJSON_GetObjectItem( jComponent, "Materials" );
    if( jMaterialArray )
    {
        int numMaterials = cJSON_GetArraySize( jMaterialArray );

        //for( int i=0; i<MAX_SUBMESHES; i++ ) { MyAssert( m_pMaterials[i] == 0 ); }

        for( int i=0; i<numMaterials; i++ )
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

    //cJSONExt_GetInt( jComponent, "PrimitiveType", &m_GLPrimitiveType );
    //cJSONExt_GetInt( jComponent, "PointSize", &m_PointSize );

    ImportVariablesFromJSON( jComponent ); //_VARIABLE_LIST
}

ComponentMesh& ComponentMesh::operator=(const ComponentMesh& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    //const ComponentMesh* pOther = &other;
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        SetMaterial( other.m_pMaterials[i], i );
    }

    m_GLPrimitiveType = other.m_GLPrimitiveType;
    m_PointSize = other.m_PointSize;

    SetMesh( other.m_pMesh );

    return *this;
}

void ComponentMesh::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
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
    MyAssert( m_EnabledState != EnabledState_Enabled );

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

void ComponentMesh::OnPlay()
{
    ComponentBase::OnPlay();

    m_pMesh->RegisterSetupCustomUniformsCallback( nullptr, nullptr );

    m_pComponentLuaScript = (ComponentLuaScript*)m_pGameObject->GetFirstComponentOfType( "LuaScriptComponent" );

    if( m_pComponentLuaScript )
    {
        m_pComponentLuaScript->RegisterOnDeleteCallback( this, StaticOnLuaScriptDeleted );
        m_pMesh->RegisterSetupCustomUniformsCallback( this, StaticSetupCustomUniformsCallback );
    }
}

bool ComponentMesh::OnEvent(MyEvent* pEvent)
{
    // When a material finishes loading, if it was our material, set the material again to fix the flags of the scenegraphobject.
    if( pEvent->GetType() == Event_MaterialFinishedLoading )
    {
        MaterialDefinition* pMaterial = static_cast<MaterialDefinition*>( pEvent->GetPointer( "Material" ) );
        MyAssert( pMaterial != nullptr );
            
        for( int i=0; i<MAX_SUBMESHES; i++ )
        {
            if( m_pMaterials[i] == pMaterial )
            {
                SetMaterial( m_pMaterials[i], i );
            }
        }

        return false; // Keep propagating the event.
    }

    if( pEvent->GetType() == Event_ShaderFinishedLoading )
    {
        BaseShader* pShader = static_cast<BaseShader*>( pEvent->GetPointer( "Shader" ) );
        MyAssert( pShader != nullptr );

        for( int i=0; i<MAX_SUBMESHES; i++ )
        {
            if( m_pMaterials[i] )
            {
                ShaderGroup* pShaderGroup = m_pMaterials[i]->GetShader();

                if( pShaderGroup && pShaderGroup->ContainsShader( pShader ) )
                {
                    SetMaterial( m_pMaterials[i], i );
                }
            }
        }

        return false; // Keep propagating the event.
    }

    return false;
}

void ComponentMesh::OnTransformChanged(Vector3& newPos, Vector3& newRot, Vector3& newScale, bool changedByUserInEditor)
{
    if( m_pMesh )
    {
        for( unsigned int i=0; i<m_pMesh->GetSubmeshListCount(); i++ )
        {
            if( m_pSceneGraphObjects[i] != nullptr )
            {
                g_pComponentSystemManager->GetSceneGraph()->ObjectMoved( m_pSceneGraphObjects[i] );
            }
        }
    }
}

void ComponentMesh::SetMaterial(MaterialDefinition* pMaterial, int submeshIndex)
{
    ComponentRenderable::SetMaterial( pMaterial, submeshIndex );

    MyAssert( submeshIndex >= 0 && submeshIndex < MAX_SUBMESHES );

    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_pMaterials[submeshIndex] );
    m_pMaterials[submeshIndex] = pMaterial;

    if( m_pSceneGraphObjects[submeshIndex] )
    {
        // Update the material on the SceneGraphObject along with the opaque/transparent flags.
        m_pSceneGraphObjects[submeshIndex]->SetMaterial( pMaterial, true );
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
    return m_pMesh->IsReady();
}

void ComponentMesh::MeshFinishedLoading()
{
}

void ComponentMesh::AddToSceneGraph()
{
    MyAssert( m_pMesh );

    // If the object has been disabled, don't add it to the scene graph.
    if( IsEnabled() == false )
        return;

    MyAssert( m_pGameObject->IsEnabled() );

    if( m_pMesh->IsReady() )
    {
        MyAssert( m_pMesh->GetSubmeshListCount() > 0 );
        MyAssert( m_pSceneGraphObjects[0] == nullptr );

        // Add the Mesh to the main scene graph.
        if( m_pMesh->GetSubmeshListCount() > 0 )
        {
            g_pComponentSystemManager->AddMeshToSceneGraph( this, m_pMesh, m_pMaterials, m_GLPrimitiveType, m_PointSize, m_LayersThisExistsOn, m_pSceneGraphObjects );
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
    if( m_pMesh == nullptr )
        return;

    if( m_WaitingToAddToSceneGraph )
    {
        m_WaitingToAddToSceneGraph = false;
        return;
    }

    MyAssert( m_pMesh );
    //MyAssert( m_pMesh->GetSubmeshListCount() > 0 );

    for( unsigned int i=0; i<m_pMesh->GetSubmeshListCount(); i++ )
    {
        if( m_pSceneGraphObjects[i] != nullptr )
        {
            g_pComponentSystemManager->RemoveObjectFromSceneGraph( m_pSceneGraphObjects[i] );
            m_pSceneGraphObjects[i] = nullptr;
        }
    }
}

void ComponentMesh::PushChangesToSceneGraphObjects()
{
    //ComponentRenderable::PushChangesToSceneGraphObjects(); // Pure virtual.

    // Sync scenegraph objects.
    for( int i=0; i<MAX_SUBMESHES; i++ )
    {
        if( m_pSceneGraphObjects[i] )
        {
            m_pSceneGraphObjects[i]->SetMaterial( this->GetMaterial( i ), true );
            m_pSceneGraphObjects[i]->m_Layers = this->m_LayersThisExistsOn;

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

    return nullptr;
}

void ComponentMesh::SetupCustomUniformsCallback(Shader_Base* pShader) // StaticSetupCustomUniformsCallback
{
    // This callback should only get called if there was a Lua script component.
    // TODO: Don't register the callback if the lua script object doesn't have a "SetupCustomUniforms" function.

    MyAssert( m_pComponentLuaScript != nullptr );

    m_pComponentLuaScript->CallFunction( "SetupCustomUniforms", pShader->m_ProgramHandle );
}

void ComponentMesh::OnLuaScriptDeleted(ComponentBase* pComponent) // StaticOnLuaScriptDeleted
{
    if( m_pComponentLuaScript == pComponent )
    {
        m_pComponentLuaScript = nullptr;
        m_pMesh->RegisterSetupCustomUniformsCallback( nullptr, nullptr );
    }
}

void ComponentMesh::TickCallback(float deltaTime)
{
    // TODO: Temp hack, if the gameobject doesn't have a transform (shouldn't happen), then don't try to add to scene graph.
    if( m_pGameObject->GetTransform() == nullptr )
        return;

    MyAssert( m_pGameObject->GetTransform() );
    MyAssert( m_pMesh );

    // If we're done waiting to be added to the scene graph (either to be added to removed), we no longer need this callback.
    if( m_WaitingToAddToSceneGraph == false )
    {
        // Callbacks can only be safely unregistered during their own callback.
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
    }
    else
    {
        if( IsMeshReady() )
        {
            MeshFinishedLoading();
            AddToSceneGraph();
        }
    }
}

void ComponentMesh::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    ComponentRenderable::Draw( pMatProj, pMatView, pShaderOverride, 0 );

    MyMatrix matViewProj = *pMatProj * *pMatView;

    if( m_pMesh )
    {
        MyMatrix worldtransform = *m_pComponentTransform->GetWorldTransform();

        // Simple frustum check.
        {
            MyAABounds* bounds = m_pMesh->GetBounds();
            Vector3 center = bounds->GetCenter();
            Vector3 half = bounds->GetHalfSize();

            MyMatrix wvp = matViewProj * worldtransform;

            Vector4 clippos[8];

            // Transform AABB extents into clip space.
            clippos[0] = wvp * Vector4(center.x - half.x, center.y - half.y, center.z - half.z, 1);
            clippos[1] = wvp * Vector4(center.x - half.x, center.y - half.y, center.z + half.z, 1);
            clippos[2] = wvp * Vector4(center.x - half.x, center.y + half.y, center.z - half.z, 1);
            clippos[3] = wvp * Vector4(center.x - half.x, center.y + half.y, center.z + half.z, 1);
            clippos[4] = wvp * Vector4(center.x + half.x, center.y - half.y, center.z - half.z, 1);
            clippos[5] = wvp * Vector4(center.x + half.x, center.y - half.y, center.z + half.z, 1);
            clippos[6] = wvp * Vector4(center.x + half.x, center.y + half.y, center.z - half.z, 1);
            clippos[7] = wvp * Vector4(center.x + half.x, center.y + half.y, center.z + half.z, 1);

            // Check visibility two planes at a time.
            bool visible = false;
            for( int component=0; component<3; component++ ) // Loop through x/y/z.
            {
                // Check if all 8 points are less than the -w extent of it's axis.
                visible = false;
                for( int i=0; i<8; i++ )
                {
                    if( clippos[i][component] >= -clippos[i].w )
                    {
                        visible = true; // This point is on the visible side of the plane, skip to next plane.
                        break;
                    }
                }
                if( visible == false ) // All points are on outside of plane, don't draw object.
                    break;

                // Check if all 8 points are greater than the -w extent of it's axis.
                visible = false;
                for( int i=0; i<8; i++ )
                {
                    if( clippos[i][component] <= clippos[i].w )
                    {
                        visible = true; // This point is on the visible side of the plane, skip to next plane.
                        break;
                    }
                }
                if( visible == false ) // All points are on outside of plane, don't draw object.
                    break;
            }

            // If all points are on outside of frustum, don't draw mesh.
            if( visible == false )
                return;
        }

        for( unsigned int i=0; i<m_pMesh->GetSubmeshListCount(); i++ )
        {
            m_pMesh->SetMaterial( m_pMaterials[i], i );
            m_pMesh->GetSubmesh( i )->m_PrimitiveType = m_GLPrimitiveType;
            m_pMesh->GetSubmesh( i )->m_PointSize = m_PointSize;
        }

        //m_pMesh->SetTransform( worldtransform );

        // Find nearest lights.
        MyLight* lights[4];
        int numlights = g_pLightManager->FindNearestLights( LightType_Point, 4, m_pComponentTransform->GetWorldTransform()->GetTranslation(), lights );

        // Find nearest shadow casting light.
        MyMatrix* pShadowVP = nullptr;
        TextureDefinition* pShadowTex = nullptr;
        if( g_ActiveShaderPass == ShaderPass_Main )
        {
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByName( "Shadow Light" );
            if( pObject )
            {
                ComponentBase* pComponent = pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera );
                ComponentCameraShadow* pShadowCam = pComponent->IsA( "CameraShadowComponent" ) ? (ComponentCameraShadow*)pComponent : nullptr;
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

        Vector3 campos;
        Vector3 camrot;
#if MYFW_USING_WX
        if( g_pEngineCore->IsInEditorMode() )
        {
            ComponentCamera* pCamera = g_pEngineCore->GetEditorState()->GetEditorCamera();

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

        m_pMesh->Draw( pMatProj, pMatView, &worldtransform, &campos, &camrot, lights, numlights, pShadowVP, pShadowTex, nullptr, pShaderOverride );
    }
}
