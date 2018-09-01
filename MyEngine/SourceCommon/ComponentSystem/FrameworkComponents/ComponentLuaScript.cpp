//
// Copyright (c) 2014-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "../../../SourceEditor/PlatformSpecific/FileOpenDialog.h"

#if MYFW_USING_LUA

#if MYFW_USING_WX
bool ComponentLuaScript::m_PanelWatchBlockVisible = true;
#endif

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentLuaScript ); //_VARIABLE_LIST

ComponentLuaScript::ComponentLuaScript()
: ComponentUpdateable()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Updateable;
    m_Type = ComponentType_LuaScript;

    m_pScriptFile = 0;
#if !MYFW_EDITOR
    m_pLuaInlineScript_OnPlay = 0;
#endif

    m_ExposedVars.AllocateObjects( MAX_EXPOSED_VARS ); // hard coded nonsense for now, max of 4 exposed vars in a script.

#if MYFW_USING_WX
    g_pComponentSystemManager->Editor_RegisterFileUpdatedCallback( &StaticOnFileUpdated, this );
#endif
}

ComponentLuaScript::~ComponentLuaScript()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

        // unregister gameobject deleted callback, if we registered one.
        if( pVariable->type == ExposedVariableType_GameObject && pVariable->pointer )
            ((GameObject*)pVariable->pointer)->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        delete pVariable;
    }

#if !MYFW_EDITOR
    delete[] m_pLuaInlineScript_OnPlay;
#endif

    SAFE_RELEASE( m_pScriptFile );
}

void ComponentLuaScript::RegisterVariables(CPPListHead* pList, ComponentLuaScript* pThis) //_VARIABLE_LIST
{
    // just want to make sure these are the same on all compilers.  They should be since this is a simple class.
#if __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif
    MyAssert( offsetof( ComponentLuaScript, m_pScriptFile ) == MyOffsetOf( pThis, &pThis->m_pScriptFile ) );
#if __GNUC__
#pragma GCC diagnostic pop
#endif

    // Script is not automatically saved/loaded
    ComponentVariable* pVar = AddVar( pList, "Script", ComponentVariableType_FilePtr, MyOffsetOf( pThis, &pThis->m_pScriptFile ), false, true, 0, (CVarFunc_ValueChanged)&ComponentLuaScript::OnValueChanged, (CVarFunc_DropTarget)&ComponentLuaScript::OnDrop, 0 );
#if MYFW_USING_IMGUI
    pVar->AddCallback_OnRightClick( (CVarFunc_wxMenu)&ComponentLuaScript::OnRightClickCallback, 0 );
#endif
#if MYFW_USING_WX
    pVar->AddCallback_OnRightClick( (CVarFunc_wxMenu)&ComponentLuaScript::OnRightClickCallback, (CVarFunc_Int)&ComponentLuaScript::OnPopupClickCallback );
#endif

    // m_pLuaInlineScript_OnPlay is not automatically saved/loaded
    pVar = AddVarPointer( pList, "OnPlay", false, true, 0, 
        (CVarFunc_GetPointerValue)&ComponentLuaScript::GetPointerValue, (CVarFunc_SetPointerValue)&ComponentLuaScript::SetPointerValue,
        (CVarFunc_GetPointerDesc)&ComponentLuaScript::GetPointerDesc, (CVarFunc_SetPointerDesc)&ComponentLuaScript::SetPointerDesc,
        (CVarFunc_ValueChanged)&ComponentLuaScript::OnValueChanged, 0, 0 );
}

void ComponentLuaScript::Reset()
{
    ComponentUpdateable::Reset();

    m_pLuaGameState = g_pLuaGameState;

    m_ScriptLoaded = false;
    m_Playing = false;
    m_ErrorInScript = false;
    m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = false;

    m_pCopyExternsFromThisComponentAfterLoadingScript = 0;

    while( m_ExposedVars.Count() )
    {
        ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex( 0 );

        // unregister gameobject deleted callback, if we registered one.
        if( pVariable->type == ExposedVariableType_GameObject && pVariable->pointer )
            ((GameObject*)pVariable->pointer)->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

        delete pVariable;
    }

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
    m_ControlIDOfFirstExtern = -1;
#endif //MYFW_USING_WX
}

void* ComponentLuaScript::GetPointerValue(ComponentVariable* pVar) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}

    return 0;
}

void ComponentLuaScript::SetPointerValue(ComponentVariable* pVar, const void* newvalue) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}
}

const char* ComponentLuaScript::GetPointerDesc(ComponentVariable* pVar) //_VARIABLE_LIST
{
    if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    {
#if MYFW_EDITOR
        return m_pLuaInlineScript_OnPlay.c_str();
#endif
    }

    return "fix me";
}

void ComponentLuaScript::SetPointerDesc(ComponentVariable* pVar, const char* newdesc) //_VARIABLE_LIST
{
    //if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    //{
    //}
}

#if MYFW_EDITOR
void ComponentLuaScript::CreateNewScriptFile()
{
    //if( m_pScriptFile == 0 )
    {
        // generally offer to create scripts in Scripts folder.
#if MYFW_USING_IMGUI
        const char* initialpath = "Data\\Scripts\\";
#elif MYFW_USING_WX
        const char* initialpath = "./Data/Scripts";
#endif

        bool ismenuactionscript = false;
        bool ismeshscript = false;

        // If a ComponentMenuPage is attached to this game object, then offer to make the file in the menus folder.
        if( m_pGameObject->GetFirstComponentOfType( "MenuPageComponent" ) != 0 )
        {
            ismenuactionscript = true;
#if MYFW_USING_IMGUI
            initialpath = "Data\\Menus\\";
#elif MYFW_USING_WX
            initialpath = "./Data/Menus";
#endif
        }

        // If a ComponentMesh is attached to this game object, then add a SetupCustomUniforms callback.
        if( m_pGameObject->GetFirstComponentOfType( "MeshComponent" ) != 0 )
        {
            ismeshscript = true;
        }

#if MYFW_USING_WX
        wxFileDialog FileDialog( g_pEngineMainFrame, _("Create Lua script file"), initialpath, "", "Lua script files (*.lua)|*.lua", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    
        if( FileDialog.ShowModal() != wxID_CANCEL )
        {
            wxString wxpath = FileDialog.GetPath();
            char fullpath[MAX_PATH];
            sprintf_s( fullpath, MAX_PATH, "%s", (const char*)wxpath );
#else
        const char* filename = FileSaveDialog( initialpath, "Lua script files\0*.lua\0All\0*.*\0" );
        if( filename[0] != 0 )
        {
            int len = strlen( filename );

            // Append '.lua' to end of filename if it wasn't already there.
            char fullpath[MAX_PATH];
            if( len > 4 && strcmp( &filename[len-4], ".lua" ) == 0 )
            {
                strcpy_s( fullpath, MAX_PATH, filename );
            }
            else
            {
                sprintf_s( fullpath, MAX_PATH, "%s.lua", filename );
            }
#endif
            const char* relativepath = GetRelativePath( fullpath );

            MyFileObject* pScriptFile = g_pComponentSystemManager->LoadDataFile( relativepath, m_pGameObject->GetSceneID(), 0, true );
            SetScriptFile( pScriptFile );

#if MYFW_USING_WX
            // update the panel so new filename shows up. // TODO: this won't refresh lua variables, so maybe refresh the whole watch panel.
            int scriptcontrolid = FindVariablesControlIDByLabel( "Script" );
            g_pPanelWatch->GetVariableProperties( scriptcontrolid )->m_Description = m_pScriptFile->GetFullPath();
#endif

            // TODO: create a template file.
            {
                FILE* file;
#if MYFW_WINDOWS
                fopen_s( &file, fullpath, "wb" );
#else
                file = fopen( fullpath, "wb" );
#endif

                if( file )
                {
                    if( ismenuactionscript )
                    {
                        fprintf( file, "-- Menu Action File\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "%s =\n", m_pScriptFile->GetFilenameWithoutExtension() );
                        fprintf( file, "{\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnVisible = function(visible)\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnAction = function(action)\n" );
                        fprintf( file, "--LogInfo( \"OnAction was called: \" .. action .. \"\\n\" );\n" );
                        fprintf( file, "end\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "}\n" );
                    }
                    else
                    {
                        fprintf( file, "-- %s Script\n", m_pScriptFile->GetFilenameWithoutExtension() );
                        fprintf( file, "\n" );
                        fprintf( file, "%s =\n", m_pScriptFile->GetFilenameWithoutExtension() );
                        fprintf( file, "{\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnLoad = function()\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnPlay = function()\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnStop = function()\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnTouch = function(action, id, x, y, pressure, size)\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "OnButtons = function(action, id)\n" );
                        fprintf( file, "end,\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "Tick = function(deltaTime)\n" );

                        if( ismeshscript )
                        {
                            fprintf( file, "end,\n" );
                            fprintf( file, "\n" );
                            fprintf( file, "SetupCustomUniforms = function(programhandle)\n" );
                        }

                        fprintf( file, "end\n" );
                        fprintf( file, "\n" );
                        fprintf( file, "}\n" );
                    }

                    fclose( file );
                }
            }

            m_pScriptFile->OSLaunchFile( true );
        }
    }
}

#if MYFW_USING_IMGUI
void ComponentLuaScript::AddAllVariablesToWatchPanel()
{
    ComponentBase::AddAllVariablesToWatchPanel();

    ImGui::Indent( 20 );

    // Add all component variables.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        switch( pVar->type )
        {
        case ExposedVariableType_Unused:
            MyAssert( false );
            break;

        case ExposedVariableType_Float:
            {
                float afloat = (float)pVar->valuedouble;
                bool modified = ImGui::DragFloat( pVar->name.c_str(), &afloat, 0.1f, 0, 0 );
                pVar->valuedouble = afloat;
                OnExposedVarValueChanged( pVar, 0, true, 0, 0 );
                //id = g_pPanelWatch->AddDouble( pVar->name.c_str(), &pVar->valuedouble, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
            }
            break;

        case ExposedVariableType_Bool:
            {
                //id = g_pPanelWatch->AddBool( pVar->name.c_str(), &pVar->valuebool, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
            }
            break;

        case ExposedVariableType_Vector3:
            {
                //id = g_pPanelWatch->AddVector3( pVar->name.c_str(), (Vector3*)&pVar->valuevector3, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
            }
            break;

        case ExposedVariableType_GameObject:
            {
                // setup name and drag and drop of game objects.
                //const char* desc = "no gameobject";
                //if( pVar->pointer )
                //    desc = ((GameObject*)pVar->pointer)->GetName();
                //id = g_pPanelWatch->AddPointerWithDescription( pVar->name.c_str(), pVar->pointer, desc, this, ComponentLuaScript::StaticOnDropExposedVar, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
            }
            break;
        }

        if( pVar->divorced )
        {
            //g_pPanelWatch->ChangeStaticTextFontStyle( id, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
            //g_pPanelWatch->ChangeStaticTextBGColor( id, wxColour( 255, 200, 200, 255 ) );
        }
    }

    ImGui::Unindent( 20 );
}
#endif

#if MYFW_USING_WX
void ComponentLuaScript::OnFileUpdated(MyFileObject* pFile)
{
    if( pFile == m_pScriptFile )
    {
        m_ScriptLoaded = false;
        m_ErrorInScript = false;

        if( m_Playing )
        {
            m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = true;
        }
    }
}

void ComponentLuaScript::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentLuaScript::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Lua script", ObjectListIcon_Component );
}

void ComponentLuaScript::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentLuaScript::FillPropertiesWindow(bool clear, bool addcomponentvariables, bool ignoreblockvisibleflag)
{
    m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Lua Script", this, ComponentBase::StaticOnComponentTitleLabelClicked );

    m_ControlIDOfFirstExtern = -1;

    if( m_PanelWatchBlockVisible || ignoreblockvisibleflag == true )
    {
        ComponentBase::FillPropertiesWindow( clear );

        if( addcomponentvariables )
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST

        //const char* desc = "no script";
        //if( m_pScriptFile )
        //    desc = m_pScriptFile->GetFullPath();
        //m_ControlID_Script = g_pPanelWatch->AddPointerWithDescription( "Script", 0, desc, this, ComponentLuaScript::StaticOnDrop );

        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            int id = -1;

            ExposedVariableDesc* pVar = m_ExposedVars[i];

            switch( pVar->type )
            {
            case ExposedVariableType_Unused:
                MyAssert( false );
                break;

            case ExposedVariableType_Float:
                {
                    id = g_pPanelWatch->AddDouble( pVar->name.c_str(), &pVar->valuedouble, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
                }
                break;

            case ExposedVariableType_Bool:
                {
                    id = g_pPanelWatch->AddBool( pVar->name.c_str(), &pVar->valuebool, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
                }
                break;

            case ExposedVariableType_Vector3:
                {
                    id = g_pPanelWatch->AddVector3( pVar->name.c_str(), (Vector3*)&pVar->valuevector3, 0, 0, this, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
                }
                break;

            case ExposedVariableType_GameObject:
                {
                    // setup name and drag and drop of game objects.
                    const char* desc = "no gameobject";
                    if( pVar->pointer )
                        desc = ((GameObject*)pVar->pointer)->GetName();
                    id = g_pPanelWatch->AddPointerWithDescription( pVar->name.c_str(), pVar->pointer, desc, this, ComponentLuaScript::StaticOnDropExposedVar, ComponentLuaScript::StaticOnPanelWatchExposedVarValueChanged, ComponentLuaScript::StaticOnRightClickExposedVariable );
                }
                break;
            }

            if( pVar->divorced )
            {
                g_pPanelWatch->ChangeStaticTextFontStyle( id, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
                g_pPanelWatch->ChangeStaticTextBGColor( id, wxColour( 255, 200, 200, 255 ) );
            }

            if( m_ControlIDOfFirstExtern == -1 )
                m_ControlIDOfFirstExtern = id;

            pVar->controlID = id;
        }
    }
}

void ComponentLuaScript::AppendItemsToRightClickMenu(wxMenu* pMenu)
{
    ComponentBase::AppendItemsToRightClickMenu( pMenu );

    pMenu->Append( RightClick_CreateNewScriptFile, "Create new script" );
 	pMenu->Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentLuaScript::OnPopupClick );
}

void ComponentLuaScript::OnPopupClick(wxEvent &evt)
{
    ComponentLuaScript* pComponent = (ComponentLuaScript*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    MyAssert( pComponent->IsA( "LuaScriptComponent" ) );

    // ComponentBase::OnPopupClick could delete component, so pComponent would point to garbage.
    ComponentBase::OnPopupClick( evt );

    int id = evt.GetId();

    switch( id )
    {
    case RightClick_CreateNewScriptFile:    pComponent->CreateNewScriptFile();     break;
    }
}
#endif //MYFW_USING_WX

void* ComponentLuaScript::OnDrop(ComponentVariable* pVar, int x, int y)
{
    void* oldPointer = 0;

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)pDropItem->m_Value;
        MyAssert( pFile );

        if( strcmp( pFile->GetExtensionWithDot(), ".lua" ) == 0 )
        {
            oldPointer = m_pScriptFile;

            // TODO: Add an undo command.
            SetScriptFile( pFile );

#if MYFW_USING_WX
            // update the panel so new filename shows up.
            // TODO: this won't refresh lua variables, so maybe refresh the whole watch panel.
            g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->m_Description = m_pScriptFile->GetFullPath();
#endif //MYFW_USING_WX
        }
    }

    return oldPointer;
}

void* ComponentLuaScript::OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
{
    void* oldpointer = 0;

    if( strcmp( pVar->m_Label, "Script" ) == 0 )
    {
        if( changedbyinterface )
        {
#if MYFW_USING_WX
            wxString text = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
            if( text == "" || text == "none" || text == "no script" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "no script" );
                oldpointer = m_pScriptFile;

                // TODO: undo/redo
                this->SetScriptFile( 0 );
            }
#endif //MYFW_USING_WX
        }
        else
        {
            MyAssert( false );
            // TODO: implement this block
        }
    }

    if( strcmp( pVar->m_Label, "OnPlay" ) == 0 )
    {
#if MYFW_USING_WX
        m_pLuaInlineScript_OnPlay = g_pPanelWatch->GetVariableProperties( pVar->m_ControlID )->GetTextCtrl()->GetValue();
#endif //MYFW_USING_WX
    }

    return oldpointer;
}

#if MYFW_USING_IMGUI
void ComponentLuaScript::OnRightClickCallback(ComponentVariable* pVar, void* pMenu)
{
    if( m_pScriptFile )
    {
        if( ImGui::MenuItem( "Edit script" ) )
        {
            m_pScriptFile->OSLaunchFile( true );
            ImGui::CloseCurrentPopup();
        }
    }

    if( ImGui::MenuItem( "Create new script" ) )
    {
        CreateNewScriptFile();
        ImGui::CloseCurrentPopup();
    }
}
#endif

#if MYFW_USING_WX
void ComponentLuaScript::OnRightClickCallback(ComponentVariable* pVar, wxMenu* pMenu)
{
    if( m_pScriptFile == 0 )
    {
        pMenu->Append( RightClick_CreateNewScriptFile, "Create new script" );
    }
    else
    {
        pMenu->Append( RightClick_LaunchScriptEditor, "Edit script" );
    }
}

void ComponentLuaScript::OnPopupClickCallback(ComponentVariable* pVar, int id)
{
    switch( id )
    {
    case RightClick_CreateNewScriptFile:
        CreateNewScriptFile();
        break;

    case RightClick_LaunchScriptEditor:
        m_pScriptFile->OSLaunchFile( true );
        break;
    }
}

void* ComponentLuaScript::ProcessOnDropExposedVar(int controlid, int x, int y)
{
    void* oldpointer = 0;

    ComponentUpdateable::OnDrop( controlid, x, y );

    DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

    if( pDropItem->m_Type == DragAndDropType_FileObjectPointer )
    {
        MyFileObject* pFile = (MyFileObject*)pDropItem->m_Value;
        MyAssert( pFile );

        if( strcmp( pFile->GetExtensionWithDot(), ".lua" ) == 0 )
        {
            oldpointer = m_pScriptFile;
            SetScriptFile( pFile );

            // update the panel so new filename shows up. // TODO: this won't refresh lua variables, so maybe refresh the whole watch panel.
            int scriptcontrolid = FindVariablesControlIDByLabel( "Script" );
            g_pPanelWatch->GetVariableProperties( scriptcontrolid )->m_Description = m_pScriptFile->GetFullPath();
        }
    }

    if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
    {
        GameObject* pGameObject = (GameObject*)pDropItem->m_Value;
        MyAssert( pGameObject );

        MyAssert( m_ControlIDOfFirstExtern != -1 );
        if( m_ControlIDOfFirstExtern != -1 )
        {
            int id = g_DragAndDropStruct.GetControlID() - m_ControlIDOfFirstExtern;
        
            MyAssert( id < (int)m_ExposedVars.Count() );
            if( id < (int)m_ExposedVars.Count() )
            {
                MyAssert( pGameObject->IsA( "GameObject" ) );
        
                // unregister the old gameobject.
                if( m_ExposedVars[id]->pointer )
                    ((GameObject*)m_ExposedVars[id]->pointer)->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

                oldpointer = m_ExposedVars[id]->pointer;
                m_ExposedVars[id]->pointer = pGameObject;
                ((GameObject*)m_ExposedVars[id]->pointer)->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

                // update the panel so new gameobject name shows up.
                g_pPanelWatch->GetVariableProperties( g_DragAndDropStruct.GetControlID() )->m_Description = pGameObject->GetName();
            }
        }
    }

    return oldpointer;
}

void ComponentLuaScript::OnDropExposedVar(int controlid, int x, int y)
{
    void* oldpointer = ProcessOnDropExposedVar( controlid, x, y );

    ExposedVariableDesc* pVar = 0;

    if( controlid != -1 )
    {
        for( unsigned int index=0; index<m_ExposedVars.Count(); index++ )
        {
            if( m_ExposedVars[index]->controlID == controlid )
            {
                pVar = m_ExposedVars[index];
                break;
            }
        }
    }

    MyAssert( pVar != 0 );

    // divorce the child value from it's parent, if it no longer matches.
    if( DoesExposedVariableMatchParent( pVar ) == false ) // returns true if no parent was found.
    {
        // if the variable no longer matches the parent, then divorce it.
        pVar->divorced = true;
        g_pPanelWatch->ChangeStaticTextFontStyle( pVar->controlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
        g_pPanelWatch->ChangeStaticTextBGColor( pVar->controlID, wxColour( 255, 200, 200, 255 ) );
    }

    UpdateChildrenWithNewValue( pVar, true, 0, oldpointer );
    ProgramVariables( m_pLuaGameState->m_pLuaState, true );
}

void ComponentLuaScript::OnPanelWatchExposedVarValueChanged(int controlid, bool finishedchanging, double oldvalue)
{
    ExposedVariableDesc* pVar = 0;
    int component = 0;
    void* oldpointer = 0;

    // find the pVar associated with the controlid, might be a subcomponent of a vec3
    if( controlid != -1 && m_ControlIDOfFirstExtern != -1 )
    {
        for( unsigned int index=0; index<m_ExposedVars.Count(); index++ )
        {
            if( m_ExposedVars[index]->controlID == controlid )
            {
                pVar = m_ExposedVars[index];
                break;
            }

            if( m_ExposedVars[index]->type == ExposedVariableType_Vector3 &&
                  ( m_ExposedVars[index]->controlID + 0 != controlid ||
                    m_ExposedVars[index]->controlID + 1 != controlid ||
                    m_ExposedVars[index]->controlID + 2 != controlid )
              )
            {
                pVar = m_ExposedVars[index];
                component = controlid - m_ExposedVars[index]->controlID;
                break;
            }
        }
    }

    // if pVar is null, the controlid wasn't found, which shouldn't happen
    MyAssert( pVar != 0 );

    // deal with parts of GameObject pointers changes here
    if( pVar->type == ExposedVariableType_GameObject )
    {
        GameObject* pGameObject = (GameObject*)pVar->pointer;

        MyAssert( pGameObject->IsA( "GameObject" ) );

        wxString text = g_pPanelWatch->GetVariableProperties( controlid )->GetTextCtrl()->GetValue();
        if( text == "" || text == "none" || text == "no gameobject" )
        {
            g_pPanelWatch->ChangeDescriptionForPointerWithDescription( controlid, "no gameobject" );

            oldpointer = pVar->pointer;

            pVar->pointer = 0;
        }
    }

    // deal with all types here (float, bool, vec3, GameObject)
    OnExposedVarValueChanged( pVar, component, finishedchanging, oldvalue, oldpointer );
}
#endif //MYFW_USING_WX

void ComponentLuaScript::OnExposedVarValueChanged(ExposedVariableDesc* pVar, int component, bool finishedchanging, double oldvalue, void* oldpointer)
{
    // Register/unregister gameobject pointer ondelete callbacks
    if( pVar->type == ExposedVariableType_GameObject )
    {
        GameObject* pOldGameObject = (GameObject*)oldpointer;
        GameObject* pGameObject = (GameObject*)pVar->pointer;

        // unregister gameobject deleted callback, if we registered one.
        if( pOldGameObject )
        {
            MyAssert( pOldGameObject->IsA( "GameObject" ) );
            pOldGameObject->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
        }

        // register new gameobject deleted callback
        if( pGameObject )
        {
            MyAssert( pGameObject->IsA( "GameObject" ) );
            pGameObject->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
        }
    }

    // divorce the child value from it's parent, if it no longer matches.
    if( DoesExposedVariableMatchParent( pVar ) == false ) // returns true if no parent was found.
    {
        // if the variable no longer matches the parent, then divorce it.
        pVar->divorced = true;
#if MYFW_USING_WX
        g_pPanelWatch->ChangeStaticTextFontStyle( pVar->controlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
        g_pPanelWatch->ChangeStaticTextBGColor( pVar->controlID, wxColour( 255, 200, 200, 255 ) );
#endif
    }

    UpdateChildrenWithNewValue( pVar, finishedchanging, oldvalue, 0 );
    ProgramVariables( m_pLuaGameState->m_pLuaState, true );
}

#if MYFW_USING_WX
void ComponentLuaScript::OnRightClickExposedVariable(int controlid)
{
    // the only right-click options involve this components gameobject having a parent, so quick early if it doesn't
    if( m_pGameObject->GetGameObjectThisInheritsFrom() == 0 )
        return;

    ExposedVariableDesc* pExposedVar = 0;

    if( controlid != -1 && m_ControlIDOfFirstExtern != -1 )
    {
        for( unsigned int index=0; index<m_ExposedVars.Count(); index++ )
        {
            if( m_ExposedVars[index]->controlID == controlid )
            {
                pExposedVar = m_ExposedVars[index];
                break;
            }
        }
    }

    if( pExposedVar == 0 )
    {
        ComponentBase::OnRightClickVariable( controlid );
        return;
    }

    wxMenu menu;
    menu.SetClientData( &m_ComponentLuaScriptEventHandlerForExposedVariables );

    m_ComponentLuaScriptEventHandlerForExposedVariables.pLuaScriptComponent = this;
    m_ComponentLuaScriptEventHandlerForExposedVariables.pExposedVar = pExposedVar;

    // if this game object inherits from another, right-clicking a variable will offer divorce/marry options.
    if( m_pGameObject->GetGameObjectThisInheritsFrom() )
    {
        if( pExposedVar->divorced == false )
        {
            menu.Append( RightClick_DivorceVariable, "Divorce value from parent" );
 	        menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentLuaScriptEventHandlerForExposedVariables::OnPopupClick );
        }
        else
        {
            menu.Append( RightClick_MarryVariable, "Reset value to parent" );
 	        menu.Connect( wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&ComponentLuaScriptEventHandlerForExposedVariables::OnPopupClick );
        }
    }

    // blocking call.
    g_pPanelWatch->PopupMenu( &menu ); // there's no reason this is using g_pPanelWatch other than convenience.
}

void ComponentLuaScriptEventHandlerForExposedVariables::OnPopupClick(wxEvent &evt)
{
    ComponentLuaScriptEventHandlerForExposedVariables* pEvtHandler = (ComponentLuaScriptEventHandlerForExposedVariables*)static_cast<wxMenu*>(evt.GetEventObject())->GetClientData();
    ComponentLuaScript* pLuaScriptComponent = pEvtHandler->pLuaScriptComponent;
    ExposedVariableDesc* pExposedVar = pEvtHandler->pExposedVar;

    int id = evt.GetId();
    switch( id )
    {
    case ComponentBase::RightClick_DivorceVariable:
        {
            pExposedVar->divorced = true;
            g_pPanelWatch->ChangeStaticTextFontStyle( pExposedVar->controlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
            g_pPanelWatch->ChangeStaticTextBGColor( pExposedVar->controlID, wxColour( 255, 200, 200, 255 ) );
        }
        break;

    case ComponentBase::RightClick_MarryVariable:
        {
            pExposedVar->divorced = false;
            g_pPanelWatch->ChangeStaticTextFontStyle( pExposedVar->controlID, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
            g_pPanelWatch->ChangeStaticTextBGColor( pExposedVar->controlID, wxNullColour );
            
            // change the value of this variable to match the parent.
            pLuaScriptComponent->CopyExposedVarValueFromParent( pExposedVar );
        }
        break;
    }
}
#endif // MYFW_USING_WX

bool ComponentLuaScript::DoesExposedVariableMatchParent(ExposedVariableDesc* pVar)
{
    MyAssert( m_pGameObject );
    if( m_pGameObject == 0 )
        return true; // the object has no parent, we say it matches.
    
    GameObject* pGameObject = m_pGameObject->GetGameObjectThisInheritsFrom();
    if( pGameObject == 0 )
        return true; // the object has no parent, we say it matches.

    // Found a game object, now find the matching component on it.
    for( unsigned int i=0; i<pGameObject->GetComponentCount(); i++ )
    {
        ComponentBase* pOtherComponent = pGameObject->GetComponentByIndex( i );

        const char* pThisCompClassName = GetClassname();
        const char* pOtherCompClassName = pOtherComponent->GetClassname();

        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
        {
            ComponentLuaScript* pOtherLuaScript = (ComponentLuaScript*)pOtherComponent;

            // find children of this gameobject and change their vars if needed.
            for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
            {
                ExposedVariableDesc* pOtherVar = (ExposedVariableDesc*)pOtherLuaScript->m_ExposedVars[varindex];
                MyAssert( pOtherVar );

                if( pVar->name == pOtherVar->name )
                {
                    switch( pVar->type )
                    {
                    case ExposedVariableType_Float:
                        return pVar->valuedouble == pOtherVar->valuedouble;

                    case ExposedVariableType_Bool:
                        return pVar->valuebool == pOtherVar->valuebool;

                    case ExposedVariableType_Vector3:
                        return pVar->valuevector3 == pOtherVar->valuevector3;

                    case ExposedVariableType_GameObject:
                        return pVar->pointer == pOtherVar->pointer;

                    case ExposedVariableType_Unused:
                    default:
                        MyAssert( false );
                        break;
                    }
                }
            }
        }
    }

    MyAssert( false ); // shouldn't get here.
    return true; // the object has no parent, we say it matches.
}

void ComponentLuaScript::UpdateChildrenWithNewValue(ExposedVariableDesc* pVar, bool finishedchanging, double oldvalue, void* oldpointer)
{
    MyAssert( pVar );

    // find children of this gameobject and change their vars if needed.
    for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
    {
        ExposedVariableDesc* pOtherVar = (ExposedVariableDesc*)m_ExposedVars[varindex];
        MyAssert( pOtherVar );

        if( pVar->name == pOtherVar->name )
        {
#if 0 //MYFW_USING_WX
            typedef std::map<int, SceneInfo>::iterator it_type;
            for( it_type iterator = g_pComponentSystemManager->m_pSceneInfoMap.begin(); iterator != g_pComponentSystemManager->m_pSceneInfoMap.end(); )
            {
                SceneID sceneid = iterator->first;
                SceneInfo* pSceneInfo = &iterator->second;
#else
            for( unsigned int i=0; i<MAX_SCENES_LOADED_INCLUDING_UNMANAGED; i++ )
            {
                if( g_pComponentSystemManager->m_pSceneInfoMap[i].m_InUse == false )
                    continue;

                SceneInfo* pSceneInfo = &g_pComponentSystemManager->m_pSceneInfoMap[i];
#endif //MYFW_USING_WX

                if( (GameObject*)pSceneInfo->m_GameObjects.GetHead() )
                {
                    GameObject* first = (GameObject*)pSceneInfo->m_GameObjects.GetHead();
                    UpdateChildrenInGameObjectListWithNewValue( pVar, varindex, first, finishedchanging, oldvalue, oldpointer );
                } 
            }
        }
    }
}

void ComponentLuaScript::UpdateChildrenInGameObjectListWithNewValue(ExposedVariableDesc* pVar, unsigned int varindex, GameObject* first, bool finishedchanging, double oldvalue, void* oldpointer)
{
    // find children of this gameobject and change their values as well, if their value matches the old value.
    for( CPPListNode* pCompNode = first; pCompNode; pCompNode = pCompNode->GetNext() )
    {
        GameObject* pGameObject = (GameObject*)pCompNode;

        if( pGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
        {
            UpdateChildGameObjectWithNewValue( pVar, varindex, pGameObject, finishedchanging, oldvalue, oldpointer );
        }

        GameObject* pFirstChild = pGameObject->GetFirstChild();
        if( pFirstChild )
        {
            UpdateChildrenInGameObjectListWithNewValue( pVar, varindex, pFirstChild, finishedchanging, oldvalue, oldpointer );
        }
    }
}

void ComponentLuaScript::UpdateChildGameObjectWithNewValue(ExposedVariableDesc* pVar, unsigned int varindex, GameObject* pChildGameObject, bool finishedchanging, double oldvalue, void* oldpointer)
{
    if( pChildGameObject->GetGameObjectThisInheritsFrom() == this->m_pGameObject )
    {
        // Found a game object, now find the matching component on it.
        for( unsigned int i=0; i<pChildGameObject->GetComponentCount(); i++ )
        {
            ComponentLuaScript* pChildLuaScript = (ComponentLuaScript*)pChildGameObject->GetComponentByIndex( i );

            const char* pThisCompClassName = GetClassname();
            const char* pChildCompClassName = pChildLuaScript->GetClassname();

            // TODO: this will fail if multiple of the same component are on an object.
            if( strcmp( pThisCompClassName, pChildCompClassName ) == 0 )
            {
                // it's possible the variables are in a different order, so find the correct variable by name
                ExposedVariableDesc* pChildVar = 0;

                // find the first variable in the other object with the same name
                for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
                {
                    if( pChildLuaScript->m_ExposedVars.Count() > i &&
                        m_ExposedVars[varindex]->name == pChildLuaScript->m_ExposedVars[i]->name )
                    {
                        pChildVar = pChildLuaScript->m_ExposedVars[i];
                        break;
                    }
                }

                if( pChildVar )
                {
                    // Found the matching component, now compare the variable.
                    if( pChildVar->type == ExposedVariableType_Float ||
                        pChildVar->type == ExposedVariableType_Bool )
                    {
                        if( fequal( pChildVar->valuedouble, oldvalue ) )
                        {
                            pChildVar->valuedouble = pVar->valuedouble;
                            //pChildLuaScript->OnExposedVarValueChanged( controlid, finishedchanging, oldvalue );

                            pChildLuaScript->ProgramVariables( m_pLuaGameState->m_pLuaState, true );
                            pChildLuaScript->UpdateChildrenWithNewValue( pChildVar, finishedchanging, oldvalue, oldpointer );
                        }
                    }

                    if( pChildVar->type == ExposedVariableType_Vector3 )
                    {
                        MyAssert( false );
                        //if( fequal( pChildVar->valuevector3[0], oldvalue ) )
                        //{
                        //    pChildVar->valuedouble = pVar->valuedouble;
                        //    pChildLuaScript->OnValueChanged( controlid, finishedchanging, oldvalue );

                        //    pChildLuaScript->ProgramVariables( m_pLuaGameState->m_pLuaState, true );
                        //    pChildLuaScript->UpdateChildrenWithNewValue( controlid, finishedchanging, oldvalue, oldpointer );
                        //}
                    }

                    if( pVar->type == ExposedVariableType_GameObject )
                    {
                        if( pChildVar->pointer == oldpointer )
                        {
                            pChildVar->pointer = pVar->pointer;
                            if( pVar->pointer )
                                ((GameObject*)pVar->pointer)->RegisterOnDeleteCallback( pChildLuaScript, StaticOnGameObjectDeleted );

                            pChildLuaScript->ProgramVariables( m_pLuaGameState->m_pLuaState, true );
                            pChildLuaScript->UpdateChildrenWithNewValue( pChildVar, finishedchanging, oldvalue, oldpointer );
                        }
                    }
                }
            }
        }
    }
}

void ComponentLuaScript::CopyExposedVarValueFromParent(ExposedVariableDesc* pVar)
{
    MyAssert( m_pGameObject );
    MyAssert( m_pGameObject->GetGameObjectThisInheritsFrom() );

    GameObject* pParentGO = m_pGameObject->GetGameObjectThisInheritsFrom();
    if( pParentGO == 0 )
        return;
    
    // Found a game object, now find the matching component on it.
    for( unsigned int i=0; i<pParentGO->GetComponentCount(); i++ )
    {
        ComponentBase* pOtherComponent = pParentGO->GetComponentByIndex( i );

        const char* pThisCompClassName = GetClassname();
        const char* pOtherCompClassName = pOtherComponent->GetClassname();

        if( strcmp( pThisCompClassName, pOtherCompClassName ) == 0 )
        {
            ComponentLuaScript* pOtherLuaScript = (ComponentLuaScript*)pOtherComponent;

            // find children of this gameobject and change their vars if needed.
            for( unsigned int varindex=0; varindex<m_ExposedVars.Count(); varindex++ )
            {
                ExposedVariableDesc* pOtherVar = (ExposedVariableDesc*)pOtherLuaScript->m_ExposedVars[varindex];
                MyAssert( pOtherVar );

                if( pVar->name == pOtherVar->name )
                {
                    switch( pVar->type )
                    {
                    case ExposedVariableType_Float:
                        {
                            double oldvalue = pVar->valuedouble;
                            double newvalue = pOtherVar->valuedouble;
                            pVar->valuedouble = pOtherVar->valuedouble;

                            // notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue, 0 );

#if MYFW_USING_WX
                            g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue - oldvalue, PanelWatchType_Double, ((char*)&pVar->valuedouble), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
#endif
                        }
                        break;

                    case ExposedVariableType_Bool:
                        {
                            bool oldvalue = pVar->valuebool;
                            bool newvalue = pOtherVar->valuebool;
                            pVar->valuedouble = pOtherVar->valuebool;

                            // notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue, 0 );

#if MYFW_USING_WX
                            g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue - oldvalue, PanelWatchType_Bool, ((char*)&pVar->valuebool), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
#endif
                        }
                        break;

                    case ExposedVariableType_Vector3:
                        {
                            Vector3 oldvalue = *(Vector3*)&pVar->valuevector3;
                            Vector3 newvalue = *(Vector3*)&pOtherVar->valuevector3;
                            *(Vector3*)&pVar->valuevector3 = *(Vector3*)&pOtherVar->valuevector3;

                            // notify component and it's children that the value changed.
                            OnExposedVarValueChanged( pVar, 0, true, oldvalue.x, 0 );
                            OnExposedVarValueChanged( pVar, 1, true, oldvalue.y, 0 );
                            OnExposedVarValueChanged( pVar, 2, true, oldvalue.z, 0 );

#if MYFW_USING_WX
                            g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.x - oldvalue.x, PanelWatchType_Float, ((char*)&pVar->valuevector3[0]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ) );
                            g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.y - oldvalue.y, PanelWatchType_Float, ((char*)&pVar->valuevector3[1]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ), true );
                            g_pGameCore->GetCommandStack()->Add( MyNew EditorCommand_PanelWatchNumberValueChanged(
                                newvalue.z - oldvalue.z, PanelWatchType_Float, ((char*)&pVar->valuevector3[2]), pVar->controlID, false,
                                g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pOnValueChangedCallbackFunc, g_pPanelWatch->GetVariableProperties( pVar->controlID )->m_pCallbackObj ), true );
#endif
                        }
                        break;

                    case ExposedVariableType_GameObject:
                        g_DragAndDropStruct.Clear();
                        g_DragAndDropStruct.SetControlID( pVar->controlID );
                        g_DragAndDropStruct.Add( DragAndDropType_GameObjectPointer, pOtherVar->pointer );
#if MYFW_USING_WX
                        OnDropExposedVar( pVar->controlID, 0, 0 );
#endif
                        break;

                    case ExposedVariableType_Unused:
                    default:
                        MyAssert( false );
                        break;
                    }
                }
            }
        }
    }
}
#endif //MYFW_EDITOR

cJSON* ComponentLuaScript::ExportAsJSONObject(bool savesceneid, bool saveid)
{
    cJSON* jComponent = ComponentUpdateable::ExportAsJSONObject( savesceneid, saveid );

    if( m_pScriptFile )
        cJSON_AddStringToObject( jComponent, "Script", m_pScriptFile->GetFullPath() );

#if MYFW_EDITOR
    // Trim white-space before saving.
    size_t left = m_pLuaInlineScript_OnPlay.find_first_not_of( " \t" );
    if( left != std::string::npos )
    {
        size_t right = m_pLuaInlineScript_OnPlay.find_last_not_of( " \t" );
        m_pLuaInlineScript_OnPlay = m_pLuaInlineScript_OnPlay.substr( left, right-left+1 );
        if( m_pLuaInlineScript_OnPlay.length() > 0 )
        {
            cJSON_AddStringToObject( jComponent, "LuaString_OnPlay", m_pLuaInlineScript_OnPlay.c_str() );
        }
    }
#endif

    // save the array of exposed variables
    if( m_ExposedVars.Count() > 0 )
    {
        cJSON* exposedvararray = cJSON_CreateArray();
        cJSON_AddItemToObject( jComponent, "ExposedVars", exposedvararray );
        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            ExposedVariableDesc* pVar = m_ExposedVars[i];
        
            cJSON* jExposedVar = cJSON_CreateObject();
            cJSON_AddItemToArray( exposedvararray, jExposedVar );

            cJSON_AddStringToObject( jExposedVar, "Name", pVar->name.c_str() );
            cJSON_AddNumberToObject( jExposedVar, "Type", pVar->type );

            if( pVar->type == ExposedVariableType_Float )
            {
                cJSON_AddNumberToObject( jExposedVar, "Value", pVar->valuedouble );
            }
            if( pVar->type == ExposedVariableType_Bool )
            {
                cJSON_AddNumberToObject( jExposedVar, "Value", pVar->valuebool );
            }
            if( pVar->type == ExposedVariableType_Vector3 )
            {
                cJSONExt_AddFloatArrayToObject( jExposedVar, "Value", pVar->valuevector3, 3 );
            }
            else if( pVar->type == ExposedVariableType_GameObject && pVar->pointer )
            {
                cJSON* gameobjectref = ((GameObject*)pVar->pointer)->ExportReferenceAsJSONObject( m_SceneIDLoadedFrom );
                cJSON_AddItemToObject( jExposedVar, "Value", gameobjectref );

                // TODO: find a way to uniquely identify a game object...
                //cJSON_AddStringToObject( jExposedVar, "Value", ((GameObject*)pVar->pointer)->GetName() );
            }

            if( pVar->divorced )
                cJSON_AddNumberToObject( jExposedVar, "Divorced", pVar->divorced );
        }
    }

    return jComponent;
}

void ComponentLuaScript::ImportFromJSONObject(cJSON* jsonobj, SceneID sceneid)
{
    //ComponentUpdateable::ImportFromJSONObject( jsonobj, sceneid );

    cJSON* scriptstringobj = cJSON_GetObjectItem( jsonobj, "Script" );
    if( scriptstringobj )
    {
        MyFileObject* pFile = g_pEngineFileManager->RequestFile( scriptstringobj->valuestring, GetSceneID() );
        MyAssert( pFile );
        if( pFile )
        {
            SetScriptFile( pFile );
            pFile->Release(); // free ref added by RequestFile
        }
    }

    cJSON* string_onplay = cJSON_GetObjectItem( jsonobj, "LuaString_OnPlay" );
    if( string_onplay )
    {
#if MYFW_EDITOR
        m_pLuaInlineScript_OnPlay = string_onplay->valuestring;
#else
        // In stand-alone build, allocate memory and make a copy of the string.
        int len = (int)strlen( string_onplay->valuestring );

        m_pLuaInlineScript_OnPlay = MyNew char[len+1];
        strcpy_s( m_pLuaInlineScript_OnPlay, len+1, string_onplay->valuestring );
#endif
    }

    // Load the array of exposed variables
    cJSON* exposedvararray = cJSON_GetObjectItem( jsonobj, "ExposedVars" );
    if( exposedvararray )
    {
        int numvars = cJSON_GetArraySize( exposedvararray );
        for( int i=0; i<numvars; i++ )
        {
            cJSON* jsonvar = cJSON_GetArrayItem( exposedvararray, i );

            cJSON* obj = cJSON_GetObjectItem( jsonvar, "Name" );
            MyAssert( obj );
            if( obj == 0 )
                continue;

            // by name, check if the variable is already in our list
            ExposedVariableDesc* pVar = 0;
            for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
            {
                MyAssert( m_ExposedVars[i] );
                if( m_ExposedVars[i]->name == obj->valuestring )
                {
                    pVar = m_ExposedVars[i];
                    break;
                }
            }

            // if not, create and add it.
            if( pVar == 0 )
            {
                pVar = MyNew ExposedVariableDesc();
                m_ExposedVars.Add( pVar );
            }

            pVar->Reset();

            pVar->name = obj->valuestring;
            cJSONExt_GetInt( jsonvar, "Type", (int*)&pVar->type );

            if( pVar->type == ExposedVariableType_Float )
            {
                cJSONExt_GetDouble( jsonvar, "Value", &pVar->valuedouble );
            }
            if( pVar->type == ExposedVariableType_Bool )
            {
                cJSONExt_GetBool( jsonvar, "Value", &pVar->valuebool );
            }
            if( pVar->type == ExposedVariableType_Vector3 )
            {
                cJSONExt_GetFloatArray( jsonvar, "Value", pVar->valuevector3, 3 );
            }
            else if( pVar->type == ExposedVariableType_GameObject )
            {
                cJSON* obj = cJSON_GetObjectItem( jsonvar, "Value" );
                if( obj )
                {
                    pVar->pointer = g_pComponentSystemManager->FindGameObjectByJSONRef( obj, m_pGameObject->GetSceneID(), false );

                    // TODO: Handle cases where the Scene containing the GameObject referenced isn't loaded.
                    //MyAssert( pVar->pointer != 0 );
                    if( pVar->pointer )
                    {
                        ((GameObject*)pVar->pointer)->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
                    }
                    else
                    {
                        LOGError( LOGTag, "LuaScript component on '%s' lost reference to GameObject.\n", m_pGameObject->GetName() );
                    }
                }
            }

            cJSONExt_GetBool( jsonvar, "Divorced", &pVar->divorced );
        }
    }
}

ComponentLuaScript& ComponentLuaScript::operator=(const ComponentLuaScript& other)
{
    MyAssert( &other != this );

    ComponentUpdateable::operator=( other );

    this->m_pScriptFile = other.m_pScriptFile;
    if( this->m_pScriptFile )
    {
        this->m_pScriptFile->AddRef();

        m_pCopyExternsFromThisComponentAfterLoadingScript = &other;
    }

    return *this;
}

void ComponentLuaScript::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, OnSurfaceChanged );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, Draw );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, OnTouch );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentLuaScript, OnKeys );
    }
}

void ComponentLuaScript::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );

        m_CallbacksRegistered = false;
    }
}

void ComponentLuaScript::SetScriptFile(MyFileObject* script)
{
    if( script )
        script->AddRef();

    SAFE_RELEASE( m_pScriptFile );
    m_pScriptFile = script;

    m_ScriptLoaded = false;
    m_ErrorInScript = false;

    if( m_Playing )
    {
        m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = true;
    }
}

void ComponentLuaScript::LoadScript()
{
    if( m_ScriptLoaded == true )
        return;

    // Script is ready, so run it.
    if( m_pScriptFile != 0 )
    {
        if( m_pScriptFile->GetFileLoadStatus() == FileLoadStatus_Success )
        {
            //LOGInfo( LOGTag, "luaL_loadstring: %s\n", m_pScriptFile->GetFilenameWithoutExtension() );

            // Mark script as loaded. "OnLoad" can be called if no errors occur otherwise m_ErrorInScript will be set as well.
            m_ScriptLoaded = true;

            const char* filename = m_pScriptFile->GetFullPath();
            char label[MAX_PATH+1];
            sprintf_s( label, MAX_PATH+1, "@%s", filename );

#if MYFW_ENABLE_LUA_DEBUGGER
            // Prevent the debugger from stopping while parsing script files.
            m_pLuaGameState->SetIsDebuggerAllowedToStop( false );
#endif

            // Load the string from the file.
            int filelen = m_pScriptFile->GetFileLength() - 1;
            int loadretcode = luaL_loadbuffer( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetBuffer(), filelen, label );

            if( loadretcode == LUA_OK )
            {
                // Run the code to do initial parsing.
                int exeretcode = lua_pcall( m_pLuaGameState->m_pLuaState, 0, LUA_MULTRET, 0 );
                if( exeretcode == LUA_OK )
                {
                    luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );

                    if( LuaObject.isTable() )
                    {
                        // Create a table to store local variables unique to this component.
                        char gameobjectname[100];
                        sprintf_s( gameobjectname, 100, "_GameObject_%d_%d", m_pGameObject->GetSceneID(), m_pGameObject->GetID() );
                        luabridge::LuaRef datatable = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, gameobjectname );

                        if( datatable.isTable() == false )
                        {
                            luabridge::LuaRef newdatatable = luabridge::newTable( m_pLuaGameState->m_pLuaState );
                            luabridge::setGlobal( m_pLuaGameState->m_pLuaState, newdatatable, gameobjectname );
                            newdatatable["gameobject"] = m_pGameObject;
                        }

                        ParseExterns( LuaObject );

                        // Call the OnLoad function in the Lua script.
                        CallFunctionEvenIfGameplayInactive( "OnLoad" );
                    }
                }
                else
                {
                    if( exeretcode == LUA_ERRRUN )
                    {
                        const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                        HandleLuaError( "LUA_ERRRUN", errorstr );
                    }
                    else
                    {
                        MyAssert( false ); // Assert until I hit this and deal with it better.
                        const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                        HandleLuaError( "!LUA_ERRRUN", errorstr );
                    }
                }
            }
            else
            {
                if( loadretcode == LUA_ERRSYNTAX )
                {
                    const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                    HandleLuaError( "LUA_ERRSYNTAX", errorstr );
                }
                else
                {
                    MyAssert( false );
                    const char* errorstr = lua_tostring( m_pLuaGameState->m_pLuaState, -1 );
                    HandleLuaError( "!LUA_ERRSYNTAX", errorstr );
                }
            }

#if MYFW_ENABLE_LUA_DEBUGGER
            // Reenable the debugger.
            m_pLuaGameState->SetIsDebuggerAllowedToStop( true );
#endif
        }
    }
}

void ComponentLuaScript::LoadInLineScripts()
{
    // Add the OnPlay string to the lua state and parse it.
#if MYFW_EDITOR
    if( m_pLuaInlineScript_OnPlay.length() > 0 )
    {
        const char* pScript = m_pLuaInlineScript_OnPlay.c_str();
#else
    if( m_pLuaInlineScript_OnPlay != 0 && m_pLuaInlineScript_OnPlay[0] != 0 )
    {
        const char* pScript = m_pLuaInlineScript_OnPlay;
#endif

        // Generate a unique name for an object/table the function can exist in.
        char inlinescriptname[100];
        sprintf_s( inlinescriptname, 100, "_InlineScript_GameObject_%d_%d", m_pGameObject->GetSceneID(), m_pGameObject->GetID() );

        // Create a small lua script to create the object/table which will contain an OnPlay function.
        char inlinescript[200];
        sprintf_s( inlinescript, 200, "%s = { OnPlay = function() %s end }", inlinescriptname, pScript );

        // Load the new script into the lua state.
        int loadretcode = luaL_loadstring( m_pLuaGameState->m_pLuaState, inlinescript );

        if( loadretcode == LUA_OK )
        {
            // Run the code to do initial parsing.
            int exeretcode = lua_pcall( m_pLuaGameState->m_pLuaState, 0, LUA_MULTRET, 0 );
            if( exeretcode == LUA_OK )
            {
                // Check if the _InlineScript_GameObject_X_X object exists, otherwise something went wrong.
                luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, inlinescriptname );

                if( LuaObject.isTable() == false )
                {
                    LOGError( LOGTag, "Inline lua script failed to compile: %s", inlinescriptname );
                }

                if( LuaObject["OnPlay"].isFunction() == false )
                {
                    LOGError( LOGTag, "Inline lua script failed to compile: %s", inlinescriptname );
                }
            }
        }
    }
}

void ComponentLuaScript::ParseExterns(luabridge::LuaRef LuaObject)
{
    // mark all variables unused.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        m_ExposedVars[i]->inuse = false;
    }

    luabridge::LuaRef Externs = LuaObject["Externs"];

    if( Externs.isTable() == true )
    {
        int len = Externs.length();
        for( int i=0; i<len; i++ )
        {
            luabridge::LuaRef variabledesc = Externs[i+1];

            //if( variabledesc.isTable() == false )
            //    return;

            luabridge::LuaRef variablename = variabledesc[1];
            luabridge::LuaRef variabletype = variabledesc[2];
            luabridge::LuaRef variableinitialvalue = variabledesc[3];

            std::string varname = variablename.tostring();
            std::string vartype = variabletype.tostring();

            ExposedVariableDesc* pVar = 0;

            // find any old variable with the same name
            for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
            {
                if( m_ExposedVars[i]->name == varname )
                {
                    pVar = m_ExposedVars[i];
                    pVar->inuse = true; // was in use.
                    break;
                }
            }

            // if not found, create a new one and add it to the list.
            if( pVar == 0 )
            {
                pVar = MyNew ExposedVariableDesc();
                m_ExposedVars.Add( pVar );
                pVar->inuse = false; // is new, will be marked inuse after being initialized
            }

            pVar->name = varname;

            if( vartype == "Float" )
            {
                // if it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_Float )
                    pVar->valuedouble = variableinitialvalue.cast<double>();

                pVar->type = ExposedVariableType_Float;
            }
            else if( vartype == "Vector3" )
            {
                // if it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_Vector3 )
                {
                    if( variableinitialvalue.isTable() == false ||
                        variableinitialvalue[1].isNil() || 
                        variableinitialvalue[2].isNil() || 
                        variableinitialvalue[3].isNil() )
                    {
                        LOGError( "LuaScript", "Initial value for vector3 isn't an array of 3 values\n" );
                    }
                    else
                    {
                        pVar->valuevector3[0] = variableinitialvalue[1].cast<float>();
                        pVar->valuevector3[1] = variableinitialvalue[2].cast<float>();
                        pVar->valuevector3[2] = variableinitialvalue[3].cast<float>();
                    }
                }

                pVar->type = ExposedVariableType_Vector3;
            }
            else if( vartype == "Bool" )
            {
                // if it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_Bool )
                    pVar->valuebool = variableinitialvalue.cast<bool>();

                pVar->type = ExposedVariableType_Bool;
            }
            else if( vartype == "GameObject" )
            {
                // if it's a new variable or it changed type, set it to it's initial value.
                if( pVar->inuse == false || pVar->type != ExposedVariableType_GameObject )
                {
                    pVar->pointer = g_pComponentSystemManager->FindGameObjectByName( variableinitialvalue.tostring().c_str() );
                    if( pVar->pointer )
                        ((GameObject*)pVar->pointer)->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
                }
                pVar->type = ExposedVariableType_GameObject;
            }
            else
            {
                MyAssert( false );
            }

            pVar->inuse = true;
        }
    }

    // delete unused variables.
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        if( m_ExposedVars[i]->inuse == false )
        {
            ExposedVariableDesc* pVariable = m_ExposedVars.RemoveIndex_MaintainOrder( i );

            // unregister gameobject deleted callback, if we registered one.
            if( pVariable->type == ExposedVariableType_GameObject && pVariable->pointer )
                ((GameObject*)pVariable->pointer)->UnregisterOnDeleteCallback( this, StaticOnGameObjectDeleted );

            delete pVariable;
            i--;
        }
    }
}

void ComponentLuaScript::ProgramVariables(luabridge::LuaRef LuaObject, bool updateexposedvariables)
{
    if( m_ScriptLoaded == false )
        return;

    // set "this" to the data table storing this gameobjects script data "_GameObject_<ID>"
    char gameobjectname[100];
    sprintf_s( gameobjectname, 100, "_GameObject_%d_%d", m_pGameObject->GetSceneID(), m_pGameObject->GetID() );
    luabridge::LuaRef datatable = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, gameobjectname );
    luabridge::setGlobal( m_pLuaGameState->m_pLuaState, datatable, "this" );

    // only program the exposed vars if they change.
    if( updateexposedvariables )
    {
        for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
        {
            ExposedVariableDesc* pVar = m_ExposedVars[i];

            if( pVar->type == ExposedVariableType_Float )
                datatable[pVar->name] = pVar->valuedouble;

            if( pVar->type == ExposedVariableType_Bool )
                datatable[pVar->name] = pVar->valuebool;

            if( pVar->type == ExposedVariableType_Vector3 )
                datatable[pVar->name] = Vector3( pVar->valuevector3[0], pVar->valuevector3[1], pVar->valuevector3[2] );

            if( pVar->type == ExposedVariableType_GameObject )
                datatable[pVar->name] = (GameObject*)pVar->pointer;
        }
    }
}

void ComponentLuaScript::HandleLuaError(const char* functionname, const char* errormessage)
{
    m_ErrorInScript = true;
    LuaBridgeExt_LogExceptionFormattedForVisualStudioOutputWindow( functionname, m_pScriptFile->GetFullPath(), errormessage );

    // pop the error message off the lua stack
    lua_pop( m_pLuaGameState->m_pLuaState, 1 );
}

void ComponentLuaScript::OnLoad()
{
    ComponentUpdateable::OnLoad();

    m_ScriptLoaded = false;
    m_ErrorInScript = false;
}

void ComponentLuaScript::OnPlay()
{
    ComponentUpdateable::OnPlay();

    if( m_ErrorInScript )
        return;

    if( m_pScriptFile && m_pScriptFile->GetFileLoadStatus() != FileLoadStatus_Success )
    {
        LOGInfo( LOGTag, "Script warning: OnPlay, script not loaded: %s\n", m_pScriptFile->GetFilenameWithoutExtension() );
    }

    m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = true;

    LoadInLineScripts();

    // Check for an inline OnPlay function and call it.
    {
        // Generate the name for the object/table that contains our inline scripts.
        char inlinescriptname[100];
        sprintf_s( inlinescriptname, 100, "_InlineScript_GameObject_%d_%d", m_pGameObject->GetSceneID(), m_pGameObject->GetID() );

        luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, inlinescriptname );
        
        if( LuaObject.isTable() )
        {
            if( LuaObject["OnPlay"].isFunction() )
            {
                try
                {
                    LuaObject["OnPlay"]();
                }
                catch(luabridge::LuaException const& e)
                {
                    HandleLuaError( "OnPlay - inline", e.what() );
                }
            }
        }
    }
}

void ComponentLuaScript::OnStop()
{
    ComponentUpdateable::OnStop();

    if( m_Playing && m_ErrorInScript == false )
    {
        CallFunction( "OnStop" );
    }

    m_ScriptLoaded = false;
    m_ErrorInScript = false;
    m_Playing = false;
}

void ComponentLuaScript::OnGameObjectEnabled()
{
    ComponentBase::OnGameObjectEnabled();
    if( g_pEngineCore->IsInEditorMode() == false )
        OnPlay();
}

void ComponentLuaScript::OnGameObjectDisabled()
{
    ComponentBase::OnGameObjectDisabled();
}

void ComponentLuaScript::TickCallback(float deltaTime)
{
    //ComponentBase::TickCallback( deltaTime );
    //ComponentUpdateable::Tick( deltaTime );

    if( m_ErrorInScript )
        return;

    if( m_ScriptLoaded == false && g_pLuaGameState )
    {
        LoadScript();
    }

    if( g_pLuaGameState == 0 || m_ScriptLoaded == false )
        return;

    // copy externed variable values after loading the script
    {
        if( m_pCopyExternsFromThisComponentAfterLoadingScript )
        {
            const ComponentLuaScript& other = *m_pCopyExternsFromThisComponentAfterLoadingScript;
            m_pCopyExternsFromThisComponentAfterLoadingScript = 0;

            for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
            {
                ExposedVariableDesc* pVar = m_ExposedVars[i];
                ExposedVariableDesc* pOtherVar = 0;// = other.m_ExposedVars[i];            

                // find the first variable in the other object with the same name
                for( unsigned int oi=0; oi<m_ExposedVars.Count(); oi++ )
                {
                    if( pVar->name == other.m_ExposedVars[oi]->name )
                    {
                        pOtherVar = other.m_ExposedVars[oi];
                        break;
                    }
                }

                if( pOtherVar != 0 )
                {
                    if( pVar->type == ExposedVariableType_Float )
                    {
                        pVar->valuedouble = pOtherVar->valuedouble;
                    }

                    if( pVar->type == ExposedVariableType_Bool )
                    {
                        pVar->valuebool = pOtherVar->valuebool;
                    }

                    if( pVar->type == ExposedVariableType_Vector3 )
                    {
                        for( int i=0; i<3; i++ )
                            pVar->valuevector3[0] = pOtherVar->valuevector3[0];
                    }

                    if( pVar->type == ExposedVariableType_GameObject )
                    {
                        pVar->pointer = pOtherVar->pointer;

                        if( pVar->pointer )
                            ((GameObject*)pVar->pointer)->RegisterOnDeleteCallback( this, StaticOnGameObjectDeleted );
                    }
                }
            }

            ProgramVariables( m_pLuaGameState->m_pLuaState, true );
        }
    }

    if( m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading )
    {
        m_CallLuaOnPlayNextTickOrAfterScriptIsFinishedLoading = false;

        // find the OnPlay function and call it, look for a table that matched our filename.
        if( m_pScriptFile )
        {
            luabridge::LuaRef LuaObject = luabridge::getGlobal( m_pLuaGameState->m_pLuaState, m_pScriptFile->GetFilenameWithoutExtension() );
        
            if( LuaObject.isNil() == false )
            {
                if( LuaObject["OnPlay"].isFunction() )
                {
                    // program the exposed variable values in the table, don't just set the table to be active.
                    ProgramVariables( LuaObject, true );
                    try
                    {
                        LuaObject["OnPlay"]();
                    }
                    catch(luabridge::LuaException const& e)
                    {
                        HandleLuaError( "OnPlay", e.what() );
                    }
                }

                m_Playing = true;
            }
        }
    }

    // find the Tick function and call it.
    if( m_Playing )
    {
        CallFunction( "Tick", deltaTime );
    }
}

bool ComponentLuaScript::OnTouchCallback(int action, int id, float x, float y, float pressure, float size)
{
    //ComponentBase::OnTouchCallback( action, id, x, y, pressure, size );

    if( m_ErrorInScript )
        return false;

    // find the OnTouch function and call it.
    if( m_Playing )
    {
        if( CallFunction( "OnTouch", action, id, x, y, pressure, size ) )
            return true;
    }

    return false;
}

bool ComponentLuaScript::OnButtonsCallback(GameCoreButtonActions action, GameCoreButtonIDs id)
{
    //ComponentBase::OnButtonsCallback( action, id );

    if( m_ErrorInScript )
        return false;

    // find the OnButtons function and call it.
    if( m_Playing )
    {
        int a = action;
        int i = id;
        if( CallFunction( "OnButtons", a, i ) )
            return true;
    }

    return false;
}

void ComponentLuaScript::OnGameObjectDeleted(GameObject* pGameObject)
{
    for( unsigned int i=0; i<m_ExposedVars.Count(); i++ )
    {
        ExposedVariableDesc* pVar = m_ExposedVars[i];

        // if any of our variables is a gameobject, clear it if that gameobject was deleted.
        if( pVar->type == ExposedVariableType_GameObject )
        {
            if( pVar->pointer == pGameObject )
            {
#if MYFW_USING_WX
                if( g_pGameCore->GetCommandStack() )
                {
                    g_pGameCore->GetCommandStack()->Do( MyNew EditorCommand_LuaExposedVariablePointerChanged(
                        0, pVar, ComponentLuaScript::StaticOnExposedVarValueChanged, this ), true );
                }
#endif
                pVar->pointer = 0;
            }
        }
    }
}

#endif //MYFW_USING_LUA
