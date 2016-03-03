//
// Copyright (c) 2016 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentAudioPlayer::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentAudioPlayer ); //_VARIABLE_LIST

ComponentAudioPlayer::ComponentAudioPlayer()
: ComponentBase()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Data;

    m_pAudioFile = 0;
}

ComponentAudioPlayer::~ComponentAudioPlayer()
{
    SAFE_RELEASE( m_pAudioFile );

    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST
}

void ComponentAudioPlayer::RegisterVariables(CPPListHead* pList, ComponentAudioPlayer* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if MYFW_IOS || MYFW_OSX || MYFW_NACL
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    MyAssert( offsetof( ComponentAudioPlayer, m_pAudioFile ) == MyOffsetOf( pThis, &pThis->m_pAudioFile ) );
#if MYFW_IOS || MYFW_OSX
#pragma GCC diagnostic default "-Winvalid-offsetof"
#endif

    AddVar( pList, "AudioFile", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pAudioFile ), false, true, 0, (CVarFunc_ValueChanged)&ComponentAudioPlayer::OnValueChanged, (CVarFunc_DropTarget)&ComponentAudioPlayer::OnDrop, 0 );
}

void ComponentAudioPlayer::Reset()
{
    ComponentBase::Reset();

    SAFE_RELEASE( m_pAudioFile );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

#if MYFW_USING_LUA
void ComponentAudioPlayer::LuaRegister(lua_State* luastate)
{
    luabridge::getGlobalNamespace( luastate )
        .beginClass<ComponentSprite>( "ComponentAudioPlayer" )
            //.addData( "m_SampleVector3", &ComponentSprite::m_SampleVector3 )
            //.addFunction( "GetVector3", &ComponentSprite::GetVector3 )
        .endClass();
}
#endif //MYFW_USING_LUA

#if MYFW_USING_WX
void ComponentAudioPlayer::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentAudioPlayer::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "AudioPlayer", ObjectListIcon_Component );
}

void ComponentAudioPlayer::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentAudioPlayer::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Audio Player", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        g_pPanelWatch->AddButton( "Play Sound", this, ComponentAudioPlayer::StaticOnButtonPlaySound );
    }
}

void* ComponentAudioPlayer::OnDrop(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_FileObjectPointer )
    {
        SetAudioFile( (MyFileObject*)g_DragAndDropStruct.m_Value );
    }

    //if( g_DragAndDropStruct.m_Type == DragAndDropType_ComponentPointer )
    //{
    //    (ComponentBase*)g_DragAndDropStruct.m_Value;
    //}

    //if( g_DragAndDropStruct.m_Type == DragAndDropType_GameObjectPointer )
    //{
    //    (GameObject*)g_DragAndDropStruct.m_Value;
    //}

    return oldvalue;
}

void* ComponentAudioPlayer::OnValueChanged(ComponentVariable* pVar, int controlid, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( pVar->m_Offset == MyOffsetOf( this, &m_pAudioFile ) )
    {
        MyAssert( pVar->m_ControlID != -1 );
    }

    return oldpointer;
}

void ComponentAudioPlayer::OnButtonPlaySound()
{
    g_pGameCore->m_pSoundPlayer->StopMusic();
    g_pGameCore->m_pSoundPlayer->PlayMusic( m_pAudioFile->m_FullPath );
}
#endif //MYFW_USING_WX

cJSON* ComponentAudioPlayer::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jComponent = ComponentBase::ExportAsJSONObject( savesceneid );

    if( m_pAudioFile )
        cJSON_AddStringToObject( jComponent, "AudioFile", m_pAudioFile->m_FullPath );

    ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST

    return jComponent;
}

void ComponentAudioPlayer::ImportFromJSONObject(cJSON* jComponent, unsigned int sceneid)
{
    ComponentBase::ImportFromJSONObject( jComponent, sceneid );

    cJSON* scriptstringobj = cJSON_GetObjectItem( jComponent, "AudioFile" );
    if( scriptstringobj )
    {
        MyFileObject* pFile = g_pEngineFileManager->RequestFile( scriptstringobj->valuestring, GetSceneID() );
        MyAssert( pFile );
        if( pFile )
        {
            SetAudioFile( pFile );
            pFile->Release(); // free ref added by RequestFile
        }
    }

    ImportVariablesFromJSON( jComponent ); //_VARIABLE_LIST
}

ComponentAudioPlayer& ComponentAudioPlayer::operator=(const ComponentAudioPlayer& other)
{
    MyAssert( &other != this );

    ComponentBase::operator=( other );

    SetAudioFile( other.m_pAudioFile );

    return *this;
}

void ComponentAudioPlayer::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAudioPlayer, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAudioPlayer, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAudioPlayer, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAudioPlayer, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAudioPlayer, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAudioPlayer, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentAudioPlayer, OnFileRenamed );
    }
}

void ComponentAudioPlayer::UnregisterCallbacks()
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

void ComponentAudioPlayer::SetAudioFile(MyFileObject* pFile)
{
    if( pFile )
        pFile->AddRef();
    SAFE_RELEASE( m_pAudioFile );
    m_pAudioFile = pFile;
}
