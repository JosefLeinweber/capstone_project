#pragma once
#include "ConnectDAWs/pluginProcessor.h"
#include "datagram.pb.h"
#include <juce_audio_basics/juce_audio_basics.h>


ConfigurationData setConfigurationData(const std::string ip,
                                       int providerPort,
                                       int consumerPort,
                                       int hostPort);

void printBuffer(juce::AudioBuffer<float> &buffer);
void fillBuffer(juce::AudioBuffer<float> &buffer, float value);


extern ConfigurationData localConfigurationData;
extern ConfigurationData remoteConfigurationData;


extern LowpassHighpassFilterAudioProcessor audioProcessor1;
extern LowpassHighpassFilterAudioProcessor audioProcessor2;
