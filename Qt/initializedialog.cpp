/*****************************************************************************
 * Copyright 2015 Haye Hinrichsen, Christoph Wick
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

#include "initializedialog.h"
#include <QApplication>
#include <QStyle>
#include "../core/messages/messagehandler.h"

QtCoreInitialisation::QtCoreInitialisation(QWidget *parent) :
    mParent(parent),
    mInitializeDialog(nullptr) {
}

QtCoreInitialisation::~QtCoreInitialisation() {
    if (mInitializeDialog) {
        delete mInitializeDialog;
        mInitializeDialog = nullptr;
    }
}

void QtCoreInitialisation::updateProgress(CoreStatusTypes type, int percentage) {
    mInitializeDialog->updateProgress(type, percentage);

    // process the message loop
    MessageHandler::getSingleton().process();
    // process the events loop
    QCoreApplication::processEvents();
}


void QtCoreInitialisation::create_impl()
{
    EptAssert(!mInitializeDialog, "init dialog already created");
    mInitializeDialog = new InitializeDialog(mParent);
}


void QtCoreInitialisation::destroy_impl()
{
    EptAssert(mInitializeDialog, "init dialog already destroyed");
    delete mInitializeDialog;
    mInitializeDialog = nullptr;
}


InitializeDialog::InitializeDialog(QWidget *parent) :
    QProgressDialog(QString(), QString(), 0, 100, parent),
    mOnceDrawn(false)
{
    //ui->setupUi(this);
    this->setModal(true);
    this->setWindowTitle(tr("Initializing the core component"));
    setWindowModality(Qt::WindowModal);

    // center on screen
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), parent->geometry()));

    // no cancel button
    setCancelButton(nullptr);

    setMinimumDuration(0);
    setValue(0);

    show();

    // wait maximum of 1sec that this windows is drawn once
    QEventLoop loop;
    while (!mOnceDrawn) {
        loop.processEvents(QEventLoop::AllEvents, 1000);
    }
}

InitializeDialog::~InitializeDialog()
{
}

void InitializeDialog::updateProgress(QtCoreInitialisation::CoreStatusTypes type, int percentage) {
    // update values
   setValue(percentage);
    switch (type) {
    case QtCoreInitialisation::CORE_INIT_START:
        setLabelText(tr("Preparing"));
        break;
    case QtCoreInitialisation::CORE_INIT_PROGRESS:
        setLabelText(tr("Initializing, please wait"));
        break;
    case QtCoreInitialisation::CORE_INIT_MIDI:
        setLabelText(tr("Initializing the midi component"));
        break;
    case QtCoreInitialisation::CORE_INIT_END:
        setLabelText(tr("Finished"));
        break;
    }
}

void InitializeDialog::paintEvent(QPaintEvent *p) {
    QProgressDialog::paintEvent(p);
    mOnceDrawn = true;
}
