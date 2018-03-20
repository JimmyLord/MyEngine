//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

ComponentVariableValue::ComponentVariableValue()
{
    m_Type = ComponentVariableType_NumTypes;
}

ComponentVariableValue::ComponentVariableValue(ComponentBase* pComponent, ComponentVariable* pVar)
{
    GetValueFromVariable( pComponent, pVar );
}

void ComponentVariableValue::GetValueFromVariable(ComponentBase* pComponent, ComponentVariable* pVar)
{
    m_Type = pVar->m_Type;

    void* memoryaddr = ((char*)pComponent + pVar->m_Offset);

    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:            m_Int = *(int*)memoryaddr;                                                  break;
    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:           m_UInt = *(unsigned int*)memoryaddr;                                        break;
    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,
    case ComponentVariableType_Bool:            m_Bool = *(bool*)memoryaddr;                                                break;
    case ComponentVariableType_Float:           m_Float = *(float*)memoryaddr;                                              break;
    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,
    case ComponentVariableType_ColorByte:       m_ColorByte = *(ColorByte*)memoryaddr;                                      break;
    case ComponentVariableType_Vector2:         m_Vector2 = *(Vector2*)memoryaddr;                                          break;
    case ComponentVariableType_Vector3:         m_Vector3 = *(Vector3*)memoryaddr;                                          break;
    case ComponentVariableType_Vector2Int:      m_Vector2Int = *(Vector2Int*)memoryaddr;                                    break;
    case ComponentVariableType_Vector3Int:      m_Vector3Int = *(Vector3Int*)memoryaddr;                                    break;
    case ComponentVariableType_GameObjectPtr:   m_GameObjectPtr = *(GameObject**)memoryaddr;                                break;
    case ComponentVariableType_FilePtr:         m_FilePtr = *(MyFileObject**)memoryaddr;                                    break;
    case ComponentVariableType_ComponentPtr:    m_ComponentPtr = *(ComponentBase**)memoryaddr;                              break;
    case ComponentVariableType_MaterialPtr:     m_MaterialPtr = *(MaterialDefinition**)memoryaddr;                          break;
    case ComponentVariableType_SoundCuePtr:     m_SoundCuePtr = *(SoundCue**)memoryaddr;                                    break;
    case ComponentVariableType_PointerIndirect: m_VoidPtr = (pComponent->*pVar->m_pGetPointerValueCallBackFunc)( pVar );    break;

    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }
}

void ComponentVariableValue::UpdateComponentAndChildrenWithValue(ComponentBase* pComponent, ComponentVariable* pVar)
{
    // If it's not a pointer, set the value directly in the child component
    CopyNonPointerValueIntoVariable( pComponent, pVar );

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
    case ComponentVariableType_SoundCuePtr:     
    case ComponentVariableType_PointerIndirect: numberofcomponents = 1; break;

    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }

#if MYFW_EDITOR
    // Inform component it's value changed.
    for( int i=0; i<numberofcomponents; i++ )
    {
        //pComponent->OnValueChangedVariable( pVar->m_ControlID+i, false, true, 0, false, this );
        pComponent->OnValueChangedVariable( pVar, i, false, true, 0, false, this );
    }
#endif //MYFW_EDITOR
}

void ComponentVariableValue::CopyNonPointerValueIntoVariable(ComponentBase* pComponent, ComponentVariable* pVar)
{
    if( m_Type < ComponentVariableType_FirstPointerType )
    {
        CopyValueIntoVariable( pComponent, pVar );
    }
}

void ComponentVariableValue::CopyValueIntoVariable(ComponentBase* pComponent, ComponentVariable* pVar)
{
    MyAssert( m_Type == pVar->m_Type );

    void* memoryaddr = ((char*)pComponent + pVar->m_Offset);

    switch( pVar->m_Type )
    {
    case ComponentVariableType_Int:
    case ComponentVariableType_Enum:            *(int*)memoryaddr = m_Int;                                                  break;
    case ComponentVariableType_UnsignedInt:
    case ComponentVariableType_Flags:           *(unsigned int*)memoryaddr = m_UInt;                                        break;
    //ComponentVariableType_Char,
    //ComponentVariableType_UnsignedChar,
    case ComponentVariableType_Bool:            *(bool*)memoryaddr = m_Bool;                                                break;
    case ComponentVariableType_Float:           *(float*)memoryaddr = m_Float;                                              break;
    //ComponentVariableType_Double,
    //ComponentVariableType_ColorFloat,
    case ComponentVariableType_ColorByte:       *(ColorByte*)memoryaddr = m_ColorByte;                                      break;
    case ComponentVariableType_Vector2:         *(Vector2*)memoryaddr = m_Vector2;                                          break;
    case ComponentVariableType_Vector3:         *(Vector3*)memoryaddr = m_Vector3;                                          break;
    case ComponentVariableType_Vector2Int:      *(Vector2Int*)memoryaddr = m_Vector2Int;                                    break;
    case ComponentVariableType_Vector3Int:      *(Vector3Int*)memoryaddr = m_Vector3Int;                                    break;
    case ComponentVariableType_GameObjectPtr:   *(GameObject**)memoryaddr = m_GameObjectPtr;                                break;
    case ComponentVariableType_FilePtr:         *(MyFileObject**)memoryaddr = m_FilePtr;                                    break;
    case ComponentVariableType_ComponentPtr:    *(ComponentBase**)memoryaddr = m_ComponentPtr;                              break;
    case ComponentVariableType_MaterialPtr:     *(MaterialDefinition**)memoryaddr = m_MaterialPtr;                          break;
    case ComponentVariableType_SoundCuePtr:     *(SoundCue**)memoryaddr = m_SoundCuePtr;                                    break;
    case ComponentVariableType_PointerIndirect: (pComponent->*pVar->m_pSetPointerValueCallBackFunc)( pVar, m_VoidPtr );     break;

    case ComponentVariableType_NumTypes:
    default:
        MyAssert( false );
        break;
    }
}
