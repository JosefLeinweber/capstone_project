#include "AudioBuffer.h"

    // Constructor for the AudioBufferFIFO class
    // Initialises the FIFO and buffer
    // bufferSize: the size of the buffer
    // numChannels: the number of channels in the buffer

AudioBufferFIFO::AudioBufferFIFO(int numChannels, int bufferSize)
        : fifo(bufferSize), buffer(numChannels, bufferSize)
    {

    };

    // Function to write data to the buffer
void AudioBufferFIFO::writeToBuffer(const juce::AudioBuffer<float>& source)
    {
        int start1, size1, start2, size2;
        fifo.prepareToWrite(source.getNumSamples(), start1, size1, start2, size2);

        if (size1 > 0)
            buffer.copyFrom(0, start1, source, 0, 0, size1);
        if (size2 > 0)
            buffer.copyFrom(0, start2, source, 0, size1, size2);

        fifo.finishedWrite(size1 + size2);
    };

    // Function to read data from the buffer
void AudioBufferFIFO::readFromBuffer(juce::AudioBuffer<float>& destination)
    {
        int start1, size1, start2, size2;
        fifo.prepareToRead(destination.getNumSamples(), start1, size1, start2, size2);

        if (size1 > 0)
            destination.copyFrom(0, 0, buffer, 0, start1, size1);
        if (size2 > 0)
            destination.copyFrom(0, size1, buffer, 0, start2, size2);

        fifo.finishedRead(size1 + size2);
    };
