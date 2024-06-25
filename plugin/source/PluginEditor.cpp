/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "YourPluginName/PluginEditor.h"
#include "YourPluginName/PluginProcessor.h"


//==============================================================================
LowpassHighpassFilterAudioProcessorEditor::
    LowpassHighpassFilterAudioProcessorEditor(
        LowpassHighpassFilterAudioProcessor &p,
        juce::AudioProcessorValueTreeState &vts)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    constexpr auto HEIGHT = 500;
    constexpr auto WIDTH = 500;

    // Set up IP text editor
    ipLabel.attachToComponent(&ipEditor, true);
    ipEditor.setMultiLine(false);
    ipEditor.setTextToShowWhenEmpty("Enter IP address", juce::Colours::grey);
    addAndMakeVisible(ipEditor);

    // Set up Port text editor
    portLabel.attachToComponent(&portEditor, true);
    portEditor.setMultiLine(false);
    portEditor.setInputFilter(
        new juce::TextEditor::LengthAndCharacterRestriction(5, "0123456789"),
        true);
    portEditor.setTextToShowWhenEmpty("Enter Port", juce::Colours::grey);
    addAndMakeVisible(portEditor);

    // Set up Submit button
    sendButton.setButtonText("Connect");
    sendButton.addListener(this);
    addAndMakeVisible(sendButton);


    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    setSize(WIDTH, HEIGHT);
}

LowpassHighpassFilterAudioProcessorEditor::
    ~LowpassHighpassFilterAudioProcessorEditor()
{
    sendButton.removeListener(this);
}

//==============================================================================
void LowpassHighpassFilterAudioProcessorEditor::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
}

void LowpassHighpassFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds().reduced(20);
    auto textFieldHeight = 30;

    ipEditor.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    portEditor.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    sendButton.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
}

void LowpassHighpassFilterAudioProcessorEditor::buttonClicked(
    juce::Button *button)
{

    if (button == &sendButton)
    {
        std::string ip = ipEditor.getText().toStdString();
        int port = portEditor.getText().getIntValue();
        audioProcessor.sendToConnectionManagerThread(ip, port);
    }
}

void LowpassHighpassFilterAudioProcessorEditor::handleMessage(
    const juce::Message &message)
{
    if (auto *m = dynamic_cast<const MyCustomMessage *>(&message))
    {
        std::cout << "Message received in PluginEditor: " << m->ip << " "
                  << m->port << std::endl;
    }
}
