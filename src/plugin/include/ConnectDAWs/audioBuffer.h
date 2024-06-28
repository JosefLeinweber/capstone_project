#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

class AudioBufferFIFO
{
public:
    AudioBufferFIFO(int numChannels, int bufferSize);

    // Function to write data to the buffer
    void writeToInternalBufferFrom(const juce::AudioBuffer<float> &source);

    // Function to read data from the buffer
    void readFromInternalBufferTo(juce::AudioBuffer<float> &destination);

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
