#include "ConnectDAWs/audioBuffer.h"

// Constructor for the RingBuffer class
// Initialises the FIFO and buffer
// bufferSize: the size of the buffer
// numChannels: the number of channels in the buffer

RingBuffer::RingBuffer(int numChannels, int bufferSize)
    : fifo(bufferSize), buffer(numChannels, bufferSize)
{
    buffer.clear();
};

// Function to write data to the buffer
void RingBuffer::read(const juce::AudioBuffer<float> &source)
{
    auto writeHandle = fifo.write(source.getNumSamples());

    for (int channel = 0; channel < source.getNumChannels(); ++channel)
    {
        if (writeHandle.blockSize1 > 0)
        {
            buffer.copyFrom(channel,
                            writeHandle.startIndex1,
                            source,
                            channel,
                            0,
                            writeHandle.blockSize1);
        }
        if (writeHandle.blockSize2 > 0)
        {
            buffer.copyFrom(channel,
                            writeHandle.startIndex2,
                            source,
                            channel,
                            writeHandle.blockSize1,
                            writeHandle.blockSize2);
        }
    }
};

// Function to read data from the buffer
void RingBuffer::write(juce::AudioBuffer<float> &destination)
{

    auto readHandle = fifo.read(destination.getNumSamples());

    for (int channel = 0; channel < destination.getNumChannels(); ++channel)
    {
        if (readHandle.blockSize1 > 0)
        {
            destination.copyFrom(channel,
                                 0,
                                 buffer,
                                 channel,
                                 readHandle.startIndex1,
                                 readHandle.blockSize1);
        }
        if (readHandle.blockSize2 > 0)
        {
            destination.copyFrom(channel,
                                 readHandle.blockSize1,
                                 buffer,
                                 channel,
                                 readHandle.startIndex2,
                                 readHandle.blockSize2);
        }
    }
};
