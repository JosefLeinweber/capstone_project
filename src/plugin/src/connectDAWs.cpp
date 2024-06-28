#include "ConnectDAWs/connectDAWs.h"

ConnectDAWs::ConnectDAWs()
{
}

ConnectDAWs::~ConnectDAWs()
{
}

std::string ConnectDAWs::getIp()
{
    //TODO: implement this function
    return "127.0.0.1";
}


void ConnectDAWs::setLocalConfigurationData(double sampleRate,
                                            int samplesPerBlock,
                                            int numInputChannels,
                                            int numOutputChannels)
{
    m_localConfigurationData.set_ip(getIp());
    m_localConfigurationData.set_host_port(7000);
    m_localConfigurationData.set_consumer_port(7001);
    m_localConfigurationData.set_provider_port(7002);
    m_localConfigurationData.set_samples_per_block(samplesPerBlock);
    m_localConfigurationData.set_sample_rate(sampleRate);
    m_localConfigurationData.set_num_input_channels(numInputChannels);
    m_localConfigurationData.set_num_output_channels(numOutputChannels);
}

void ConnectDAWs::initFIFOBuffers(int numInputChannels,
                                  int numOutputChannels,
                                  int samplesPerBlock)
{
    //TODO: remove magic number (10) determin what the best value is and set a constant
    int bufferSize = samplesPerBlock * 10;

    m_inputBufferFIFO =
        std::make_shared<AudioBufferFIFO>(numInputChannels, bufferSize);
    m_outputBufferFIFO =
        std::make_shared<AudioBufferFIFO>(numOutputChannels, bufferSize);
}

void ConnectDAWs::prepareToPlay(double sampleRate,
                                int samplesPerBlock,
                                int numInputChannels,
                                int numOutputChannels)
{


    initFIFOBuffers(numInputChannels, numOutputChannels, samplesPerBlock);

    setLocalConfigurationData(sampleRate,
                              samplesPerBlock,
                              numInputChannels,
                              numOutputChannels);

    m_connectionManagerThread =
        std::make_unique<ConnectionManagerThread>(m_guiMessenger,
                                                  m_cmtMessenger,
                                                  m_localConfigurationData,
                                                  *m_inputBufferFIFO,
                                                  *m_outputBufferFIFO,
                                                  m_startConnection,
                                                  m_stopConnection);
    m_connectionManagerThread->startThread();
}


void ConnectDAWs::releaseResources()
{
    if (m_connectionManagerThread != nullptr)
    {
        m_connectionManagerThread->signalThreadShouldExit();
        m_connectionManagerThread->waitForThreadToExit(1000);
    }
}

void ConnectDAWs::processBlock(juce::AudioBuffer<float> &buffer)
{
    // write and read from fifo buffers
}
