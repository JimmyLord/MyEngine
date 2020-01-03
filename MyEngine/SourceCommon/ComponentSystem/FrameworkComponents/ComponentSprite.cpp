//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentSprite.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "../../../Framework/MyFramework/SourceCommon/RenderGraphs/RenderGraph_Base.h"

#if MYFW_EDITOR
#include "../SourceEditor/Commands/EngineEditorCommands.h"
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentSprite ); //_VARIABLE_LIST

ComponentSprite::ComponentSprite(EngineCore* pEngineCore, ComponentSystemManager* pComponentSystemManager)
: ComponentRenderable( pEngineCore, pComponentSystemManager )
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_WaitingToAddToRenderGraph = false;
    m_pRenderGraphObject = 0;

    m_pSprite = 0;
}

ComponentSprite::~ComponentSprite()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    SAFE_RELEASE( m_pSprite );

    MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
    m_pGameObject->GetTransform()->UnregisterTransformChangedCallbacks( this );

    RemoveFromRenderGraph();

    MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Tick );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnSurfaceChanged );
    MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( Draw );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnTouch );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnButtons );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnKeys );
    //MYFW_ASSERT_COMPONENT_CALLBACK_IS_NOT_REGISTERED( OnFileRenamed );
}

void ComponentSprite::RegisterVariables(TCPPListHead<ComponentVariable*>* pList, ComponentSprite* pThis) //_VARIABLE_LIST
{
    ComponentRenderable::RegisterVariables( pList, pThis );

#if MYFW_USING_WX
    AddVar( pList, "Tint",     ComponentVariableType::ColorByte, MyOffsetOf( pThis, &pThis->m_Tint ),  true,  true, 0, (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged, (CVarFunc_DropTarget)&ComponentSprite::OnDrop, 0 );
    AddVar( pList, "Size",     ComponentVariableType::Vector2,   MyOffsetOf( pThis, &pThis->m_Size ),  true,  true, 0, (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged, (CVarFunc_DropTarget)&ComponentSprite::OnDrop, 0 );
    AddVarPointer( pList, "Material", true,  true, 0,
       (CVarFunc_GetPointerValue)&ComponentSprite::GetPointerValue, (CVarFunc_SetPointerValue)&ComponentSprite::SetPointerValue, (CVarFunc_GetPointerDesc)&ComponentSprite::GetPointerDesc, (CVarFunc_SetPointerDesc)&ComponentSprite::SetPointerDesc,
       (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged, (CVarFunc_DropTarget)&ComponentSprite::OnDrop, 0 );
#else
    AddVar( pList, "Tint",     ComponentVariableType::ColorByte, MyOffsetOf( pThis, &pThis->m_Tint ),  true,  true, 0, (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged, 0, 0 );
    AddVar( pList, "Size",     ComponentVariableType::Vector2,   MyOffsetOf( pThis, &pThis->m_Size ),  true,  true, 0, (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged, 0, 0 );
    ComponentVariable* pVar = AddVarPointer( pList, "Material", true,  true, 0,
       (CVarFunc_GetPointerValue)&ComponentSprite::GetPointerValue,
       (CVarFunc_SetPointerValue)&ComponentSprite::SetPointerValue,
       (CVarFunc_GetPointerDesc)&ComponentSprite::GetPointerDesc,
       (CVarFunc_SetPointerDesc)&ComponentSprite::SetPointerDesc,
       (CVarFunc_ValueChanged)&ComponentSprite::OnValueChanged,
       (CVarFunc_DropTarget)&ComponentSprite::OnDrop, 0 );

#if MYFW_EDITOR
    //pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentSprite::ShouldVariableBeAddedToWatchPanel) );
    pVar->AddCallback_VariableAddedToInterface( (CVarFunc_VariableAddedToInterface)(&ComponentSprite::VariableAddedToWatchPanel) );
#endif
#endif

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
        m_pSprite = MyNew MySprite();

    m_pGameObject->GetTransform()->RegisterTransformChangedCallback( this, StaticOnTransformChanged );

    m_Size.Set( 1.0f, 1.0f );
    m_Tint.Set( 255,255,255,255 );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentSprite::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate ).addFunction( "CastAs_ComponentSprite", CastAs_ComponentSprite ); // ComponentSprite* CastAs_ComponentSprite(ComponentBase* pComponent)

    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSprite>( "ComponentSprite" )
            //.addData( "localmatrix", &ComponentSprite::m_LocalTransform )
            .addFunction( "SetMaterial", &ComponentSprite::SetMaterial ) // void SetMaterial(MaterialDefinition* pMaterial, int submeshIndex);
            .addFunction( "GetSprite", &ComponentSprite::GetSprite ) // MySprite* ComponentSprite::GetSprite()
        .endClass();
}
#endif //MYFW_USING_LUA

void* ComponentSprite::GetPointerValue(ComponentVariable* pVar) //_VARIABLE_LIST // StaticGetPointerValue
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        if( m_pSprite )
        {
            if( m_pSprite->GetMaterial() )
            {
                if( m_pSprite->GetMaterial()->GetFile() )
                    return m_pSprite->GetMaterial();
            }
        }
    }

    return 0;
}

void ComponentSprite::SetPointerValue(ComponentVariable* pVar, const void* newvalue) // StaticSetPointerValue
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
#if MYFW_EDITOR
        g_pEngineCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeMaterialOnMesh( this, pVar, 0, (MaterialDefinition*)newvalue ) );
#else
        SetMaterial( (MaterialDefinition*)newvalue, 0 );
#endif
    }
}

const char* ComponentSprite::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST // StaticGetPointerDesc
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        MyAssert( m_pSprite );
        MaterialDefinition* pMaterial = m_pSprite->GetMaterial();
        if( pMaterial && pMaterial->GetFile() )
            return pMaterial->GetMaterialDescription();
        else
            return "none";
    }

    return "fix me";
}

void ComponentSprite::SetPointerDesc(ComponentVariable* pVar, const char* newdesc) //_VARIABLE_LIST  // StaticSetPointerDesc
{
    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        MyAssert( newdesc );
        if( newdesc )
        {
            // Load the material throught the engine, so the ComponentSystemManager can keep a reference.
            MyFileInfo* pFileInfo = g_pComponentSystemManager->LoadDataFile( newdesc, m_SceneIDLoadedFrom, nullptr, false );
            MaterialDefinition* pMaterial = pFileInfo->GetMaterial();
            pMaterial->AddRef();
#if MYFW_EDITOR
            g_pEngineCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeMaterialOnMesh( this, pVar, 0, pMaterial ) );
#else
            SetMaterial( pMaterial, 0 );
#endif
            pMaterial->Release();
        }
    }
}

cJSON* ComponentSprite::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentRenderable::ExportAsJSONObject( savesceneid, saveid );

    return jComponent;
}

void ComponentSprite::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jsonobj, sceneid );
}

ComponentSprite& ComponentSprite::operator=(const ComponentSprite& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    this->m_Tint = other.m_Tint;
    this->m_Size = other.m_Size;
    this->SetMaterial( other.m_pSprite->GetMaterial(), 0 );

    return *this;
}

void ComponentSprite::RegisterCallbacks()
{
    MyAssert( m_EnabledState == EnabledState_Enabled );

    if( m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnSurfaceChanged );
        MYFW_FILL_COMPONENT_CALLBACK_STRUCT( ComponentSprite, Draw ); //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, OnFileRenamed );
    }
}

void ComponentSprite::UnregisterCallbacks()
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

void ComponentSprite::OnLoad()
{
    ComponentRenderable::OnLoad();

    BufferManager* pBufferManager = m_pEngineCore->GetManagers()->GetBufferManager();
    m_pSprite->Create( pBufferManager, "ComponentSprite", m_Size.x, m_Size.y, 0, 1, 0, 1, Justify_Center, false );

    if( m_pRenderGraphObject == 0 )
        AddToRenderGraph();
}

void ComponentSprite::OnTransformChanged(Vector3& newpos, Vector3& newrot, Vector3& newscale, bool changedbyuserineditor)
{
    if( m_pRenderGraphObject != 0 )
    {
        g_pComponentSystemManager->GetRenderGraph()->ObjectMoved( m_pRenderGraphObject );
    }
}

void ComponentSprite::SetMaterial(MaterialDefinition* pMaterial, int submeshIndex)
{
    // Sprites only have one submesh.
    MyAssert( submeshIndex == 0 );

    ComponentRenderable::SetMaterial( pMaterial, submeshIndex );

    if( m_pSprite )
    {
        m_pSprite->SetMaterial( pMaterial );

        // Create a RenderGraph object if this is the first time we set the material.
        if( m_pRenderGraphObject == 0 && pMaterial != 0 )
        {
            BufferManager* pBufferManager = m_pEngineCore->GetManagers()->GetBufferManager();
            m_pSprite->Create( pBufferManager, "ComponentSprite", m_Size.x, m_Size.y, 0, 1, 0, 1, Justify_Center, false );

            AddToRenderGraph();
        }

        if( pMaterial == 0 )
        {
            RemoveFromRenderGraph();
        }
    }

    if( m_pRenderGraphObject )
    {
        m_pRenderGraphObject->SetMaterial( pMaterial, true );
    }
}

void ComponentSprite::SetVisible(bool visible)
{
    ComponentRenderable::SetVisible( visible );

    if( m_pRenderGraphObject )
    {
        m_pRenderGraphObject->m_Visible = visible;
    }
}

void ComponentSprite::AddToRenderGraph()
{
    MaterialDefinition* pMaterial = m_pSprite->GetMaterial();

    if( pMaterial == 0 )
        return;

    if( m_EnabledState != EnabledState_Enabled )
        return;

    MyAssert( m_pRenderGraphObject == 0 );
    MyAssert( m_pSprite );

    // Add the submesh to the scene graph if the material is loaded, otherwise check again each tick.
    // TODO: Either register with a material finished loading callback or use events instead of this tick callback.
    if( pMaterial->IsFullyLoaded() )
    {
        m_pRenderGraphObject = g_pComponentSystemManager->AddSubmeshToRenderGraph( this, m_pSprite, pMaterial, MyRE::PrimitiveType_Triangles, 1, m_LayersThisExistsOn );

        m_WaitingToAddToRenderGraph = false;
    }
    else if( m_WaitingToAddToRenderGraph == false )
    {
        m_WaitingToAddToRenderGraph = true;
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentSprite, Tick );
    }
}

void ComponentSprite::RemoveFromRenderGraph()
{
    if( m_WaitingToAddToRenderGraph )
    {
        m_WaitingToAddToRenderGraph = false;
        return;
    }

    if( m_pRenderGraphObject != 0 )
    {
        g_pComponentSystemManager->RemoveObjectFromRenderGraph( m_pRenderGraphObject );    
        m_pRenderGraphObject = 0;
    }
}

void ComponentSprite::PushChangesToRenderGraphObjects()
{
    //ComponentRenderable::PushChangesToRenderGraphObjects(); // pure virtual

    // Sync RenderGraph object
    if( m_pRenderGraphObject )
    {
        m_pRenderGraphObject->SetMaterial( this->GetMaterial( 0 ), true );
        m_pRenderGraphObject->m_Layers = this->m_LayersThisExistsOn;
        m_pRenderGraphObject->m_Visible = this->m_Visible;

        //m_pRenderGraphObject->m_GLPrimitiveType = this->m_GLPrimitiveType;
        //m_pRenderGraphObject->m_PointSize = this->m_PointSize;
    }
}

void ComponentSprite::TickCallback(float deltaTime)
{
    MyAssert( m_pGameObject->GetTransform() );

    // If we're done waiting to be added to the scene graph (either to be added to removed), we no longer need this callback.
    if( m_WaitingToAddToRenderGraph == false )
    {
        // Callbacks can only be safely unregistered during their own callback.
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
    }
    else
    {
        AddToRenderGraph();
    }
}

void ComponentSprite::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatProj, MyMatrix* pMatView, ShaderGroup* pShaderOverride)
{
    ComponentRenderable::Draw( pMatProj, pMatView, pShaderOverride, 0 );

    //m_pSprite->SetPosition( m_pComponentTransform->GetWorldTransform() );
    //m_pSprite->SetTint( m_Tint );
    //m_pSprite->Create( "ComponentSprite", m_Size.x, m_Size.y, 0, 1, 0, 1, Justify_Center, false );
    m_pSprite->Draw( pMatProj, pMatView, m_pComponentTransform->GetWorldTransform(), pShaderOverride, true );
}

#if MYFW_EDITOR
ComponentVariable* ComponentSprite::GetComponentVariableForMaterial(int submeshIndex)
{
    return FindComponentVariableByLabel( &m_ComponentVariableList_ComponentSprite, "Material" );
}

bool ComponentSprite::IsReferencingFile(MyFileObject* pFile)
{
    if( m_pSprite->GetMaterial() && m_pSprite->GetMaterial()->GetFile() == pFile )
        return true;

    return ComponentBase::IsReferencingFile( pFile );
}

#if MYFW_USING_WX
void ComponentSprite::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentSprite::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Sprite", ObjectListIcon_Component );
}

void ComponentSprite::OnLeftClick(unsigned int count, bool clear) // StaticOnLeftClick
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
#endif //MYFW_USING_WX

//bool ComponentSprite::ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar)
//{
//    return true;
//}

void ComponentSprite::VariableAddedToWatchPanel(ComponentVariable* pVar)
{
#if MYFW_USING_IMGUI

#if _DEBUG && MYFW_WINDOWS
    if( ImGui::Button( "Trigger Breakpoint on Next Draw" ) )
    {
        TriggerBreakpointOnNextDraw();
    }
#endif //_DEBUG && MYFW_WINDOWS

#endif //MYFW_USING_IMGUI
}

void* ComponentSprite::OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        MaterialDefinition* pMaterial = (MaterialDefinition*)pDropItem->m_Value;
        //MyAssert( pMaterial ); // Double-clicking material to clear drops a null material in imgui editor.
        MyAssert( m_pSprite );

        oldPointer = m_pSprite->GetMaterial();

        SetMaterial( pMaterial, 0 );

#if MYFW_USING_WX
        g_pPanelWatch->SetNeedsRefresh();
#endif //MYFW_USING_WX
    }

    return oldPointer;
}

void* ComponentSprite::OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    if( strcmp( pVar->m_Label, "Material" ) == 0 )
    {
        if( changedByInterface )
        {
#if MYFW_USING_WX
            wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
            if( text == "" || text == "none" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

                oldpointer = m_pSprite->GetMaterial();
                g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_ChangeMaterialOnMesh( this, pVar, 0, 0 ) );
            }
#endif //MYFW_USING_WX
        }
        else if( pNewValue->GetMaterialPtr() != 0 )
        {
            oldpointer = GetMaterial( 0 );

            MaterialDefinition* pNewMaterial = pNewValue ? pNewValue->GetMaterialPtr() : 0;
            SetMaterial( pNewMaterial, 0 );
        }
    }

    //if( strcmp( pVar->m_Label, "Size" ) == 0 )
    {
        BufferManager* pBufferManager = m_pEngineCore->GetManagers()->GetBufferManager();
        m_pSprite->Create( pBufferManager, "ComponentSprite", m_Size.x, m_Size.y, 0, 1, 0, 1, Justify_Center, false );
    }

    PushChangesToRenderGraphObjects();

    return oldpointer;
}

#if _DEBUG && MYFW_WINDOWS
void ComponentSprite::TriggerBreakpointOnNextDraw()
{
    if( m_pSprite )
    {
        m_pSprite->TriggerBreakpointOnNextDraw();
    }
}
#endif //_DEBUG && MYFW_WINDOWS

#endif //MYFW_EDITOR
