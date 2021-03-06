//
// Copyright (c) 2015-2018 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "EngineEditorCommands.h"
#include "ComponentSystem/BaseComponents/ComponentRenderable.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentAudioPlayer.h"
#include "ComponentSystem/FrameworkComponents/Physics2D/Component2DCollisionObject.h"
#include "ComponentSystem/FrameworkComponents/ComponentMeshPrimitive.h"
#include "Core/EngineCore.h"
#include "../SourceEditor/EditorState.h"

#if MYFW_USING_IMGUI
//====================================================================================================
// EditorCommand_ImGuiPanelWatchNumberValueChanged
//====================================================================================================

EditorCommand_ImGuiPanelWatchNumberValueChanged::EditorCommand_ImGuiPanelWatchNumberValueChanged(void* pObject, ComponentVariable* pVar, ComponentVariableValue newValue, ComponentVariableValue oldValue, bool directlyChanged, ComponentBase* pComponent)
{
    m_Name = "EditorCommand_ImGuiPanelWatchNumberValueChanged";

    m_pObject = pObject;
    m_pComponent = pComponent;
    m_pVar = pVar;

    m_NewValue = newValue;
    m_OldValue = oldValue;
    m_DirectlyChanged = directlyChanged;
}

EditorCommand_ImGuiPanelWatchNumberValueChanged::~EditorCommand_ImGuiPanelWatchNumberValueChanged()
{
}

void EditorCommand_ImGuiPanelWatchNumberValueChanged::Do()
{
    int controlcomponent = 0;
    double previousvalue = 0;
    ColorByte previousColorByte;
    std::string previousString;

    switch( m_pVar->m_Type )
    {
    case ComponentVariableType::Int:
        previousvalue = (double)m_OldValue.GetInt();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::Enum:
        previousvalue = (double)m_OldValue.GetEnum();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::Flags:
        previousvalue = (double)m_OldValue.GetFlags();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::UnsignedInt:
        previousvalue = (double)m_OldValue.GetUnsignedInt();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    //case ComponentVariableType::Char:
    //case ComponentVariableType::UnsignedChar:
    //    MyAssert( false );
    //    break;

    case ComponentVariableType::Bool:
        previousvalue = (double)m_OldValue.GetBool();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::Float:
        previousvalue = (double)m_OldValue.GetFloat();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    //case ComponentVariableType::Double:
    //case ComponentVariableType::ColorFloat:
    //    MyAssert( false );
    //    break;

    case ComponentVariableType::ColorByte:
        previousColorByte = m_OldValue.GetColorByte();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::Vector2:
        //Vector2 previousvalue = m_OldValue.GetVector2();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );

        // Determine which component of the control changed, assert if more than 1 changed.
        controlcomponent = -1;
        if( m_OldValue.GetVector2().x != m_NewValue.GetVector2().x )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 0;
        }
        if( m_OldValue.GetVector2().y != m_NewValue.GetVector2().y )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        break;

    case ComponentVariableType::Vector3:
        //Vector3 previousvalue = m_OldValue.GetVector3();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        controlcomponent = -1;
        if( m_OldValue.GetVector3().x != m_NewValue.GetVector3().x )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 0;
        }
        if( m_OldValue.GetVector3().y != m_NewValue.GetVector3().y )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        if( m_OldValue.GetVector3().z != m_NewValue.GetVector3().z )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 2;
        }
        break;

    case ComponentVariableType::Vector2Int:
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );

        // Determine which component of the control changed, assert if more than 1 changed.
        controlcomponent = -1;
        if( m_OldValue.GetVector2Int().x != m_NewValue.GetVector2Int().x )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 0;
        }
        if( m_OldValue.GetVector2Int().y != m_NewValue.GetVector2Int().y )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        break;

    case ComponentVariableType::Vector3Int:
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );

        // Determine which component of the control changed, assert if more than 1 changed.
        controlcomponent = -1;
        if( m_OldValue.GetVector3Int().x != m_NewValue.GetVector3Int().x )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 0;
        }
        if( m_OldValue.GetVector3Int().y != m_NewValue.GetVector3Int().y )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        if( m_OldValue.GetVector3Int().z != m_NewValue.GetVector3Int().z )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 2;
        }
        break;

    case ComponentVariableType::CharArray:
        previousString = m_OldValue.GetCharArray();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::String:
        previousString = m_OldValue.GetString();
        m_NewValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::GameObjectPtr:
    case ComponentVariableType::ComponentPtr:
    case ComponentVariableType::FilePtr:
    case ComponentVariableType::MaterialPtr:
    case ComponentVariableType::TexturePtr:
    case ComponentVariableType::SoundCuePtr:
    case ComponentVariableType::PointerIndirect:
    case ComponentVariableType::NumTypes:
    default:
        MyAssert( false );
    }

    if( m_pComponent )
    {
        m_pComponent->OnValueChangedVariable( m_pVar, controlcomponent, m_DirectlyChanged, true, previousvalue, false, &m_NewValue );
    }
    m_DirectlyChanged = false; // always pass false if this isn't the first time 'Do' is called
}

void EditorCommand_ImGuiPanelWatchNumberValueChanged::Undo()
{
    int controlcomponent = 0;
    double previousvalue = 0;
    ColorByte previousColorByte;
    std::string previousString;

    switch( m_pVar->m_Type )
    {
    case ComponentVariableType::Int:
        previousvalue = (double)m_NewValue.GetInt();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::Enum:
        previousvalue = (double)m_NewValue.GetEnum();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::Flags:
        previousvalue = (double)m_NewValue.GetFlags();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::UnsignedInt:
        previousvalue = (double)m_NewValue.GetUnsignedInt();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    //case ComponentVariableType::Char:
    //    oldvalue = *(char*)m_Pointer;
    //    *(char*)m_Pointer -= (char)m_Difference;
    //    break;

    //case ComponentVariableType::UnsignedChar:
    //    oldvalue = *(unsigned char*)m_Pointer;
    //    *(unsigned char*)m_Pointer -= (unsigned char)m_Difference;
    //    break;

    case ComponentVariableType::Bool:
        previousvalue = (double)m_NewValue.GetBool();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::Float:
        previousvalue = (double)m_NewValue.GetFloat();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    //case ComponentVariableType::Double:
    //case ComponentVariableType::ColorFloat:
    //    break;

    case ComponentVariableType::ColorByte:
        previousColorByte = m_NewValue.GetColorByte();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::Vector2:
        //Vector2 previousvalue = m_OldValue.GetVector2();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );

        // Determine which component of the control changed, assert if more than 1 changed.
        controlcomponent = -1;
        if( m_OldValue.GetVector2().x != m_NewValue.GetVector2().x )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 0;
        }
        if( m_OldValue.GetVector2().y != m_NewValue.GetVector2().y )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        break;

    case ComponentVariableType::Vector3:
        //Vector3 previousvalue = m_OldValue.GetVector3();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );

        // Determine which component of the control changed, assert if more than 1 changed.
        controlcomponent = -1;
        if( m_OldValue.GetVector3().x != m_NewValue.GetVector3().x )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 0;
        }
        if( m_OldValue.GetVector3().y != m_NewValue.GetVector3().y )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        if( m_OldValue.GetVector3().z != m_NewValue.GetVector3().z )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 2;
        }
        break;

    case ComponentVariableType::Vector2Int:
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );

        // Determine which component of the control changed, assert if more than 1 changed.
        controlcomponent = -1;
        if( m_OldValue.GetVector2Int().x != m_NewValue.GetVector2Int().x )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 0;
        }
        if( m_OldValue.GetVector2Int().y != m_NewValue.GetVector2Int().y )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        break;

    case ComponentVariableType::Vector3Int:
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );

        // Determine which component of the control changed, assert if more than 1 changed.
        controlcomponent = -1;
        if( m_OldValue.GetVector3Int().x != m_NewValue.GetVector3Int().x )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 0;
        }
        if( m_OldValue.GetVector3Int().y != m_NewValue.GetVector3Int().y )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        if( m_OldValue.GetVector3Int().z != m_NewValue.GetVector3Int().z )
        {
            MyAssert( controlcomponent == -1 );
            controlcomponent = 1;
        }
        break;

    case ComponentVariableType::CharArray:
        previousString = m_NewValue.GetCharArray();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::String:
        previousString = m_NewValue.GetString();
        m_OldValue.CopyValueIntoVariable( m_pObject, m_pVar, m_pComponent );
        break;

    case ComponentVariableType::GameObjectPtr:
    case ComponentVariableType::ComponentPtr:
    case ComponentVariableType::FilePtr:
    case ComponentVariableType::MaterialPtr:
    case ComponentVariableType::TexturePtr:
    case ComponentVariableType::SoundCuePtr:
    case ComponentVariableType::PointerIndirect:
    case ComponentVariableType::NumTypes:
    default:
        MyAssert( false );
    }

    if( m_pComponent )
    {
        m_pComponent->OnValueChangedVariable( m_pVar, controlcomponent, m_DirectlyChanged, true, previousvalue, false, &m_OldValue );
    }
    m_DirectlyChanged = false; // always pass false if this isn't the first time 'Do' is called
}

EditorCommand* EditorCommand_ImGuiPanelWatchNumberValueChanged::Repeat()
{
    EditorCommand_ImGuiPanelWatchNumberValueChanged* pCommand;
    pCommand = MyNew EditorCommand_ImGuiPanelWatchNumberValueChanged( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_ImGuiPanelWatchColorChanged
//====================================================================================================

EditorCommand_ImGuiPanelWatchColorChanged::EditorCommand_ImGuiPanelWatchColorChanged(ColorFloat newColor, PanelWatch_Types type, void* pPointer, int controlID, bool directlyChanged, PanelWatchCallbackValueChanged* callbackFunc, void* callbackObj)
{
    m_Name = "EditorCommand_ImGuiPanelWatchColorChanged";

    MyAssert( type == PanelWatchType_ColorFloat || type == PanelWatchType_ColorByte );

    m_NewColor = newColor;
    m_Type = type;
    m_Pointer = pPointer;
    m_ControlID = controlID;
    m_DirectlyChanged = directlyChanged;

    if( m_Type == PanelWatchType_ColorFloat )
        m_OldColor = *(ColorFloat*)pPointer;
    else
        m_OldColor = ((ColorByte*)pPointer)->AsColorFloat();

    m_pOnValueChangedCallBackFunc = callbackFunc;
    m_pCallbackObj = callbackObj;
}

EditorCommand_ImGuiPanelWatchColorChanged::~EditorCommand_ImGuiPanelWatchColorChanged()
{
}

void EditorCommand_ImGuiPanelWatchColorChanged::Do()
{
    MyAssert( m_Type == PanelWatchType_ColorFloat || m_Type == PanelWatchType_ColorByte );

    double oldvalue = 0;

    if( m_Type == PanelWatchType_ColorFloat )
    {
        *(ColorFloat*)m_Pointer = m_NewColor;

        // TODO: same as below for colorfloats
    }
    else
    {
        // store the old color in a local var.
        // send the pointer to that var via callback in the double.
        ColorByte oldcolor = *(ColorByte*)m_Pointer;
        *(uintptr_t*)&oldvalue = (uintptr_t)&oldcolor;

        // Update the ColorByte stored at the pointer.
        *(ColorByte*)m_Pointer = m_NewColor.AsColorByte();
    }

    //g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
    {
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_ControlID, m_DirectlyChanged, true, oldvalue, false );
        m_DirectlyChanged = false; // always pass false if this isn't the first time 'Do' is called
    }
}

void EditorCommand_ImGuiPanelWatchColorChanged::Undo()
{
    MyAssert( m_Type == PanelWatchType_ColorFloat || m_Type == PanelWatchType_ColorByte );

    if( m_Type == PanelWatchType_ColorFloat )
        *(ColorFloat*)m_Pointer = m_OldColor;
    else
        *(ColorByte*)m_Pointer = m_OldColor.AsColorByte();

    //g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
    {
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_ControlID, false, true, 0, false );
    }
}

EditorCommand* EditorCommand_ImGuiPanelWatchColorChanged::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ImGuiPanelWatchPointerChanged
//====================================================================================================

EditorCommand_ImGuiPanelWatchPointerChanged::EditorCommand_ImGuiPanelWatchPointerChanged(void* newValue, PanelWatch_Types type, void** pPointer, int controlID, bool directlyChanged, PanelWatchCallbackValueChanged callbackFunc, void* callbackObj)
{
    m_Name = "EditorCommand_ImGuiPanelWatchPointerChanged";

    MyAssert( type == PanelWatchType_PointerWithDesc );

    m_NewValue = newValue;
    m_Type = type;
    m_pPointer = pPointer;
    m_ControlID = controlID;
    m_DirectlyChanged = directlyChanged;

    m_OldValue = *pPointer;

    m_pOnValueChangedCallBackFunc = callbackFunc;
    m_pCallbackObj = callbackObj;
}

EditorCommand_ImGuiPanelWatchPointerChanged::~EditorCommand_ImGuiPanelWatchPointerChanged()
{
}

void EditorCommand_ImGuiPanelWatchPointerChanged::Do()
{
    MyAssert( m_Type == PanelWatchType_PointerWithDesc );

    *m_pPointer = m_NewValue;

    //g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
    {
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_ControlID, m_DirectlyChanged, true, 0, false );
        m_DirectlyChanged = false; // always pass false if this isn't the first time 'Do' is called
    }
}

void EditorCommand_ImGuiPanelWatchPointerChanged::Undo()
{
    MyAssert( m_Type == PanelWatchType_PointerWithDesc );

    *m_pPointer = m_OldValue;

    //g_pPanelWatch->UpdatePanel();

    // this could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
    {
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_ControlID, false, true, 0, false );
    }
}

EditorCommand* EditorCommand_ImGuiPanelWatchPointerChanged::Repeat()
{
    return 0;
}
#endif //MYFW_USING_IMGUI

//====================================================================================================
// EditorCommand_MoveObjects
//====================================================================================================

EditorCommand_MoveObjects::EditorCommand_MoveObjects(Vector3 distancemoved, const std::vector<GameObject*>& selectedobjects)
{
    m_Name = "EditorCommand_MoveObjects";

    m_DistanceMoved = distancemoved;

    //LOGInfo( LOGTag, "EditorCommand_MoveObjects:: %f,%f,%f\n", m_DistanceMoved.x, m_DistanceMoved.y, m_DistanceMoved.z );

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_ObjectsMoved.push_back( selectedobjects[i] );
    }
}

EditorCommand_MoveObjects::~EditorCommand_MoveObjects()
{
}

void EditorCommand_MoveObjects::Do()
{
    for( unsigned int i=0; i<m_ObjectsMoved.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsMoved[i]->GetTransform();

        Vector3 newpos = pTransform->GetLocalTransform()->GetTranslation() + m_DistanceMoved;
        pTransform->SetPositionByEditor( newpos );
        pTransform->UpdateTransform();
    }
}

void EditorCommand_MoveObjects::Undo()
{
    //LOGInfo( LOGTag, "EditorCommand_MoveObjects::Undo %f,%f,%f\n", m_DistanceMoved.x, m_DistanceMoved.y, m_DistanceMoved.z );
    for( unsigned int i=0; i<m_ObjectsMoved.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsMoved[i]->GetTransform();

        Vector3 newpos = pTransform->GetLocalTransform()->GetTranslation() - m_DistanceMoved;
        pTransform->SetPositionByEditor( newpos );
        pTransform->UpdateTransform();
    }
}

EditorCommand* EditorCommand_MoveObjects::Repeat()
{
    EditorCommand_MoveObjects* pCommand;
    pCommand = MyNew EditorCommand_MoveObjects( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_ScaleObjects
//====================================================================================================

EditorCommand_ScaleObjects::EditorCommand_ScaleObjects(Vector3 amountscaled, bool localspace, Vector3 pivot, const std::vector<GameObject*>& selectedobjects)
{
    m_Name = "EditorCommand_ScaleObjects";

    m_AmountScaled = amountscaled;
    m_TransformedInLocalSpace = localspace;
    m_WorldSpacePivot = pivot;

    //LOGInfo( LOGTag, "EditorCommand_ScaleObjects:: %f,%f,%f\n", m_AmountScaled.x, m_AmountScaled.y, m_AmountScaled.z );

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_ObjectsScaled.push_back( selectedobjects[i] );
    }
}

EditorCommand_ScaleObjects::~EditorCommand_ScaleObjects()
{
}

void EditorCommand_ScaleObjects::Do()
{
    for( unsigned int i=0; i<m_ObjectsScaled.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsScaled[i]->GetTransform();

        // only scale in local space
        //if( m_TransformedInLocalSpace == true )
        {
            Vector3 newscale = pTransform->GetLocalTransform()->GetScale() * m_AmountScaled;

            pTransform->SetScaleByEditor( newscale );
            pTransform->UpdateTransform();
        }
        //else
        //{
        //    MyMatrix matscale;
        //    matscale.CreateScale( m_AmountScaled );
        //    pTransform->Scale( &matscale, m_WorldSpacePivot );
        //}
    }
}

void EditorCommand_ScaleObjects::Undo()
{
    //LOGInfo( LOGTag, "EditorCommand_ScaleObjects::Undo %f,%f,%f\n", m_AmountScaled.x, m_AmountScaled.y, m_AmountScaled.z );
    for( unsigned int i=0; i<m_ObjectsScaled.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsScaled[i]->GetTransform();

        // only scale in local space
        //if( m_TransformedInLocalSpace == true )
        {
            Vector3 newscale = pTransform->GetLocalTransform()->GetScale() / m_AmountScaled;

            pTransform->SetScaleByEditor( newscale );
            pTransform->UpdateTransform();
        }
        //else
        //{
        //    MyMatrix matscale;
        //    matscale.CreateScale( 1/m_AmountScaled );
        //    pTransform->Scale( &matscale, m_WorldSpacePivot );
        //}
    }
}

EditorCommand* EditorCommand_ScaleObjects::Repeat()
{
    EditorCommand_ScaleObjects* pCommand;
    pCommand = MyNew EditorCommand_ScaleObjects( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_RotateObjects
//====================================================================================================

EditorCommand_RotateObjects::EditorCommand_RotateObjects(Vector3 amountRotated, bool localspace, Vector3 pivot, const std::vector<GameObject*>& selectedobjects)
{
    m_Name = "EditorCommand_RotateObjects";

    m_AmountRotated = amountRotated;
    m_TransformedInLocalSpace = localspace;
    m_WorldSpacePivot = pivot;

    //LOGInfo( LOGTag, "EditorCommand_RotateObjects:: %f,%f,%f\n", m_AmountRotated.x, m_AmountRotated.y, m_AmountRotated.z );

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_ObjectsRotated.push_back( selectedobjects[i] );
    }
}

EditorCommand_RotateObjects::~EditorCommand_RotateObjects()
{
}

void EditorCommand_RotateObjects::Do()
{
    for( unsigned int i=0; i<m_ObjectsRotated.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsRotated[i]->GetTransform();

        if( m_TransformedInLocalSpace == true )
        {
            MyMatrix objectRotation;
            MyMatrix newRotation;
            MyMatrix combinedRotation;

            objectRotation.CreateRotation( pTransform->GetWorldRotation() );
            newRotation.CreateRotation( m_AmountRotated );
            combinedRotation = objectRotation * newRotation;
            
            Vector3 eulerdegrees = combinedRotation.GetEulerAngles() * 180.0f / PI;
            pTransform->SetWorldRotation( eulerdegrees );
            pTransform->UpdateTransform();
        }
        else
        {
            MyMatrix newRotation;

            newRotation.CreateRotation( m_AmountRotated );
            pTransform->Rotate( &newRotation, m_WorldSpacePivot );
        }

        //pTransform->SetWorldTransform( &combinedRotation );
    }
}

void EditorCommand_RotateObjects::Undo()
{
    //LOGInfo( LOGTag, "EditorCommand_RotateObjects::Undo %f,%f,%f\n", m_AmountRotated.x, m_AmountRotated.y, m_AmountRotated.z );
    for( unsigned int i=0; i<m_ObjectsRotated.size(); i++ )
    {
        ComponentTransform* pTransform = m_ObjectsRotated[i]->GetTransform();

        if( m_TransformedInLocalSpace == true )
        {
            MyMatrix objectRotation;
            MyMatrix newRotation;
            MyMatrix combinedRotation;

            objectRotation.CreateRotation( pTransform->GetWorldRotation() );
            newRotation.CreateRotation( m_AmountRotated * -1 );
            combinedRotation = objectRotation * newRotation;

            Vector3 eulerdegrees = combinedRotation.GetEulerAngles() * 180.0f / PI;
            pTransform->SetWorldRotation( eulerdegrees );
            pTransform->UpdateTransform();
        }
        else
        {
            MyMatrix newRotation;

            newRotation.CreateRotation( m_AmountRotated * -1 );
            pTransform->Rotate( &newRotation, m_WorldSpacePivot );
        }
    }
}

EditorCommand* EditorCommand_RotateObjects::Repeat()
{
    EditorCommand_RotateObjects* pCommand;
    pCommand = MyNew EditorCommand_RotateObjects( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_DeleteObjects
//====================================================================================================

bool IsAnyParentAlreadyInList(const std::vector<GameObject*>& selectedobjects, GameObject* pObject)
{
    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        if( pObject->IsParentedTo( selectedobjects[i], false ) )
        {
            return true;
        }
    }

    return false;
}

EditorCommand_DeleteObjects::EditorCommand_DeleteObjects(const std::vector<GameObject*>& selectedobjects)
{
    m_Name = "EditorCommand_DeleteObjects";

    MyAssert( selectedobjects.size() > 0 );

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        GameObject* pObject = selectedobjects[i];

        // Skip over objects if any parent is also in list.
        if( IsAnyParentAlreadyInList( selectedobjects, pObject ) )
            continue;

        // Don't allow same object to be in the list twice.
        if( std::find( m_ObjectsDeleted.begin(), m_ObjectsDeleted.end(), pObject ) == m_ObjectsDeleted.end() )
        {
            m_PreviousGameObjectsInObjectList.push_back( pObject->GetPrev() );
            m_ObjectsDeleted.push_back( pObject );
        }
    }

    m_DeleteGameObjectsWhenDestroyed = false;
}

EditorCommand_DeleteObjects::~EditorCommand_DeleteObjects()
{
    if( m_DeleteGameObjectsWhenDestroyed )
    {
        for( unsigned int i=0; i<m_ObjectsDeleted.size(); i++ )
        {
            g_pComponentSystemManager->DeleteGameObject( m_ObjectsDeleted[i], true );
        }
    }
}

void EditorCommand_DeleteObjects::Do()
{
    g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_ObjectsDeleted.size(); i++ )
    {
        GameObject* pDeletedObject = m_ObjectsDeleted[i];
        GameObject* pDeletedObjectsParent = m_ObjectsDeleted[i]->GetParentGameObject();

        //pDeletedObject->UnregisterAllComponentCallbacks( true );
        pDeletedObject->SetEnabled( false, true );
        g_pComponentSystemManager->UnmanageGameObject( pDeletedObject, true );
        pDeletedObject->GetSceneInfo()->m_GameObjects.MoveTail( pDeletedObject );
        pDeletedObject->NotifyOthersThisWasDeleted();

        // If this object has a parent, add this prefab child id to the "deleted" list.
        if( pDeletedObjectsParent != 0 )
        {
            uint32 childID = pDeletedObject->GetPrefabRef()->GetChildID();
            if( childID != 0 )
            {
                pDeletedObjectsParent->AddPrefabChildIDToListOfDeletedPrefabChildIDs( childID );
            }
        }
    }
    m_DeleteGameObjectsWhenDestroyed = true;
}

void EditorCommand_DeleteObjects::Undo()
{
    g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();

    // Undo delete in opposite order, so editor can place things in correct order in tree.
    for( unsigned int i=0; i<m_ObjectsDeleted.size(); i++ )
    {
        GameObject* pDeletedObject = m_ObjectsDeleted[i];
        GameObject* pDeletedObjectsParent = pDeletedObject->GetParentGameObject();

        // Place gameobject in old spot in scene's GameObject list.
        if( m_PreviousGameObjectsInObjectList[i] == 0 )
        {
            if( pDeletedObjectsParent )
            {
                pDeletedObjectsParent->GetChildList()->MoveHead( pDeletedObject );
            }
            else
            {
                pDeletedObject->GetSceneInfo()->m_GameObjects.MoveHead( pDeletedObject );
            }
        }
        else
        {
            pDeletedObject->MoveAfter( m_PreviousGameObjectsInObjectList[i] );
        }

        // If this object had a parent, remove this prefab child id from the "deleted" list.
        if( pDeletedObjectsParent )
        {
            uint32 childID = pDeletedObject->GetPrefabRef()->GetChildID();
            if( childID != 0 )
            {
                pDeletedObjectsParent->RemovePrefabChildIDFromListOfDeletedPrefabChildIDs( childID );
            }
        }

        // Undo everything we did to "delete" this object
        g_pComponentSystemManager->ManageGameObject( m_ObjectsDeleted[i], true );
        m_ObjectsDeleted[i]->SetEnabled( true, true );
        //m_ObjectsDeleted[i]->RegisterAllComponentCallbacks( false );

        // Place gameobject in old spot in tree.
        if( m_ObjectsDeleted[i]->Prev && m_ObjectsDeleted[i]->GetPrev() != 0 )
        {
#if MYFW_USING_WX
            g_pPanelObjectList->Tree_MoveObject( m_ObjectsDeleted[i], m_ObjectsDeleted[i]->GetPrev(), false );
#endif //MYFW_USING_WX
        }
        else
        {
#if MYFW_USING_WX
            if( m_ObjectsDeleted[i]->GetParentGameObject() )
            {
                g_pPanelObjectList->Tree_MoveObject( m_ObjectsDeleted[i], m_ObjectsDeleted[i]->GetParentGameObject(), true );
            }
            else
            {
                wxTreeItemId treeidtomove = g_pPanelObjectList->FindObject( m_ObjectsDeleted[i] );
                wxTreeItemId rootid = g_pComponentSystemManager->GetTreeIDForScene( m_ObjectsDeleted[i]->GetSceneID() );
                g_pPanelObjectList->Tree_MoveObject( treeidtomove, rootid, true );
            }
#endif //MYFW_USING_WX
        }
    }

    m_DeleteGameObjectsWhenDestroyed = false;
}

EditorCommand* EditorCommand_DeleteObjects::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_DeleteComponents
//====================================================================================================

EditorCommand_DeleteComponents::EditorCommand_DeleteComponents(const std::vector<ComponentBase*>& selectedComponents)
{
    m_Name = "EditorCommand_DeleteComponents";

    for( unsigned int i=0; i<selectedComponents.size(); i++ )
    {
        m_ComponentsDeleted.push_back( selectedComponents[i] );
        m_ComponentWasDisabled.push_back( selectedComponents[i]->IsEnabled() );
    }

    m_DeleteComponentsWhenDestroyed = false;
}

EditorCommand_DeleteComponents::~EditorCommand_DeleteComponents()
{
    if( m_DeleteComponentsWhenDestroyed )
    {
        for( unsigned int i=0; i<m_ComponentsDeleted.size(); i++ )
        {
            g_pComponentSystemManager->DeleteComponent( m_ComponentsDeleted[i] );
        }
    }
}

void EditorCommand_DeleteComponents::Do()
{
    for( unsigned int i=0; i<m_ComponentsDeleted.size(); i++ )
    {
        m_ComponentsDeleted[i]->GetGameObject()->RemoveComponent( m_ComponentsDeleted[i] );
        m_ComponentsDeleted[i]->SetEnabled( false );
    }
    m_DeleteComponentsWhenDestroyed = true;
}

void EditorCommand_DeleteComponents::Undo()
{
    for( unsigned int i=0; i<m_ComponentsDeleted.size(); i++ )
    {
        m_ComponentsDeleted[i]->GetGameObject()->AddExistingComponent( m_ComponentsDeleted[i], false );
        m_ComponentsDeleted[i]->SetEnabled( m_ComponentWasDisabled[i] );
    }
    m_DeleteComponentsWhenDestroyed = false;
}

EditorCommand* EditorCommand_DeleteComponents::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_CreateGameObject
//====================================================================================================

EditorCommand_CreateGameObject::EditorCommand_CreateGameObject(GameObject* objectcreated)
{
    m_Name = "EditorCommand_CreateGameObject";

    MyAssert( m_ObjectCreated );
    m_ObjectCreated = objectcreated;
    m_DeleteGameObjectsWhenDestroyed = false;
}

EditorCommand_CreateGameObject::~EditorCommand_CreateGameObject()
{
    if( m_DeleteGameObjectsWhenDestroyed )
    {
        g_pComponentSystemManager->DeleteGameObject( m_ObjectCreated, true );
    }
}

void EditorCommand_CreateGameObject::Do()
{
    g_pComponentSystemManager->ManageGameObject( m_ObjectCreated, true );
    m_ObjectCreated->SetEnabled( true, true );
    m_DeleteGameObjectsWhenDestroyed = false;
}

void EditorCommand_CreateGameObject::Undo()
{
    g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();

    g_pComponentSystemManager->UnmanageGameObject( m_ObjectCreated, true );
    m_ObjectCreated->SetEnabled( false, true );
    m_DeleteGameObjectsWhenDestroyed = true;
}

EditorCommand* EditorCommand_CreateGameObject::Repeat()
{
    EditorCommand_CopyGameObject* pCommand;
    pCommand = MyNew EditorCommand_CopyGameObject( m_ObjectCreated, false );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_CreateComponent
//====================================================================================================

EditorCommand_CreateComponent::EditorCommand_CreateComponent(GameObject* pGameObject, int componentType)
{
    m_Name = "EditorCommand_CreateComponent";

    MyAssert( pGameObject );
    MyAssert( componentType >= 0 );

    m_pGameObject = pGameObject;
    m_ComponentType = componentType;

    m_pComponentCreated = 0;
    m_DeleteComponentWhenDestroyed = false;
}

EditorCommand_CreateComponent::~EditorCommand_CreateComponent()
{
    if( m_DeleteComponentWhenDestroyed )
    {
        g_pComponentSystemManager->DeleteComponent( m_pComponentCreated );
    }
}

void EditorCommand_CreateComponent::Do()
{
    if( m_pComponentCreated == 0 )
        m_pComponentCreated = m_pGameObject->AddNewComponent( m_ComponentType, m_pGameObject->GetSceneID(), g_pComponentSystemManager );
    else
        m_pGameObject->AddExistingComponent( m_pComponentCreated, false );

    m_DeleteComponentWhenDestroyed = false;
}

void EditorCommand_CreateComponent::Undo()
{
    g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();

    m_pGameObject->RemoveComponent( m_pComponentCreated );
    m_DeleteComponentWhenDestroyed = true;
}

EditorCommand* EditorCommand_CreateComponent::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_CopyGameObject
//====================================================================================================

EditorCommand_CopyGameObject::EditorCommand_CopyGameObject(GameObject* objecttocopy, bool NewObjectInheritsFromOld)
{
    m_Name = "EditorCommand_CopyGameObject";

    m_ObjectToCopy = objecttocopy;
    m_ObjectCreated = 0;
    m_DeleteGameObjectWhenDestroyed = false;
    m_NewObjectInheritsFromOld = NewObjectInheritsFromOld;
}

EditorCommand_CopyGameObject::~EditorCommand_CopyGameObject()
{
    MyAssert( m_ObjectCreated );
    if( m_DeleteGameObjectWhenDestroyed && m_ObjectCreated )
        g_pComponentSystemManager->DeleteGameObject( m_ObjectCreated, true );
}

void CreateUniqueName(char* newname, int SizeInBytes, const char* oldname)
{
    int oldnamelen = (int)strlen( oldname );

    // Find number at end of string.
    int indexofnumber = -1;
    {
        for( int i=oldnamelen-1; i>=0; i-- )
        {
            if( oldname[i] < '0' || oldname[i] > '9' )
            {
                if( i != oldnamelen-1 )
                {
                    indexofnumber = i+1;
                }
                break;
            }
        }
    }

    // Find the old number, or 0 if one didn't exist.
    int number = 0;
    if( indexofnumber != -1 )
        number = atoi( &oldname[indexofnumber] );

    // Keep incrementing number until unique name is found.
    do
    {
        number += 1;
        if( indexofnumber == -1 )
        {
            // If the string didn't end with a number, print the whole name followed by a number.
            sprintf_s( newname, SizeInBytes, "%s%d", oldname, number );
        }
        else
        {
            // If the string did end with a number, copy the name then print a number.
            sprintf_s( newname, SizeInBytes, "%s", oldname );
            snprintf_s( newname+indexofnumber, SizeInBytes-indexofnumber, SizeInBytes-1-indexofnumber, "%d", number );
        }
    } while( g_pComponentSystemManager->FindGameObjectByName( newname ) != 0 );
}

void EditorCommand_CopyGameObject::Do()
{
    if( m_ObjectCreated == 0 )
    {
        char newname[50];
        const char* oldname = m_ObjectToCopy->GetName();

        if( m_NewObjectInheritsFromOld == false ) // If making a copy.
        {
            CreateUniqueName( newname, 50, oldname );
            m_ObjectCreated = g_pComponentSystemManager->CopyGameObject( m_ObjectToCopy, newname, false );
        }
        else // If making a child object.
        {
            snprintf_s( newname, 50, 49, "%s - child", m_ObjectToCopy->GetName() );
            m_ObjectCreated = g_pComponentSystemManager->CopyGameObject( m_ObjectToCopy, newname, false );
        }
    }
    else
    {
        g_pComponentSystemManager->ManageGameObject( m_ObjectCreated, true );
        m_ObjectCreated->SetEnabled( m_ObjectToCopy->IsEnabled(), true );
    }

    // If done/redone, then object exists in the scene, don't destroy it if undo stack get wiped.
    m_DeleteGameObjectWhenDestroyed = false;
}

void EditorCommand_CopyGameObject::Undo()
{
    g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();

    g_pComponentSystemManager->UnmanageGameObject( m_ObjectCreated, true );
    m_ObjectCreated->SetEnabled( false, true );

    // If undone then object only exists here, destroy object when this command gets deleted (when redo stack gets wiped).
    m_DeleteGameObjectWhenDestroyed = true;
}

EditorCommand* EditorCommand_CopyGameObject::Repeat()
{
    EditorCommand_CopyGameObject* pCommand;
    pCommand = MyNew EditorCommand_CopyGameObject( m_ObjectToCopy, m_NewObjectInheritsFromOld );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_ClearParentOfGameObjects
//====================================================================================================

EditorCommand_ClearParentOfGameObjects::EditorCommand_ClearParentOfGameObjects(GameObject* pObjectToClear)
{
    m_Name = "EditorCommand_ClearParentOfGameObjects";

    m_pObjectsToClear.push_back( pObjectToClear );
    m_pOldParents.push_back( pObjectToClear->GetGameObjectThisInheritsFrom() );
    m_OldPrefabRefs.push_back( *pObjectToClear->GetPrefabRef() );
}

EditorCommand_ClearParentOfGameObjects::EditorCommand_ClearParentOfGameObjects(std::vector<GameObject*>* pObjectsToClear)
{
    m_Name = "EditorCommand_ClearParentOfGameObjects";

    for( unsigned int i=0; i<pObjectsToClear->size(); i++ )
    {
        m_pObjectsToClear.push_back( (*pObjectsToClear)[i] );
        m_pOldParents.push_back( (*pObjectsToClear)[i]->GetGameObjectThisInheritsFrom() );
        m_OldPrefabRefs.push_back( *(*pObjectsToClear)[i]->GetPrefabRef() );
    }
}

EditorCommand_ClearParentOfGameObjects::~EditorCommand_ClearParentOfGameObjects()
{
}

void EditorCommand_ClearParentOfGameObjects::Do()
{
    for( unsigned int i=0; i<m_pObjectsToClear.size(); i++ )
    {
        if( m_OldPrefabRefs[i].GetPrefab() )
        {
            PrefabReference prefabRef;
            m_pObjectsToClear[i]->Editor_SetPrefab( &prefabRef );
        }
        else
        {
            m_pObjectsToClear[i]->SetGameObjectThisInheritsFrom( 0 );
        }
    }
}

void EditorCommand_ClearParentOfGameObjects::Undo()
{
    for( unsigned int i=0; i<m_pObjectsToClear.size(); i++ )
    {
        if( m_OldPrefabRefs[i].GetPrefab() )
        {
            PrefabReference prefabRef;
            m_pObjectsToClear[i]->Editor_SetPrefab( &m_OldPrefabRefs[i] );
        }
        else
        {
            m_pObjectsToClear[i]->SetGameObjectThisInheritsFrom( m_pOldParents[i] );
        }
    }
}

EditorCommand* EditorCommand_ClearParentOfGameObjects::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_EnableObject
//====================================================================================================

EditorCommand_EnableObject::EditorCommand_EnableObject(GameObject* pObject, bool enabled, bool affectChildren)
{
    m_Name = "EditorCommand_EnableObject";

    m_pGameObject = pObject;
    m_ObjectWasEnabled = enabled;
    m_AffectChildren = affectChildren;
}

EditorCommand_EnableObject::~EditorCommand_EnableObject()
{
}

void EditorCommand_EnableObject::Do()
{
    m_pGameObject->SetEnabled( m_ObjectWasEnabled, m_AffectChildren );
#if MYFW_USING_WX
    g_pPanelWatch->SetNeedsRefresh();
#endif
}

void EditorCommand_EnableObject::Undo()
{
    m_pGameObject->SetEnabled( !m_ObjectWasEnabled, m_AffectChildren );
#if MYFW_USING_WX
    g_pPanelWatch->SetNeedsRefresh();
#endif
}

EditorCommand* EditorCommand_EnableObject::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_DragAndDropEvent
//====================================================================================================

EditorCommand_DragAndDropEvent::EditorCommand_DragAndDropEvent(ComponentBase* pComponent, ComponentVariable* pVar, int controlcomponent, int x, int y, DragAndDropTypes type, void* newValue, void* oldValue)
{
    m_Name = "EditorCommand_DragAndDropEvent";

    m_pComponent = pComponent;
    m_pVar = pVar;
    m_ControlComponent = controlcomponent;
    m_X = x;
    m_Y = y;

    m_Type = type;
    m_pNewValue = newValue;
    m_pOldValue = oldValue;
}

EditorCommand_DragAndDropEvent::~EditorCommand_DragAndDropEvent()
{
}

void EditorCommand_DragAndDropEvent::Do()
{
    g_DragAndDropStruct.Clear();
    g_DragAndDropStruct.Add( m_Type, m_pNewValue );

    void* pReturnedValue = m_pComponent->OnDropVariable( m_pVar, m_ControlComponent, m_X, m_Y );

    MyAssert( pReturnedValue == m_pOldValue );
}

void EditorCommand_DragAndDropEvent::Undo()
{
    g_DragAndDropStruct.Clear();
    g_DragAndDropStruct.Add( m_Type, m_pOldValue );

    void* pReturnedValue = m_pComponent->OnDropVariable( m_pVar, m_ControlComponent, m_X, m_Y );

    MyAssert( pReturnedValue == m_pNewValue );
}

EditorCommand* EditorCommand_DragAndDropEvent::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ChangeMaterialOnMesh
//====================================================================================================

EditorCommand_ChangeMaterialOnMesh::EditorCommand_ChangeMaterialOnMesh(ComponentRenderable* pComponent, ComponentVariable* pVar, int submeshindex, MaterialDefinition* pMaterial)
{
    m_Name = "EditorCommand_ChangeMaterialOnMesh";

    MyAssert( pComponent );

    m_pComponent = pComponent;
    m_SubmeshIndex = submeshindex;
    m_pNewMaterial = pMaterial;

    m_pVar = pVar;
    m_VariableWasDivorced = pComponent->IsDivorced( m_pVar->m_Index );
}

EditorCommand_ChangeMaterialOnMesh::~EditorCommand_ChangeMaterialOnMesh()
{
}

void EditorCommand_ChangeMaterialOnMesh::Do()
{
    m_pOldMaterial = m_pComponent->GetMaterial( m_SubmeshIndex );

    m_pComponent->SetMaterial( m_pNewMaterial, m_SubmeshIndex );

#if MYFW_USING_WX
    g_pPanelWatch->SetNeedsRefresh();
#endif
}

void EditorCommand_ChangeMaterialOnMesh::Undo()
{
    m_pComponent->SetMaterial( m_pOldMaterial, m_SubmeshIndex );
#if MYFW_USING_WX
    g_pPanelWatch->SetNeedsRefresh();
#endif
}

EditorCommand* EditorCommand_ChangeMaterialOnMesh::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeTextureOnMaterial
//====================================================================================================

EditorCommand_ChangeTextureOnMaterial::EditorCommand_ChangeTextureOnMaterial(MaterialDefinition* material, TextureDefinition* texture)
{
    m_Name = "EditorCommand_ChangeTextureOnMaterial";

    m_pMaterial = material;
    m_pNewTexture = texture;
}

EditorCommand_ChangeTextureOnMaterial::~EditorCommand_ChangeTextureOnMaterial()
{
}

void EditorCommand_ChangeTextureOnMaterial::Do()
{
    m_pOldTexture = m_pMaterial->GetTextureColor();

    m_pMaterial->SetTextureColor( m_pNewTexture );
}

void EditorCommand_ChangeTextureOnMaterial::Undo()
{
    m_pMaterial->SetTextureColor( m_pOldTexture );
}

EditorCommand* EditorCommand_ChangeTextureOnMaterial::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeShaderOnMaterial
//====================================================================================================

EditorCommand_ChangeShaderOnMaterial::EditorCommand_ChangeShaderOnMaterial(MaterialDefinition* material, ShaderGroup* shadergroup)
{
    m_Name = "EditorCommand_ChangeShaderOnMaterial";

    m_pMaterial = material;
    m_pNewShaderGroup = shadergroup;
}

EditorCommand_ChangeShaderOnMaterial::~EditorCommand_ChangeShaderOnMaterial()
{
}

void EditorCommand_ChangeShaderOnMaterial::Do()
{
    m_pOldShaderGroup = m_pMaterial->GetShader();

    m_pMaterial->SetShader( m_pNewShaderGroup );
}

void EditorCommand_ChangeShaderOnMaterial::Undo()
{
    m_pMaterial->SetShader( m_pOldShaderGroup );
}

EditorCommand* EditorCommand_ChangeShaderOnMaterial::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeAllScriptsOnGameObject
//====================================================================================================

EditorCommand_ChangeAllScriptsOnGameObject::EditorCommand_ChangeAllScriptsOnGameObject(GameObject* object, MyFileObject* scriptfile)
{
    m_Name = "EditorCommand_ChangeAllScriptsOnGameObject";

    m_pGameObject = object;
    m_pNewScriptFile = scriptfile;
}

EditorCommand_ChangeAllScriptsOnGameObject::~EditorCommand_ChangeAllScriptsOnGameObject()
{
}

void EditorCommand_ChangeAllScriptsOnGameObject::Do()
{
    for( unsigned int i=0; i<m_pGameObject->GetComponentCount(); i++ )
    {
#if MYFW_USING_LUA
        ComponentLuaScript* pLuaComponent = dynamic_cast<ComponentLuaScript*>( m_pGameObject->GetComponentByIndex( i ) );

        if( pLuaComponent )
        {
            m_ComponentsChanged.push_back( pLuaComponent );
            m_OldScriptFiles.push_back( pLuaComponent->GetScriptFile() );
        }
#endif //MYFW_USING_LUA
    }

    m_pGameObject->SetScriptFile( m_pNewScriptFile );
}

void EditorCommand_ChangeAllScriptsOnGameObject::Undo()
{
    for( unsigned int i=0; i<m_ComponentsChanged.size(); i++ )
    {
#if MYFW_USING_LUA
        ComponentLuaScript* pLuaComponent = dynamic_cast<ComponentLuaScript*>( m_ComponentsChanged[i] );
        MyAssert( pLuaComponent );

        if( pLuaComponent )
        {
            pLuaComponent->SetScriptFile( m_OldScriptFiles[i] );
        }
#endif //MYFW_USING_LUA
    }
}

EditorCommand* EditorCommand_ChangeAllScriptsOnGameObject::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_ChangeSoundCue
//====================================================================================================

EditorCommand_ChangeSoundCue::EditorCommand_ChangeSoundCue(ComponentAudioPlayer* pComponent, SoundCue* pSoundCue)
{
    m_Name = "EditorCommand_ChangeSoundCue";

    m_pComponent = pComponent;
    m_pSoundCue = pSoundCue;

    m_pOldSoundCue = pComponent->GetSoundCue();
}

EditorCommand_ChangeSoundCue::~EditorCommand_ChangeSoundCue()
{
}

void EditorCommand_ChangeSoundCue::Do()
{
    m_pComponent->SetSoundCue( m_pSoundCue );
}

void EditorCommand_ChangeSoundCue::Undo()
{
    m_pComponent->SetSoundCue( m_pOldSoundCue );
}

EditorCommand* EditorCommand_ChangeSoundCue::Repeat()
{
    // Do nothing.

    return 0;
}

#if MYFW_USING_BOX2D

//====================================================================================================
// EditorCommand_Move2DPoint
//====================================================================================================

EditorCommand_Move2DPoint::EditorCommand_Move2DPoint(Vector2 distancemoved, Component2DCollisionObject* pCollisionObject, int indexmoved)
{
    m_Name = "EditorCommand_Move2DPoint";

    MyAssert( m_pCollisionObject );
    MyAssert( indexmoved >= 0 && indexmoved < (int)pCollisionObject->m_Vertices.size() );

    m_pCollisionObject = pCollisionObject;
    m_DistanceMoved = distancemoved;
    m_IndexOfPointMoved = indexmoved;

    //LOGInfo( LOGTag, "EditorCommand_Move2DPoint:: %f,%f\n", m_DistanceMoved.x, m_DistanceMoved.y );
}

EditorCommand_Move2DPoint::~EditorCommand_Move2DPoint()
{
}

void EditorCommand_Move2DPoint::Do()
{
    m_pCollisionObject->m_Vertices[m_IndexOfPointMoved].x += m_DistanceMoved.x;
    m_pCollisionObject->m_Vertices[m_IndexOfPointMoved].y += m_DistanceMoved.y;
}

void EditorCommand_Move2DPoint::Undo()
{
    m_pCollisionObject->m_Vertices[m_IndexOfPointMoved].x -= m_DistanceMoved.x;
    m_pCollisionObject->m_Vertices[m_IndexOfPointMoved].y -= m_DistanceMoved.y;
}

EditorCommand* EditorCommand_Move2DPoint::Repeat()
{
    EditorCommand_Move2DPoint* pCommand;
    pCommand = MyNew EditorCommand_Move2DPoint( *this );

    pCommand->Do();
    return pCommand;
}

//====================================================================================================
// EditorCommand_Insert2DPoint
//====================================================================================================

EditorCommand_Insert2DPoint::EditorCommand_Insert2DPoint(Component2DCollisionObject* pCollisionObject, int indexinserted)
{
    m_Name = "EditorCommand_Insert2DPoint";

    MyAssert( m_pCollisionObject );
    MyAssert( indexinserted >= 0 && indexinserted < (int)pCollisionObject->m_Vertices.size() );

    m_pCollisionObject = pCollisionObject;
    m_IndexOfPointInserted = indexinserted;
}

EditorCommand_Insert2DPoint::~EditorCommand_Insert2DPoint()
{
}

void EditorCommand_Insert2DPoint::Do()
{
    std::vector<Vector2>::iterator it = m_pCollisionObject->m_Vertices.begin();
    m_pCollisionObject->m_Vertices.insert( it + m_IndexOfPointInserted, m_pCollisionObject->m_Vertices[m_IndexOfPointInserted] );
}

void EditorCommand_Insert2DPoint::Undo()
{
    std::vector<Vector2>::iterator it = m_pCollisionObject->m_Vertices.begin();
    m_pCollisionObject->m_Vertices.erase( it + m_IndexOfPointInserted );
}

EditorCommand* EditorCommand_Insert2DPoint::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_Delete2DPoint
//====================================================================================================

EditorCommand_Delete2DPoint::EditorCommand_Delete2DPoint(Component2DCollisionObject* pCollisionObject, int indexdeleted, Vector2 position)
{
    m_Name = "EditorCommand_Delete2DPoint";

    MyAssert( m_pCollisionObject );
    MyAssert( indexdeleted >= 0 && indexdeleted < (int)pCollisionObject->m_Vertices.size() );

    m_pCollisionObject = pCollisionObject;
    m_IndexOfPointDeleted = indexdeleted;
    m_Position = position;
}

EditorCommand_Delete2DPoint::~EditorCommand_Delete2DPoint()
{
}

void EditorCommand_Delete2DPoint::Do()
{
    std::vector<Vector2>::iterator it = m_pCollisionObject->m_Vertices.begin();
    m_pCollisionObject->m_Vertices.erase( it + m_IndexOfPointDeleted );
}

void EditorCommand_Delete2DPoint::Undo()
{
    std::vector<Vector2>::iterator it = m_pCollisionObject->m_Vertices.begin();
    m_pCollisionObject->m_Vertices.insert( it + m_IndexOfPointDeleted, m_Position );
}

EditorCommand* EditorCommand_Delete2DPoint::Repeat()
{
    // Do nothing.

    return 0;
}

#endif //MYFW_USING_BOX2D

//====================================================================================================
// EditorCommand_ComponentVariablePointerChanged
//====================================================================================================

EditorCommand_ComponentVariablePointerChanged::EditorCommand_ComponentVariablePointerChanged(ComponentVariableCallbackInterface* pCallbackObject, ComponentVariable* pVar, ComponentVariableValue* pOldValue, ComponentVariableValue* pNewValue)
{
    m_Name = "EditorCommand_ComponentVariablePointerChanged";

    MyAssert( pCallbackObject && pVar );

    m_pCallbackObject = pCallbackObject;
    m_pVar = pVar;
    m_VarType = pVar->m_Type; // Stored since needed in destructor and pVar might be deleted by then. // TODO: Destroy command stack before most other things on shutdown to avoid this?

    m_OldValue = *pOldValue;
    m_NewValue = *pNewValue;

    m_ReleaseObjectsWhenDestroyed = false;

    if( m_VarType == ComponentVariableType::FilePtr )
    {
        // Add a ref to each of the files so they don't get free'd when undo/redoing.
        if( m_OldValue.GetFilePtr() )
            m_OldValue.GetFilePtr()->AddRef();
        if( m_NewValue.GetFilePtr() )
            m_NewValue.GetFilePtr()->AddRef();

        m_ReleaseObjectsWhenDestroyed = true;
    }
    else if( m_VarType == ComponentVariableType::MaterialPtr )
    {
        // Add a ref to each of the materials so they don't get free'd when undo/redoing.
        if( m_OldValue.GetMaterialPtr() )
            m_OldValue.GetMaterialPtr()->AddRef();
        if( m_NewValue.GetMaterialPtr() )
            m_NewValue.GetMaterialPtr()->AddRef();

        m_ReleaseObjectsWhenDestroyed = true;
    }
    else if( m_VarType == ComponentVariableType::TexturePtr )
    {
        // Add a ref to each of the materials so they don't get free'd when undo/redoing.
        if( m_OldValue.GetTexturePtr() )
            m_OldValue.GetTexturePtr()->AddRef();
        if( m_NewValue.GetTexturePtr() )
            m_NewValue.GetTexturePtr()->AddRef();

        m_ReleaseObjectsWhenDestroyed = true;
    }
    else if( m_VarType == ComponentVariableType::SoundCuePtr )
    {
        // Add a ref to each of the materials so they don't get free'd when undo/redoing.
        if( m_OldValue.GetSoundCuePtr() )
            m_OldValue.GetSoundCuePtr()->AddRef();
        if( m_NewValue.GetSoundCuePtr() )
            m_NewValue.GetSoundCuePtr()->AddRef();

        m_ReleaseObjectsWhenDestroyed = true;
    }
    else if( m_VarType == ComponentVariableType::PointerIndirect )
    {
        MyAssert( false );
    }
}

EditorCommand_ComponentVariablePointerChanged::~EditorCommand_ComponentVariablePointerChanged()
{
    if( m_ReleaseObjectsWhenDestroyed )
    {
        if( m_VarType == ComponentVariableType::FilePtr )
        {
            if( m_OldValue.GetFilePtr() )
                m_OldValue.GetFilePtr()->Release();
            if( m_NewValue.GetFilePtr() )
                m_NewValue.GetFilePtr()->Release();
        }
        else if( m_VarType == ComponentVariableType::MaterialPtr )
        {
            if( m_OldValue.GetMaterialPtr() )
                m_OldValue.GetMaterialPtr()->Release();
            if( m_NewValue.GetMaterialPtr() )
                m_NewValue.GetMaterialPtr()->Release();
        }
        else if( m_VarType == ComponentVariableType::TexturePtr )
        {
            if( m_OldValue.GetTexturePtr() )
                m_OldValue.GetTexturePtr()->Release();
            if( m_NewValue.GetTexturePtr() )
                m_NewValue.GetTexturePtr()->Release();
        }
        else if( m_VarType == ComponentVariableType::SoundCuePtr )
        {
            if( m_OldValue.GetSoundCuePtr() )
                m_OldValue.GetSoundCuePtr()->Release();
            if( m_NewValue.GetSoundCuePtr() )
                m_NewValue.GetSoundCuePtr()->Release();
        }
    }
}

void EditorCommand_ComponentVariablePointerChanged::Do()
{
#if MYFW_USING_WX
    g_pPanelWatch->UpdatePanel();
#endif

    if( m_pVar->m_pOnValueChangedCallbackFunc )
    {
        // This could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
        (m_pCallbackObject->*(m_pVar->m_pOnValueChangedCallbackFunc))( m_pVar, false, true, 0, &m_NewValue );
    }
    else
    {
        void** pPtr = (void**)((char*)m_pCallbackObject + m_pVar->m_Offset);

        if( m_VarType == ComponentVariableType::GameObjectPtr )
        {
            *pPtr = m_NewValue.GetGameObjectPtr();
        }
        else if( m_VarType == ComponentVariableType::ComponentPtr )
        {
            *pPtr = m_NewValue.GetComponentPtr();
        }
        else if( m_VarType == ComponentVariableType::FilePtr )
        {
            if( m_OldValue.GetFilePtr() )
                m_OldValue.GetFilePtr()->Release();
            if( m_NewValue.GetFilePtr() )
                m_NewValue.GetFilePtr()->AddRef();

            *pPtr = m_NewValue.GetFilePtr();
        }
        else if( m_VarType == ComponentVariableType::MaterialPtr )
        {
            if( m_OldValue.GetMaterialPtr() )
                m_OldValue.GetMaterialPtr()->Release();
            if( m_NewValue.GetMaterialPtr() )
                m_NewValue.GetMaterialPtr()->AddRef();

            *pPtr = m_NewValue.GetMaterialPtr();
        }
        else if( m_VarType == ComponentVariableType::TexturePtr )
        {
            if( m_OldValue.GetTexturePtr() )
                m_OldValue.GetTexturePtr()->Release();
            if( m_NewValue.GetTexturePtr() )
                m_NewValue.GetTexturePtr()->AddRef();

            *pPtr = m_NewValue.GetTexturePtr();
        }
        else if( m_VarType == ComponentVariableType::SoundCuePtr )
        {
            if( m_OldValue.GetSoundCuePtr() )
                m_OldValue.GetSoundCuePtr()->Release();
            if( m_NewValue.GetSoundCuePtr() )
                m_NewValue.GetSoundCuePtr()->AddRef();

            *pPtr = m_NewValue.GetSoundCuePtr();
        }
        else
        {
            MyAssert( false );
        }
    }
}

void EditorCommand_ComponentVariablePointerChanged::Undo()
{
#if MYFW_USING_WX
    g_pPanelWatch->UpdatePanel();
#endif

    if( m_pVar->m_pOnValueChangedCallbackFunc )
    {
        // This could likely be dangerous, the object might not be in focus anymore and how it handles callbacks could cause issues.
        (m_pCallbackObject->*(m_pVar->m_pOnValueChangedCallbackFunc))( m_pVar, false, true, 0, &m_OldValue );
    }
    else
    {
        void** pPtr = (void**)((char*)m_pCallbackObject + m_pVar->m_Offset);

        if( m_VarType == ComponentVariableType::GameObjectPtr )
        {
            *pPtr = m_OldValue.GetGameObjectPtr();
        }
        else if( m_VarType == ComponentVariableType::ComponentPtr )
        {
            *pPtr = m_OldValue.GetComponentPtr();
        }
        else if( m_VarType == ComponentVariableType::FilePtr )
        {
            if( m_NewValue.GetFilePtr() )
                m_NewValue.GetFilePtr()->Release();
            if( m_OldValue.GetFilePtr() )
                m_OldValue.GetFilePtr()->AddRef();

            *pPtr = m_OldValue.GetFilePtr();
        }
        else if( m_VarType == ComponentVariableType::MaterialPtr )
        {
            if( m_NewValue.GetMaterialPtr() )
                m_NewValue.GetMaterialPtr()->Release();
            if( m_OldValue.GetMaterialPtr() )
                m_OldValue.GetMaterialPtr()->AddRef();

            *pPtr = m_OldValue.GetMaterialPtr();
        }
        else if( m_VarType == ComponentVariableType::TexturePtr )
        {
            if( m_NewValue.GetTexturePtr() )
                m_NewValue.GetTexturePtr()->Release();
            if( m_OldValue.GetTexturePtr() )
                m_OldValue.GetTexturePtr()->AddRef();

            *pPtr = m_OldValue.GetTexturePtr();
        }
        else if( m_VarType == ComponentVariableType::SoundCuePtr )
        {
            if( m_NewValue.GetSoundCuePtr() )
                m_NewValue.GetSoundCuePtr()->Release();
            if( m_OldValue.GetSoundCuePtr() )
                m_OldValue.GetSoundCuePtr()->AddRef();

            *pPtr = m_OldValue.GetSoundCuePtr();
        }
        else
        {
            MyAssert( false );
        }
    }
}

EditorCommand* EditorCommand_ComponentVariablePointerChanged::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ExposedVariableChanged
//====================================================================================================

EditorCommand_ExposedVariableChanged::EditorCommand_ExposedVariableChanged(ExposedVariableValue newValue, ExposedVariableDesc* pVar, ExposedVarValueChangedCallback* callbackFunc, void* callbackObj)
{
    m_Name = "EditorCommand_ExposedVariableChanged";

    m_NewValue = newValue;
    m_pVar = pVar;

    m_OldValue = pVar->value;

    m_pOnValueChangedCallBackFunc = callbackFunc;
    m_pCallbackObj = callbackObj;
}

EditorCommand_ExposedVariableChanged::~EditorCommand_ExposedVariableChanged()
{
}

void EditorCommand_ExposedVariableChanged::Do()
{
    m_pVar->value = m_NewValue;

    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_pVar, 0, true, m_OldValue, nullptr );
}

void EditorCommand_ExposedVariableChanged::Undo()
{
    m_pVar->value = m_OldValue;

    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_pVar, 0, true, m_NewValue, nullptr );
}

EditorCommand* EditorCommand_ExposedVariableChanged::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ExposedVariablePointerChanged
//====================================================================================================

EditorCommand_ExposedVariablePointerChanged::EditorCommand_ExposedVariablePointerChanged(void* newValue, ExposedVariableDesc* pVar, ExposedVarValueChangedCallback* callbackFunc, void* callbackObj)
{
    m_Name = "EditorCommand_ExposedVariablePointerChanged";

    m_NewValue = newValue;
    m_pVar = pVar;

    m_OldValue = pVar->value.valuePointer;

    m_pOnValueChangedCallBackFunc = callbackFunc;
    m_pCallbackObj = callbackObj;
}

EditorCommand_ExposedVariablePointerChanged::~EditorCommand_ExposedVariablePointerChanged()
{
}

void EditorCommand_ExposedVariablePointerChanged::Do()
{
    m_pVar->value.valuePointer = m_NewValue;

    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_pVar, 0, true, ExposedVariableValue((float)0), m_OldValue );
}

void EditorCommand_ExposedVariablePointerChanged::Undo()
{
    m_pVar->value.valuePointer = m_OldValue;

    if( m_pCallbackObj && m_pOnValueChangedCallBackFunc )
        m_pOnValueChangedCallBackFunc( m_pCallbackObj, m_pVar, 0, true, ExposedVariableValue((float)0), m_NewValue );
}

EditorCommand* EditorCommand_ExposedVariablePointerChanged::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ScriptClearExposedVariables
//====================================================================================================

EditorCommand_ScriptClearExposedVariables::EditorCommand_ScriptClearExposedVariables(ComponentScriptBase* pScriptComponent, MyList<ExposedVariableDesc*>& exposedVariablesList)
: m_OriginalExposedVariablesListFromComponent( exposedVariablesList )
{
    m_Name = "EditorCommand_ScriptClearExposedVariables";

    m_pScriptComponent = pScriptComponent;

    // Make a backup of all the exposed variable pointers.
    for( uint32 i=0; i<exposedVariablesList.size(); i++ )
    {
        m_CopyOfExposedVariables.push_back( exposedVariablesList[i] );
    }

    m_DeleteExposedVarsWhenDestroyed = false;
}

EditorCommand_ScriptClearExposedVariables::~EditorCommand_ScriptClearExposedVariables()
{
    // If deleting this command from the redo stack, then delete the exposed variables.
    if( m_DeleteExposedVarsWhenDestroyed )
    {
        while( m_CopyOfExposedVariables.size() > 0 )
        {
            delete m_CopyOfExposedVariables[m_CopyOfExposedVariables.size()-1];
            m_CopyOfExposedVariables.pop_back();
        }
    }
}

void EditorCommand_ScriptClearExposedVariables::Do()
{
    MyAssert( m_OriginalExposedVariablesListFromComponent.size() == m_CopyOfExposedVariables.size() );

    // Unregister GameObject deleted callbacks, if we registered any.
    for( uint32 i=0; i<m_CopyOfExposedVariables.size(); i++ )
    {
        ExposedVariableDesc* pVariable = m_CopyOfExposedVariables[i];

        if( pVariable->value.type == ExposedVariableType::GameObject && pVariable->value.valuePointer )
        {
            GameObject* pGameObject = static_cast<GameObject*>( pVariable->value.valuePointer );
            pGameObject->UnregisterOnDeleteCallback( m_pScriptComponent, ComponentScriptBase::StaticOnGameObjectDeleted );
        }
    }

    // Clear the component's list of exposed variables.
    m_OriginalExposedVariablesListFromComponent.Clear();

    m_DeleteExposedVarsWhenDestroyed = true;
}

void EditorCommand_ScriptClearExposedVariables::Undo()
{
    MyAssert( m_OriginalExposedVariablesListFromComponent.size() == 0 );

    // Restore the component's list of exposed variables.
    for( uint32 i=0; i<m_CopyOfExposedVariables.size(); i++ )
    {
        m_OriginalExposedVariablesListFromComponent.Add( m_CopyOfExposedVariables[i] );
    }

    // Register OnDelete callbacks with GameObject, if necessary.
    for( uint32 i=0; i<m_OriginalExposedVariablesListFromComponent.size(); i++ )
    {
        ExposedVariableDesc* pVariable = m_OriginalExposedVariablesListFromComponent[i];

        // Unregister gameobject deleted callback, if we registered one.
        if( pVariable->value.type == ExposedVariableType::GameObject && pVariable->value.valuePointer )
            static_cast<GameObject*>( pVariable->value.valuePointer )->RegisterOnDeleteCallback( m_pScriptComponent, ComponentScriptBase::StaticOnGameObjectDeleted );
    }

    m_DeleteExposedVarsWhenDestroyed = false;
}

EditorCommand* EditorCommand_ScriptClearExposedVariables::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_DeletePrefabs
//====================================================================================================

EditorCommand_DeletePrefabs::EditorCommand_DeletePrefabs(const std::vector<PrefabObject*>& selectedprefabs)
{
    m_Name = "EditorCommand_DeletePrefabs";

    MyAssert( selectedprefabs.size() > 0 );

    for( unsigned int i=0; i<selectedprefabs.size(); i++ )
    {
        PrefabObject* pPrefab = selectedprefabs[i];

        PrefabInfo info;
        info.m_pPrefab = pPrefab;
        info.m_pPreviousPrefabInObjectList = (PrefabObject*)pPrefab->GetPrev();
        g_pComponentSystemManager->Editor_GetListOfGameObjectsThatUsePrefab( &info.m_pListOfGameObjectsThatUsedPrefab, pPrefab );

        // Make a copy of the PrefabReference in each GameObject, for undo.
        for( unsigned int j=0; j<info.m_pListOfGameObjectsThatUsedPrefab.size(); j++ )
        {
            GameObject* pGameObject = info.m_pListOfGameObjectsThatUsedPrefab[j];
            info.m_CopyOfPrefabRefsInEachGameObjectBeforePrefabWasDeleted.push_back( *pGameObject->GetPrefabRef() );
        }

        m_PrefabInfo.push_back( info );
    }

    m_DeletePrefabsWhenDestroyed = false;
}

EditorCommand_DeletePrefabs::~EditorCommand_DeletePrefabs()
{
    if( m_DeletePrefabsWhenDestroyed )
    {
        for( unsigned int i=0; i<m_PrefabInfo.size(); i++ )
        {
            delete m_PrefabInfo[i].m_pPrefab;
        }
    }
}

void EditorCommand_DeletePrefabs::Do()
{
    g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_PrefabInfo.size(); i++ )
    {
        PrefabObject* pPrefab = m_PrefabInfo[i].m_pPrefab;
        PrefabFile* pFile = pPrefab->GetPrefabFile();

        // Loop through gameobjects and unset which prefab they inherit from.
        for( unsigned int j=0; j<m_PrefabInfo[i].m_pListOfGameObjectsThatUsedPrefab.size(); j++ )
        {
            GameObject* pGameObject = m_PrefabInfo[i].m_pListOfGameObjectsThatUsedPrefab[j];

            // Set each GameObject to a blank PrefabReference.
            PrefabReference prefabRef;
            pGameObject->Editor_SetPrefab( &prefabRef );
        }
        
#if MYFW_EDITOR
        // Remove prefab from PrefabFile
        pFile->RemovePrefab( pPrefab );
#endif
    }

    m_DeletePrefabsWhenDestroyed = true;
}

void EditorCommand_DeletePrefabs::Undo()
{
    g_pEngineCore->GetEditorState()->ClearSelectedObjectsAndComponents();

    for( unsigned int i=0; i<m_PrefabInfo.size(); i++ )
    {
        PrefabObject* pPrefab = m_PrefabInfo[i].m_pPrefab;
        PrefabObject* pPreviousPrefab = m_PrefabInfo[i].m_pPreviousPrefabInObjectList;
        PrefabFile* pFile = pPrefab->GetPrefabFile();

        // Place prefab in old spot in PrefabFile and object list
#if MYFW_EDITOR
        pFile->AddExistingPrefab( pPrefab, pPreviousPrefab );

        // Loop through gameobjects and reset which prefab they inherited from.
        for( unsigned int j=0; j<m_PrefabInfo[i].m_pListOfGameObjectsThatUsedPrefab.size(); j++ )
        {
            m_PrefabInfo[i].m_pListOfGameObjectsThatUsedPrefab[j]->Editor_SetPrefab( &m_PrefabInfo[i].m_CopyOfPrefabRefsInEachGameObjectBeforePrefabWasDeleted[j] );
        }
#endif
    }

    m_DeletePrefabsWhenDestroyed = false;
}

EditorCommand* EditorCommand_DeletePrefabs::Repeat()
{
    // Do nothing.

    return 0;
}

//====================================================================================================
// EditorCommand_DivorceOrMarryComponentVariable
//====================================================================================================

EditorCommand_DivorceOrMarryComponentVariable::EditorCommand_DivorceOrMarryComponentVariable(ComponentBase* pComponent, ComponentVariable* pVar, bool divorcethevariable)
{
    m_Name = "EditorCommand_DivorceOrMarryComponentVariable";

    MyAssert( pComponent && pVar );

    m_pComponent = pComponent;
    m_pVar = pVar;

    if( divorcethevariable )
    {
        MyAssert( m_pComponent->IsDivorced( pVar->m_Index ) == false );
    }
    else
    {
        MyAssert( m_pComponent->IsDivorced( pVar->m_Index ) == true );
    }

    m_DivorceTheVariable = divorcethevariable;

    m_OldValue.GetValueFromVariable( pComponent, pVar, pComponent );
}

EditorCommand_DivorceOrMarryComponentVariable::~EditorCommand_DivorceOrMarryComponentVariable()
{
}

void EditorCommand_DivorceOrMarryComponentVariable::Do()
{
    if( m_DivorceTheVariable )
    {
        // Divorce the variables.
        m_pComponent->SetDivorced( m_pVar->m_Index, true );
        //if( m_pVar->m_ControlID >= 0 )
        //{
        //    g_pPanelWatch->ChangeStaticTextFontStyle( m_pVar->m_ControlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
        //    g_pPanelWatch->ChangeStaticTextBGColor( m_pVar->m_ControlID, wxColour( 255, 200, 200, 255 ) );
        //}

#if MYFW_USING_WX
        g_pPanelWatch->SetNeedsRefresh();
#endif
    }
    else
    {
        // Marry the variables.
        m_pComponent->SetDivorced( m_pVar->m_Index, false );

        // Set this components value to the parent's value.
        ComponentBase* pParentComponent = m_pComponent->FindMatchingComponentInParent();
        MyAssert( pParentComponent );
        if( pParentComponent )
        {
            // Get parent objects value.
            ComponentVariableValue newvalue( pParentComponent, m_pVar, pParentComponent );

            // Update and inform component and children.
            newvalue.UpdateComponentAndChildrenWithValue( m_pComponent, m_pVar, m_pComponent );
        }

#if MYFW_USING_WX
        g_pPanelWatch->SetNeedsRefresh();
#endif
    }
}

void EditorCommand_DivorceOrMarryComponentVariable::Undo()
{
    // Do the opposite
    if( m_DivorceTheVariable )
    {
        // Marry the variables.
        m_pComponent->SetDivorced( m_pVar->m_Index, false );
        //if( m_pVar->m_ControlID >= 0 )
        //{
        //    g_pPanelWatch->ChangeStaticTextFontStyle( m_pVar->m_ControlID, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
        //    g_pPanelWatch->ChangeStaticTextBGColor( m_pVar->m_ControlID, wxNullColour );
        //}

#if MYFW_USING_WX
        g_pPanelWatch->SetNeedsRefresh();
#endif
    }
    else
    {
        // Divorce the variables.
        m_pComponent->SetDivorced( m_pVar->m_Index, true );
        //if( m_pVar->m_ControlID >= 0 )
        //{
        //    g_pPanelWatch->ChangeStaticTextFontStyle( m_pVar->m_ControlID, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_BOLD );
        //    g_pPanelWatch->ChangeStaticTextBGColor( m_pVar->m_ControlID, wxColour( 255, 200, 200, 255 ) );
        //}

        // Update and inform component and children.
        m_OldValue.UpdateComponentAndChildrenWithValue( m_pComponent, m_pVar, m_pComponent );

#if MYFW_USING_WX
        g_pPanelWatch->SetNeedsRefresh();
#endif
    }
}

EditorCommand* EditorCommand_DivorceOrMarryComponentVariable::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ComponentVariableIndirectPointerChanged
//====================================================================================================

EditorCommand_ComponentVariableIndirectPointerChanged::EditorCommand_ComponentVariableIndirectPointerChanged(ComponentBase* pComponent, ComponentVariable* pVar, void* newvalue)
{
    m_Name = "EditorCommand_ComponentVariableIndirectPointerChanged";

    MyAssert( pComponent && pVar );

    m_pComponent = pComponent;
    m_pVar = pVar;

    m_OldValue = (pComponent->*(pVar->m_pGetPointerValueCallBackFunc))( pVar );
    m_NewValue = newvalue;
}

EditorCommand_ComponentVariableIndirectPointerChanged::~EditorCommand_ComponentVariableIndirectPointerChanged()
{
}

void EditorCommand_ComponentVariableIndirectPointerChanged::Do()
{
    // Set the value in the component.
    (m_pComponent->*(m_pVar->m_pSetPointerValueCallBackFunc))( m_pVar, m_NewValue );

#if MYFW_USING_WX
    g_pPanelWatch->SetNeedsRefresh();
#endif
}

void EditorCommand_ComponentVariableIndirectPointerChanged::Undo()
{
    // Set the value in the component.
    (m_pComponent->*(m_pVar->m_pSetPointerValueCallBackFunc))( m_pVar, m_OldValue );

#if MYFW_USING_WX
    g_pPanelWatch->SetNeedsRefresh();
#endif
}

EditorCommand* EditorCommand_ComponentVariableIndirectPointerChanged::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_ReorderOrReparentGameObjects
//====================================================================================================

EditorCommand_ReorderOrReparentGameObjects::EditorCommand_ReorderOrReparentGameObjects(const std::vector<GameObject*>& selectedobjects, GameObject* pObjectDroppedOn, SceneID sceneid, bool setaschild)
{
    m_Name = "EditorCommand_ReorderOrReparentGameObjects";

    for( unsigned int i=0; i<selectedobjects.size(); i++ )
    {
        m_SelectedObjects.push_back( selectedobjects[i] );

        m_OldSceneIDs.push_back( selectedobjects[i]->GetSceneID() );
        m_OldPreviousObjectInList.push_back( selectedobjects[i]->GetPrev() );
        m_OldParent.push_back( selectedobjects[i]->GetParentGameObject() );
    }

    m_pObjectDroppedOn = pObjectDroppedOn;
    m_SceneIDDroppedOn = sceneid;
    m_MakeSelectedObjectsChildren = setaschild;

    MyAssert( m_pObjectDroppedOn == 0 || m_pObjectDroppedOn->GetSceneID() == sceneid );
}

EditorCommand_ReorderOrReparentGameObjects::~EditorCommand_ReorderOrReparentGameObjects()
{
}

void EditorCommand_ReorderOrReparentGameObjects::Do()
{
    // Move/Reparent all of the selected items.
    for( int i=(int)m_SelectedObjects.size()-1; i>=0; i-- )
    {
        GameObject* pGameObject = m_SelectedObjects[i];

        // Change the selected gameobject's sceneid to match the one dropped on.
        pGameObject->SetSceneID( m_SceneIDDroppedOn );

        if( m_MakeSelectedObjectsChildren == false )
        {
            // Move below the selected item.
#if MYFW_USING_WX
            g_pPanelObjectList->Tree_MoveObject( pGameObject, m_pObjectDroppedOn, false );
#endif
            GameObject* thisparent = m_pObjectDroppedOn->GetParentGameObject();
            pGameObject->SetParentGameObject( thisparent );
            pGameObject->MoveAfter( m_pObjectDroppedOn );
        }
        else
        {
            if( m_pObjectDroppedOn )
            {
                // Parent the object dropped to this.
                pGameObject->SetParentGameObject( m_pObjectDroppedOn );

                // Move as first item in parent.
#if MYFW_USING_WX
                g_pPanelObjectList->Tree_MoveObject( pGameObject, m_pObjectDroppedOn, true );
#endif
            }
            else
            {
                // If no m_pObjectDroppedOn, then we'll add these objects as the first objects of the scene.

                // We dropped it directly on the scene, so it shouldn't have a parent anymore.
                pGameObject->SetParentGameObject( 0 );

                SceneInfo* pSceneInfo = g_pComponentSystemManager->GetSceneInfo( m_SceneIDDroppedOn );
                pSceneInfo->m_GameObjects.MoveHead( pGameObject );

                // Move the wx tree item to the correct spot.
#if MYFW_USING_WX
                wxTreeItemId treeidtomove = g_pPanelObjectList->FindObject( pGameObject );
                g_pPanelObjectList->Tree_MoveObject( treeidtomove, pSceneInfo->m_TreeID, true );
#endif
            }
        }
    }
}

void EditorCommand_ReorderOrReparentGameObjects::Undo()
{
    // Move/Reparent all of the selected items.
    for( unsigned int i=0; i<m_SelectedObjects.size(); i++ )
    {
        GameObject* pGameObject = m_SelectedObjects[i];

        // Change the selected gameobject's sceneid back to it's original.
        pGameObject->SetSceneID( m_OldSceneIDs[i] );

        // If this wasn't the first child of a parent object.
        if( m_OldPreviousObjectInList[i] != 0 )
        {
            // Move back to old position.
#if MYFW_USING_WX
            g_pPanelObjectList->Tree_MoveObject( pGameObject, m_OldPreviousObjectInList[i], false );
#endif
            pGameObject->SetParentGameObject( m_OldParent[i] );
            pGameObject->MoveAfter( m_OldPreviousObjectInList[i] );

            MyAssert( m_OldPreviousObjectInList[i]->GetParentGameObject() == m_OldParent[i] );
        }
        else
        {
            // If this was the first object in the scene.
            if( m_OldParent[i] == 0 )
            {
                // Unparent the gameobject and move it be be first in the scene list.
                pGameObject->SetParentGameObject( 0 );
                g_pComponentSystemManager->GetSceneInfo( m_OldSceneIDs[i] )->m_GameObjects.MoveHead( pGameObject );

                // Move the wx tree item to the correct spot.
#if MYFW_USING_WX
                wxTreeItemId treeidtomove = g_pPanelObjectList->FindObject( pGameObject );
                wxTreeItemId scenetreeid = g_pComponentSystemManager->GetSceneInfo( m_OldSceneIDs[i] )->m_TreeID;
                g_pPanelObjectList->Tree_MoveObject( treeidtomove, scenetreeid, true );
#endif
            }
            else // This was the first child of a parent.
            {
                // Reset parent to original parent.
                pGameObject->SetParentGameObject( m_OldParent[i] );

                // Move as first item in parent.
#if MYFW_USING_WX
                g_pPanelObjectList->Tree_MoveObject( pGameObject, m_OldParent[i], true );
#endif
            }
        }
    }
}

EditorCommand* EditorCommand_ReorderOrReparentGameObjects::Repeat()
{
    return 0;
}

//====================================================================================================
// EditorCommand_RestorePrefabComponent
//====================================================================================================

EditorCommand_RestorePrefabComponent::EditorCommand_RestorePrefabComponent(GameObject* pObject, uint32 deletedPrefabComponentID)
{
    m_Name = "EditorCommand_RestorePrefabComponent";

    MyAssert( m_pGameObject != 0 );
    MyAssert( deletedPrefabComponentID != 0 );

    m_pGameObject = pObject;
    m_DeletedPrefabComponentID = deletedPrefabComponentID;
    m_pComponentCreated = 0;
}

EditorCommand_RestorePrefabComponent::~EditorCommand_RestorePrefabComponent()
{
}

void EditorCommand_RestorePrefabComponent::Do()
{
    MyAssert( m_pGameObject->GetPrefabRef() );
    MyAssert( m_pGameObject->GetPrefabRef()->GetPrefab() );

    cJSON* jPrefab = m_pGameObject->GetPrefabRef()->GetPrefab()->GetJSONObject();
    MyAssert( jPrefab );

    // If inheriting from a prefab.
    if( jPrefab )
    {
        // Create matching components in new GameObject.
        cJSON* jComponentArray = cJSON_GetObjectItem( jPrefab, "Components" );
        if( jComponentArray )
        {
            int componentarraysize = cJSON_GetArraySize( jComponentArray );

            for( int i=0; i<componentarraysize; i++ )
            {
                cJSON* jComponent = cJSON_GetArrayItem( jComponentArray, i );

                uint32 componentID = 0;
                cJSONExt_GetUnsignedInt( jComponent, "PrefabComponentID", &componentID );

                if( componentID == m_DeletedPrefabComponentID )
                {
                    m_pComponentCreated = g_pComponentSystemManager->CreateComponentFromJSONObject( m_pGameObject, jComponent );
                    MyAssert( m_pComponentCreated );
                    if( m_pComponentCreated )
                    {
                        m_pComponentCreated->ImportFromJSONObject( jComponent, m_pGameObject->GetSceneID() );
                        m_pComponentCreated->OnLoad();

                        // Remove this prefab component id from the "deleted" list.
                        uint32 id = m_pComponentCreated->GetPrefabComponentID();
                        if( id != 0 )
                        {
                            // Make sure it's in the list.
                            MyAssert( std::find( m_pGameObject->m_DeletedPrefabComponentIDs.begin(), m_pGameObject->m_DeletedPrefabComponentIDs.end(), id ) != m_pGameObject->m_DeletedPrefabComponentIDs.end() );

                            std::vector<uint32>::iterator lastremoved = std::remove( m_pGameObject->m_DeletedPrefabComponentIDs.begin(), m_pGameObject->m_DeletedPrefabComponentIDs.end(), id );
                            m_pGameObject->m_DeletedPrefabComponentIDs.erase( lastremoved, m_pGameObject->m_DeletedPrefabComponentIDs.end() );
                        }
                    }
                }
            }
        }
    }
}

void EditorCommand_RestorePrefabComponent::Undo()
{
    g_pComponentSystemManager->DeleteComponent( m_pComponentCreated );
}

EditorCommand* EditorCommand_RestorePrefabComponent::Repeat()
{
    return 0;
}

//====================================================================================================
//====================================================================================================

EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh::EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh(ComponentMeshPrimitive* pComponent, MyMesh* pOldMesh, ComponentMeshPrimitives newMeshPrimitiveType)
{
    m_Name = "EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh";

    MyAssert( pComponent != nullptr );
    MyAssert( pOldMesh != nullptr );
    MyAssert( newMeshPrimitiveType < ComponentMeshPrimitive_NumTypesAccessibleFromInterface );

    m_pComponent = pComponent;
    
    m_pOldMesh = pOldMesh;
    m_pOldMesh->AddRef();
    m_pNewMesh = MyNew MyMesh( g_pComponentSystemManager->GetEngineCore() );

    m_OldGLPrimitiveType = m_pComponent->m_GLPrimitiveType;
    m_NewGLPrimitiveType = m_pComponent->m_GLPrimitiveType;
    m_NewMeshPrimitiveType = newMeshPrimitiveType;
}

EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh::~EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh()
{
    SAFE_RELEASE( m_pOldMesh );
    SAFE_RELEASE( m_pNewMesh );
}

void EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh::Do()
{
    m_pComponent->SetMesh( m_pNewMesh );
    m_pComponent->m_GLPrimitiveType = m_NewGLPrimitiveType;
    m_pComponent->m_MeshPrimitiveType = m_NewMeshPrimitiveType;

    m_pComponent->CreatePrimitive();
}

void EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh::Undo()
{
    m_NewGLPrimitiveType = m_pComponent->m_GLPrimitiveType;

    m_pComponent->SetMesh( m_pOldMesh );
    m_pComponent->m_GLPrimitiveType = m_OldGLPrimitiveType;
    m_pComponent->m_MeshPrimitiveType = ComponentMeshPrimitive_ReferenceToAnotherMeshPrimitive;
}

EditorCommand* EditorCommand_ReplaceMeshPrimitiveCopyWithNewMesh::Repeat()
{
    return 0;
}

//====================================================================================================
//====================================================================================================
