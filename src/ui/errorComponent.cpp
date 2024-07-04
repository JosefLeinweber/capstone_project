#include "ConnectDAWsComponents/errorComponent.h"

ErrorComponent::ErrorComponent(
    std::function<void(juce::Button *, bool)> buttonClickedCallback)
    : m_buttonClickedCallback(buttonClickedCallback)
{
    addAndMakeVisible(m_button);
    m_button.addListener(this);
    m_button.setButtonText("Continue");
}

ErrorComponent::~ErrorComponent()
{
    m_button.removeListener(this);
}

void ErrorComponent::paint(juce::Graphics &g)
{
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ErrorComponent::resized()
{
    auto textFieldHeight = 30;
    auto area = getLocalBounds();
    m_button.setBounds(area.removeFromTop(textFieldHeight).reduced(0, 5));
}

void ErrorComponent::buttonClicked(juce::Button *button)
{
    repaint();
    m_buttonClickedCallback(button, true);
}
