#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 



class AudioBufferFIFO
{
public:
    AudioBufferFIFO(int numChannels, int bufferSize);

    // Function to write data to the buffer
    void writeToBuffer(const juce::AudioBuffer<float>& source);

    // Function to read data from the buffer
    void readFromBuffer(juce::AudioBuffer<float>& destination);

private:
    juce::AbstractFifo fifo;
    juce::AudioBuffer<float> buffer;
};