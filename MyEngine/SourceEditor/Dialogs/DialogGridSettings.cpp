//
// Copyright (c) 2015 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "EngineCommonHeader.h"
#include "DialogGridSettings.h"

BEGIN_EVENT_TABLE(DialogGridSettings, wxDialog)
    EVT_COMMAND(wxID_OK, wxEVT_BUTTON, DialogGridSettings::OnOk)
END_EVENT_TABLE()

DialogGridSettings::DialogGridSettings(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& position, const wxSize& size, long style )
: wxDialog( parent, id, title, position, size, style)
{
    char tempstr[20];
    wxPoint pos( 6, 2 );

    new wxStaticText( this, -1, "Grid step size:", pos );
    pos.y += 20;
    
    new wxStaticText( this, -1, "X", pos );
    pos.x += 10;
    sprintf_s( tempstr, 20, "%0.02f", g_pEngineMainFrame->m_GridSettings.stepsize.x );
    m_GridStepX = new wxTextCtrl( this, -1, tempstr, wxPoint( pos.x, pos.y - 3 ), wxSize(40, 20) );
    pos.x += 50;

    new wxStaticText( this, -1, "Y", pos );
    pos.x += 10;
    sprintf_s( tempstr, 20, "%0.02f", g_pEngineMainFrame->m_GridSettings.stepsize.y );
    m_GridStepY = new wxTextCtrl( this, -1, tempstr, wxPoint( pos.x, pos.y - 3 ), wxSize(40, 20) );
    pos.x += 50;

    new wxStaticText( this, -1, "Z", pos );
    pos.x += 10;
    sprintf_s( tempstr, 20, "%0.02f", g_pEngineMainFrame->m_GridSettings.stepsize.z );
    m_GridStepZ = new wxTextCtrl( this, -1, tempstr, wxPoint( pos.x, pos.y - 3 ), wxSize(40, 20) );
    pos.x += 50;

    pos.x = 6;
    pos.y += size.GetHeight() - 76;
    new wxButton( this, wxID_OK, _("OK"), pos, wxDefaultSize );

    pos.x += 106;
    new wxButton( this, wxID_CANCEL, _("Cancel"), pos, wxDefaultSize );

    //CreateStdDialogButtonSizer( wxOK | wxCANCEL );
}

void DialogGridSettings::OnOk(wxCommandEvent& event)
{
    LOGInfo( LOGTag, "OnOk()\n" );

    double value;

    m_GridStepX->GetValue().ToDouble( &value );
    float x = (float)value;

    m_GridStepY->GetValue().ToDouble( &value );
    float y = (float)value;

    m_GridStepZ->GetValue().ToDouble( &value );
    float z = (float)value;

    g_pEngineMainFrame->m_GridSettings.stepsize.Set( x, y, z );

    event.Skip();
}
