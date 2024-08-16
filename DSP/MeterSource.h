#pragma once

#include "JuceHeader.h"

namespace MyJUCEModules {
    // Things I want in the Meter:
    // - Mono/Stereo compatibility
    // - RMS with configurable window size
    // - Clip indicator

    class MeterSource
    {
    public:
        MeterSource() {
            spec = {44'100.0, 512, 1};
            numSamplesToIntegrate = 512;
            rms = 0.f;
            clip = false;
        }

        ~MeterSource() {
            internalBuffer.clear();
        }

        prepare(juce::dsp::ProcessSpec spec) {
            this->spec = spec;

            numSamplesToIntegrate = static_cast<size_t>(timeMs * spec.sampleRate / 1000.0);
            internalBuffer.setSize((int)spec.numChannels, (int)numSamplesToIntegrate);
            internalBuffer.clear();
        }

        setIntegrationTime(int timeMs) {
            numSamplesToIntegrate = timeMs * spec.sampleRate / 1000;
        }

        measureBlock(const juce::AudioBuffer<float>& buffer) {
            int numSamples = buffer.getNumSamples();
            int numChannels = buffer.getNumChannels();

            jassert(numChannels == internalBuffer.getNumChannels());

            if (numSamples >= internalBuffer.getNumSamples()) {
                for (int channel = 0; channel < numChannels; ++channel) {
                    rmsLevels[channel] = buffer.getRMSLevel(channel, numSamples - numSamplesToIntegrate, numSamplesToIntegrate);
                    clips[channel] = buffer.getMagnitude(channel, 0, numSamples) > 1.0f;
                }
            }
            else {
                for (int channel = 0; channel < numChannels; ++channel) {
                    // Get pointers to the internal buffer and the incoming buffer
                    float* internalData = internalBuffer.getWritePointer(channel);
                    const float* incomingData = buffer.getReadPointer(channel);

                    // Shift existing samples left by numSamples positions
                    std::shift_left(internalData, internalData + numSamplesToIntegrate, numSamples);

                    // Copy the incoming samples to the end of the internal buffer
                    jassert(false);
                    // CHECK ranges here
                    std::copy(incomingData, incomingData + numSamples, internalData + numSamplesToIntegrate);
                    
                    rmsLevels[channel] = rmsBuffer.getRMSLevel(channel, 0, numSamplesToIntegrate);
                    clips[channel] = rmsBuffer.getMagnitude(channel, 0, numSamplesToIntegrate) > 1.0f;

                    // It would be nice to send a message to the GUI thread here if the clip status has changed
                }
            }
        }

        getNumChannels() const {
            return internalBuffer.getNumChannels();
        }

        float getRMS(int channel) const {
            return rmsLevels[channel];
        }

        bool getClip(int channel) const {
            return clips [channel];
        }

    private:
        juce::dsp::ProcessSpec              spec;
        size_t                              numSamplesToIntegrate;
        juce::AudioBuffer<float>            internalBuffer;
        std::vector<float>                  rmsLevels;
        std::vector<bool>                   clips;
    };
}