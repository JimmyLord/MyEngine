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

#include "Core/EngineCore.h"
#include "Mono/MonoGameState.h"
#include "Mono/Framework/MonoMaterialDefinition.h"
#include "Mono/Framework/MonoMaterialManager.h"
#include "Mono/Core/MonoGameObject.h"

//============================================================================================================
// MaterialManager methods.
//============================================================================================================
static MonoObject* GetFirstMaterial(MaterialManager* pMaterialManager)
{
    MaterialDefinition* pMaterial = pMaterialManager->GetFirstMaterial();
    return Mono_ConstructMaterialDefinition( pMaterial );
}

//============================================================================================================
// Registration.
//============================================================================================================
void RegisterMonoMaterialManager(MonoGameState* pMonoState)
{
    // Set m_pNativeObject in the static C# MaterialManager class.
    {
        MonoClass* pClass = mono_class_from_name( MonoGameState::g_pMonoImage, "MyEngine", "MaterialManager" );
        if( pClass )
        {
            MonoVTable* pVTable = mono_class_vtable( MonoGameState::g_pActiveDomain, pClass );
            mono_runtime_class_init( pVTable );

            MonoClassField* pField = mono_class_get_field_from_name( pClass, "m_pNativeObject" );
            MaterialManager* pManager = g_pEngineCore->GetManagers()->GetMaterialManager();
            mono_field_static_set_value( pVTable, pField, &pManager );
        }
    }

    // MaterialManager methods.
    mono_add_internal_call( "MyEngine.MaterialManager::GetFirstMaterial", GetFirstMaterial );
}
