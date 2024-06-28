#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "audioBuffer.h"
#include "connectionManagerThread.h"
#include "datagram.pb.h"
#include "messenger.h"


class ConnectDAWs
{
public:
    //==============================================================================
    ConnectDAWs();
    ~ConnectDAWs();

    //==============================================================================
    void prepareToPlay(double sampleRate,
                       int samplesPerBlock,
                       int numInputChannels,
                       int numOutputChannels);
    void releaseResources();
    void processBlock(juce::AudioBuffer<float> &);

    //==============================================================================

    std::shared_ptr<Messenger> m_guiMessenger;
    std::shared_ptr<Messenger> m_cmtMessenger;

private:
    std::string getIp();
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
    std::unique_ptr<ConnectionManagerThread> m_connectionManagerThread;
    std::shared_ptr<AudioBufferFIFO> m_outputBufferFIFO;
    std::shared_ptr<AudioBufferFIFO> m_inputBufferFIFO;
};
