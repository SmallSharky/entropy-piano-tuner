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

//=============================================================================
//                       A simple sine wave synthesizer
//=============================================================================

#include "synthesizer.h"

#include <algorithm>

#include "../system/log.h"
#include "../math/mathtools.h"
#include "../system/eptexception.h"
#include "../system/timer.h"


Sound::Sound() :
    mChannels(0),
    mSampleRate(0),
    mSineWave(),
    mSpectrum(),
    mStereo(0.5),
    mTime(0),
    mWaveForm(),
    mReady(false),
    mHash(0)
{}

void Sound::set (const int channels, const int samplerate, const WaveForm &sinewave,
                 const Spectrum &spectrum, const double stereo, const double time)
{
    std::lock_guard<std::mutex> lock(mMutex);
    mChannels = channels;
    mSampleRate = samplerate;
    mSineWave = sinewave;
    mSpectrum = spectrum;
    mStereo = stereo;
    mTime = time;
    mWaveForm.clear();
    mReady = false;
    mHash = computeHash(spectrum);
    start();
}


Sound::WaveForm Sound::getWaveForm()
{
    std::lock_guard<std::mutex> lock(mMutex);
    return mWaveForm;
}



void Sound::workerFunction()
{
    std::lock_guard<std::mutex> lock(mMutex);
    auto round = [] (double x) { return static_cast<int64_t>(x+0.5); };
    const int64_t SampleLength = round(mSampleRate * mTime);
    const int64_t BufferSize = SampleLength * mChannels;
    const double leftvol  = sqrt(0.7-0.4*mStereo);
    const double rightvol = sqrt(0.3+0.4*mStereo);
    mWaveForm.resize(BufferSize);
    mWaveForm.assign(BufferSize,0);

    double sum=0;
    for (auto &mode : mSpectrum) sum+=mode.second;
    if (sum<=0) return;

    const int64_t SineLength = mSineWave.size();
    for (auto &mode : mSpectrum)
    {
        const double f = mode.first;
        const double volume = pow(mode.second / sum,0.4);

        if (f>24 and f<10000 and volume>0.001)
        {
            const int64_t periods = round((SampleLength * f) / mSampleRate);
            if (mChannels==1)
            {
                const int64_t phase = rand();
                for (int64_t i=0; i<SampleLength; ++i)
                    mWaveForm[i] += volume *
                        mSineWave[((i*periods*SineLength)/SampleLength+phase)%SineLength];
            }
            else if (mChannels==2)
            {
                const int64_t phasediff = round(periods * mSampleRate *
                                                (0.5-mStereo) / 500);
                const int64_t leftphase  = rand();
                const int64_t rightphase = leftphase + phasediff;
                for (int64_t i=0; i<SampleLength; ++i)
                {
                    mWaveForm[2*i] += volume * leftvol *
                        mSineWave[((i*periods*SineLength)/SampleLength+leftphase)%SineLength];
                    mWaveForm[2*i+1] += volume * rightvol *
                        mSineWave[((i*periods*SineLength)/SampleLength+rightphase)%SineLength];
                }
            }
        }
        if (cancelThread()) return;
    }
    mReady = true;
    LogI ("Created waveform");

//    if (id !=0) return;
//    std::ofstream os("0000-waveform.dat");
//    for (int64_t i=0; i<SampleLength; ++i)  os << mWaveForms[0][2*i] << std::endl;
//    os << "&" << std::endl;
//    for (int64_t i=0; i<SampleLength; ++i)  os << mWaveForms[0][2*i+1] << std::endl;
//    os.close();


}



size_t Sound::computeHash (const Spectrum &spectrum)
{
    size_t hash = 0;
    for (auto &f : spectrum) hash ^= std::hash<double>() (f.second);
    return hash;
}


bool Sound::coincidesWith (const Spectrum &spectrum)
{
    return mHash == computeHash(spectrum);
}


//=============================================================================
//                          CLASS SYNTHESIZER
//=============================================================================


//-----------------------------------------------------------------------------
//	                             Constructor
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Constructor, intitializes the member variables.
///
/// \param audioadapter : Pointer to the implementation of the AudioPlayer
///////////////////////////////////////////////////////////////////////////////

Synthesizer::Synthesizer (AudioPlayerAdapter *audioadapter) :
    mSineWave(),
    mScheduler(),
    mAudioPlayer(audioadapter)
{}


//-----------------------------------------------------------------------------
//	                      Initialize and start thread
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Initialize and start the synthesizer.
///
/// This function initializes the synthesizer in that it pre-calculates a sine
/// function and starts the main loop of the synthesizer in an indpendent thread.
///////////////////////////////////////////////////////////////////////////////

void Synthesizer::init ()
{
    if (mAudioPlayer)
    {
        // Pre-calculate a sine wave for speedup
        mSineWave.resize(SineLength);
        for (int i=0; i<SineLength; ++i)
            mSineWave[i]=(float)(sin(MathTools::TWO_PI * i / SineLength));

    }
    else LogW("Could not start synthesizer: AudioPlayer not connected.");
}


//-----------------------------------------------------------------------------
//	                             Shut down
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Stop the synthesizer, request its execution thread to terminate.
///////////////////////////////////////////////////////////////////////////////

void Synthesizer::exit ()
{
    stop();
}


//-----------------------------------------------------------------------------
//	                    Main Loop (worker function)
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Main loop of the synthesizer running in an independent thread.
///////////////////////////////////////////////////////////////////////////////

void Synthesizer::workerFunction (void)
{
    setThreadName("Synthesizer");
    std::cout << "************** Synthi started *************" << std::endl;

    while (not cancelThread())
    {
        mSchedulerMutex.lock();
        bool active = (mAudioPlayer and mScheduler.size()>0);
        mSchedulerMutex.unlock();
        if (active)
        {
            //first remove all sounds with an amplitude below the cutoff:
            mSchedulerMutex.lock();
            for (auto it = mScheduler.begin(); it != mScheduler.end();)
                if (it->stage>=2 and it->amplitude<CutoffVolume)
                { it=mScheduler.erase(it);
                    std::cout << "********* Deleted sound in schedule, size=" << mScheduler.size() << std::endl;
                }
                else ++it;
            mSchedulerMutex.unlock();

            generateAudioSignal();
        }
        else
        {
            msleep(10);
        }
    }
}


//-----------------------------------------------------------------------------
//	                     Create a new sound
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Create a new sound (note).
///
/// This function creates or new (or recreates an existing) sound.
/// \param id : Identifier of the sound (usually the piano key number)
/// \param volume : Overall volume of the sound (intensity of keypress)
/// with typical values between 0 and 1.
/// \param stereo : Stereo position of the sound, ranging from 0 (left) to 1 (right).
/// \param attack : Rate of initial volume increase in units of 1/sec.
/// \param decayrate : Rate of the subsequent volume decrease in units of 1/sec.
/// If this rate is zero the decay phase is omitted and the volume
/// increases directly towards the sustain level controlled by the attack rate.
/// \param sustain : Level at which the volume saturates after decay in (0..1).
/// \param release : Rate at which the sound disappears after release in units of 1/sec.
///////////////////////////////////////////////////////////////////////////////

void Synthesizer::createSound (int id, double volume, double stereo,
        double attack, double decayrate, double sustain, double release)
{
    std::cout << "create sound *********************************** " << std::endl;
    Tone tone;
    tone.id=id;
    tone.amplitude=0;
    tone.clock=0;
    tone.stage=0;
    tone.volume=volume;
    tone.stereo=stereo;
    tone.attack=attack;
    tone.decayrate=decayrate;
    tone.sustain=sustain;
    tone.release=release;
    tone.stage = 1;
    tone.waveform = mSoundCollection[id].getWaveForm();
    mSchedulerMutex.lock();
    mScheduler.push_back(tone);
    mSchedulerMutex.unlock();
}

//-----------------------------------------------------------------------------
//	                        Generate the waveform
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Generate waveform.
///
/// This is the heart of the synthesizer. It fills the circular buffer
/// until it reaches the maximum size. It consists of two parts.
/// First the envelope is computed, rendering the actual amplitude of the
/// sound. Then a loop over all Fourier modes is carried out and a sine
/// wave with the corresponding frequency is added to the buffer.
/// \param snd : Reference of the sound to be converted.
///////////////////////////////////////////////////////////////////////////////

void Synthesizer::generateAudioSignal ()
{
    mSchedulerMutex.lock();
    size_t N = mScheduler.size();
    mSchedulerMutex.unlock();

    // If there is nothing to play return
    if (N==0) return;

    int SampleRate = mAudioPlayer->getSamplingRate();
    int channels = mAudioPlayer->getChannelCount();
    if (channels<=0 or channels>2) return;

    int writtenSamples = 0;

    while (mAudioPlayer->getFreeSize()>=2 and writtenSamples < 10)
    {
        ++writtenSamples;
        double left=0, right=0, mono=0;
        mSchedulerMutex.lock();
        for (auto &ch : mScheduler)
        {
            int id = ch.id;
            double y = ch.amplitude;       // get last amplitude
            switch (ch.stage)          // Manage ADSR
            {
                case 1: // ATTACK
                        y += ch.attack*ch.volume/SampleRate;
                        if (ch.decayrate>0)
                        {
                            if (y >= ch.volume) ch.stage++;
                        }
                        else
                        {
                            if (y >= ch.sustain*ch.volume) ch.stage+=2;
                        }
                        break;
                case 2: // DECAY
                        y *= (1-ch.decayrate/SampleRate); // DECAY
                        if (y <= ch.sustain*ch.volume) ch.stage++;
                        break;
                case 3: // SUSTAIN
                        y += (ch.sustain-y) * ch.release/SampleRate;
                        break;
                case 4: // RELEASE
                        y *= (1-ch.release/SampleRate);
                        break;
            }
            ch.amplitude = y;
            ch.clock ++;

            int size = ch.waveform.size();
            if (size!=0)
            {
                left  += y*ch.waveform[(2*ch.clock)%size];
                right += y*ch.waveform[(2*ch.clock+1)%size];
            }
        }
        mSchedulerMutex.unlock();
        if (channels==1) mAudioPlayer->pushSingleSample(static_cast<AudioBase::PacketDataType>((left+right)/2));
        else // if stereo
        {
            mAudioPlayer->pushSingleSample(static_cast<AudioBase::PacketDataType>(left));
            mAudioPlayer->pushSingleSample(static_cast<AudioBase::PacketDataType>(right));
        }
    }
}


//-----------------------------------------------------------------------------
// 	                      Get sound (private)
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Get a pointer to the sound addressed by a given ID.
///
/// Note that this function has to be mutexed.
///
/// \param id : Identifier of the sound
///////////////////////////////////////////////////////////////////////////////

Synthesizer::Tone* Synthesizer::getSchedulerPointer (int id)
{
    Tone *snd(nullptr);
    mSchedulerMutex.lock();
    for (auto &ch : mScheduler) if (ch.id==id) { snd=&ch; break; }
    mSchedulerMutex.unlock();
    return snd;
}


//-----------------------------------------------------------------------------
// 	                         Terminate a sound
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Terminate a sound.
///
/// This function forces the sound to fade out, entering the release phase.
/// \param id : identity tag of the sound (number of key).
///////////////////////////////////////////////////////////////////////////////

void Synthesizer::releaseSound (int id)
{
    Tone *snd(nullptr);
    bool released=false;
    mSchedulerMutex.lock();
    for (auto &ch : mScheduler) if (ch.id==id) { ch.stage=4; released=true; }
    mSchedulerMutex.unlock();
    if (not released) LogW("Sound #%d does not exist.",id);
}


//-----------------------------------------------------------------------------
// 	             Check whether a certain sound is still active
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Check whether a sound with given id is still playing.
///
/// \param id : Identifier of the sound
/// \return Boolean telling whether the sound is still playing.
///////////////////////////////////////////////////////////////////////////////

bool Synthesizer::isPlaying (int id)
{
    return (getSchedulerPointer(id) != nullptr);
}


//-----------------------------------------------------------------------------
// 	                       Change the sustain level
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
/// \brief Change the level (sustain level) of a constantly playing sound
///
/// This function changes the sustain volume of a constantly playing sound
/// The synthesizer will adjust to the new volume adiabatically with the
/// respective decay rate.
///
/// \param id : Identity tag of the sound (number of key).
/// \param level : New sustain level in (0...1).
///////////////////////////////////////////////////////////////////////////////

void Synthesizer::ModifySustainLevel (int id, double level)
{
    auto snd = getSchedulerPointer(id);
    mSchedulerMutex.lock();
    if (snd) snd->sustain = level;
    else LogW ("id does not exist");
    mSchedulerMutex.unlock();
}



////-----------------------------------------------------------------------------

void Synthesizer::addSound  (const int id, const Spectrum &spectrum,
                             const double stereo, const double time)
{
    Sound &sound = mSoundCollection[id];
    if (sound.coincidesWith(spectrum))
    {
        std::cout << "*********** Not necessary to add sound " << id << std::endl;
        return;
    }
    sound.stop();
    sound.set(mAudioPlayer->getChannelCount(),
              mAudioPlayer->getSamplingRate(),
              mSineWave, spectrum, stereo,time);
    std::cout << "*********** Added sound #  " << id << std::endl;
}




