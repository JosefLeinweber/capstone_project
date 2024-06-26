#include "ConnectDAWs/audioBuffer.h"
#include "ConnectDAWs/udpHost.h"
#include <catch2/catch_test_macros.hpp>
#include <thread>


TEST_CASE("Host & Host| back & forth")
{
    std::cout << "Starting " << std::endl;

    auto thread_1 = std::jthread([]() {
        std::cout << "Thread 1" << std::endl;
        addressData hostAddress("127.0.0.1", 8001);
        Host host(hostAddress);
        host.setupSocket();
        std::cout << "Thread 1 Host created" << std::endl;
        host.waitForHandshake();
        std::cout << "Thread 1 Connected" << std::endl;

        juce::AudioBuffer<float> originalBuffer(2, 10);
        fillBuffer(originalBuffer, 1.022f);
        host.sendTo(originalBuffer);
        std::cout << "Thread 1 Sent buffer" << std::endl;

        juce::AudioBuffer<float> recievedBuffer(2, 10);
        recievedBuffer.clear();
        host.recieveFrom(recievedBuffer);
        std::cout << "Thread 1 Recieved buffer" << std::endl;

        printBuffer(originalBuffer);
        printBuffer(recievedBuffer);
        REQUIRE(originalBuffer == recievedBuffer);
        host.stopHost();
    });

    auto thread_2 = std::jthread([]() {
        std::cout << "Thread 2" << std::endl;
        addressData hostAddress("127.0.0.1", 8010);
        Host host(hostAddress);
        host.setupSocket();
        std::cout << "Thread 2 Sending connection request" << std::endl;
        addressData remoteAddress("127.0.0.1", 8001);
        host.sendHandshake(remoteAddress);
        juce::AudioBuffer<float> recievedBuffer(2, 10);
        std::cout << "Thread 2 Waiting for buffer" << std::endl;
        host.recieveFrom(recievedBuffer);
        std::cout << "Thread 2 Recieved buffer" << std::endl;


        host.sendTo(recievedBuffer);
        std::cout << "Thread 2 Sent buffer back" << std::endl;
        host.stopHost();
    });

    thread_1.join();
    thread_2.join();
    std::cout << "Threads joined" << std::endl;
}


TEST_CASE("Host & AudioBuffer | back & forth with abstractFIFO write and read "
          "on each side")
{
    std::cout << "Starting " << std::endl;

    auto thread_1 = std::jthread([]() {
        std::cout << "Thread 1" << std::endl;
        addressData hostAddress("127.0.0.1", 8001);
        Host host(hostAddress);
        host.setupSocket();
        AudioBufferFIFO audioBufferFifo(2, 20);
        std::cout << "Thread 1 Host created" << std::endl;
        host.waitForHandshake();
        std::cout << "Thread 1 Connected" << std::endl;

        juce::AudioBuffer<float> originalBuffer(2, 10);
        fillBuffer(originalBuffer, 1.022f);

        audioBufferFifo.writeToBuffer(originalBuffer);
        juce::AudioBuffer<float> bufferReadFromFifo(2, 10);
        audioBufferFifo.readFromBuffer(bufferReadFromFifo);

        host.sendTo(bufferReadFromFifo);
        std::cout << "Thread 1 Sent buffer" << std::endl;

        juce::AudioBuffer<float> recievedBuffer(2, 10);
        recievedBuffer.clear();
        host.recieveFrom(recievedBuffer);
        std::cout << "Thread 1 Recieved buffer" << std::endl;

        printBuffer(originalBuffer);
        printBuffer(recievedBuffer);
        REQUIRE(originalBuffer == recievedBuffer);
        host.stopHost();
    });

    auto thread_2 = std::jthread([]() {
        std::cout << "Thread 2" << std::endl;
        addressData hostAddress("127.0.0.1", 8010);
        Host host(hostAddress);
        host.setupSocket();
        AudioBufferFIFO audioBufferFifo2(2, 20);
        std::cout << "Thread 2 Sending connection request" << std::endl;
        addressData remoteAddress("127.0.0.1", 8001);
        host.sendHandshake(remoteAddress);
        juce::AudioBuffer<float> recievedBuffer(2, 10);
        std::cout << "Thread 2 Waiting for buffer" << std::endl;
        host.recieveFrom(recievedBuffer);
        std::cout << "Thread 2 Recieved buffer" << std::endl;

        audioBufferFifo2.writeToBuffer(recievedBuffer);
        juce::AudioBuffer<float> bufferReadFromFifo(2, 10);
        audioBufferFifo2.readFromBuffer(bufferReadFromFifo);

        host.sendTo(bufferReadFromFifo);
        std::cout << "Thread 2 Sent buffer back" << std::endl;
        host.stopHost();
    });

    thread_1.join();
    thread_2.join();
    std::cout << "Threads joined" << std::endl;
}
