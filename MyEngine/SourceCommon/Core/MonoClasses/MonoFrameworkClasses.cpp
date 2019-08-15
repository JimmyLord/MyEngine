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

#include "ComponentSystem/BaseComponents/ComponentTransform.h"
#include "ComponentSystem/Core/GameObject.h"
#include "Core/EngineCore.h"
#include "Core/MonoGameState.h"

static MonoDomain* g_pActiveDomain;
static MonoImage* g_pMonoImage;

//============================================================================================================
// LOG.
//============================================================================================================
void Mono_LOGInfo(MonoString* monoStr)
{
    char* str = mono_string_to_utf8( monoStr );
    LOGInfo( "MonoLog", "Received string: %s\n", str );
}

void Mono_LOGError(MonoString* monoStr)
{
    char* str = mono_string_to_utf8( monoStr );
    LOGError( "MonoLog", "Received string: %s\n", str );
}

//============================================================================================================
// Temp GameObject stuff to remove.
//============================================================================================================
MonoObject* Mono_GetPosition(GameObject* pGameObject)
{
    MonoClass* pVec3Class = mono_class_from_name( g_pMonoImage, "MyEngine", "vec3" );
    MonoObject* pVec3Instance = mono_object_new( g_pActiveDomain, pVec3Class );

    MonoMethod* pConstructor = mono_class_get_method_from_name( pVec3Class, ".ctor", 3 );
    Vector3 pos = pGameObject->GetTransform()->GetLocalPosition();
    void* args[3];
    args[0] = &pos.x;
    args[1] = &pos.y;
    args[2] = &pos.z;
    mono_runtime_invoke( pConstructor, pVec3Instance, args, nullptr );

    return pVec3Instance;
}

void Mono_SetPosition(GameObject* pGameObject, float x, float y, float z)
{
    pGameObject->GetTransform()->SetLocalPosition( Vector3( x, y, z ) );
}

void Mono_SetLocalTransform(GameObject* pGameObject, char* pMat4)
{
    // HACK: Passing a mat4 from C# gives a pointer 8 bytes earlier than data... this is a bad way to deal with it.
    pGameObject->GetTransform()->SetLocalTransform( (MyMatrix*)(pMat4+8) );
}

//============================================================================================================
// Vector3.
//============================================================================================================
void Mono_vec3_Length(Vector3* pVec3)        { pVec3->Length(); }
void Mono_vec3_LengthSquared(Vector3* pVec3) { pVec3->LengthSquared(); }
void Mono_vec3_Normalize(Vector3* pVec3)     { pVec3->Normalize(); }

//============================================================================================================
// MyMatrix.
//============================================================================================================
void Mono_mat4_SetIdentity(MyMatrix* pMat4) { pMat4->SetIdentity(); }
void Mono_mat4_CreateSRT(MyMatrix* pMat4, Vector3* scale, Vector3* rot, Vector3* pos) { pMat4->CreateSRT( *scale, *rot, *pos ); }

//============================================================================================================
// Registration.
//============================================================================================================
void RegisterMonoFrameworkClasses(MonoGameState* pMonoState)
{
    // Set globals for some of the "C" functions above.
    g_pActiveDomain = pMonoState->GetActiveDomain();
    g_pMonoImage = pMonoState->GetImage();

    // Log.
    mono_add_internal_call( "MyEngine.Log::Info",  Mono_LOGInfo );
    mono_add_internal_call( "MyEngine.Log::Error", Mono_LOGError );

    // Temp.
    mono_add_internal_call( "MyEngine.GameObject::GetPosition", Mono_GetPosition );
    mono_add_internal_call( "MyEngine.GameObject::SetPosition", Mono_SetPosition );
    mono_add_internal_call( "MyEngine.GameObject::SetLocalTransform", Mono_SetLocalTransform );

    // Vector3.
    mono_add_internal_call( "MyEngine.vec3::Length",        Mono_vec3_Length );
    mono_add_internal_call( "MyEngine.vec3::LengthSquared", Mono_vec3_LengthSquared );
    mono_add_internal_call( "MyEngine.vec3::Normalize",     Mono_vec3_Normalize );

    // MyMatrix.
    mono_add_internal_call( "MyEngine.mat4::SetIdentity",   Mono_mat4_SetIdentity );
    mono_add_internal_call( "MyEngine.mat4::CreateSRT",     Mono_mat4_CreateSRT );
}
