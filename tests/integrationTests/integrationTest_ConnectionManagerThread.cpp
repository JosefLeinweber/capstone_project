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
    std::atomic<bool> remoteStartConnection = false;
    std::atomic<bool> remoteStopConnection = false;

    juce::AudioBuffer<float> audioBuffer(2, 256);
    audioBuffer.clear();
    fillBuffer(audioBuffer, 1.0f);
    remoteOutputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    remoteOutputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    auto myLambda = [](const juce::Message &message) {
        juce::ignoreUnused(message);
    };
    guiMessenger1 = std::make_shared<Messenger>(myLambda);
    ConnectionManagerThread remoteThread(guiMessenger1,
                                         cmtMessenger1,
                                         remoteConfigurationData,
                                         remoteInputRingBuffer,
                                         remoteOutputRingBuffer,
                                         remoteStartConnection,
                                         remoteStopConnection);
    remoteThread.startThread();
    std::cout << "started remoteThread" << std::endl;
    std::cout << "RemoteThread | inputRingBuffer" << std::endl;
    printBuffer(remoteInputRingBuffer.buffer);
    std::cout << "RemoteThread | outputRingBuffer" << std::endl;
    printBuffer(remoteOutputRingBuffer.buffer);
    std::cout << "RemoteThread | EXIT" << std::endl;

    audioBuffer.clear();
    fillBuffer(audioBuffer, 2.0f);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    guiMessenger2 = std::make_shared<Messenger>(myLambda);
    ConnectionManagerThread localThread(guiMessenger2,
                                        cmtMessenger2,
                                        localConfigurationData,
                                        inputRingBuffer,
                                        outputRingBuffer,
                                        startConnection,
                                        stopConnection);


    localThread.startThread();
    while (!cmtMessenger2)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "started localThread" << std::endl;
    localThread.handleMessage(
        MessageToCMT(remoteConfigurationData.ip(),
                     remoteConfigurationData.host_port()));

    while (startConnection)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // remoteThread.join();
    localThread.signalThreadShouldExit();
    remoteThread.signalThreadShouldExit();
    localThread.waitForThreadToExit(1000);
    remoteThread.waitForThreadToExit(1000);
    std::cout << "LocalThread | inputRingBuffer" << std::endl;
    printBuffer(inputRingBuffer.buffer);
    std::cout << "LocalThread | outputRingBuffer" << std::endl;
    printBuffer(outputRingBuffer.buffer);
    bool areSendAndReceivedBuffersEqual = false;
    for (int channel = 0; channel < 2; channel++)
    {
        for (int j = 0; j < 256; j++)
        {
            if (inputRingBuffer.buffer.getSample(channel, j) != 1.0f)
            {
                printf("inputRingBuffer.buffer.getSample(channel, j): %f\n",
                       inputRingBuffer.buffer.getSample(channel, j));
                areSendAndReceivedBuffersEqual = false;
                break;
            }
            else
            {
                areSendAndReceivedBuffersEqual = true;
            }
        }
    }
    REQUIRE(areSendAndReceivedBuffersEqual);
}
