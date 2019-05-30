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

ComponentVariableValue::ComponentVariableValue()
{
    m_Type = ComponentVariableType_NumTypes;
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
    MyAssert( pVar->m_Type != ComponentVariableType_PointerIndirect || pObjectAsComponent != nullptr );

    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:            m_Int = *(int*)memoryAddr;                                                  break;
    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:           m_UInt = *(unsigned int*)memoryAddr;                                        break;
    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,
    case ComponentVariableType_Bool:            m_Bool = *(bool*)memoryAddr;                                                        break;
    case ComponentVariableType_Float:           m_Float = *(float*)memoryAddr;                                                      break;
    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,
    case ComponentVariableType_ColorByte:       m_ColorByte = *(ColorByte*)memoryAddr;                                              break;
    case ComponentVariableType_Vector2:         m_Vector2 = *(Vector2*)memoryAddr;                                                  break;
    case ComponentVariableType_Vector3:         m_Vector3 = *(Vector3*)memoryAddr;                                                  break;
    case ComponentVariableType_Vector2Int:      m_Vector2Int = *(Vector2Int*)memoryAddr;                                            break;
    case ComponentVariableType_Vector3Int:      m_Vector3Int = *(Vector3Int*)memoryAddr;                                            break;
    case ComponentVariableType_GameObjectPtr:   m_GameObjectPtr = *(GameObject**)memoryAddr;                                        break;
    case ComponentVariableType_FilePtr:         m_FilePtr = *(MyFileObject**)memoryAddr;                                            break;
    case ComponentVariableType_ComponentPtr:    m_ComponentPtr = *(ComponentBase**)memoryAddr;                                      break;
    case ComponentVariableType_MaterialPtr:     m_MaterialPtr = *(MaterialDefinition**)memoryAddr;                                  break;
    case ComponentVariableType_TexturePtr:      m_TexturePtr = *(TextureDefinition**)memoryAddr;                                  break;
    case ComponentVariableType_SoundCuePtr:     m_SoundCuePtr = *(SoundCue**)memoryAddr;                                            break;
    case ComponentVariableType_PointerIndirect: m_VoidPtr = (pObjectAsComponent->*pVar->m_pGetPointerValueCallBackFunc)( pVar );    break;

    case ComponentVariableType_NumTypes:
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
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:            
    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:           
    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,
    case ComponentVariableType_Bool:            
    case ComponentVariableType_Float:           numberofcomponents = 1; break;
    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,
    case ComponentVariableType_ColorByte:       numberofcomponents = 2; break;
    case ComponentVariableType_Vector2:         numberofcomponents = 2; break;
    case ComponentVariableType_Vector3:         numberofcomponents = 3; break;
    case ComponentVariableType_Vector2Int:      numberofcomponents = 2; break;
    case ComponentVariableType_Vector3Int:      numberofcomponents = 3; break;
    case ComponentVariableType_GameObjectPtr:   
    case ComponentVariableType_FilePtr:         
    case ComponentVariableType_ComponentPtr:    
    case ComponentVariableType_MaterialPtr:     
    case ComponentVariableType_TexturePtr:      
    case ComponentVariableType_SoundCuePtr:     
    case ComponentVariableType_PointerIndirect: numberofcomponents = 1; break;

    case ComponentVariableType_NumTypes:
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
    if( m_Type < ComponentVariableType_FirstPointerType )
    {
        CopyValueIntoVariable( pObject, pVar, pObjectAsComponent );
    }
}

void ComponentVariableValue::CopyValueIntoVariable(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent)
{
    MyAssert( m_Type == pVar->m_Type );

    void* memoryAddr = ((char*)pObject + pVar->m_Offset);

    // If this is an indirect pointer type, make sure we have a pointer to a component.
    MyAssert( pVar->m_Type != ComponentVariableType_PointerIndirect || pObjectAsComponent != nullptr );

    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:            *(int*)memoryAddr = m_Int;                                                  break;
    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:           *(unsigned int*)memoryAddr = m_UInt;                                                break;
    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,
    case ComponentVariableType_Bool:            *(bool*)memoryAddr = m_Bool;                                                        break;
    case ComponentVariableType_Float:           *(float*)memoryAddr = m_Float;                                                      break;
    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,
    case ComponentVariableType_ColorByte:       *(ColorByte*)memoryAddr = m_ColorByte;                                              break;
    case ComponentVariableType_Vector2:         *(Vector2*)memoryAddr = m_Vector2;                                                  break;
    case ComponentVariableType_Vector3:         *(Vector3*)memoryAddr = m_Vector3;                                                  break;
    case ComponentVariableType_Vector2Int:      *(Vector2Int*)memoryAddr = m_Vector2Int;                                            break;
    case ComponentVariableType_Vector3Int:      *(Vector3Int*)memoryAddr = m_Vector3Int;                                            break;
    case ComponentVariableType_GameObjectPtr:   *(GameObject**)memoryAddr = m_GameObjectPtr;                                        break;
    case ComponentVariableType_FilePtr:         *(MyFileObject**)memoryAddr = m_FilePtr;                                            break;
    case ComponentVariableType_ComponentPtr:    *(ComponentBase**)memoryAddr = m_ComponentPtr;                                      break;
    case ComponentVariableType_MaterialPtr:     *(MaterialDefinition**)memoryAddr = m_MaterialPtr;                                  break;
    case ComponentVariableType_TexturePtr:      *(TextureDefinition**)memoryAddr = m_TexturePtr;                                  break;
    case ComponentVariableType_SoundCuePtr:     *(SoundCue**)memoryAddr = m_SoundCuePtr;                                            break;
    case ComponentVariableType_PointerIndirect: (pObjectAsComponent->*pVar->m_pSetPointerValueCallBackFunc)( pVar, m_VoidPtr );     break;

    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }
}
