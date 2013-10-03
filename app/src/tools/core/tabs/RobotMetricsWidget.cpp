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

#include "RobotMetricsWidget.h"

#include "ui_RobotMetricsWidget.h"

#include "MainWindow.h"
#include "RobotMetricsSchema.h"
#include "ImagePrintPreviewDlg.h"
#include "TargetsCollection.h"

RobotMetricsWidget::RobotMetricsWidget( QWidget* parent ) :
    Tool( parent, CreateSchema() ),
    m_ui( new Ui::RobotMetricsWidget )
{
    m_ui->setupUi( this );

    using namespace RobotMetricsSchema;
    AddMapper( dimensionsHeightKey,     m_ui->m_dimensionsHeightSpinBox );
    AddMapper( targetDiagonalCmKey,     m_ui->m_dimensionsTopRadiusSpinBox );
    AddMapper( dimensionsBaseRadiusKey, m_ui->m_dimensionsBaseRadiusSpinBox );

    AddMapper( targetTypeKey, m_ui->m_targetTypeComboBox );

    AddMapper( targetOffsetXKey,  m_ui->m_targetOffsetXSpinBox );
    AddMapper( targetOffsetYKey,  m_ui->m_targetOffsetYSpinBox );
    AddMapper( targetRotationKey, m_ui->m_targetRotationSpinBox );

    AddMapper( brushBarLengthKey,  m_ui->m_brushBarLengthSpinBox );
    AddMapper( brushBarOffsetKey,  m_ui->m_brushBarOffsetSpinBox );

    RegisterCollectionCombo( m_ui->m_targetTypeComboBox, TargetsCollection() );
}

RobotMetricsWidget::~RobotMetricsWidget()
{
    delete m_ui;
}

bool RobotMetricsWidget::IsDataValid() const
{
    if (GetCurrentConfig().IsNull()) return true;

    bool valid = true;

    if ( m_ui->m_dimensionsHeightSpinBox->value() == 0.0 )
    {
        valid = valid && false;
        Tool::HighlightLabel(m_ui->m_dimensionsHeightLabel, true);
    }
    else {
        Tool::HighlightLabel(m_ui->m_dimensionsHeightLabel, false);
    }
    if ( m_ui->m_dimensionsTopRadiusSpinBox->value() == 0.0 )
    {
        valid = valid && false;
        Tool::HighlightLabel(m_ui->m_dimensionsTopRadiusLabel, true);
    }
    else {
        Tool::HighlightLabel(m_ui->m_dimensionsTopRadiusLabel, false);
    }
    if ( m_ui->m_dimensionsBaseRadiusSpinBox->value() == 0.0 )
    {
        valid = valid && false;
        Tool::HighlightLabel(m_ui->m_dimensionsBaseRadiusLabel, true);
    }
    else {
        Tool::HighlightLabel(m_ui->m_dimensionsBaseRadiusLabel, false);
    }

    if ( m_ui->m_targetTypeComboBox->currentText().isEmpty() )
    {
        valid = valid && false;
        Tool::HighlightLabel(m_ui->m_targetTypeLabel, true);
    }
    else {
        Tool::HighlightLabel(m_ui->m_targetTypeLabel, false);
    }
        return valid;
}

bool RobotMetricsWidget::CanClose() const
{
    if ( !IsDataValid() )
        return false;

    return true;
}

const QString RobotMetricsWidget::CannotCloseReason() const
{
    return tr("Please complete all highlighted boxes before leaving this tab.");
}

const WbSchema RobotMetricsWidget::CreateSchema()
{
    using namespace RobotMetricsSchema;

    WbSchema schema( CreateWorkbenchSubSchema( schemaName, tr( "Metrics" ) ) );

    schema.AddKeyGroup( dimensionsGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << dimensionsHeightKey
                                      << dimensionsBaseRadiusKey );

    schema.AddKeyGroup( targetGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << targetOffsetXKey
                                      << targetOffsetYKey
                                      << targetRotationKey
                                      << targetDiagonalCmKey
                                      << targetTypeKey );

    schema.AddKeyGroup( brushBarGroup,
                        WbSchemaElement::Multiplicity::One,
                        KeyNameList() << brushBarLengthKey
                                      << brushBarOffsetKey );

    return schema;
}

const QString RobotMetricsWidget::GetSubSchemaDefaultFileName() const
{
    return "metrics.xml";
}

void RobotMetricsWidget::ReloadCurrentConfigToolSpecific()
{
}
