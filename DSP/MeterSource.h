#pragma once

#include "JuceHeader.h"
#include <algorithm>

namespace MyJUCEModules {

    class MeterSource
    {
    public:
        MeterSource() {
			integrationTimeMs = 100;
			rmsLevels.resize(1);
            rmsLevels[0] = 3.f;
			clips.resize(1);
            internalBuffer.setSize(1, 512);
        }

        ~MeterSource() {
            internalBuffer.clear();
        }

        void prepare(juce::dsp::ProcessSpec spec) {
            this->spec = spec;

            numSamplesToIntegrate = static_cast<size_t>(integrationTimeMs * spec.sampleRate / 1000.0);
            internalBuffer.setSize((int)spec.numChannels, (int)numSamplesToIntegrate);
            internalBuffer.clear();

			rmsLevels.resize(spec.numChannels);
			clips.resize(spec.numChannels);

            rmsLevels[0] = 0.6f;
        }

        void setIntegrationTime(int timeMs) {
			integrationTimeMs = timeMs;
            numSamplesToIntegrate = integrationTimeMs * spec.sampleRate / 1000;
        }

        void measureBlock(const juce::AudioBuffer<float>& buffer) {
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
                    
                    rmsLevels[channel] = internalBuffer.getRMSLevel(channel, 0, numSamplesToIntegrate);
                    clips[channel] = internalBuffer.getMagnitude(channel, 0, numSamplesToIntegrate) > 1.0f;

                    // It would be nice to send a message to the GUI thread here if the clip status has changed
                }
            }
        }

        int getNumChannels() const {
            return internalBuffer.getNumChannels();
        }

        float getRMS(int channel) const {
            return rmsLevels[channel];
        }

        bool getClip(int channel) const {
            return clips [channel];
        }

    private:
        juce::dsp::ProcessSpec      spec;
		size_t                      integrationTimeMs;
        size_t                      numSamplesToIntegrate;
        juce::AudioBuffer<float>    internalBuffer;
        std::vector<float>          rmsLevels;
        std::vector<bool>           clips;
    };
}