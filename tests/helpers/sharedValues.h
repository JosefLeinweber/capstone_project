#pragma once
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
