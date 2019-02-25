//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VisualScriptNodes_H__
#define __VisualScriptNodes_H__

#include "MyNode.h"
#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentBase.h"
#include "../../SourceCommon/ComponentSystem/BaseComponents/ComponentVariable.h"
#include "../../SourceCommon/ComponentSystem/Core/GameObject.h"

class ComponentBase;

// Visual Script node types.
class VisualScriptNode_Float;
class VisualScriptNode_Color;
class VisualScriptNode_GameObject;
class VisualScriptNode_Component;
class VisualScriptNode_Add;
class VisualScriptNode_Event_KeyPress;
class VisualScriptNode_Disable_GameObject;

//====================================================================================================
// VisualScriptNodeTypeManager
//====================================================================================================

class VisualScriptNodeTypeManager : public MyNodeTypeManager
{
protected:

public:
    VisualScriptNodeTypeManager();

    virtual MyNodeGraph::MyNode* AddCreateNodeItemsToContextMenu(Vector2 pos, MyNodeGraph* pNodeGraph) override;
    virtual MyNodeGraph::MyNode* CreateNode(const char* typeName, Vector2 pos, MyNodeGraph* pNodeGraph) override;
};

class VisualScriptNode : public MyNodeGraph::MyNode
{
public:
    VisualScriptNode(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, int inputsCount, int outputsCount)
    : MyNode( pNodeGraph, id, name, pos, inputsCount, outputsCount )
    {
    }
};

#define VSNAddVar ComponentBase::AddVariable_Base

//====================================================================================================
// VisualScriptNode_Float
//====================================================================================================

class VisualScriptNode_Float : public VisualScriptNode
{
protected:
    float m_Float;

public:
    VisualScriptNode_Float(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, float value)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_Float = value;
        VSNAddVar( &m_VariablesList, " ", ComponentVariableType_Float, MyOffsetOf( this, &this->m_Float ), false, true, nullptr, nullptr, nullptr, nullptr );
    }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
            MyNode::DrawTitle();
        else
            ImGui::Text( "%s: %0.2f", m_Name, m_Float );
    }
};

//====================================================================================================
// VisualScriptNode_Color
//====================================================================================================

class VisualScriptNode_Color : public VisualScriptNode
{
protected:
    ColorByte m_Color;

public:
    VisualScriptNode_Color(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, const ColorByte& color)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_Color = color;
        VSNAddVar( &m_VariablesList, " ", ComponentVariableType_ColorByte, MyOffsetOf( this, &this->m_Color ), false, true, nullptr, nullptr, nullptr, nullptr );
    }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
        {
            MyNode::DrawTitle();
        }
        else
        {
            ImGui::Text( "%s", m_Name );
            ImGui::SameLine();
            ImVec4 color = *(ImVec4*)&m_Color.AsColorFloat();
            ImGui::ColorButton( "", color, ImGuiColorEditFlags_NoPicker );
        }
    }
};

//====================================================================================================
// VisualScriptNode_GameObject
//====================================================================================================

class VisualScriptNode_GameObject : public VisualScriptNode
{
protected:
    GameObject* m_pGameObject;

public:
    VisualScriptNode_GameObject(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, GameObject* pGameObject)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_pGameObject = pGameObject;
        VSNAddVar( &m_VariablesList, " ", ComponentVariableType_GameObjectPtr, MyOffsetOf( this, &this->m_pGameObject ), false, true, nullptr,
            (CVarFunc_ValueChanged)&VisualScriptNode_GameObject::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_GameObject::OnDrop, nullptr );
    }

    void* OnDrop(ComponentVariable* pVar, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
        {
            m_pGameObject = (GameObject*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }
};

//====================================================================================================
// VisualScriptNode_Component
//====================================================================================================

class VisualScriptNode_Component : public VisualScriptNode
{
protected:
    ComponentBase* m_pComponent;

public:
    VisualScriptNode_Component(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, ComponentBase* pComponent)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_pComponent = pComponent;
        VSNAddVar( &m_VariablesList, " ", ComponentVariableType_ComponentPtr, MyOffsetOf( this, &this->m_pComponent ), false, true, nullptr,
            (CVarFunc_ValueChanged)&VisualScriptNode_Component::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_Component::OnDrop, nullptr );
    }

    void* OnDrop(ComponentVariable* pVar, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
        {
            m_pComponent = (ComponentBase*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }
};

//====================================================================================================
// VisualScriptNode_Add
//====================================================================================================

class VisualScriptNode_Add : public VisualScriptNode
{
protected:
public:
    VisualScriptNode_Add(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos)
    : VisualScriptNode( pNodeGraph, id, name, pos, 2, 1 ) {}

    virtual void DrawContents() override
    {
        MyNode::DrawContents();

        ImGui::Text( "+" );
    }
};

//====================================================================================================
// VisualScriptNode_Event_KeyPress
//====================================================================================================

class VisualScriptNode_Event_KeyPress : public VisualScriptNode
{
protected:
    int m_KeyCode;

public:
    VisualScriptNode_Event_KeyPress(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, int keyCode)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        m_KeyCode = keyCode;
        VSNAddVar( &m_VariablesList, " ", ComponentVariableType_Int, MyOffsetOf( this, &this->m_KeyCode ), false, true, nullptr, nullptr, nullptr, nullptr );

        g_pEventManager->RegisterForEvents( "Keyboard", this, &VisualScriptNode_Event_KeyPress::StaticOnEvent );
    }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
            MyNode::DrawTitle();
        else
            ImGui::Text( "%s: %c", m_Name, m_KeyCode );
    }

    virtual void DrawContents() override
    {
        MyNode::DrawContents();

        ImGui::Text( "Key: %c", m_KeyCode );
    }

    static bool StaticOnEvent(void* pObjectPtr, MyEvent* pEvent) { return ((VisualScriptNode_Event_KeyPress*)pObjectPtr)->OnEvent( pEvent ); }
    bool OnEvent(MyEvent* pEvent)
    {
        MyAssert( pEvent->IsType( "Keyboard" ) );

        int action = pEvent->GetInt( "Action" );
        int keyCode = pEvent->GetInt( "KeyCode" );

        if( action == GCBA_Down && keyCode == m_KeyCode )
        {
            int count = 0;

            MyNode* pNode;
            while( pNode = m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
            {
                pNode->Trigger();
            }
        }

        return false;
    }
};

//====================================================================================================
// VisualScriptNode_Disable_GameObject
//====================================================================================================

class VisualScriptNode_Disable_GameObject : public VisualScriptNode
{
protected:
    GameObject* m_pGameObject;

public:
    VisualScriptNode_Disable_GameObject(MyNodeGraph* pNodeGraph, int id, const char* name, const Vector2& pos, GameObject* pGameObject)
    : VisualScriptNode( pNodeGraph, id, name, pos, 1, 0 )
    {
        m_pGameObject = pGameObject;
        VSNAddVar( &m_VariablesList, " ", ComponentVariableType_GameObjectPtr, MyOffsetOf( this, &this->m_pGameObject ), false, true, nullptr,
            (CVarFunc_ValueChanged)&VisualScriptNode_GameObject::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_GameObject::OnDrop, nullptr );
    }

    virtual void DrawTitle() override
    {
        if( m_Expanded )
        {
            MyNode::DrawTitle();
        }
        else
        {
            if( m_pGameObject )
                ImGui::Text( "%s: %s", m_Name, m_pGameObject->GetName() );
            else
                ImGui::Text( "%s: not set" );
        }
    }

    virtual void Trigger() override
    {
        if( m_pGameObject )
        {
            m_pGameObject->SetEnabled( !m_pGameObject->IsEnabled(), true );
        }
    }

    void* OnDrop(ComponentVariable* pVar, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
        {
            m_pGameObject = (GameObject*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedbyinterface, bool finishedchanging, double oldvalue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }
};

#endif //__VisualScriptNodes_H__
