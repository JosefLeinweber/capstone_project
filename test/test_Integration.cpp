#include <catch2/catch_test_macros.hpp>
#include "AudioBuffer.h"
#include "NetworkingThread.h"

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

TEST_CASE("AudioBufferFifo Integration Test") {
    // Create an AudioBufferFifo
    AudioBufferFifo<float> audioBufferFifo(2, 1024);

    // Fill the buffer with test data
    juce::AudioBuffer<float> originalBuffer(2, 512);
    fillBuffer(originalBuffer, 0.5f);

    // Write data to the fifo
    audioBufferFifo.wirteToBuffer(originalBuffer);

    // Read data from the fifo
    juce::AudioBuffer<float> readBuffer(2, 512);
    audioBufferFifo.readFromBuffer(readBuffer);

    REQUIRE(readBuffer == originalBuffer);

    // Create Server
    NetworkingClass server(audioBufferFifo, "127.0.0.1", "8001");

    // Create Client
    NetworkingThread client(audioBufferFifo);

    

    // Receive data from the client (assuming NetworkingThread::receiveDataFromClient() function exists)

    // Compare the received data with the original data
    REQUIRE(receivedBuffer.getNumChannels() == originalBuffer.getNumChannels());
    REQUIRE(receivedBuffer.getNumSamples() == originalBuffer.getNumSamples());

    for (int channel = 0; channel < receivedBuffer.getNumChannels(); channel++) {
        for (int i = 0; i < receivedBuffer.getNumSamples(); i++) {
            REQUIRE(receivedBuffer.getSample(channel, i) == originalBuffer.getSample(channel, i));
        }
    }
}