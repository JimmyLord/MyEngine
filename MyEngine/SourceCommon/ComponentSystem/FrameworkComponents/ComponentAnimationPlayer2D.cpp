//
// Copyright (c) 2016-2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentAnimationPlayer2D::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentAnimationPlayer2D ); //_VARIABLE_LIST

ComponentAnimationPlayer2D::ComponentAnimationPlayer2D()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pAnimationFile = 0;
    m_pAnimInfo = 0;

#if MYFW_USING_WX
    g_pComponentSystemManager->Editor_RegisterFileUpdatedCallback( &StaticOnFileUpdated, this );
#endif
}

ComponentAnimationPlayer2D::~ComponentAnimationPlayer2D()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    delete( m_pAnimInfo );
    SAFE_RELEASE( m_pAnimationFile );
}

void ComponentAnimationPlayer2D::RegisterVariables(CPPListHead* pList, ComponentAnimationPlayer2D* pThis) //_VARIABLE_LIST
{
    AddVar( pList, "Animation Index", ComponentVariableType_UnsignedInt, MyOffsetOf( pThis, &pThis->m_AnimationIndex ), true, true, 0, (CVarFunc_ValueChanged)&ComponentAnimationPlayer2D::OnValueChanged, (CVarFunc_DropTarget)&ComponentAnimationPlayer2D::OnDrop, 0 );
    AddVar( pList, "Animation Frame", ComponentVariableType_Float, MyOffsetOf( pThis, &pThis->m_AnimationTime ), true, true, 0, (CVarFunc_ValueChanged)&ComponentAnimationPlayer2D::OnValueChanged, (CVarFunc_DropTarget)&ComponentAnimationPlayer2D::OnDrop, 0 );
    AddVar( pList, "Frame Index", ComponentVariableType_UnsignedInt, MyOffsetOf( pThis, &pThis->m_FrameIndex ), true, true, 0, (CVarFunc_ValueChanged)&ComponentAnimationPlayer2D::OnValueChanged, (CVarFunc_DropTarget)&ComponentAnimationPlayer2D::OnDrop, 0 );

    // Animation File is not automatically saved/loaded
    AddVar( pList, "Animation File", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pAnimationFile ), false, true, 0, (CVarFunc_ValueChanged)&ComponentAnimationPlayer2D::OnValueChanged, (CVarFunc_DropTarget)&ComponentAnimationPlayer2D::OnDrop, 0 );
}

void ComponentAnimationPlayer2D::Reset()
{
    ComponentBase::Reset();

    m_pSpriteComponent = 0;

    m_AnimationIndex = 0;
    m_AnimationTime = 0;
    m_FrameIndex = 0;

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentAnimationPlayer2D::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentAnimationPlayer2D>( "ComponentAnimationPlayer2D" )
            //.addData( "m_TimeBetweenFrames", &ComponentAnimationPlayer2D::m_TimeBetweenFrames )
            //m_AnimationIndex
            //m_AnimationTime
            //m_FrameIndex
            //.addFunction( "GetTimeBetweenFrames", &ComponentAnimationPlayer2D::GetTimeBetweenFrames )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void ComponentAnimationPlayer2D::OnFileUpdated(MyFileObject* pFile)
{
    if( pFile == m_pAnimationFile )
    {
        LOGInfo( LOGTag, "TODO: animation file changed, update it\n" );
    }
}

void ComponentAnimationPlayer2D::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentAnimationPlayer2D::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "2D Animation Player", ObjectListIcon_Component );
}

void ComponentAnimationPlayer2D::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentAnimationPlayer2D::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "2D Animation Player", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
    }
}

void* ComponentAnimationPlayer2D::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldpointer = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    {
        (ComponentBase*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    {
        (GameObject*)g_DragAndDropStruct.m_Value;
    }

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)g_DragAndDropStruct.m_Value;
        MyAssert( pFile );

        if( strcmp( pFile->GetExtensionWithDot(), ".my2daniminfo" ) == 0 )
        {
            oldpointer = m_pAnimationFile;
            SetAnimationFile( pFile );

            // update the panel so new filename shows up.
            g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->m_Description = m_pAnimationFile->GetFullPath();
        }
    }

    return oldpointer;
}

void* ComponentAnimationPlayer2D::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue, void* newpointer)
{
    void* oldpointer = 0;

    //if( pVar->m_Offset == MyOffsetOf( this, &m_TimeBetweenFrames ) )
    //{
    //    MyAssert( pVar->m_ControlID != -1 );
    //}

    if( strcmp( pVar->m_Label, "Animation File" ) == 0 )
    {
        if( newpointer != 0 )
        {
            MyAssert( false );
            // TODO: implement this block
        }
        else if( pVar->m_ControlID != -1 )
        {
            wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->m_Handle_TextCtrl->GetValue();
            if( text == "" || text == "none" || text == "no file" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "no file" );
                oldpointer = m_pAnimationFile;
                this->SetAnimationFile( 0 );
            }
        }
    }

    return oldpointer;
}

void ComponentAnimationPlayer2D::AddRightClickOptionsToMenu(wxMenu* pMenu, int baseid)
{
    pMenu->Append( baseid + 1, "Edit 2D Animation Info" );
}

void ComponentAnimationPlayer2D::OnRightClickOptionClicked(wxEvent &evt, int baseid)
{
    int id = evt.GetId();

    if( id == baseid + 1 )
    {
        My2DAnimInfo* p2DAnimInfo = Get2DAnimInfoObject();
        p2DAnimInfo->FillPropertiesWindow( true );
    }
}
#endif //MYFW_USING_WX

cJSON* ComponentAnimationPlayer2D::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid, saveid );

    if( m_pAnimationFile )
        cJSON_AddStringToObject( jComponent, "AnimFile", m_pAnimationFile->GetFullPath() );

    return jComponent;
}

void ComponentAnimationPlayer2D::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jComponent, sceneid );

    cJSON* animfilestringobj = cJSON_GetObjectItem( jComponent, "AnimFile" );
    if( animfilestringobj )
    {
        MyFileObject* pFile = g_pEngineFileManager->RequestFile( animfilestringobj->valuestring, GetSceneID() );
        MyAssert( pFile );
        if( pFile )
        {
            SetAnimationFile( pFile );
            pFile->Release(); // free ref added by RequestFile
        }
    }
}

ComponentAnimationPlayer2D& ComponentAnimationPlayer2D::operator=(const ComponentAnimationPlayer2D& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    // TODO: replace this with a CopyComponentVariablesFromOtherObject... or something similar.
    m_AnimationIndex = other.m_AnimationIndex;
    m_AnimationTime = other.m_AnimationTime;
    m_FrameIndex = other.m_FrameIndex;

    this->m_pAnimationFile = other.m_pAnimationFile;
    if( this->m_pAnimationFile )
    {
        this->m_pAnimationFile->AddRef();
    }

    return *this;
}

void ComponentAnimationPlayer2D::SetAnimationFile(MyFileObject* pFile)
{
    if( pFile )
        pFile->AddRef();

    SAFE_RELEASE( m_pAnimationFile );
    m_pAnimationFile = pFile;
}

void ComponentAnimationPlayer2D::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAnimationPlayer2D, OnFileRenamed );
    }
}

void ComponentAnimationPlayer2D::UnregisterCallbacks()
{
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

void ComponentAnimationPlayer2D::TickCallback(double TimePassed)
{
    if( m_pSpriteComponent == 0 )
    {
        ComponentBase* pComponent = m_pGameObject->GetFirstComponentOfBaseType( BaseComponentType_Renderable );
        if( pComponent )
            m_pSpriteComponent = pComponent->IsA( "SpriteComponent" ) ? (ComponentSprite*)pComponent : 0;
    }

    if( m_pSpriteComponent == 0 )
        return;

    if( m_pAnimInfo == 0 )
    {
        if( m_pAnimationFile && m_pAnimationFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            m_pAnimInfo = MyNew My2DAnimInfo();
            m_pAnimInfo->SetSourceFile( m_pAnimationFile );
            m_pAnimInfo->LoadAnimationControlFile( m_pAnimationFile->GetBuffer() );
        }
    }

    if( m_pAnimInfo == 0 )
        return;

    // Get the current animation/frame being played
    My2DAnimation* pAnim = m_pAnimInfo->GetAnimationByIndexClamped( m_AnimationIndex );
    if( pAnim->GetFrameCount() == 0 )
        return;

    My2DAnimationFrame* pFrame = pAnim->GetFrameByIndexClamped( m_FrameIndex );

    // Advance time and the current frame if needed
    m_AnimationTime += (float)TimePassed;

    float frameduration = pFrame->GetDuration();

    while( m_AnimationTime > frameduration && frameduration > 0 )
    {
        m_AnimationTime -= frameduration;
        m_FrameIndex++;

        uint32 framecount = pAnim->GetFrameCount();
        if( m_FrameIndex >= framecount )
            m_FrameIndex = 0;
    }

    pFrame = pAnim->GetFrameByIndexClamped( m_FrameIndex );

    // Set the material
    MaterialDefinition* pMaterial = pFrame->GetMaterial();
    m_pSpriteComponent->SetMaterial( pMaterial, 0 );

    //LOGInfo( LOGTag, "%d, %s\n", m_FrameIndex, (*pMaterial).GetName() );
}
