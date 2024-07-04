#include "ConnectDAWsComponents/inConnectionComponent.h"

InConnectionComponent::InConnectionComponent(
    std::function<void(juce::Button *, bool)> buttonClickedCallback)
    : m_buttonClickedCallback(buttonClickedCallback)
{
    addAndMakeVisible(m_button);
    m_button.addListener(this);
    m_button.setButtonText("Cancel");
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
    m_button.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
}

void InConnectionComponent::buttonClicked(juce::Button *button)
{
    repaint();
    m_buttonClickedCallback(button, true);
}
