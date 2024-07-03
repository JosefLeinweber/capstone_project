#include "ConnectDAWsComponents/inConnectionComponent.h"

InConnectionComponent::InConnectionComponent(
    std::function<void(juce::Button *, bool)> buttonClickedCallback)
    : m_buttonClickedCallback(buttonClickedCallback)
{
    addAndMakeVisible(m_button);
    m_button.addListener(this);
    m_button.setButtonText("Cancel");

    addAndMakeVisible(m_statusLabel);
    m_statusLabel.setText("Trying to connect...", juce::dontSendNotification);
    m_statusLabel.setColour(juce::TextEditor::textColourId,
                            juce::Colours::white);
}

InConnectionComponent::~InConnectionComponent()
{
    m_button.removeListener(this);
}

void InConnectionComponent::paint(juce::Graphics &g)
{
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void InConnectionComponent::resized()
{
    auto textFieldHeight = 30;
    auto area = getLocalBounds();
    m_statusLabel.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
    m_button.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
}

void InConnectionComponent::buttonClicked(juce::Button *button)
{
    m_statusLabel.setText("Cancelling...", juce::dontSendNotification);
    repaint();
    m_buttonClickedCallback(button, true);
}

void InConnectionComponent::setStatusLabel(const juce::String &status)
{
    m_statusLabel.setText(status, juce::dontSendNotification);
}
