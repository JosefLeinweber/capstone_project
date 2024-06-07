#include <catch2/catch_test_macros.hpp>
#include "AudioBuffer.h"
#include "Server.h"
#include <thread>


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

TEST_CASE("Server | back & forth"){
  std::cout << "Starting " << std::endl;
  
  auto thread_1 = std::jthread([] () {
    std::cout << "Thread 1" << std::endl;
    Server server("127.0.0.1", 8001);
    std::cout << "Thread 1 Server created" << std::endl;
    server.waitForConnection();
    std::cout << "Thread 1 Connected" << std::endl;
    
    juce::AudioBuffer<float> originalBuffer(2, 10);
    fillBuffer(originalBuffer, 1.022f);
    server.sendTo(originalBuffer);
    std::cout << "Thread 1 Sent buffer" << std::endl;

    juce::AudioBuffer<float> recievedBuffer(2, 10);
    recievedBuffer.clear();
    server.recieveFrom(recievedBuffer);
    std::cout << "Thread 1 Recieved buffer" << std::endl;

    printBuffer(originalBuffer);
    printBuffer(recievedBuffer);
    REQUIRE(originalBuffer == recievedBuffer);
  });

  auto thread_2 = std::jthread([] () {
    std::cout << "Thread 2" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Server client("127.0.0.1", 8010);
    std::cout << "Thread 2 Sending connection request" << std::endl;
    client.inititalizeConnection("127.0.0.1", 8001);
    juce::AudioBuffer<float> recievedBuffer(2, 10);
    std::cout << "Thread 2 Waiting for buffer" << std::endl;
    client.recieveFrom(recievedBuffer);
    std::cout << "Thread 2 Recieved buffer" << std::endl;


    client.sendTo(recievedBuffer);
    std::cout << "Thread 2 Sent buffer back" << std::endl;
  });

  thread_1.join();
  thread_2.join();
  std::cout << "Threads joined" << std::endl;

}


TEST_CASE("Server & AudioBuffer | back & forth with abstractFIFO write and read on each side") {
  std::cout << "Starting " << std::endl;
  
  auto thread_1 = std::jthread([] () {
    std::cout << "Thread 1" << std::endl;
    Server server("127.0.0.1", 8001);
    AudioBufferFIFO audioBufferFifo(2, 20);
    std::cout << "Thread 1 Server created" << std::endl;
    server.waitForConnection();
    std::cout << "Thread 1 Connected" << std::endl;
    
    juce::AudioBuffer<float> originalBuffer(2, 10);
    fillBuffer(originalBuffer, 1.022f);

    audioBufferFifo.writeToBuffer(originalBuffer);
    juce::AudioBuffer<float> bufferReadFromFifo(2, 10);
    audioBufferFifo.readFromBuffer(bufferReadFromFifo);

    server.sendTo(bufferReadFromFifo);
    std::cout << "Thread 1 Sent buffer" << std::endl;

    juce::AudioBuffer<float> recievedBuffer(2, 10);
    recievedBuffer.clear();
    server.recieveFrom(recievedBuffer);
    std::cout << "Thread 1 Recieved buffer" << std::endl;

    printBuffer(originalBuffer);
    printBuffer(recievedBuffer);
    REQUIRE(originalBuffer == recievedBuffer);
  });

  auto thread_2 = std::jthread([] () {
    std::cout << "Thread 2" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    Server client("127.0.0.1", 8010);
    AudioBufferFIFO audioBufferFifo2(2, 20);
    std::cout << "Thread 2 Sending connection request" << std::endl;
    client.inititalizeConnection("127.0.0.1", 8001);
    juce::AudioBuffer<float> recievedBuffer(2, 10);
    std::cout << "Thread 2 Waiting for buffer" << std::endl;
    client.recieveFrom(recievedBuffer);
    std::cout << "Thread 2 Recieved buffer" << std::endl;

    audioBufferFifo2.writeToBuffer(recievedBuffer);
    juce::AudioBuffer<float> bufferReadFromFifo(2, 10);
    audioBufferFifo2.readFromBuffer(bufferReadFromFifo);

    client.sendTo(bufferReadFromFifo);
    std::cout << "Thread 2 Sent buffer back" << std::endl;
  });

  thread_1.join();
  thread_2.join();
  std::cout << "Threads joined" << std::endl;

}