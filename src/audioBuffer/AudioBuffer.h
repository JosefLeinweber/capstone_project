#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

class AudioBufferFIFO
{
public:
    AudioBufferFIFO(int numChannels, int bufferSize);

    // Function to write data to the buffer
    void writeToInternalBufferFrom(const juce::AudioBuffer<float> &source);

    // Function to read data from the buffer
    void readFromInternalBufferTo(juce::AudioBuffer<float> &destination);

    int getNumReady() const
    {
        return fifo.getNumReady();
    }

    int getTotalSize() const
    {
        return fifo.getTotalSize();
    }

    juce::AudioBuffer<float> buffer;
    juce::AbstractFifo fifo;

private:
};
