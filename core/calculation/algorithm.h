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

#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "../system/simplethreadhandler.h"
#include "../system/log.h"
#include "../piano/piano.h"

#include "algorithmfactorydescription.h"

////////////////////////////////////////////////////////////////////////
/// \brief The Algorithm class is a basic abstract class for any
///        algorithm
///
/// It creates a copy of the actual Piano in mPiano. The algorithm is
/// allowed to change this data in any way.
///
////////////////////////////////////////////////////////////////////////

class Algorithm : public SimpleThreadHandler
{
public:
    Algorithm(const Piano &piano, const AlgorithmFactoryDescription &desciption);
    virtual ~Algorithm();

    virtual void workerFunction() override final;

protected:
    virtual void algorithmWorkerFunction() = 0;

    void updateTuningCurve(int keynumber, double frequency);

    bool terminateThread() { return cancelThread(); }

    void  showCalculationProgress   (double fraction);
    void  showCalculationProgress   (const Key &key, double start=0, double range=1);


protected:
    using Keys = Keyboard::Keys;
    using SpectrumType = Key::SpectrumType;
    const size_t NumberOfBins = Key::NumberOfBins;

    Piano mPiano;               ///< Copy of the piano
    const AlgorithmFactoryDescription &mFactoryDescription;

    // useful variables that
    Keyboard& mKeyboard;        ///< Reference to the keyboard
    Keys& mKeys;                ///< Reference to the keys
    const int mNumberOfKeys;    ///< The number of keys
    const int mKeyNumberOfA4;   ///< Number of A4
};

#endif // ALGORITHM_H
