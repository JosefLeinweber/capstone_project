#pragma once
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>


class ErrorComponent : public juce::Component, public juce::Button::Listener
{
public:
    ErrorComponent(
        std::function<void(juce::Button *, bool)> buttonClickedCallback);
    ~ErrorComponent();

    void paint(juce::Graphics &) override;
    void resized() override;

    void buttonClicked(juce::Button *button) override;

private:
    juce::TextButton m_button{"Continue"};
    std::function<void(juce::Button *, bool success)> m_buttonClickedCallback;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ErrorComponent)
};
