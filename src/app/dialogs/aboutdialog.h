/*****************************************************************************
 * Copyright 2018 Haye Hinrichsen, Christoph Wick
 *
 * This file is part of Entropy Piano Tuner.
 *
 * Entropy Piano Tuner is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Entropy Piano Tuner is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Entropy Piano Tuner. If not, see http://www.gnu.org/licenses/.
 *****************************************************************************/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QUrl>
#include <QHBoxLayout>

#include "config/prerequisites.h"

class AboutDialog : public QDialog {
    Q_OBJECT
public:
    AboutDialog(QWidget *parent, QString iconPostfix);

private:
    QHBoxLayout *mTitleBarLayout;

private slots:
    ///////////////////////////////////////////////////////////////////////////////
    /// \brief Slot called by the about dialog when clicked a url.
    /// \param url : The url to handle.
    ///
    ///  If usually will open a web browser if url is a web link.
    ///////////////////////////////////////////////////////////////////////////////
    void onOpenAboutUrl(QUrl url);

    void onOpenAboutLink(QString link);

};

#endif // ABOUTDIALOG_H
