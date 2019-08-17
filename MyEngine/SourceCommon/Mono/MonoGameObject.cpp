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

#include "MonoFrameworkClasses.h"
#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "Core/MonoGameState.h"

//============================================================================================================
// GameObject.
//============================================================================================================
MonoObject* Mono_GameObject_GetPosition(GameObject* pGameObject)
{
    Vector3 pos = pGameObject->GetTransform()->GetLocalPosition();
    return Mono_ConstructVec3( pos );
}

void Mono_GameObject_SetPosition(GameObject* pGameObject, float x, float y, float z)
{
    pGameObject->GetTransform()->SetLocalPosition( Vector3( x, y, z ) );
}

void Mono_GameObject_SetLocalTransform(GameObject* pGameObject, char* pMat4)
{
    pGameObject->GetTransform()->SetLocalTransform( fixMat4(pMat4) );
}

//============================================================================================================
// Registration.
//============================================================================================================
void RegisterMonoGameObject(MonoGameState* pMonoState)
{
    // GameObject.
    mono_add_internal_call( "MyEngine.GameObject::GetPosition",       Mono_GameObject_GetPosition );
    mono_add_internal_call( "MyEngine.GameObject::SetPosition",       Mono_GameObject_SetPosition );
    mono_add_internal_call( "MyEngine.GameObject::SetLocalTransform", Mono_GameObject_SetLocalTransform );
}
