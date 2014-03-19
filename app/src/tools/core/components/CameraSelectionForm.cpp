/*
 * Copyright (C) 2007-2013 Dyson Technology Ltd, all rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CameraSelectionForm.h"

#include "ui_CameraSelectionForm.h"

#include "CameraDescription.h"
#include "CameraSelectionFormContents.h"
#include "Message.h"

#include <iostream>

/** @brief Create a dialog listing connected cameras to select from with a preview.
**/
CameraSelectionForm::CameraSelectionForm( QWidget* const parent ) :
    QDialog       ( parent ),
    m_ui          ( new Ui::CameraSelectionForm ),
    m_contents    ( new CameraSelectionFormContents( this ) )
{
    m_ui->setupUi( this );

    QObject::connect( m_contents,
                      SIGNAL( CameraChosen() ),
                      this,
                      SLOT( accept() ) );
}

CameraSelectionForm::~CameraSelectionForm()
{
    delete m_ui;
}

/** @brief Display the dialog to select a camera.
 *
 *  If the camera list is empty, then display a warning message and close.
 *
 *  @param cameras List of a description of each camera to display.
 *  @param fps     The frame rate to use for the preview image (or as close as possible).
 *
 *  @return A CameraDescription describing the camera selected by the user,
 *  or a an invalid description if the user does not select a camera, there
 *  are no cameras to select from, or the user cancels.
**/
const CameraDescription
CameraSelectionForm::ChooseConnectedCamera( const CameraApi::CameraList& cameras )
{
    CameraDescription camera;

    if ( m_contents->StartUp( cameras ) )
    {
        int success = exec();

        if ( success )
        {
            camera = m_contents->GetChosenCamera();
        }
    }
    else
    {
        Message::Show( parentWidget(),
                       tr( "Camera Selection" ),
                       tr( "No cameras found!" ),
                       Message::Severity_Critical );
    }

    m_contents->Shutdown(); // Shutdown so that it closes the camera
                            // conection BEFORE exiting the dialog!
    return camera;
}
