#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp> 
#include "AudioBuffer.h"
#include "Host.h"




class NetworkThread : public juce::Thread
{
public:
  NetworkThread(addressData hostAddress);

  virtual void run();

  void setupHost();

  void receiveHandshake();

  void sendHandshake();


private:
  addressData m_hostAddress;
  std::unique_ptr<Host> m_host;

};
