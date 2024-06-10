#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 
#include "AudioBuffer.h"
#include "Host.h"

class ProviderThread : public juce::Thread
{
public:
  ProviderThread( addressData& hostAddress, 
                  addressData& remoteAddress, 
                  AudioBufferFIFO& outputRingBuffer, 
                  std::atomic<bool>& isProviderConnected);

  void run() override;

  void setupHost();

  bool validateConnection();

  void startSendingAudio();


private:
  addressData m_hostAddress;
  addressData m_remoteAddress;
  std::unique_ptr<Host> m_host;


  AudioBufferFIFO& m_outputRingBuffer;
  juce::AudioBuffer<float> m_outputBuffer;
  std::atomic<bool>& m_isProviderConnected;
};
