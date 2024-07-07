/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"


//==============================================================================
ConnectDAWsAudioProcessorEditor::ConnectDAWsAudioProcessorEditor(
    ConnectDAWsAudioProcessor &p,
    juce::AudioProcessorValueTreeState &vts,
    std::shared_ptr<Messenger> &guiMessenger,
    std::shared_ptr<Messenger> &cmtMessenger)
    : AudioProcessorEditor(&p), m_audioProcessor(p),
      m_connectDAWsComponent(
          guiMessenger,
          cmtMessenger,
          std::bind(&ConnectDAWsAudioProcessorEditor::resized, this))
{
    constexpr auto HEIGHT = 500;
    constexpr auto WIDTH = 500;

    addAndMakeVisible(m_connectDAWsComponent);

    addAndMakeVisible(m_audioProcessor.m_visualiser);
    m_audioProcessor.m_visualiser.setColours(
        juce::Colours::black,
        juce::Colours::whitesmoke.withAlpha(0.5f));

    addAndMakeVisible(m_audioProcessor.m_outputVisualiser);
    m_audioProcessor.m_outputVisualiser.setColours(
        juce::Colours::black,
        juce::Colours::whitesmoke.withAlpha(0.5f));

    addAndMakeVisible(m_outputVisualiserLabel);
    m_outputVisualiserLabel.setText("Outgoing Audio",
                                    juce::dontSendNotification);
    m_outputVisualiserLabel.setColour(juce::Label::textColourId,
                                      juce::Colours::white);

    addAndMakeVisible(m_inputVisualiserLabel);
    m_inputVisualiserLabel.setText("Incoming Audio",
                                   juce::dontSendNotification);
    m_inputVisualiserLabel.setColour(juce::Label::textColourId,
                                     juce::Colours::white);


    setSize(WIDTH, HEIGHT);
}

ConnectDAWsAudioProcessorEditor::~ConnectDAWsAudioProcessorEditor()
{
}

//==============================================================================
void ConnectDAWsAudioProcessorEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void ConnectDAWsAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds();
    auto textFieldHeight = 30;
    m_connectDAWsComponent.setBounds(area.removeFromTop(150).reduced(0, 5));
    m_outputVisualiserLabel.setBounds(
        area.removeFromTop(textFieldHeight).reduced(0, 5));
    m_audioProcessor.m_visualiser.setBounds(
        area.removeFromTop(150).reduced(0, 5));
    m_inputVisualiserLabel.setBounds(
        area.removeFromTop(textFieldHeight).reduced(0, 5));
    m_audioProcessor.m_outputVisualiser.setBounds(
        area.removeFromTop(150).reduced(0, 5));
    if (m_connectDAWsComponent.m_isConnected)
    {
        m_audioProcessor.m_visualiser.setVisible(true);
        m_audioProcessor.m_outputVisualiser.setVisible(true);
        m_outputVisualiserLabel.setVisible(true);
        m_inputVisualiserLabel.setVisible(true);
    }
    else
    {
        m_audioProcessor.m_visualiser.setVisible(false);
        m_audioProcessor.m_outputVisualiser.setVisible(false);
        m_outputVisualiserLabel.setVisible(false);
        m_inputVisualiserLabel.setVisible(false);
    }
}
