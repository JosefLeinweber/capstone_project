#pragma once
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>


class StartConnectionComponent : public juce::Component,
                                 public juce::Button::Listener
{
public:
    StartConnectionComponent(
        std::function<void(juce::Button *, bool)> buttonClickedCallback);
    ~StartConnectionComponent();

    void paint(juce::Graphics &) override;
    void resized() override;

    void buttonClicked(juce::Button *button) override;

private:
    juce::TextEditor m_ipEditor;
    juce::TextButton m_button{"Connect"};
    std::function<void(juce::Button *, bool success)> m_buttonClickedCallback;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StartConnectionComponent)
};
