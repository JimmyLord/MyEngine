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
class VisualScriptNode_Debug_VariableTest;
class VisualScriptNode_Value_Float;
class VisualScriptNode_Value_Color;
class VisualScriptNode_Value_GameObject;
class VisualScriptNode_Value_Component;
class VisualScriptNode_MathOp_Add;
class VisualScriptNode_Condition_GreaterEqual;
class VisualScriptNode_Condition_Keyboard;
class VisualScriptNode_Event_Keyboard;
class VisualScriptNode_Disable_GameObject;

//====================================================================================================
// VisualScriptNode
//====================================================================================================

class VisualScriptNode : public MyNodeGraph::MyNode
{
public:
    VisualScriptNode(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos, int inputsCount, int outputsCount)
    : MyNode( pNodeGraph, id, name, pos, inputsCount, outputsCount )
    {
    }

    virtual uint32 EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth) { return 0; }

    virtual float GetValueFloat() { return FLT_MAX; }
};

#define VSNAddVar ComponentBase::AddVariable_Base
#define VSNAddVarEnum ComponentBase::AddVariableEnum_Base
#define VSNAddVarFlags ComponentBase::AddVariableFlags_Base

//====================================================================================================
// VisualScriptNode_Debug_VariableTest
//====================================================================================================
#define HACK_TestVariableTypes 1

#if HACK_TestVariableTypes
static const char* g_pTestFlags[5] =
{
    "Flag1",
    "Flag2",
    "Flag3",
    "Flag4",
    "Flag5",
};

class VisualScriptNode_Debug_VariableTest : public VisualScriptNode
{
protected:
    int m_TestInt;                       //ComponentVariableType::Int
    int m_TestEnum;                      //ComponentVariableType::Enum
    int m_TestFlags;                     //ComponentVariableType::Flags
    unsigned int m_TestUInt;             //ComponentVariableType::UnsignedInt,
                                         ////ComponentVariableType::Char,
                                         ////ComponentVariableType::UnsignedChar,
    bool m_TestBool;                     //ComponentVariableType::Bool,
    float m_TestFloat;                   //ComponentVariableType::Float,
                                         ////ComponentVariableType::Double,
                                         ////ComponentVariableType::ColorFloat,
    ColorByte m_TestColorByte;           //ComponentVariableType::ColorByte,
    Vector2 m_TestVec2;                  //ComponentVariableType::Vector2,
    Vector3 m_TestVec3;                  //ComponentVariableType::Vector3,
    Vector2Int m_TestVec2Int;            //ComponentVariableType::Vector2Int,
    Vector3Int m_TestVec3Int;            //ComponentVariableType::Vector3Int,
                                         ////ComponentVariableType::FirstPointerType,
    GameObject* m_pTestGameObject;       //ComponentVariableType::GameObjectPtr = ComponentVariableType::FirstPointerType,
    ComponentBase* m_pTestComponent;     //ComponentVariableType::ComponentPtr,
    MyFileObject* m_pTestFile;           //ComponentVariableType::FilePtr,
    MaterialDefinition* m_pTestMaterial; //ComponentVariableType::MaterialPtr,
    TextureDefinition* m_pTestTexture;   //ComponentVariableType::TexturePtr,
                                         //ComponentVariableType::SoundCuePtr,
                                         //ComponentVariableType::PointerIndirect,

public:
    VisualScriptNode_Debug_VariableTest(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos)
        : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        SetTooltipStrings( nullptr, nullptr );

        m_TestInt = 0;
        m_TestEnum = 0;
        m_TestFlags = 0;
        m_TestUInt = 0;
        m_TestBool = false;
        m_TestFloat = 0.0f;
        m_TestColorByte = ColorByte::Black();
        m_TestVec2.Set( 0, 0 );
        m_TestVec3.Set( 0, 0, 0 );
        m_TestVec2Int.Set( 0, 0 );
        m_TestVec3Int.Set( 0, 0, 0 );

        m_pTestGameObject = nullptr;
        m_pTestComponent = nullptr;
        m_pTestFile = nullptr;
        m_pTestMaterial = nullptr;
        m_pTestTexture = nullptr;
        
        VSNAddVar( &m_VariablesList, "Int",   ComponentVariableType::Int,           MyOffsetOf( this, &this->m_TestInt ),   false, true, "", nullptr, nullptr, nullptr );
        VSNAddVarEnum(  &m_VariablesList, "Enum",                                  MyOffsetOf( this, &this->m_TestEnum ),  false, true, "", 5, g_pTestFlags, nullptr, nullptr, nullptr );
        VSNAddVarFlags( &m_VariablesList, "Flags",                                 MyOffsetOf( this, &this->m_TestFlags ), false, true, "", 5, g_pTestFlags, nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "UInt",  ComponentVariableType::UnsignedInt,   MyOffsetOf( this, &this->m_TestUInt ),  false, true, "", nullptr, nullptr, nullptr );
        ////ComponentVariableType::Char,
        ////ComponentVariableType::UnsignedChar,
        VSNAddVar( &m_VariablesList, "Bool",  ComponentVariableType::Bool,          MyOffsetOf( this, &this->m_TestBool ),  false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "Float", ComponentVariableType::Float,         MyOffsetOf( this, &this->m_TestFloat ), false, true, "", nullptr, nullptr, nullptr );
        ////ComponentVariableType::Double,
        ////ComponentVariableType::ColorFloat,
        VSNAddVar( &m_VariablesList, "Color", ComponentVariableType::ColorByte,     MyOffsetOf( this, &this->m_TestColorByte ), false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "vec2",  ComponentVariableType::Vector2,       MyOffsetOf( this, &this->m_TestVec2 ),      false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "vec3",  ComponentVariableType::Vector3,       MyOffsetOf( this, &this->m_TestVec3 ),      false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "ivec2", ComponentVariableType::Vector2Int,    MyOffsetOf( this, &this->m_TestVec2Int ),   false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "ivec3", ComponentVariableType::Vector3Int,    MyOffsetOf( this, &this->m_TestVec3Int ),   false, true, "", nullptr, nullptr, nullptr );
        ////ComponentVariableType::FirstPointerType,
        VSNAddVar( &m_VariablesList, "GO",    ComponentVariableType::GameObjectPtr, MyOffsetOf( this, &this->m_pTestGameObject ), false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "Nent",  ComponentVariableType::ComponentPtr,  MyOffsetOf( this, &this->m_pTestComponent ),  false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "File",  ComponentVariableType::FilePtr,       MyOffsetOf( this, &this->m_pTestFile ),       false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "Mat",   ComponentVariableType::MaterialPtr,   MyOffsetOf( this, &this->m_pTestMaterial ),   false, true, "", nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "Tex",   ComponentVariableType::TexturePtr,    MyOffsetOf( this, &this->m_pTestTexture ),    false, true, "", nullptr, nullptr, nullptr );
        //ComponentVariableType::TexturePtr,
        //ComponentVariableType::SoundCuePtr,
        //ComponentVariableType::PointerIndirect,
    }

    ~VisualScriptNode_Debug_VariableTest()
    {
        if( m_pTestFile )
            m_pTestFile->Release();

        if( m_pTestMaterial )
            m_pTestMaterial->Release();

        if( m_pTestTexture )
            m_pTestTexture->Release();
    }

    const char* GetType() { return "Debug_VariableTest"; }
};
#endif //HACK_TestVariableTypes

//====================================================================================================
// VisualScriptNode_Value_Float
//====================================================================================================
static const char* m_VisualScriptNode_Value_Float_OutputLabels[] = { "Value" };

class VisualScriptNode_Value_Float : public VisualScriptNode
{
protected:
    float m_Float;

public:
    VisualScriptNode_Value_Float(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos, float value)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        SetTooltipStrings( nullptr, m_VisualScriptNode_Value_Float_OutputLabels );

        m_Float = value;
        VSNAddVar( &m_VariablesList, "Float", ComponentVariableType::Float, MyOffsetOf( this, &this->m_Float ), true, true, "", nullptr, nullptr, nullptr );
    }

    const char* GetType() { return "Value_Float"; }
    virtual uint32 EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth) override;

    virtual void DrawTitle() override
    {
        if( m_Expanded )
            MyNode::DrawTitle();
        else
            ImGui::Text( "%s: %0.2f", m_Name, m_Float );
    }

    virtual float GetValueFloat() override { return m_Float; }
};

//====================================================================================================
// VisualScriptNode_Value_Color
//====================================================================================================
static const char* m_VisualScriptNode_Value_Color_OutputLabels[] = { "Value" };

class VisualScriptNode_Value_Color : public VisualScriptNode
{
protected:
    ColorByte m_Color;

public:
    VisualScriptNode_Value_Color(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos, const ColorByte& color)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        SetTooltipStrings( nullptr, m_VisualScriptNode_Value_Color_OutputLabels );

        m_Color = color;
        VSNAddVar( &m_VariablesList, "Color", ComponentVariableType::ColorByte, MyOffsetOf( this, &this->m_Color ), true, true, "", nullptr, nullptr, nullptr );
    }

    const char* GetType() { return "Value_Color"; }

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
// VisualScriptNode_Value_GameObject
//====================================================================================================
static const char* m_VisualScriptNode_Value_GameObject_OutputLabels[] = { "Value" };

class VisualScriptNode_Value_GameObject : public VisualScriptNode
{
protected:
    GameObject* m_pGameObject;

public:
    VisualScriptNode_Value_GameObject(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos, GameObject* pGameObject)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        SetTooltipStrings( nullptr, m_VisualScriptNode_Value_GameObject_OutputLabels );

        m_pGameObject = pGameObject;
        VSNAddVar( &m_VariablesList, "GameObject", ComponentVariableType::GameObjectPtr, MyOffsetOf( this, &this->m_pGameObject ), true, true, "",
            (CVarFunc_ValueChanged)&VisualScriptNode_Value_GameObject::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_Value_GameObject::OnDrop, nullptr );
    }

    const char* GetType() { return "Value_GameObject"; }

    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
        {
            m_pGameObject = (GameObject*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }
};

//====================================================================================================
// VisualScriptNode_Value_Component
//====================================================================================================
static const char* m_VisualScriptNode_Value_Component_OutputLabels[] = { "Value" };

class VisualScriptNode_Value_Component : public VisualScriptNode
{
protected:
    ComponentBase* m_pComponent;

public:
    VisualScriptNode_Value_Component(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos, ComponentBase* pComponent)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        SetTooltipStrings( nullptr, m_VisualScriptNode_Value_Component_OutputLabels );

        m_pComponent = pComponent;
        VSNAddVar( &m_VariablesList, "Component", ComponentVariableType::ComponentPtr, MyOffsetOf( this, &this->m_pComponent ), true, true, "",
            (CVarFunc_ValueChanged)&VisualScriptNode_Value_Component::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_Value_Component::OnDrop, nullptr );
    }

    const char* GetType() { return "Value_Component"; }

    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_ComponentPointer )
        {
            m_pComponent = (ComponentBase*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }
};

//====================================================================================================
// VisualScriptNode_MathOp_Add
//====================================================================================================
static const char* m_VisualScriptNode_MathOp_Add_InputLabels[] = { "Value1", "Value2" };
static const char* m_VisualScriptNode_MathOp_Add_OutputLabels[] = { "Sum" };

class VisualScriptNode_MathOp_Add : public VisualScriptNode
{
protected:
public:
    VisualScriptNode_MathOp_Add(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos)
    : VisualScriptNode( pNodeGraph, id, name, pos, 2, 1 )
    {
        SetTooltipStrings( m_VisualScriptNode_MathOp_Add_InputLabels, m_VisualScriptNode_MathOp_Add_OutputLabels );
    }

    const char* GetType() { return "MathOp_Add"; }
    virtual uint32 EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth) override;

    virtual bool DrawContents() override
    {
        bool modified = MyNode::DrawContents();

        ImGui::Text( "+" );

        return modified;
    }

    virtual float GetValueFloat() override
    {
        VisualScriptNode* pNode1 = static_cast<VisualScriptNode*>( m_pNodeGraph->FindNodeConnectedToInput( m_ID, 0 ) );
        VisualScriptNode* pNode2 = static_cast<VisualScriptNode*>( m_pNodeGraph->FindNodeConnectedToInput( m_ID, 1 ) );

        if( pNode1 == nullptr || pNode2 == nullptr )
            return FLT_MAX;

        float value1 = pNode1->GetValueFloat();
        float value2 = pNode2->GetValueFloat();

        return value1 + value2;
    }
};

//====================================================================================================
// VisualScriptNode_Condition_GreaterEqual
//====================================================================================================
static const char* m_VisualScriptNode_Condition_GreaterEqual_InputLabels[] = { "Trigger", "Value1", "Value2" };
static const char* m_VisualScriptNode_Condition_GreaterEqual_OutputLabels[] = { "If Greater or Equal", "If Less" };

class VisualScriptNode_Condition_GreaterEqual : public VisualScriptNode
{
protected:
public:
    VisualScriptNode_Condition_GreaterEqual(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos)
    : VisualScriptNode( pNodeGraph, id, name, pos, 3, 2 )
    {
        SetTooltipStrings( m_VisualScriptNode_Condition_GreaterEqual_InputLabels, m_VisualScriptNode_Condition_GreaterEqual_OutputLabels );
    }

    const char* GetType() { return "Condition_GreaterEqual"; }
    virtual uint32 EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth) override;

    virtual bool DrawContents() override
    {
        bool modified = MyNode::DrawContents();

        ImGui::Text( ">=" );

        return modified;
    }

    virtual bool Trigger(MyEvent* pEvent) override
    {
        VisualScriptNode* pNode1 = static_cast<VisualScriptNode*>( m_pNodeGraph->FindNodeConnectedToInput( m_ID, 1 ) );
        VisualScriptNode* pNode2 = static_cast<VisualScriptNode*>( m_pNodeGraph->FindNodeConnectedToInput( m_ID, 2 ) );

        if( pNode1 == nullptr || pNode2 == nullptr )
            return false;

        float value1 = pNode1->GetValueFloat();
        float value2 = pNode2->GetValueFloat();

        if( value1 >= value2 )
        {
            int count = 0;
            while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
            {
                pNode->Trigger();
            }
        }
        else
        {
            int count = 0;
            while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 1, count++ ) )
            {
                pNode->Trigger();
            }
        }

        return false;
    }
};

//====================================================================================================
// VisualScriptNode_Condition_Keyboard
//====================================================================================================
static const char* m_VisualScriptNode_Condition_Keyboard_InputLabels[] = { "Keyboard Event" };
static const char* m_VisualScriptNode_Condition_Keyboard_OutputLabels[] = { "Trigger" };

class VisualScriptNode_Condition_Keyboard : public VisualScriptNode
{
protected:
    int m_ButtonAction;
    int m_KeyCode;

public:
    VisualScriptNode_Condition_Keyboard(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos, int buttonAction, int keyCode)
    : VisualScriptNode( pNodeGraph, id, name, pos, 1, 1 )
    {
        SetTooltipStrings( m_VisualScriptNode_Condition_Keyboard_InputLabels, m_VisualScriptNode_Condition_Keyboard_OutputLabels );

        MyAssert( GCBA_Down == 0 );
        MyAssert( GCBA_Up == 1 );
        MyAssert( GCBA_Held == 2 );

        m_ButtonAction = buttonAction;
        m_KeyCode = keyCode;

        VSNAddVarEnum( &m_VariablesList, "Action", MyOffsetOf( this, &this->m_ButtonAction ), true, true, "", 3, g_GameCoreButtonActionStrings, nullptr, nullptr, nullptr );
        VSNAddVar( &m_VariablesList, "KeyCode", ComponentVariableType::Int, MyOffsetOf( this, &this->m_KeyCode ), true, true, "", nullptr, nullptr, nullptr );
    }

    const char* GetType() { return "Condition_Keyboard"; }
    virtual uint32 EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth) override;

    virtual void DrawTitle() override
    {
        if( m_Expanded )
            MyNode::DrawTitle();
        else
            ImGui::Text( "%s: %c %s", m_Name, m_KeyCode, g_GameCoreButtonActionStrings[m_ButtonAction] );
    }

    virtual bool DrawContents() override
    {
        bool modified = MyNode::DrawContents();

        ImGui::Text( "Key: %c", m_KeyCode );

        return modified;
    }

    virtual bool Trigger(MyEvent* pEvent) override
    {
        MyAssert( pEvent->IsType( "Keyboard" ) );

        int action = pEvent->GetInt( "Action" );
        int keyCode = pEvent->GetInt( "KeyCode" );

        if( action == m_ButtonAction && keyCode == m_KeyCode )
        {
            int count = 0;
            while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
            {
                if( pNode->Trigger() == true )
                    return true;
            }

            return true;
        }

        return false;
    }
};

//====================================================================================================
// VisualScriptNode_Event_Keyboard
//====================================================================================================
static const char* m_VisualScriptNode_Event_Keyboard_OutputLabels[] = { "Trigger" };

class VisualScriptNode_Event_Keyboard : public VisualScriptNode
{
protected:
    EventManager* m_pEventManager;

public:
    VisualScriptNode_Event_Keyboard(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos, EventManager* pEventManager)
    : VisualScriptNode( pNodeGraph, id, name, pos, 0, 1 )
    {
        SetTooltipStrings( nullptr, m_VisualScriptNode_Event_Keyboard_OutputLabels );

        m_pEventManager = pEventManager;

        // Don't allow node graph to be triggered directly.
        // This will now get triggered through lua script when attached to an object.
        m_pEventManager->RegisterForEvents( "Keyboard", this, &VisualScriptNode_Event_Keyboard::StaticOnEvent );
    }

    ~VisualScriptNode_Event_Keyboard()
    {
        m_pEventManager->UnregisterForEvents( "Keyboard", this, &VisualScriptNode_Event_Keyboard::StaticOnEvent );
    }

    const char* GetType() { return "Event_Keyboard"; }
    virtual uint32 ExportAsLuaString(char* string, uint32 offset, uint32 bytesAllocated) override;

    virtual void DrawTitle() override
    {
        MyNode::DrawTitle();
    }

    virtual bool DrawContents() override
    {
        return MyNode::DrawContents();
    }

    static bool StaticOnEvent(void* pObjectPtr, MyEvent* pEvent) { return ((VisualScriptNode_Event_Keyboard*)pObjectPtr)->OnEvent( pEvent ); }
    bool OnEvent(MyEvent* pEvent)
    {
        MyAssert( pEvent->IsType( "Keyboard" ) );

        int count = 0;
        while( VisualScriptNode* pNode = (VisualScriptNode*)m_pNodeGraph->FindNodeConnectedToOutput( m_ID, 0, count++ ) )
        {
            if( pNode->Trigger( pEvent ) == true )
                return true;
        }

        return false;
    }
};

//====================================================================================================
// VisualScriptNode_Disable_GameObject
//====================================================================================================
static const char* m_VisualScriptNode_Disable_GameObject_InputLabels[] = { "Trigger" };

class VisualScriptNode_Disable_GameObject : public VisualScriptNode
{
protected:
    GameObject* m_pGameObject;

public:
    VisualScriptNode_Disable_GameObject(MyNodeGraph* pNodeGraph, MyNodeGraph::NodeID id, const char* name, const Vector2& pos, GameObject* pGameObject)
    : VisualScriptNode( pNodeGraph, id, name, pos, 1, 0 )
    {
        m_pGameObject = pGameObject;
        VSNAddVar( &m_VariablesList, "GameObject", ComponentVariableType::GameObjectPtr, MyOffsetOf( this, &this->m_pGameObject ), true, true, "",
            (CVarFunc_ValueChanged)&VisualScriptNode_Disable_GameObject::OnValueChanged,
            (CVarFunc_DropTarget)&VisualScriptNode_Disable_GameObject::OnDrop, nullptr );

        m_InputTooltips = m_VisualScriptNode_Disable_GameObject_InputLabels;
        m_OutputTooltips = nullptr;
    }

    const char* GetType() { return "Disable_GameObject"; }
    virtual uint32 ExportAsLuaVariablesString(char* string, uint32 offset, uint32 bytesAllocated) override;
    virtual uint32 EmitLua(char* string, uint32 offset, uint32 bytesAllocated, uint32 tabDepth) override;

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

    void* OnDrop(ComponentVariable* pVar, bool changedByInterface, int x, int y)
    {
        DragAndDropItem* pDropItem = g_DragAndDropStruct.GetItem( 0 );

        if( pDropItem->m_Type == DragAndDropType_GameObjectPointer )
        {
            m_pGameObject = (GameObject*)pDropItem->m_Value;
        }

        return nullptr;
    }

    void* OnValueChanged(ComponentVariable* pVar, bool changedByInterface, bool finishedChanging, double oldValue, ComponentVariableValue* pNewValue)
    {
        return nullptr;
    }

    virtual bool Trigger(MyEvent* pEvent) override
    {
        if( m_pGameObject )
        {
            m_pGameObject->SetEnabled( !m_pGameObject->IsEnabled(), true );
        }

        return false;
    }
};

#endif //__VisualScriptNodes_H__
