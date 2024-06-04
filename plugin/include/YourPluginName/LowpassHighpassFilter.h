#pragma once
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

class LowpassHighpassFilter
{
public:
  void setHighpass(bool highpass);
 void setCutoffFrequency(float cutoffFrequency);
  void setSamplingRate(float samplingRate);
 void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&);

private:
  bool highpass;
 float cutoffFrequency;
  float samplingRate;
 std::vector<float> dnBuffer;
};
