#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

class RingBuffer
{
public:
    RingBuffer(int numChannels, int bufferSize);

    // Function to copy data from source buffer to the ring buffer
    void copyFrom(const juce::AudioBuffer<float> &source);

    // Function to copy data from the ring buffer to the destination buffer
    void copyTo(juce::AudioBuffer<float> &destination);

    //TODO: rename to getNumReadyToRead
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
