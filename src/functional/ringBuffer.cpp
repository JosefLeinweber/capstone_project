#include "ConnectDAWs/ringBuffer.h"

// Constructor for the RingBuffer class
// bufferSize: the size of the m_buffer
// numChannels: the number of channels in the m_buffer

RingBuffer::RingBuffer(int numChannels, int bufferSize)
    : m_fifo(bufferSize), m_buffer(numChannels, bufferSize)
{
    m_buffer.clear();
};

void RingBuffer::copyFrom(const juce::AudioBuffer<float> &source)
{
    auto writeHandle = m_fifo.write(source.getNumSamples());

    for (int channel = 0; channel < source.getNumChannels(); ++channel)
    {
        if (writeHandle.blockSize1 > 0)
        {
            m_buffer.copyFrom(channel,
                              writeHandle.startIndex1,
                              source,
                              channel,
                              0,
                              writeHandle.blockSize1);
        }
        if (writeHandle.blockSize2 > 0)
        {
            m_buffer.copyFrom(channel,
                              writeHandle.startIndex2,
                              source,
                              channel,
                              writeHandle.blockSize1,
                              writeHandle.blockSize2);
        }
    }
};

void RingBuffer::copyTo(juce::AudioBuffer<float> &destination)
{

    auto readHandle = m_fifo.read(destination.getNumSamples());

    for (int channel = 0; channel < destination.getNumChannels(); ++channel)
    {
        if (readHandle.blockSize1 > 0)
        {
            destination.copyFrom(channel,
                                 0,
                                 m_buffer,
                                 channel,
                                 readHandle.startIndex1,
                                 readHandle.blockSize1);
        }
        if (readHandle.blockSize2 > 0)
        {
            destination.copyFrom(channel,
                                 readHandle.blockSize1,
                                 m_buffer,
                                 channel,
                                 readHandle.startIndex2,
                                 readHandle.blockSize2);
        }
    }
};
