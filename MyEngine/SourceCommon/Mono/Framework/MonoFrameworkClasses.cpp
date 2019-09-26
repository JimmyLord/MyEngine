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
#include "Core/EngineCore.h"
#include "Mono/MonoGameState.h"

//============================================================================================================
// Helpers to create mat4 objects in managed memory.
//============================================================================================================
MonoObject* Mono_ConstructMat4()
{
    MonoClass* pClass = mono_class_from_name( MonoGameState::g_pMonoImage, "MyEngine", "mat4" );
    if( pClass )
    {
        MonoObject* pInstance = mono_object_new( MonoGameState::g_pActiveDomain, pClass );

        MonoMethod* pConstructor = mono_class_get_method_from_name( pClass, ".ctor", 0 );
        mono_runtime_invoke( pConstructor, pInstance, nullptr, nullptr );

        return pInstance;
    }

    return nullptr;
}

//============================================================================================================
// LOG methods.
//============================================================================================================
static void Mono_LOGInfo(MonoString* monoStr)
{
    char* str = mono_string_to_utf8( monoStr );
    LOGInfo( "MonoLog", "Received string: %s\n", str );
}

static void Mono_LOGError(MonoString* monoStr)
{
    char* str = mono_string_to_utf8( monoStr );
    LOGError( "MonoLog", "Received string: %s\n", str );
}

//============================================================================================================
// Vector3 methods.
//============================================================================================================
static float Mono_vec3_Length(Vector3 vec)          { return vec.Length(); }
static float Mono_vec3_LengthSquared(Vector3 vec)   { return vec.LengthSquared(); }
static void Mono_vec3_Normalize(Vector3 vec)        { vec.Normalize(); }

static Vector3 Mono_vec3_OperatorMinus(Vector3 vec, Vector3 o)
{
    return vec.operator-( o );
}

//============================================================================================================
// MyMatrix methods.
//============================================================================================================
static void Mono_mat4_SetIdentity(MyMatrix* pMat4)  { pMat4->SetIdentity(); }

static void Mono_mat4_CreateSRT(MyMatrix* pMat4, Vector3 scale, Vector3 rot, Vector3 pos)
{
    pMat4->CreateSRT( scale, rot, pos );
}

//============================================================================================================
// Registration.
//============================================================================================================
void RegisterMonoFrameworkClasses(MonoGameState* pMonoState)
{
    // Log methods.
    mono_add_internal_call( "MyEngine.Log::Info",  Mono_LOGInfo );
    mono_add_internal_call( "MyEngine.Log::Error", Mono_LOGError );

    // Vector3 methods.
    mono_add_internal_call( "MyEngine.vec3::Length",        Mono_vec3_Length );
    mono_add_internal_call( "MyEngine.vec3::LengthSquared", Mono_vec3_LengthSquared );
    mono_add_internal_call( "MyEngine.vec3::Normalize",     Mono_vec3_Normalize );
    mono_add_internal_call( "MyEngine.vec3::OperatorMinus", Mono_vec3_OperatorMinus );

    // MyMatrix methods.
    mono_add_internal_call( "MyEngine.mat4::SetIdentity",   Mono_mat4_SetIdentity );
    mono_add_internal_call( "MyEngine.mat4::CreateSRT",     Mono_mat4_CreateSRT );
}
