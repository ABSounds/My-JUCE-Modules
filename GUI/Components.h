#pragma once

#include "JuceHeader.h"
#include "LookAndFeel.h"
#include "../PresetManager/PresetManager.h"
#include "../DSP/MeterSource.h"

namespace MyJUCEModules {

    // ====================== ARROW BUTTON ======================
    /**
    *   @brief Arrow button with custom paint method.
    **/
    class ArrowButton : public juce::Button
    {
    public:
        /**
        *   @brief Arrow button with custom paint method. 
        *   @param buttonName Name of the button
        *   @param arrowDirection The direction the arrow should point in, where 0.0 is pointing right, 0.25 is down, 0.5 is left, 0.75 is up.
        *   @param arrowColour juce::Colour to use for the arrow.
        **/
        ArrowButton(const juce::String& buttonName, float arrowDirection, juce::Colour arrowColour);
        ~ArrowButton() override;
        void paintButton(juce::Graphics&, bool, bool) override;

    private:
        juce::Colour colour;
        juce::Path path;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArrowButton)
    };

    // ====================== MY TEXT BUTTON ======================
    /**
    *   @brief Text button with custom paint method.
    **/
    class MyTextButton : public juce::TextButton
	{
    public:
        using juce::TextButton::TextButton;
        void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
        void setFont(juce::Font font) { this->font = font; }
        void setColour(juce::Colour colour) { this->colour = colour; }
    private:
        juce::Font font;
        juce::Colour colour = juce::Colours::gainsboro.darker().darker().darker().darker();
    };

    // ====================== PLUGIN PANEL ======================
    /**
    *   @brief Top panel containing the GUI elements for the Preset Manager, undo/redo, resize and A/B configurations functionalities as well as the logo and plugin's version number.
    **/
    class PluginPanel : public juce::Component, juce::Button::Listener, juce::ComboBox::Listener, juce::ChangeListener
    {
    public:
        /**
        *   @param presetManager Reference to the plugin's PresetManager object.
        *   @param undoManager Reference to the plugin's UndoManager object.
        *   @param apvts Reference to the plugin's AudioProcessorValueTreeState to be affected by the undoManager.
        **/
        PluginPanel(PresetManager& presetManager, juce::UndoManager& undoManager, juce::AudioProcessorValueTreeState& apvts);
        ~PluginPanel();
        void paint(juce::Graphics& g) override;
        void resized() override;

    private:
        void buttonClicked(juce::Button* button) override;
        void comboBoxChanged(juce::ComboBox* comboBox) override;
        void configureComboBox(juce::ComboBox& comboBox, const juce::String& textWhenNothingSelected);
        void configureIconButton(juce::Button& button, std::unique_ptr<juce::Drawable> icon);
        void configureTextButton(MyJUCEModules::MyTextButton& button, const juce::String& buttonText);
        void configureArrowButton(juce::Button& button);

        void changeListenerCallback(juce::ChangeBroadcaster* source) override;

        PresetManager& presetManager;
        juce::UndoManager& undoManager;
        juce::AudioProcessorValueTreeState& pluginApvts;
        SharedResourcePointer<TooltipWindow> tooltipWindow;
        
        juce::Font font = juce::Font(juce::String("Tahoma"), 11.0f, juce::Font::FontStyleFlags::plain);
        juce::String pluginName = "  " + juce::String(JucePlugin_Name) + " ";
        juce::String pluginVersion = " v" + juce::String(JucePlugin_VersionString);

        std::unique_ptr<juce::Drawable> undoIcon, redoIcon, copyIcon, optionsIcon, oversamplingIcon, bypassIcon;

        juce::DrawableButton undoButton{ "undo", juce::DrawableButton::ButtonStyle::ImageFitted }, redoButton{ "redo", juce::DrawableButton::ButtonStyle::ImageFitted },
            copyButton{ "copy", juce::DrawableButton::ButtonStyle::ImageFitted }, optionsButton{ "options", juce::DrawableButton::ButtonStyle::ImageFitted },
            oversamplingButton{ "oversampling", juce::DrawableButton::ButtonStyle::ImageFitted }, bypassButton{ "bypass", juce::DrawableButton::ButtonStyle::ImageFitted };

        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> oversamplingAttachment, bypassAttachment;
        
        MyJUCEModules::MyTextButton aButton, copyAtoBButton, bButton;
        
        ArrowButton previousPresetButton, nextPresetButton;
        
        juce::ComboBox presetComboBox;
        
        std::unique_ptr<juce::FileChooser> presetFileChooser;

        juce::Colour textBaseColour = juce::Colours::gainsboro.darker().darker().darker().darker();

        PluginPanelLookAndFeel lookAndFeel;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginPanel)
    };

    // ====================== LEVEL METER COMPONENT ======================
    /**
    *   @brief Meter component that displays the level of an audio buffer.
    **/
    class LevelMeter : public juce::Component
    {
    public:

        enum Orientation{
            Vertical,
            Horizontal,
            Free // Adapts the orientation depending on the aspect ratio of the component
        };

        struct MeterColours {
            juce::Colour backgroundColour;
            juce::Colour fillColour;
            juce::Colour warningColour;
            juce::Colour clipColour;
        };

        /**
        *   @param source Reference to the MeterSource object to use for the meter.
        *   @param minDb Minimum value in dB.
        *   @param maxDb Maximum value in dB.
        *   @param orientation Orientation of the meter.
        *   @param showClipIndicator Whether to show the clip indicator.
        **/
        LevelMeter(MeterSource& source, float minDb, float maxDb, MeterColours colours, bool showClipIndicator);
        void resized() override;
        void update();

    private:
        class MeterBar;
        class ClipIndicator;

        MeterSource& meterSource;
        float minDb, maxDb;
        bool clipped = false;

        Orientation setupOrientation = Orientation::Free;
        Orientation orientationToUse;

        bool showClipIndicator;
        size_t numChannels = 1;

        juce::OwnedArray<MeterBar> meterBars;
        juce::OwnedArray<ClipIndicator> clipIndicators;

        class MeterBar : public juce::Component
        {
        public:
            MeterBar(Orientation orientation, float warningThreshold = 1.0f, float clipThreshold = 1.0f);
            ~MeterBar();
            void setOrientation(Orientation orientation);
            void setColours(LevelMeter::MeterColours colours);
            void setRanges(float warningThreshold, float clipThreshold);
            void setBarFill(float fillAmount);
            void paint(juce::Graphics& g) override;
        private:
            float fill = 0.0f;
            float warningThreshold;
            float clipThreshold;

            LevelMeter::MeterColours colours;
            Orientation orientation;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterBar)
        };

        class ClipIndicator : public juce::Component, juce::MouseListener
        {
        public:
            ClipIndicator();
            ~ClipIndicator();
            void paint(juce::Graphics& g) override;
            void setClipped(bool clipped);
            void mouseDown (const MouseEvent &event) override;
        private:
            juce::Colour colour;
            bool clipped = false;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClipIndicator)
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
    };

}
