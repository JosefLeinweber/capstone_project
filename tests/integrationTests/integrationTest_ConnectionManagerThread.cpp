#include "ConnectDAWs/audioBuffer.h"
#include "ConnectDAWs/connectionManagerThread.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>

TEST_CASE("ConnectionManagerThread & ConnectionManagerThread | successfully "
          "send data between two threads")
{
    auto remoteThread = std::thread([&]() {
        std::atomic<bool> remoteStartConnection = false;
        std::atomic<bool> remoteStopConnection = false;

        juce::AudioBuffer<float> audioBuffer(2, 256);
        audioBuffer.clear();
        fillBuffer(audioBuffer, 1.0f);
        outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
        outputRingBuffer.writeToInternalBufferFrom(audioBuffer);

        ConnectionManagerThread remoteThread(guiMessenger1,
                                             cmtMessenger1,
                                             remoteConfigurationData,
                                             remoteInputRingBuffer,
                                             remoteOutputRingBuffer,
                                             remoteStartConnection,
                                             remoteStopConnection);
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
        DBG("RemoteThread | EXIT");
    });

    juce::AudioBuffer<float> audioBuffer(2, 256);
    audioBuffer.clear();
    fillBuffer(audioBuffer, 2.0f);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    ConnectionManagerThread localThread(guiMessenger2,
                                        cmtMessenger2,
                                        localConfigurationData,
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
