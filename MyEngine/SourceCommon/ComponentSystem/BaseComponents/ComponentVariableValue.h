//
// Copyright (c) 2017 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __ComponentVariableValue_H__
#define __ComponentVariableValue_H__

#include "ComponentVariable.h"

class ComponentVariable;
class GameObject;
class ComponentBase;
class MyFileObject;
class MaterialDefinition;
class SoundCue;

#if MYFW_EDITOR

class ComponentVariableValue
{
protected:
    ComponentVariableType m_Type; // Just for asserts/debugging.

    ColorByte m_ColorByte;
    Vector2 m_Vector2;
    Vector3 m_Vector3;
    Vector2Int m_Vector2Int;
    Vector3Int m_Vector3Int;
    std::string m_String;

    union
    {
        int32 m_Int;
        uint32 m_UInt;
        bool m_Bool;
        float m_Float;
        GameObject* m_GameObjectPtr;
        ComponentBase* m_ComponentPtr;
        MyFileObject* m_FilePtr;
        MaterialDefinition* m_MaterialPtr;
        TextureDefinition* m_TexturePtr;
        SoundCue* m_SoundCuePtr;
        void* m_VoidPtr;
    };

public:
    ComponentVariableValue();
    ComponentVariableValue(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent);

    void GetValueFromVariable(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent);
    void UpdateComponentAndChildrenWithValue(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent);
    void CopyNonPointerValueIntoVariable(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent);
    void CopyValueIntoVariable(void* pObject, ComponentVariable* pVar, ComponentBase* pObjectAsComponent);

    int32               GetInt()             { MyAssert( m_Type == ComponentVariableType::Int );             return m_Int; }
    int32               GetEnum()            { MyAssert( m_Type == ComponentVariableType::Enum );            return m_Int; }
    uint32              GetUnsignedInt()     { MyAssert( m_Type == ComponentVariableType::UnsignedInt );     return m_UInt; }
    uint32              GetFlags()           { MyAssert( m_Type == ComponentVariableType::Flags );           return m_UInt; }
    //char                GetChar()            { MyAssert( m_Type == ComponentVariableType::Char );            return m_Char; }
    //unsigned char       GetChar()            { MyAssert( m_Type == ComponentVariableType::UnsignedChar );    return m_UChar; }
    bool                GetBool()            { MyAssert( m_Type == ComponentVariableType::Bool );            return m_Bool; }
    float               GetFloat()           { MyAssert( m_Type == ComponentVariableType::Float );           return m_Float; }
    //double              GetDouble()          { MyAssert( m_Type == ComponentVariableType::Double );          return m_Double; }
    //ColorFloat          GetColorFloat()      { MyAssert( m_Type == ComponentVariableType::ColorFloat );      return m_ColorFloat; }
    ColorByte           GetColorByte()       { MyAssert( m_Type == ComponentVariableType::ColorByte );       return m_ColorByte; }
    Vector2             GetVector2()         { MyAssert( m_Type == ComponentVariableType::Vector2 );         return m_Vector2; }
    Vector3             GetVector3()         { MyAssert( m_Type == ComponentVariableType::Vector3 );         return m_Vector3; }
    Vector2Int          GetVector2Int()      { MyAssert( m_Type == ComponentVariableType::Vector2Int );      return m_Vector2Int; }
    Vector3Int          GetVector3Int()      { MyAssert( m_Type == ComponentVariableType::Vector3Int );      return m_Vector3Int; }
    std::string         GetCharArray()       { MyAssert( m_Type == ComponentVariableType::CharArray );       return m_String; }
    std::string         GetString()          { MyAssert( m_Type == ComponentVariableType::String );          return m_String; }
    GameObject*         GetGameObjectPtr()   { MyAssert( m_Type == ComponentVariableType::GameObjectPtr );   return m_GameObjectPtr; }
    MyFileObject*       GetFilePtr()         { MyAssert( m_Type == ComponentVariableType::FilePtr );         return m_FilePtr; }
    ComponentBase*      GetComponentPtr()    { MyAssert( m_Type == ComponentVariableType::ComponentPtr );    return m_ComponentPtr; }
    MaterialDefinition* GetMaterialPtr()     { MyAssert( m_Type == ComponentVariableType::MaterialPtr );     return m_MaterialPtr; }
    TextureDefinition*  GetTexturePtr()      { MyAssert( m_Type == ComponentVariableType::TexturePtr );      return m_TexturePtr; }
    SoundCue*           GetSoundCuePtr()     { MyAssert( m_Type == ComponentVariableType::SoundCuePtr );     return m_SoundCuePtr; }
    void*               GetPointerIndirect() { MyAssert( m_Type == ComponentVariableType::PointerIndirect ); return m_VoidPtr; }
};

#endif //MYFW_EDITOR

#endif //__ComponentVariableValue_H__
