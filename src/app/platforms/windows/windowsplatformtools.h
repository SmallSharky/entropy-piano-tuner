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

#ifndef WINDOWSPLATFORMTOOLS_H
#define WINDOWSPLATFORMTOOLS_H

#ifndef _WIN32
#   error "This file may only be included on windows platforms."
#endif

#include "implementations/platformtools.h"

class WindowsPlatformTools : public PlatformToolsImplementation<WindowsPlatformTools> {
public:
    virtual bool loadStartupFile(const QStringList args) override;
    virtual void disableScreensaver() override;
    virtual void enableScreensaver() override;
    virtual unsigned long long getInstalledPhysicalMemoryInB() const override final;
};


#endif  // WINDOWSPLATFORMTOOLS_H
