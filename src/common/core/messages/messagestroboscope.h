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

//=============================================================================
//             Message sending a complex vector to the stroboscope
//=============================================================================

#ifndef MESSAGESTROBOSCOPE_H
#define MESSAGESTROBOSCOPE_H

#include "core/config.h"

#include "message.h"
#include "core/audio/recorder/stroboscope.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Message sending a complex vector to the stroboscope
///////////////////////////////////////////////////////////////////////////////

class MessageStroboscope : public Message
{
public:
    MessageStroboscope(const Stroboscope::ComplexVector &data);
    ~MessageStroboscope(){};

    const Stroboscope::ComplexVector &getData() const;

private:
    const Stroboscope::ComplexVector data;
};

#endif // MESSAGESTROBOSCOPE_H
