#include <catch2/catch_test_macros.hpp>
#include "AudioBuffer.h"

void fillBuffer(juce::AudioBuffer<float>& buffer, float value) {
  for (int i = 0; i < buffer.getNumSamples(); i++) {
    for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
      buffer.setSample(channel, i, value);
    }
  }
}

void printBuffer(auto& buffer) {
  for (int channel = 0; channel < buffer.getNumChannels(); channel++) {
    std::cout << "Channel " << channel << ": ";
    for (int i = 0; i < buffer.getNumSamples(); i++) {
      std::cout << buffer.getSample(channel, i) << " ";
    }
    std::cout << std::endl;
  }
}


TEST_CASE("AudioBufferFIFO | Constructor") {
  SECTION("Initialize with valid parameters") {
    const int numChannels = 2;
    const int bufferSize = 1024;

    AudioBufferFIFO audioBufferFIFO(numChannels, bufferSize);

    // Assert that the FIFO and buffer are initialized correctly
    REQUIRE(audioBufferFIFO.getTotalSize() == bufferSize);
    REQUIRE(audioBufferFIFO.buffer.getNumChannels() == numChannels);
    REQUIRE(audioBufferFIFO.buffer.getNumSamples() == bufferSize);

  }

  SECTION("Initialize with invalid parameters") {
    const int numChannels = 0;
    const int bufferSize = 0;

    // Assert that constructing with invalid parameters throws an exception
    REQUIRE_THROWS_AS(AudioBufferFIFO(numChannels, bufferSize), std::exception);
  }
}




TEST_CASE("AudioBufferFIFO | Write") {
  const int numChannels = 2;
  const int fifoBufferSize = 1024;
  const int bufferSize = 512;

  AudioBufferFIFO audioBufferFIFO(numChannels, fifoBufferSize);

  SECTION("Write data to buffer") {
    juce::AudioBuffer<float> sourceBuffer(numChannels, bufferSize);
    sourceBuffer.clear();

    // Fill the buffer
    fillBuffer(sourceBuffer, 1.0f);

    // Write data to the buffer
    audioBufferFIFO.writeToBuffer(sourceBuffer);

    bool areBuffersEqual = true;

    for (int channel = 0; channel < numChannels; channel++) {
       for (int i = 0; i < bufferSize; i++){
        if (audioBufferFIFO.buffer.getSample(channel, i) != sourceBuffer.getSample(channel, i)) {
          areBuffersEqual = false;
          // std::cout << "Buffers are not equal at index " << i << std::endl;
          // std::cout << "Expected: " << sourceBuffer.getSample(channel, i) << std::endl;
          // std::cout << "Actual: " << audioBufferFIFO.buffer.getSample(channel, i) << std::endl;
          break;
        }
      }
    }

    std::cout << "Source buffer: " << std::endl;
    std::cout << "Source buffer length: " << sourceBuffer.getNumSamples() << std::endl;
    printBuffer(sourceBuffer);
    std::cout << "Destination buffer: " << std::endl;
    std::cout << "Destination buffer length: " << audioBufferFIFO.buffer.getNumSamples() << std::endl;
    printBuffer(audioBufferFIFO.buffer);

    REQUIRE(areBuffersEqual);


  }
}


TEST_CASE("AudioBufferFIFO | Read") {
  const int numChannels = 2;
  const int fifoBufferSize = 1024;
  const int bufferSize = 512;

  AudioBufferFIFO audioBufferFIFO(numChannels, fifoBufferSize);

  SECTION("Read data from buffer") {
    juce::AudioBuffer<float> destinationBuffer(numChannels, bufferSize);
    destinationBuffer.clear();
    fillBuffer(destinationBuffer, 1.0f);

    juce::AudioBuffer<float> sourceBuffer(numChannels, bufferSize);
    fillBuffer(sourceBuffer, 0.2f);

    // Fill the buffer
    audioBufferFIFO.writeToBuffer(sourceBuffer);



    // Read data from the buffer
    audioBufferFIFO.readFromBuffer(destinationBuffer);

    bool areBuffersEqual = true;

    for (int channel = 0; channel < numChannels; channel++) {
      for (int i = 0; i < bufferSize; i++) {
        if (destinationBuffer.getSample(channel, i) != audioBufferFIFO.buffer.getSample(channel, i)) {
          areBuffersEqual = false;
          break;
        }
      }
    }

    std::cout << "Source buffer: " << std::endl;
    std::cout << "Source buffer length: " << audioBufferFIFO.buffer.getNumSamples() << std::endl;
    printBuffer(audioBufferFIFO.buffer);

    std::cout << "Destination buffer: " << std::endl;
    std::cout << "Destination buffer length: " << destinationBuffer.getNumSamples() << std::endl;
    printBuffer(destinationBuffer);

    REQUIRE(areBuffersEqual);
  }
}
