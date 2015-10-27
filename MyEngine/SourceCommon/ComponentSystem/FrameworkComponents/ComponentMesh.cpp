//
// Copyright (c) 2014-2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"

#if MYFW_USING_WX
bool ComponentMesh::m_PanelWatchBlockVisible = true;

static const char* g_MaterialLabels[] = { "Material1", "Material2", "Material3", "Material4" };
#endif

const char* OpenGLPrimitiveTypeStrings[7] =
{
    "Points",
    "Lines",
    "LineLoop",
    "LineStrip",
    "Triangles",
    "TriangleStrip",
    "TriangleFan",
};

// Component Variable List
MYFW_COMPONENT_IMPLEMENT_VARIABLE_LIST( ComponentMesh ); //_VARIABLE_LIST

ComponentMesh::ComponentMesh()
: ComponentRenderable()
{
    MYFW_COMPONENT_VARIABLE_LIST_CONSTRUCTOR(); //_VARIABLE_LIST

    ClassnameSanityCheck();

    m_BaseType = BaseComponentType_Renderable;

    m_pMesh = 0;
    for( int i=0; i<MAX_SUBMESHES; i++ )
        m_MaterialList[i] = 0;

    m_GLPrimitiveType = GL_TRIANGLES;
    m_PointSize = 1;
}

ComponentMesh::~ComponentMesh()
{
    MYFW_COMPONENT_VARIABLE_LIST_DESTRUCTOR(); //_VARIABLE_LIST

    SAFE_RELEASE( m_pMesh );
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
        SAFE_RELEASE( m_MaterialList[i] );
}

void ComponentMesh::RegisterVariables(CPPListHead* pList, ComponentMesh* pThis) //_VARIABLE_LIST
{
    ComponentRenderable::RegisterVariables( pList, pThis );

    for( int i=0; i<MAX_SUBMESHES; i++ )
    {
        // materials are not automatically saved/loaded
        MyAssert( MAX_SUBMESHES == 4 );
        ComponentVariable* pVar = AddVariable( pList, g_MaterialLabels[i], ComponentVariableType_MaterialPtr,
                                               MyOffsetOf( pThis, &pThis->m_MaterialList[i] ), false, true, 
                                               0, (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, (CVarFunc_DropTarget)&ComponentMesh::OnDropMaterial, 0 );

        pVar->AddCallback_ShouldVariableBeAdded( (CVarFunc_ShouldVariableBeAdded)(&ComponentMesh::ShouldVariableBeAddedToWatchPanel) );
    }

    AddVariableEnum( pList, "PrimitiveType", MyOffsetOf( pThis, &pThis->m_GLPrimitiveType ),  true,  true, "Primitive Type", 7, OpenGLPrimitiveTypeStrings, (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, 0, 0 );
    AddVariable( pList, "PointSize", ComponentVariableType_Int, MyOffsetOf( pThis, &pThis->m_PointSize ),  true,  true, "Point Size", (CVarFunc_ValueChanged)&ComponentMesh::OnValueChanged, 0, 0 );
}

void ComponentMesh::Reset()
{
    ComponentRenderable::Reset();

    SAFE_RELEASE( m_pMesh );
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
        SAFE_RELEASE( m_MaterialList[i] );

#if MYFW_USING_WX
    m_pPanelWatchBlockVisible = &m_PanelWatchBlockVisible;
#endif //MYFW_USING_WX
}

void ComponentMesh::LuaRegister(lua_State* luastate)
{
}

#if MYFW_USING_WX
void ComponentMesh::AddToObjectsPanel(wxTreeItemId gameobjectid)
{
    MyAssert( gameobjectid.IsOk() );
    //wxTreeItemId id =
    g_pPanelObjectList->AddObject( this, ComponentMesh::StaticOnLeftClick, ComponentBase::StaticOnRightClick, gameobjectid, "Mesh" );
}

void ComponentMesh::OnLeftClick(unsigned int count, bool clear)
{
    ComponentBase::OnLeftClick( count, clear );
}

void ComponentMesh::FillPropertiesWindow(bool clear, bool addcomponentvariables)
{
    //m_ControlID_ComponentTitleLabel = g_pPanelWatch->AddSpace( "Mesh", this, ComponentBase::StaticOnComponentTitleLabelClicked );
    //MyAssert( m_pMesh );

    if( m_PanelWatchBlockVisible )
    {
        ComponentRenderable::FillPropertiesWindow( clear );

        if( addcomponentvariables )
        {
            FillPropertiesWindowWithVariables(); //_VARIABLE_LIST
        }
    }
}

bool ComponentMesh::ShouldVariableBeAddedToWatchPanel(ComponentVariable* pVar)
{
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        // only show enough material variables for the number of submeshes in the mesh.
        if( pVar->m_Label == g_MaterialLabels[i] )
        {
            if( m_pMesh == 0 || i >= m_pMesh->m_SubmeshList.Count() )
                return false;
        }
    }

    return true;
}

void* ComponentMesh::OnValueChanged(ComponentVariable* pVar, bool finishedchanging, double oldvalue)
{
    void* oldpointer = 0;

    if( finishedchanging )
    {
        if( strncmp( pVar->m_Label, "Material", strlen("Material") ) == 0 )
        {
            int materialthatchanged = -1;
            for( int i=0; i<MAX_SUBMESHES; i++ )
            {
                if( pVar->m_Label == g_MaterialLabels[i] )
                    materialthatchanged = i;
            }

            MyAssert( pVar->m_ControlID != -1 );

            wxString text = g_pPanelWatch->m_pVariables[pVar->m_ControlID].m_Handle_TextCtrl->GetValue();
            if( text == "" || text == "none" )
            {
                g_pPanelWatch->ChangeDescriptionForPointerWithDescription( pVar->m_ControlID, "none" );

                oldpointer = GetMaterial( materialthatchanged );
                SetMaterial( 0, materialthatchanged );
            }
        }

        //if( controlid == m_ControlID_PrimitiveType )
        {
            g_pPanelWatch->m_NeedsRefresh = true;
        }
    }

    return oldpointer;
}

void* ComponentMesh::OnDropMaterial(ComponentVariable* pVar, wxCoord x, wxCoord y)
{
    void* oldvalue = 0;

    if( g_DragAndDropStruct.m_Type == DragAndDropType_MaterialDefinitionPointer )
    {
        int materialthatchanged = -1;
        for( int i=0; i<MAX_SUBMESHES; i++ )
        {
            if( pVar->m_Label == g_MaterialLabels[i] )
                materialthatchanged = i;
        }

        MyAssert( materialthatchanged != -1 );
        if( materialthatchanged != -1 )
        {
            MaterialDefinition* pMaterial = (MaterialDefinition*)g_DragAndDropStruct.m_Value;
            MyAssert( pMaterial );

            oldvalue = GetMaterial( materialthatchanged );
            SetMaterial( pMaterial, materialthatchanged );

            // update the panel so new Material name shows up.
            g_pPanelWatch->m_pVariables[g_DragAndDropStruct.m_ID].m_Description = pMaterial->GetName();
        }
    }

    return oldvalue;
}
#endif //MYFW_USING_WX

cJSON* ComponentMesh::ExportAsJSONObject(bool savesceneid)
{
    cJSON* jComponent = ComponentRenderable::ExportAsJSONObject( savesceneid );

    cJSON* jMaterialArray = cJSON_CreateArray();
    cJSON_AddItemToObject( jComponent, "Materials", jMaterialArray );

    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        MyAssert( m_MaterialList[i] == 0 || m_MaterialList[i]->m_pFile ); // new materials should be saved as files before the state is saved.

        cJSON* jMaterial = 0;
        if( m_MaterialList[i] && m_MaterialList[i]->m_pFile )
            jMaterial = cJSON_CreateString( m_MaterialList[i]->m_pFile->m_FullPath );

        if( jMaterial )
            cJSON_AddItemToArray( jMaterialArray, jMaterial );
    }

    //cJSON_AddNumberToObject( jComponent, "PrimitiveType", m_GLPrimitiveType );
    //cJSON_AddNumberToObject( jComponent, "PointSize", m_PointSize );
    
    // called in ComponentBase::ExportAsJSONObject
    //ExportVariablesToJSON( jComponent ); //_VARIABLE_LIST

    return jComponent;
}

void ComponentMesh::ImportFromJSONObject(cJSON* jComponentMesh, unsigned int sceneid)
{
    ComponentRenderable::ImportFromJSONObject( jComponentMesh, sceneid );

    // TODO: remove this "Material" block, it's for old scenes before I changed to multiple materials.
    cJSON* jMaterial = cJSON_GetObjectItem( jComponentMesh, "Material" );
    if( jMaterial )
    {
        MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMaterial->valuestring );
        if( pMaterial )
        {
            SetMaterial( pMaterial, 0 );
            pMaterial->Release();
        }
    }

    cJSON* jMaterialArray = cJSON_GetObjectItem( jComponentMesh, "Materials" );
    if( jMaterialArray )
    {
        int nummaterials = cJSON_GetArraySize( jMaterialArray );

        //for( int i=0; i<MAX_SUBMESHES; i++ ) { MyAssert( m_MaterialList[i] == 0 ); }

        for( int i=0; i<nummaterials; i++ )
        {
            cJSON* jMaterial = cJSON_GetArrayItem( jMaterialArray, i );
            MaterialDefinition* pMaterial = g_pMaterialManager->LoadMaterial( jMaterial->valuestring );
            if( pMaterial )
            {
                SetMaterial( pMaterial, i );
                pMaterial->Release();
            }
        }
    }

    //cJSONExt_GetInt( jComponentMesh, "PrimitiveType", &m_GLPrimitiveType );
    //cJSONExt_GetInt( jComponentMesh, "PointSize", &m_PointSize );

    ImportVariablesFromJSON( jComponentMesh ); //_VARIABLE_LIST
}

ComponentMesh& ComponentMesh::operator=(const ComponentMesh& other)
{
    MyAssert( &other != this );

    ComponentRenderable::operator=( other );

    if( other.m_pMesh )
        other.m_pMesh->AddRef();
    SAFE_RELEASE( m_pMesh );
    m_pMesh = other.m_pMesh;

    //const ComponentMesh* pOther = &other;
    //MyAssert( other.m_MaterialList.Count() == m_MaterialList.Count() );
    for( unsigned int i=0; i<MAX_SUBMESHES; i++ )
    {
        SetMaterial( other.m_MaterialList[i], i );
    }

    m_GLPrimitiveType = other.m_GLPrimitiveType;
    m_PointSize = other.m_PointSize;

    return *this;
}

void ComponentMesh::RegisterCallbacks()
{
    if( m_Enabled && m_CallbacksRegistered == false )
    {
        m_CallbacksRegistered = true;

        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, Tick );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnSurfaceChanged );
        MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, Draw );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnTouch );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnButtons );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnKeys );
        //MYFW_REGISTER_COMPONENT_CALLBACK( ComponentMesh, OnFileRenamed );
    }
}

void ComponentMesh::UnregisterCallbacks()
{
    if( m_CallbacksRegistered == true )
    {
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( Tick );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnSurfaceChanged );
        MYFW_UNREGISTER_COMPONENT_CALLBACK( Draw );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnTouch );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnButtons );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnKeys );
        //MYFW_UNREGISTER_COMPONENT_CALLBACK( OnFileRenamed );

        m_CallbacksRegistered = false;
    }
}

void ComponentMesh::SetMaterial(MaterialDefinition* pMaterial, int submeshindex)
{
    ComponentRenderable::SetMaterial( pMaterial, submeshindex );

    MyAssert( submeshindex >= 0 && submeshindex < MAX_SUBMESHES );

    if( pMaterial )
        pMaterial->AddRef();
    SAFE_RELEASE( m_MaterialList[submeshindex] );
    m_MaterialList[submeshindex] = pMaterial;
}

void ComponentMesh::DrawCallback(ComponentCamera* pCamera, MyMatrix* pMatViewProj, ShaderGroup* pShaderOverride)
{
    ComponentRenderable::Draw( pMatViewProj, pShaderOverride, 0 );

    if( m_pMesh )
    {
        for( unsigned int i=0; i<m_pMesh->m_SubmeshList.Count(); i++ )
        {
            m_pMesh->SetMaterial( m_MaterialList[i], i );
            m_pMesh->m_SubmeshList[i]->m_PrimitiveType = m_GLPrimitiveType;
            m_pMesh->m_SubmeshList[i]->m_PointSize = m_PointSize;
        }

        m_pMesh->SetTransform( m_pComponentTransform->m_Transform );

        // Find nearest lights.
        MyLight* lights;
        int numlights = g_pLightManager->FindNearestLights( 4, m_pComponentTransform->m_Transform.GetTranslation(), &lights );

        // Find nearest shadow casting light.
        MyMatrix* pShadowVP = 0;
        TextureDefinition* pShadowTex = 0;
        if( g_ActiveShaderPass == ShaderPass_Main )
        {
            GameObject* pObject = g_pComponentSystemManager->FindGameObjectByName( "Shadow Light" );
            if( pObject )
            {
                ComponentBase* pComponent = pObject->GetFirstComponentOfBaseType( BaseComponentType_Camera );
                ComponentCameraShadow* pShadowCam = pComponent->IsA( "CameraShadowComponent" ) ? (ComponentCameraShadow*)pComponent : 0;
                if( pShadowCam )
                {
                    pShadowVP = &pShadowCam->m_matViewProj;
#if 1
                    pShadowTex = pShadowCam->m_pDepthFBO->m_pDepthTexture;
#else
                    pShadowTex = pShadowCam->m_pDepthFBO->m_pColorTexture;
#endif
                }
            }
        }

        Vector3 campos;
#if MYFW_USING_WX
        if( g_pEngineCore->m_EditorMode )
        {
            campos = g_pEngineCore->m_pEditorState->GetEditorCamera()->m_pComponentTransform->GetPosition();
        }
        else
#endif
        {
            ComponentCamera* pCamera = g_pComponentSystemManager->GetFirstCamera();
            if( pCamera )
                campos = g_pComponentSystemManager->GetFirstCamera()->m_pComponentTransform->GetPosition();
            else
                campos.Set( 0, 0, 0 );
        }

        m_pMesh->Draw( pMatViewProj, &campos, lights, numlights, pShadowVP, pShadowTex, 0, pShaderOverride );
    }
}
