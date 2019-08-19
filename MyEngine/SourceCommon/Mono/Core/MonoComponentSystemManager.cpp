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

#include "ComponentSystem/Core/ComponentSystemManager.h"
#include "Mono/MonoGameState.h"
#include "Mono/Core/MonoComponentSystemManager.h"
#include "Mono/Core/MonoGameObject.h"

//============================================================================================================
// ComponentSystemManager methods.
//============================================================================================================
static MonoObject* CreateGameObject(ComponentSystemManager* pComponentSystemManager, MonoObject* nameMonoStr, int sceneID, bool isFolder, bool hasTransform)
{
    char* nameStr = mono_string_to_utf8( mono_object_to_string( nameMonoStr, nullptr ) );
    GameObject* pGameObject = pComponentSystemManager->EditorLua_CreateGameObject( nameStr, (SceneID)sceneID, isFolder, hasTransform );
    return Mono_ConstructGameObject( pGameObject );
}

//============================================================================================================
// Registration.
//============================================================================================================
void RegisterMonoComponentSystemManager(MonoGameState* pMonoState)
{
    // Set m_pNativeObject in the static C# ComponentSystemManager class.
    {
        MonoClass* pClass = mono_class_from_name( MonoGameState::g_pMonoImage, "MyEngine", "ComponentSystemManager" );
        MonoVTable* pVTable = mono_class_vtable( MonoGameState::g_pActiveDomain, pClass );
        mono_runtime_class_init( pVTable );

        MonoClassField* pField = mono_class_get_field_from_name( pClass, "m_pNativeObject" );
        mono_field_static_set_value( pVTable, pField, &g_pComponentSystemManager );
    }

    // ComponentSystemManager methods.
    mono_add_internal_call( "MyEngine.ComponentSystemManager::CreateGameObject", CreateGameObject );
}
