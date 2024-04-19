/*
  ==============================================================================

    KnobSend.cpp
    Created: 1 Feb 2022 8:27:43am
    Author:  Estudio

  ==============================================================================
*/

#include "Components.h"

namespace MyJUCEModules {
	// =====================================  ArrowButton  ================================================

	ArrowButton::ArrowButton(const juce::String& buttonName, float arrowDirection, juce::Colour arrowColour) :
		Button(buttonName), colour(arrowColour)
	{
		path.addTriangle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
		path.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::twoPi * arrowDirection, 0.5f, 0.5f));
	}

	ArrowButton::~ArrowButton() {}

	void ArrowButton::paintButton(juce::Graphics& g, bool /*shouldDrawButtonAsHighlighted*/, bool shouldDrawButtonAsDown)
	{
		juce::Path p(path);

		const float offset = shouldDrawButtonAsDown ? 1.0f : 0.0f;
		p.applyTransform(path.getTransformToScaleToFit(offset, offset, (float)getWidth(), (float)getHeight(), false));

		juce::DropShadow(juce::Colours::black.withAlpha(0.3f), shouldDrawButtonAsDown ? 2 : 4, juce::Point<int>()).drawForPath(g, p);
		g.setColour(colour);
		if (!this->isEnabled())
			g.setOpacity(0.5f);
		g.fillPath(p);
	}


    // =====================================  PluginPanel  ================================================

	PluginPanel::PluginPanel(PresetManager& pm, juce::RangedAudioParameter& gS, juce::UndoManager& uM) :
		presetManager(pm),
		undoManager(uM),
        guiSize(dynamic_cast<juce::AudioParameterChoice&>(gS)),
		previousPresetButton("Previous", 0.5f, juce::Colours::white),
		nextPresetButton("Next", 1.0f, juce::Colours::white)
	{
		configureTextButton(undoButton, "<");
		undoButton.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnRight);
		configureTextButton(redoButton, ">");
		redoButton.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnLeft);
		configureTextButton(copyButton, "C");
		configureArrowButton(previousPresetButton);
		configureComboBox(presetComboBox, "No preset");
		configureArrowButton(nextPresetButton);
		configureTextButton(optionsButton, "...");
		configureTextButton(aButton, "A");
		aButton.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnRight);
		aButton.setClickingTogglesState(true);
		aButton.setToggleState(true, juce::dontSendNotification);
		configureTextButton(copyAtoBButton, "/");
		copyAtoBButton.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnLeft | juce::Button::ConnectedEdgeFlags::ConnectedOnRight);
		configureTextButton(bButton, "B");
		bButton.setConnectedEdges(juce::Button::ConnectedEdgeFlags::ConnectedOnLeft);
		bButton.setClickingTogglesState(true);
		configureTextButton(bypassButton, "O");

		const auto allPresets = presetManager.getAllPresets();
		const auto currentPreset = presetManager.getCurrentPresetName();
		presetComboBox.setJustificationType(juce::Justification::centred);
		presetComboBox.addItemList(allPresets, 1);
		presetComboBox.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);

		presetManager.copyCurrentConfigToOther();
	}

	PluginPanel::~PluginPanel() {
		undoButton.removeListener(this);
		undoButton.setLookAndFeel(nullptr);

		redoButton.removeListener(this);
		redoButton.setLookAndFeel(nullptr);

		copyButton.removeListener(this);
		copyButton.setLookAndFeel(nullptr);

		previousPresetButton.removeListener(this);
		presetComboBox.removeListener(this);
		nextPresetButton.removeListener(this);

		optionsButton.removeListener(this);
		optionsButton.setLookAndFeel(nullptr);

		aButton.removeListener(this);
		aButton.setLookAndFeel(nullptr);
		copyAtoBButton.removeListener(this);
		copyAtoBButton.setLookAndFeel(nullptr);
		bButton.removeListener(this);
		bButton.setLookAndFeel(nullptr);

		bypassButton.removeListener(this);
		bypassButton.setLookAndFeel(nullptr);

		previousPresetButton.setLookAndFeel(nullptr);
		nextPresetButton.setLookAndFeel(nullptr);
		presetComboBox.setLookAndFeel(nullptr);
	}

	void PluginPanel::paint(juce::Graphics& g) {
		g.fillAll(juce::Colours::grey.darker());
		g.setColour(juce::Colours::grey);
		g.drawText(pluginInfo, getLocalBounds(), juce::Justification::left);
	}

	void PluginPanel::resized() {

		const auto panelBounds = getLocalBounds();
		const auto buttonHeight = panelBounds.proportionOfHeight(0.9f);
	
		auto rightSideBounds = panelBounds;
		auto leftSideBounds = panelBounds;

		auto presetComboBoxAndArrowsBounds = getLocalBounds().withSizeKeepingCentre(11.5f * buttonHeight, panelBounds.getHeight());
		rightSideBounds.removeFromLeft(presetComboBoxAndArrowsBounds.getX() + presetComboBoxAndArrowsBounds.getWidth());
		leftSideBounds.removeFromRight(rightSideBounds.getWidth() + presetComboBoxAndArrowsBounds.getWidth());
		presetComboBoxAndArrowsBounds = presetComboBoxAndArrowsBounds.reduced(0.1f * buttonHeight);
		leftSideBounds = leftSideBounds.reduced(0.1f * buttonHeight);
		rightSideBounds = rightSideBounds.reduced(0.1f * buttonHeight);

		lookAndFeel.setCornerSize(0.25f * buttonHeight);
		juce::Font f;
		auto pluginInfoWidth = f.getStringWidth(pluginInfo);

		previousPresetButton.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(.7f * buttonHeight).reduced(0.21f * buttonHeight));
		presetComboBox.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(10.1f * buttonHeight).reduced(0.05f * buttonHeight));
		nextPresetButton.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(.7f * buttonHeight).reduced(0.21f * buttonHeight));

		leftSideBounds.removeFromLeft(pluginInfoWidth);
		copyButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.05f * buttonHeight));
		leftSideBounds.removeFromRight(2.f * buttonHeight);
		redoButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.05f * buttonHeight));
		undoButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.05f * buttonHeight));

		optionsButton.setBounds(rightSideBounds.removeFromLeft(1.f * buttonHeight).reduced(0.025f * buttonHeight));
		rightSideBounds.removeFromLeft(2.f * buttonHeight);
		aButton.setBounds(rightSideBounds.removeFromLeft(.75f * buttonHeight).reduced(0.025f * buttonHeight));
		copyAtoBButton.setBounds(rightSideBounds.removeFromLeft(.5f * buttonHeight).reduced(0.025f * buttonHeight));
		bButton.setBounds(rightSideBounds.removeFromLeft(.75f * buttonHeight).reduced(0.025f * buttonHeight));
		bypassButton.setBounds(rightSideBounds.removeFromRight(buttonHeight).reduced(0.025f * buttonHeight));
	}

	void PluginPanel::buttonClicked(juce::Button* button) {
		if (button == &undoButton) {
			undoManager.undo();
			if (undoManager.canUndo()) {
				undoButton.setEnabled(true);
			}
			else {
				undoButton.setEnabled(false);
			}
		}
		else if (button == &redoButton) {
			undoManager.redo();
		}
		if (button == &previousPresetButton) {
			presetManager.loadPreviousPreset();
			const auto allPresets = presetManager.getAllPresets();
			const auto currentPreset = presetManager.getCurrentPresetName();
			presetComboBox.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);
		}
		else if (button == &nextPresetButton) {
			presetManager.loadNextPreset();
			const auto allPresets = presetManager.getAllPresets();
			const auto currentPreset = presetManager.getCurrentPresetName();
			presetComboBox.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);
		}
		else if (button == &aButton) {
			presetManager.switchToConfig("A");
			bButton.setToggleState(false, juce::dontSendNotification);
			aButton.setToggleState(true, juce::dontSendNotification);
		}
		else if (button == &bButton) {
			presetManager.switchToConfig("B");
			bButton.setToggleState(true, juce::dontSendNotification);
			aButton.setToggleState(false, juce::dontSendNotification);
		}
		else if (button == &copyAtoBButton) {
			presetManager.copyCurrentConfigToOther();
		}
		else if (button == &optionsButton) {
			juce::PopupMenu m;
			m.setLookAndFeel(&lookAndFeel);
			m.addItem("Load", [this] {
				presetFileChooser = std::make_unique<juce::FileChooser>("Select a preset to load", presetManager.defaultDirectory, "*." + presetManager.extension);
				presetFileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [&](const juce::FileChooser& fc) {
					const auto file = fc.getResult();
					presetManager.loadPreset(file);
					const auto allPresets = presetManager.getAllPresets();
					const auto currentPreset = presetManager.getCurrentPresetName();
					presetComboBox.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);
				});
			});
			m.addItem("Save", [this] {
				presetFileChooser = std::make_unique<juce::FileChooser>("Save as", presetManager.defaultDirectory, "*." + presetManager.extension);
				presetFileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting, [&](const juce::FileChooser& fc) {
					const auto file = fc.getResult();
					presetManager.savePreset(file);

					const auto allPresets = presetManager.getAllPresets();
					const auto currentPreset = presetManager.getCurrentPresetName();

					presetComboBox.clear(juce::dontSendNotification);
					presetComboBox.addItemList(allPresets, 1);
					presetComboBox.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);
				});
			});
			m.addItem("Copy", [this] { presetManager.copyPreset(); });
			m.addItem("Paste", [this] { presetManager.pastePreset(); });
			m.addSeparator();
			auto choices = guiSize.getAllValueStrings();
			for (auto i = 0; i < choices.size(); i++) {
				m.addItem(choices[i], !(i == guiSize.getIndex()), false, [this, i] {
					guiSize.operator=(i);
				});
			}
			m.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(&optionsButton));
			m.setLookAndFeel(nullptr);
		}
	}

	void PluginPanel::comboBoxChanged(juce::ComboBox* comboBox) {
		if (comboBox == &presetComboBox) {
			juce::File file(presetManager.defaultDirectory.getChildFile(presetComboBox.getItemText(presetComboBox.getSelectedItemIndex()) + "." + presetManager.extension));
			presetManager.loadPreset(file);
		}
	}

	void PluginPanel::configureTextButton(juce::Button& button, const juce::String& buttonText) {
		button.setButtonText(buttonText);
		button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
		button.setLookAndFeel(&lookAndFeel);
		addAndMakeVisible(button);
		button.addListener(this);
	}

	void PluginPanel::configureArrowButton(juce::Button& button)
	{
		button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
		button.setLookAndFeel(&lookAndFeel);
		addAndMakeVisible(button);
		button.addListener(this);
	}

	void PluginPanel::configureComboBox(juce::ComboBox& comboBox, const juce::String& textWhenNothingSelected)
	{
		comboBox.setTextWhenNothingSelected(textWhenNothingSelected);
		comboBox.setMouseCursor(juce::MouseCursor::PointingHandCursor);
		comboBox.setLookAndFeel(&lookAndFeel);
		addAndMakeVisible(comboBox);
		comboBox.addListener(this);
	}
}
