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

#include "TargetRegistry.h"

#include <cassert>

#include <QtCore/QVariant>

namespace TargetRegistry
{
    const size_t numTrackingTargets = 7;
    const TargetDetails TrackingTarget( const size_t index )
    {
        assert( index < numTrackingTargets );
        static const TargetDetails trackingTargets[ numTrackingTargets ] =
        {
            {
                "Standard1",
                QObject::tr( "Standard Target 1", "TargetRegistry" ),
                ":/target1.png",
                ":/target1-print.png"
            },
            {
                "Standard2",
                QObject::tr( "Standard Target 2", "TargetRegistry" ),
                ":/target2.png",
                ":/target2-print.png"
            },
            {
                "Standard3",
                QObject::tr( "Standard Target 3", "TargetRegistry" ),
                ":/target3.png",
                ":/target3-print.png"
            },
            {
                "Standard4",
                QObject::tr( "Standard Target 4", "TargetRegistry" ),
                ":/target4.png",
                ":/target4-print.png"
            },
            {
                "Standard5",
                QObject::tr( "Standard Target 5", "TargetRegistry" ),
                ":/target5.png",
                ":/target5-print.png"
            },
            {
                "Standard6",
                QObject::tr( "Standard Target 6", "TargetRegistry" ),
                ":/target6.png",
                ":/target6-print.png"
            },
            {
                "Standard7",
                QObject::tr( "Standard Target 7", "TargetRegistry" ),
                ":/target7.png",
                ":/target7-print.png"
            }
        };

        return trackingTargets[ index ];
    }

    const TargetDetails GetTargetById( const QString& targetId )
    {
        for ( size_t i = 0; i < numTrackingTargets; ++i )
        {
            if ( TrackingTarget( i ).id == targetId )
            {
                return TrackingTarget( i );
            }
        }

        assert( !"ID not found" );
        return TargetDetails();
    }

    void FillOutTargetTypeCombo( QComboBox& comboBox )
    {
        comboBox.clear();
        for ( size_t i = 0; i < numTrackingTargets; ++i )
        {
            const TargetDetails target = TrackingTarget( i );
            comboBox.addItem( target.displayName, QVariant( target.id ) );
        }
    }
}



