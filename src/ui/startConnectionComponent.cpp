#include "ConnectDAWsComponents/startConnectionComponent.h"

StartConnectionComponent::StartConnectionComponent(
    std::function<void(juce::Button *, bool)> buttonClickedCallback)
    : m_buttonClickedCallback(buttonClickedCallback)
{
    addAndMakeVisible(m_button);
    m_button.addListener(this);
    m_button.setButtonText("Connect");

    addAndMakeVisible(m_ipEditor);
    m_ipEditor.setMultiLine(false);
    m_ipEditor.setTextToShowWhenEmpty("Enter IP address", juce::Colours::grey);
    m_ipEditor.setInputFilter(
        new juce::TextEditor::LengthAndCharacterRestriction(15, "0123456789."),
        true);

    addAndMakeVisible(m_ipTextLabel);

    m_ipTextLabel.setText("Your IP Address: loading...",
                          juce::dontSendNotification);
    m_ipTextLabel.setColour(juce::TextEditor::textColourId,
                            juce::Colours::white);
}

StartConnectionComponent::~StartConnectionComponent()
{
    m_button.removeListener(this);
}

void StartConnectionComponent::paint(juce::Graphics &g)
{
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void StartConnectionComponent::resized()
{
    auto textFieldHeight = 30;
    auto area = getLocalBounds();
    m_ipTextLabel.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    m_ipEditor.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    m_button.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
}

void StartConnectionComponent::buttonClicked(juce::Button *button)
{
    if (!m_ipEditor.isEmpty())
    {
        std::cout << "IP: " << m_ipEditor.getText() << std::endl;
        m_buttonClickedCallback(button, true);
    }
    else
    {
        std::cout << "Please enter an IP address!" << std::endl;
        m_ipEditor.setTextToShowWhenEmpty("Please enter an IP addres!",
                                          juce::Colours::grey);
        repaint();
    }
}
