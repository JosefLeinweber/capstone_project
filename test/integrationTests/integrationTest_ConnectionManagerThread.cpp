#include "AudioBuffer.h"
#include "ConnectionManagerThread.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>

ConfigurationData localConfigurationData =
    setConfigurationData("127.0.0.1", 5000, 5001, 5002);
ConfigurationData remoteConfigurationData =
    setConfigurationData("127.0.0.1", 7000, 7001, 7002);

TEST_CASE("ConnectionManagerThread & ConnectionManagerThread | successfully "
          "send data between two threads")
{
    auto remoteThread = std::thread([&]() {
        AudioBufferFIFO inputRingBuffer(2, 80);
        AudioBufferFIFO outputRingBuffer(2, 80);
        std::atomic<bool> startConnection = false;
        std::atomic<bool> stopConnection = false;

        juce::AudioBuffer<float> audioBuffer(2, 20);
        audioBuffer.clear();
        fillBuffer(audioBuffer, 1.0f);
        outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
        outputRingBuffer.writeToInternalBufferFrom(audioBuffer);


        ConnectionManagerThread remoteThread(remoteConfigurationData,
                                             inputRingBuffer,
                                             outputRingBuffer,
                                             startConnection,
                                             stopConnection);
        remoteThread.startThread();
        std::cout << "started remoteThread" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        remoteThread.signalThreadShouldExit();
        std::cout << "signaled remoteThread to exit" << std::endl;
        remoteThread.waitForThreadToExit(1000);
        std::cout << "remoteThread exited" << std::endl;
        std::cout << "RemoteThread | inputRingBuffer" << std::endl;
        printBuffer(inputRingBuffer.buffer);
        std::cout << "RemoteThread | outputRingBuffer" << std::endl;
        printBuffer(outputRingBuffer.buffer);
    });


    AudioBufferFIFO inputRingBuffer(2, 80);
    AudioBufferFIFO outputRingBuffer(2, 80);
    std::atomic<bool> startConnection = false;
    std::atomic<bool> stopConnection = false;
    juce::AudioBuffer<float> audioBuffer(2, 20);
    audioBuffer.clear();
    fillBuffer(audioBuffer, 2.0f);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);

    ConnectionManagerThread localThread(localConfigurationData,
                                        inputRingBuffer,
                                        outputRingBuffer,
                                        startConnection,
                                        stopConnection);

    // Notify the localThread to start the connection
    auto notifyThread = std::thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (localThread.isThreadRunning())
        {
            startConnection = true;
            std::cout << "set startConnection to true" << std::endl;
        }
    });

    localThread.startThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(2200));
    remoteThread.join();
    notifyThread.join();
    localThread.signalThreadShouldExit();
    localThread.waitForThreadToExit(1000);
    std::cout << "LocalThread | inputRingBuffer" << std::endl;
    printBuffer(inputRingBuffer.buffer);
    std::cout << "LocalThread | outputRingBuffer" << std::endl;
    printBuffer(outputRingBuffer.buffer);
    bool success = false;
    for (int channel = 0; channel < 2; channel++)
    {
        for (int j = 0; j < 20; j++)
        {
            if (inputRingBuffer.buffer.getSample(channel, j) != 1.0f)
            {
                printf("inputRingBuffer.buffer.getSample(channel, j): %f\n",
                       inputRingBuffer.buffer.getSample(channel, j));
                success = false;
                break;
            }
            else
            {
                success = true;
            }
        }
    }
    REQUIRE(success);
}
