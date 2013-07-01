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

#include "AboutDialog.h"

#include "ui_AboutDialog.h"

#include "Version.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>


AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AboutDialog)
{
    m_ui->setupUi(this);
    UpdateAboutText(m_ui->m_aboutTextBox);

    QObject::connect( m_ui->m_okBtn,
                      SIGNAL(clicked()),
                      this,
                      SLOT(accept() ) );
}

AboutDialog::~AboutDialog()
{
    delete m_ui;
}

void AboutDialog::UpdateAboutText(QTextEdit* const aboutTextBox)
{
    QString text(aboutTextBox->toHtml());
    text.replace("%buildDate%", GTS_BUILD_DATE);
    text.replace("%revId%", GTS_GIT_REVID);

    // gts license
    QFile licenseFile(":/gts_license.txt");
    licenseFile.open(QFile::ReadOnly);
    QTextStream gts_licenseStream(&licenseFile);
    QString gts_license(gts_licenseStream.readAll().replace("\n", "<br/>"));
    text.replace("%gts_license%", gts_license);
    aboutTextBox->setHtml(text);

    // open source licenses
    QFile opensourceLicenseFile(":/opensource_licenses.txt");
    opensourceLicenseFile.open(QFile::ReadOnly);
    QTextStream opensourceLicenseStream(&opensourceLicenseFile);
    QString opensource_license(opensourceLicenseStream.readAll().replace("\n", "<br/>"));
    text.replace("%opensource_licenses%", opensource_license);
    aboutTextBox->setHtml(text);

}
