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

#ifndef PRINTCALIBRATIONGRID_H_
#define PRINTCALIBRATIONGRID_H_

#include "ChessboardImage.h"
#include "ImagePrintPreviewDlg.h"
#include "Message.h"

#include <QtGui/QFileDialog>

namespace
{
    inline void PrintCalibrationGrid( const int rows, const int columns )
    {
        ImagePrintPreviewDlg dialog( ChessboardImage::CreateImage( rows, columns ) );
        const int choice = dialog.exec();

        if ( choice == QDialog::Accepted )
        {
            Message::Show( 0,
                           QObject::tr( "Print Calibration Grid" ),
                           QObject::tr( "Warning - Please specify square size!" ),
                           Message::Severity_Warning );
        }
    }
}

#endif


