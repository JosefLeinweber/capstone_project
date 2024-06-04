#include "YourPluginName/LowpassHighpassFilter.h"

void LowpassHighpassFilter::setHighpass(bool newHighpass) {
  this->highpass = newHighpass;
}

void LowpassHighpassFilter::setCutoffFrequency(float newCutoffFrequency) {
  this->cutoffFrequency = newCutoffFrequency;
}

void LowpassHighpassFilter::setSamplingRate(float newSamplingRate) {
  this->samplingRate = newSamplingRate;
}

void LowpassHighpassFilter::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer&) {
  constexpr auto PI = 3.14159265359f;
  dnBuffer.resize(buffer.getNumChannels(), 0.f);
  auto sign = highpass ? -1.f : 1.f;
  for (auto channel = 0; channel < buffer.getNumChannels(); ++channel) {
    auto channelSamples = buffer.getWritePointer(channel);
    for (auto i = 0; i < buffer.getNumSamples(); ++i) {
      const auto tan = std::tan(PI * cutoffFrequency / samplingRate);

      const auto a1 = (tan - 1.f) / (tan + 1.f);
      const auto inputSample = channelSamples[i];
      const auto allpassFilteredSample = a1 * inputSample + dnBuffer[channel];
      dnBuffer[channel] = inputSample - a1 * allpassFilteredSample;

      const auto filterOutput =
          0.5f * (inputSample + sign * allpassFilteredSample);

      channelSamples[i] = filterOutput;
    }
  }
}
