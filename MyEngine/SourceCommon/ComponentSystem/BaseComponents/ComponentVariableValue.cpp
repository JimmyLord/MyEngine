//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "ComponentBase.h"
#include "ComponentVariableValue.h"

#if MYFW_EDITOR

ComponentVariableValue::ComponentVariableValue()
{
    m_Type = ComponentVariableType::NumTypes;
}

ComponentVariableValue::ComponentVariableValue(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent)
{
    GetValueFromVariable( pObject, pVar, pObjectAsComponent );
}

void ComponentVariableValue::GetValueFromVariable(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent)
{
    m_Type = pVar->m_Type;

    void* memoryAddr = ((char*)pObject + pVar->m_Offset);

    // If this is an indirect pointer type, make sure we have a pointer to a component.
    MyAssert( pVar->m_Type != ComponentVariableType::PointerIndirect || pObjectAsComponent != nullptr );

    switch( pVar->m_Type )
    {
    case ComponentVariableType::Int:
    case ComponentVariableType::Enum:            m_Int = *(int*)memoryAddr;                                                          break;
    case ComponentVariableType::UnsignedInt:
    case ComponentVariableType::Flags:           m_UInt = *(unsigned int*)memoryAddr;                                                break;
    //ComponentVariableType::Char,
    //ComponentVariableType::UnsignedChar,
    case ComponentVariableType::Bool:            m_Bool = *(bool*)memoryAddr;                                                        break;
    case ComponentVariableType::Float:           m_Float = *(float*)memoryAddr;                                                      break;
    //ComponentVariableType::Double,
    //ComponentVariableType::ColorFloat,
    case ComponentVariableType::ColorByte:       m_ColorByte = *(ColorByte*)memoryAddr;                                              break;
    case ComponentVariableType::Vector2:         m_Vector2 = *(Vector2*)memoryAddr;                                                  break;
    case ComponentVariableType::Vector3:         m_Vector3 = *(Vector3*)memoryAddr;                                                  break;
    case ComponentVariableType::Vector2Int:      m_Vector2Int = *(Vector2Int*)memoryAddr;                                            break;
    case ComponentVariableType::Vector3Int:      m_Vector3Int = *(Vector3Int*)memoryAddr;                                            break;
    case ComponentVariableType::String:          m_String = (char*)memoryAddr;                                                      break;
    case ComponentVariableType::GameObjectPtr:   m_GameObjectPtr = *(GameObject**)memoryAddr;                                        break;
    case ComponentVariableType::FilePtr:         m_FilePtr = *(MyFileObject**)memoryAddr;                                            break;
    case ComponentVariableType::ComponentPtr:    m_ComponentPtr = *(ComponentBase**)memoryAddr;                                      break;
    case ComponentVariableType::MaterialPtr:     m_MaterialPtr = *(MaterialDefinition**)memoryAddr;                                  break;
    case ComponentVariableType::TexturePtr:      m_TexturePtr = *(TextureDefinition**)memoryAddr;                                    break;
    case ComponentVariableType::SoundCuePtr:     m_SoundCuePtr = *(SoundCue**)memoryAddr;                                            break;
    case ComponentVariableType::PointerIndirect: m_VoidPtr = (pObjectAsComponent->*pVar->m_pGetPointerValueCallBackFunc)( pVar );    break;

    case ComponentVariableType::NumTypes:
    default:
        MyAssert( false );
        break;
    }
}

void ComponentVariableValue::UpdateComponentAndChildrenWithValue(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent)
{
    // If it's not a pointer, set the value directly in the child component
    CopyNonPointerValueIntoVariable( pObject, pVar, pObjectAsComponent );

    int numberofcomponents = 1;
    switch( pVar->m_Type )
    {
    case ComponentVariableType::Int:
    case ComponentVariableType::Enum:            
    case ComponentVariableType::UnsignedInt:
    case ComponentVariableType::Flags:           
    //ComponentVariableType::Char,
    //ComponentVariableType::UnsignedChar,
    case ComponentVariableType::Bool:            
    case ComponentVariableType::Float:           numberofcomponents = 1; break;
    //ComponentVariableType::Double,
    //ComponentVariableType::ColorFloat,
    case ComponentVariableType::ColorByte:       numberofcomponents = 2; break;
    case ComponentVariableType::Vector2:         numberofcomponents = 2; break;
    case ComponentVariableType::Vector3:         numberofcomponents = 3; break;
    case ComponentVariableType::Vector2Int:      numberofcomponents = 2; break;
    case ComponentVariableType::Vector3Int:      numberofcomponents = 3; break;
    case ComponentVariableType::String:
    case ComponentVariableType::GameObjectPtr:   
    case ComponentVariableType::FilePtr:         
    case ComponentVariableType::ComponentPtr:    
    case ComponentVariableType::MaterialPtr:     
    case ComponentVariableType::TexturePtr:      
    case ComponentVariableType::SoundCuePtr:     
    case ComponentVariableType::PointerIndirect: numberofcomponents = 1; break;

    case ComponentVariableType::NumTypes:
    default:
        MyAssert( false );
        break;
    }

#if MYFW_EDITOR
    // Inform component it's value changed.
    if( pObjectAsComponent )
    {
        for( int i=0; i<numberofcomponents; i++ )
        {
            //pComponent->OnValueChangedVariable( pVar->m_ControlID+i, false, true, 0, false, this );
            pObjectAsComponent->OnValueChangedVariable( pVar, i, false, true, 0, false, this );
        }
    }
#endif //MYFW_EDITOR
}

void ComponentVariableValue::CopyNonPointerValueIntoVariable(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent)
{
    if( m_Type < ComponentVariableType::FirstPointerType )
    {
        CopyValueIntoVariable( pObject, pVar, pObjectAsComponent );
    }
}

void ComponentVariableValue::CopyValueIntoVariable(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent)
{
    MyAssert( m_Type == pVar->m_Type );

    void* memoryAddr = ((char*)pObject + pVar->m_Offset);

    // If this is an indirect pointer type, make sure we have a pointer to a component.
    MyAssert( pVar->m_Type != ComponentVariableType::PointerIndirect || pObjectAsComponent != nullptr );

    switch( pVar->m_Type )
    {
    case ComponentVariableType::Int:
    case ComponentVariableType::Enum:            *(int*)memoryAddr = m_Int;                                                          break;
    case ComponentVariableType::UnsignedInt:
    case ComponentVariableType::Flags:           *(unsigned int*)memoryAddr = m_UInt;                                                break;
    //ComponentVariableType::Char,
    //ComponentVariableType::UnsignedChar,
    case ComponentVariableType::Bool:            *(bool*)memoryAddr = m_Bool;                                                        break;
    case ComponentVariableType::Float:           *(float*)memoryAddr = m_Float;                                                      break;
    //ComponentVariableType::Double,
    //ComponentVariableType::ColorFloat,
    case ComponentVariableType::ColorByte:       *(ColorByte*)memoryAddr = m_ColorByte;                                              break;
    case ComponentVariableType::Vector2:         *(Vector2*)memoryAddr = m_Vector2;                                                  break;
    case ComponentVariableType::Vector3:         *(Vector3*)memoryAddr = m_Vector3;                                                  break;
    case ComponentVariableType::Vector2Int:      *(Vector2Int*)memoryAddr = m_Vector2Int;                                            break;
    case ComponentVariableType::Vector3Int:      *(Vector3Int*)memoryAddr = m_Vector3Int;                                            break;
#pragma warning( push )
#pragma warning( disable : 4996 ) // TODO: ComponentVariableType::String, fix this strcpy... it's completely unsafe.
    case ComponentVariableType::String:          strcpy( (char*)memoryAddr, m_String.c_str() );                                      break;
#pragma warning( pop )
    case ComponentVariableType::GameObjectPtr:   *(GameObject**)memoryAddr = m_GameObjectPtr;                                        break;
    case ComponentVariableType::FilePtr:         *(MyFileObject**)memoryAddr = m_FilePtr;                                            break;
    case ComponentVariableType::ComponentPtr:    *(ComponentBase**)memoryAddr = m_ComponentPtr;                                      break;
    case ComponentVariableType::MaterialPtr:     *(MaterialDefinition**)memoryAddr = m_MaterialPtr;                                  break;
    case ComponentVariableType::TexturePtr:      *(TextureDefinition**)memoryAddr = m_TexturePtr;                                    break;
    case ComponentVariableType::SoundCuePtr:     *(SoundCue**)memoryAddr = m_SoundCuePtr;                                            break;
    case ComponentVariableType::PointerIndirect: (pObjectAsComponent->*pVar->m_pSetPointerValueCallBackFunc)( pVar, m_VoidPtr );     break;

    case ComponentVariableType::NumTypes:
    default:
        MyAssert( false );
        break;
    }
}

#endif //MYFW_EDITOR
