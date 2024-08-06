#pragma once

#include <boost/asio.hpp>
#include <juce_audio_basics/juce_audio_basics.h>

#include "connectionManagerThread.h"
#include "datagram.pb.h"
#include "ringBuffer.h"

#include "messenger.h"


class ConnectDAWs
{
public:
    //==============================================================================
    ConnectDAWs();
    ~ConnectDAWs();

    //==============================================================================
    void startUpConnectionManagerThread(double sampleRate,
                                        int samplesPerBlock,
                                        int numInputChannels,
                                        int numOutputChannels);
    void releaseResources();
    void processBlock(juce::AudioBuffer<float> &);
    // TODO: is this the best place to get the ip???
    std::string getIp();

    //==============================================================================

    std::shared_ptr<Messenger> m_guiMessenger;
    std::shared_ptr<Messenger> m_cmtMessenger;
    std::unique_ptr<ConnectionManagerThread> m_connectionManagerThread;

private:
    void initFIFOBuffers(int numInputChannels,
                         int numOutputChannels,
                         int samplesPerBlock);
    void setLocalConfigurationData(double sampleRate,
                                   int samplesPerBlock,
                                   int numInputChannels,
                                   int numOutputChannels);

    ConfigurationData m_localConfigurationData;
    std::atomic<bool> m_startConnection = false;
    std::atomic<bool> m_stopConnection = false;
    std::shared_ptr<RingBuffer> m_outputBufferFIFO;
    std::shared_ptr<RingBuffer> m_inputBufferFIFO;
};
