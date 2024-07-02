#include "ConnectDAWs/audioBuffer.h"
#include "ConnectDAWs/connectionManagerThread.h"
#include "sharedValues.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <juce_core/juce_core.h>
#include <thread>


TEST_CASE(
    "ConnectionManagerThread & ConnectionManagerThread | sendConfiguration and "
    "receiveConfiguration successfull")
{
    //GIVEN: a ConnectionManagerThread which is connected to a remote host
    auto sendingThread = std::jthread([]() {
        remoteConnectionManagerThread.setup();
        remoteConnectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(5000));
        // need to wait because asyncWaitForConnection is non-blocking
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        remoteConnectionManagerThread.sendConfigurationData(
            remoteConfigurationData);
    });


    connectionManagerThread.setup();
    connectionManagerThread.initializeConnection(remoteConfigurationData);

    //WHEN: the ConnectionManagerThread tries to receive configuration data
    bool received = connectionManagerThread.receiveConfigurationData();

    //THEN: the configuration data should be received successfully and the data should be correct
    REQUIRE(received);

    ConfigurationData receivedConfigurationData =
        connectionManagerThread.m_remoteConfigurationData;

    std::cout << "Provider IP: " << receivedConfigurationData.ip()
              << " Provider Port: " << receivedConfigurationData.provider_port()
              << std::endl;
    REQUIRE(receivedConfigurationData.ip() == remoteConfigurationData.ip());
    REQUIRE(receivedConfigurationData.provider_port() ==
            remoteConfigurationData.provider_port());
    REQUIRE(receivedConfigurationData.consumer_port() ==
            remoteConfigurationData.consumer_port());
    sendingThread.join();
}

TEST_CASE("ConnectionManagerThread & ConnectionManagerThread | "
          "exchangeConfigurationDataWithRemote "
          "successfully")
{
    //GIVEN: a ConnectionManagerThread which is connected to a remote host
    auto remoteThread = std::jthread([]() {
        remoteConnectionManagerThread.setupHost();
        remoteConnectionManagerThread.asyncWaitForConnection(
            std::chrono::milliseconds(5000));
        // need to wait because asyncWaitForConnection is non-blocking
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        remoteConnectionManagerThread.exchangeConfigurationDataWithRemote(
            remoteConfigurationData);
    });


    connectionManagerThread.setupHost();
    connectionManagerThread.asyncWaitForConnection(
        std::chrono::milliseconds(5000));
    connectionManagerThread.initializeConnection(remoteConfigurationData);

    //WHEN: the ConnectionManagerThread tries to receive configuration data
    REQUIRE_NOTHROW(connectionManagerThread.exchangeConfigurationDataWithRemote(
        localConfigurationData));

    //THEN: the configuration data should be received successfully and the data should be correct
    ConfigurationData receivedConfigurationData =
        connectionManagerThread.m_remoteConfigurationData;

    std::cout << "Provider IP: " << receivedConfigurationData.ip()
              << " Provider Port: " << receivedConfigurationData.provider_port()
              << std::endl;
    REQUIRE(receivedConfigurationData.ip() == remoteConfigurationData.ip());
    REQUIRE(receivedConfigurationData.provider_port() ==
            remoteConfigurationData.provider_port());
    REQUIRE(receivedConfigurationData.consumer_port() ==
            remoteConfigurationData.consumer_port());
    remoteThread.join();
}

TEST_CASE("ConnectionManagerThread & ConnectionManagerThread | successfully "
          "send data between two threads")
{
    // 1. Setting up the remote connection buffer
    std::atomic<bool> remoteStartConnection = false;
    std::atomic<bool> remoteStopConnection = false;
    juce::AudioBuffer<float> audioBuffer(2, 256);
    audioBuffer.clear();
    fillBuffer(audioBuffer, 1.0f);
    remoteOutputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    remoteOutputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    remoteOutputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    remoteOutputRingBuffer.writeToInternalBufferFrom(audioBuffer);

    ConnectionManagerThread remoteThread(guiMessenger1,
                                         cmtMessenger1,
                                         remoteConfigurationData,
                                         remoteInputRingBuffer,
                                         remoteOutputRingBuffer,
                                         remoteStartConnection,
                                         remoteStopConnection,
                                         "RemoteCMT");

    std::cout << "RemoteThread | inputRingBuffer" << std::endl;
    printBuffer(remoteInputRingBuffer.buffer);
    std::cout << "RemoteThread | outputRingBuffer" << std::endl;
    printBuffer(remoteOutputRingBuffer.buffer);

    // 3. Setting up the local thread
    audioBuffer.clear();
    fillBuffer(audioBuffer, 2.0f);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    outputRingBuffer.writeToInternalBufferFrom(audioBuffer);
    ConnectionManagerThread localThread(guiMessenger2,
                                        cmtMessenger2,
                                        localConfigurationData,
                                        inputRingBuffer,
                                        outputRingBuffer,
                                        startConnection,
                                        stopConnection,
                                        "LocalCMT");

    // 4. Start both threads -> both are waiting for a connection
    remoteThread.startThread();
    localThread.startThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "started localThread" << std::endl;

    // 5. Mock an incoming message on the local thread to make it connect to the remote thread
    localThread.handleMessage(MessageToCMT("127.0.0.1", 6000));

    // 6. while the connection is not established, wait
    while (startConnection)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 7. shutdown both threads
    localThread.signalThreadShouldExit();
    remoteThread.signalThreadShouldExit();
    localThread.waitForThreadToExit(1000);
    remoteThread.waitForThreadToExit(1000);
    std::cout << "LocalThread | inputRingBuffer" << std::endl;
    printBuffer(inputRingBuffer.buffer);
    std::cout << "LocalThread | outputRingBuffer" << std::endl;
    printBuffer(outputRingBuffer.buffer);

    // 8. Check if the data was successfully sent from the local to the remote thread
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
