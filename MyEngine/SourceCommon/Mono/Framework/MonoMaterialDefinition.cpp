//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "MyEnginePCH.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"

#include "Mono/MonoGameState.h"
#include "Mono/Framework/MonoMaterialDefinition.h"
#include "Mono/Core/MonoGameObject.h"

//============================================================================================================
// Create a MaterialDefinition object in managed memory and give it the pointer to the unmanaged object.
//============================================================================================================
MonoObject* Mono_ConstructMaterialDefinition(MaterialDefinition* pObject)
{
    MonoClass* pClass = mono_class_from_name( MonoGameState::g_pMonoImage, "MyEngine", "MyMaterial" );
    if( pClass )
    {
        MonoObject* pInstance = mono_object_new( MonoGameState::g_pActiveDomain, pClass );
        mono_runtime_object_init( pInstance );

        MonoClassField* pField = mono_class_get_field_from_name( pClass, "m_pNativeObject" );
        mono_field_set_value( pInstance, pField, &pObject );

        return pInstance;
    }

    return nullptr;
}

//============================================================================================================
// MaterialDefinition methods.
//============================================================================================================
MonoString* GetName(MaterialDefinition* pMaterial)
{
    MonoString* name = mono_string_new( MonoGameState::g_pActiveDomain, pMaterial->GetName() );
    return name;
}

//============================================================================================================
// Registration.
//============================================================================================================
void RegisterMonoMaterialDefinition(MonoGameState* pMonoState)
{
    // MaterialDefinition methods.
    mono_add_internal_call( "MyEngine.MyMaterial::GetName", GetName );
}
