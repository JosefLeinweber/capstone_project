#include "ConnectDAWs/audioBuffer.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("RingBuffer | Constructor")
{
    SECTION("Initialize with valid parameters")
    {
        const int numChannels = 2;
        const int bufferSize = 1024;

        RingBuffer ringBuffer(numChannels, bufferSize);

        // Assert that the FIFO and buffer are initialized correctly
        REQUIRE(ringBuffer.getTotalSize() == bufferSize);
        REQUIRE(ringBuffer.buffer.getNumChannels() == numChannels);
        REQUIRE(ringBuffer.buffer.getNumSamples() == bufferSize);
    }
}


TEST_CASE("RingBuffer | Write")
{
    const int numChannels = 2;
    const int fifoBufferSize = 20;
    const int bufferSize = 10;

    RingBuffer ringBuffer(numChannels, fifoBufferSize);

    SECTION("Write data to buffer")
    {
        juce::AudioBuffer<float> sourceBuffer(numChannels, bufferSize);
        sourceBuffer.clear();

        // Fill the buffer
        fillBuffer(sourceBuffer, 1.0f);

        // Write data to the buffer
        ringBuffer.read(sourceBuffer);

        bool areBuffersEqual = true;

        for (int channel = 0; channel < numChannels; channel++)
        {
            for (int i = 0; i < bufferSize; i++)
            {
                if (ringBuffer.buffer.getSample(channel, i) !=
                    sourceBuffer.getSample(channel, i))
                {
                    areBuffersEqual = false;
                    // std::cout << "Buffers are not equal at index " << i << std::endl;
                    // std::cout << "Expected: " << sourceBuffer.getSample(channel, i) << std::endl;
                    // std::cout << "Actual: " << ringBuffer.buffer.getSample(channel, i) << std::endl;
                    break;
                }
            }
        }

        std::cout << "Source buffer: " << std::endl;
        std::cout << "Source buffer length: " << sourceBuffer.getNumSamples()
                  << std::endl;
        printBuffer(sourceBuffer);
        std::cout << "Destination buffer: " << std::endl;
        std::cout << "Destination buffer length: "
                  << ringBuffer.buffer.getNumSamples() << std::endl;
        printBuffer(ringBuffer.buffer);

        REQUIRE(areBuffersEqual);
    }
}


TEST_CASE("RingBuffer | Read")
{
    const int numChannels = 2;
    const int fifoBufferSize = 20;
    const int bufferSize = 10;

    RingBuffer ringBuffer(numChannels, fifoBufferSize);

    SECTION("Read data from buffer")
    {
        juce::AudioBuffer<float> destinationBuffer(numChannels, bufferSize);
        destinationBuffer.clear();
        fillBuffer(destinationBuffer, 1.0f);

        juce::AudioBuffer<float> sourceBuffer(numChannels, bufferSize);
        fillBuffer(sourceBuffer, 0.2f);

        // Fill the buffer
        ringBuffer.read(sourceBuffer);


        // Read data from the buffer
        ringBuffer.write(destinationBuffer);

        bool areBuffersEqual = true;

        for (int channel = 0; channel < numChannels; channel++)
        {
            for (int i = 0; i < bufferSize; i++)
            {
                if (destinationBuffer.getSample(channel, i) !=
                    ringBuffer.buffer.getSample(channel, i))
                {
                    areBuffersEqual = false;
                    break;
                }
            }
        }

        std::cout << "Source buffer: " << std::endl;
        std::cout << "Source buffer length: "
                  << ringBuffer.buffer.getNumSamples() << std::endl;
        printBuffer(ringBuffer.buffer);

        std::cout << "Destination buffer: " << std::endl;
        std::cout << "Destination buffer length: "
                  << destinationBuffer.getNumSamples() << std::endl;
        printBuffer(destinationBuffer);

        REQUIRE(areBuffersEqual);
    }
}

TEST_CASE("AudioBuffer | continously read and write to buffer")
{
    const int numChannels = 2;
    const int fifoBufferSize = 20;
    const int bufferSize = 10;

    RingBuffer ringBuffer(numChannels, fifoBufferSize);

    juce::AudioBuffer<float> sourceBuffer(numChannels, bufferSize);
    fillBuffer(sourceBuffer, 0.2f);

    juce::AudioBuffer<float> destinationBuffer(numChannels, bufferSize);
    fillBuffer(destinationBuffer, 1.0f);

    for (int i = 0; i < 10; i++)
    {
        ringBuffer.read(sourceBuffer);
        ringBuffer.write(destinationBuffer);
    }

    bool areBuffersEqual = true;

    for (int channel = 0; channel < numChannels; channel++)
    {
        for (int i = 0; i < bufferSize; i++)
        {
            if (destinationBuffer.getSample(channel, i) !=
                sourceBuffer.getSample(channel, i))
            {
                areBuffersEqual = false;
                break;
            }
        }
    }
    printBuffer(destinationBuffer);
    printBuffer(sourceBuffer);
    printBuffer(ringBuffer.buffer);

    REQUIRE(areBuffersEqual);
}


TEST_CASE("AudioBuffer | getNumReady test")
{
    const int numChannels = 2;
    const int fifoBufferSize = 20;
    const int bufferSize = 10;

    RingBuffer ringBuffer(numChannels, fifoBufferSize);

    juce::AudioBuffer<float> sourceBuffer(numChannels, bufferSize);
    fillBuffer(sourceBuffer, 0.2f);

    REQUIRE(ringBuffer.getNumReady() == 0);

    ringBuffer.read(sourceBuffer);
    REQUIRE(ringBuffer.getNumReady() == bufferSize);
}
