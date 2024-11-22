#pragma once
#include "ConnectDAWs/connectionManagerThread.h"
#include "ConnectDAWs/messenger.h"
#include "ConnectDAWs/ringBuffer.h"
#include "benchmark.h"
#include "datagram.pb.h"
#include "logger.h"
#include <juce_audio_basics/juce_audio_basics.h>


ConfigurationData setConfigurationData(const std::string ip,
                                       int providerPort,
                                       int consumerPort,
                                       int hostPort,
                                       int sampleRate,
                                       int samplesPerBlock,
                                       int numInputChannels,
                                       int numOutputChannels);

void printBuffer(juce::AudioBuffer<float> &buffer);
void fillBuffer(juce::AudioBuffer<float> &buffer, float value);


extern ConfigurationData localConfigurationData;
extern ConfigurationData remoteConfigurationData;

extern std::shared_ptr<Messenger> guiMessenger1;
extern std::shared_ptr<Messenger> cmtMessenger1;

extern std::shared_ptr<Messenger> guiMessenger2;
extern std::shared_ptr<Messenger> cmtMessenger2;

extern RingBuffer inputRingBuffer;
extern RingBuffer outputRingBuffer;

extern RingBuffer remoteInputRingBuffer;
extern RingBuffer remoteOutputRingBuffer;

extern std::atomic<bool> startConnection;
extern std::atomic<bool> stopConnection;

extern ConnectionManagerThread remoteConnectionManagerThread;
extern ConnectionManagerThread connectionManagerThread;

extern std::shared_ptr<std::vector<std::int64_t>> differenceBuffer;

extern std::shared_ptr<Benchmark> benchmark1;
extern std::shared_ptr<Benchmark> benchmark2;

extern std::shared_ptr<FileLogger> fileLogger;
