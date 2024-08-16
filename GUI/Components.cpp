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

		//juce::DropShadow(juce::Colours::black.withAlpha(0.3f), shouldDrawButtonAsDown ? 2 : 4, juce::Point<int>()).drawForPath(g, p);
		g.setColour(colour);
		if (!this->isEnabled())
			g.setOpacity(0.5f);
		g.fillPath(p);
	}


    // =====================================  PluginPanel  ================================================

	PluginPanel::PluginPanel(PresetManager& pm, juce::RangedAudioParameter& gS, juce::UndoManager& uM) :
		presetManager(pm), undoManager(uM), guiSize(dynamic_cast<juce::AudioParameterChoice&>(gS)), previousPresetButton("Previous", 0.5f, juce::Colours::gainsboro.darker().darker().darker()),
		nextPresetButton("Next", 1.0f, juce::Colours::gainsboro.darker().darker().darker())
	{
		undoManager.addChangeListener(this);

		undoIcon = juce::Drawable::createFromImageData(BinaryData::arrowgobackline_svg, BinaryData::arrowgobackline_svgSize);
		configureIconButton(undoButton, undoIcon->createCopy());
		undoButton.setEnabled(false);
		redoIcon = juce::Drawable::createFromImageData(BinaryData::arrowgoforwardline_svg, BinaryData::arrowgobackline_svgSize);
		configureIconButton(redoButton, redoIcon->createCopy());
		redoButton.setEnabled(false);

		copyIcon = juce::Drawable::createFromImageData(BinaryData::filecopyline_svg, BinaryData::filecopyline_svgSize);
		configureIconButton(copyButton, copyIcon->createCopy());
		
		configureArrowButton(previousPresetButton);
		configureComboBox(presetComboBox, "No preset");
		configureArrowButton(nextPresetButton);
		
		optionsIcon = juce::Drawable::createFromImageData(BinaryData::menuline_svg, BinaryData::menuline_svgSize);
		configureIconButton(optionsButton, optionsIcon->createCopy());
		
		configureTextButton(aButton, "A");
		aButton.setClickingTogglesState(true);
		aButton.setToggleState(true, juce::dontSendNotification);
		configureTextButton(copyAtoBButton, ">");
		configureTextButton(bButton, "B");
		bButton.setClickingTogglesState(true);
		
		bypassIcon= juce::Drawable::createFromImageData(BinaryData::shutdownline_svg, BinaryData::shutdownline_svgSize);
		configureIconButton(bypassButton, bypassIcon->createCopy());
		bypassButton.setClickingTogglesState(true);

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
		g.fillAll(juce::Colours::gainsboro.darker());
		g.setColour(textBaseColour);
		auto bounds = getLocalBounds();

		juce::Font f1;
		auto pluginNameBounds = bounds.removeFromLeft(f1.getStringWidth(pluginName));
		g.setFont(f1);
		g.drawText(pluginName, pluginNameBounds, juce::Justification::left);
		juce::Font f2 {f1.getHeight() * 0.75f};
		auto pluginVersionBounds = bounds.removeFromLeft(f2.getStringWidth(pluginVersion));
		g.setFont(f2);
		g.drawText(pluginVersion, pluginVersionBounds, juce::Justification::left);
	}

	void PluginPanel::resized() {

		const auto panelBounds = getLocalBounds();
		const auto buttonHeight = panelBounds.proportionOfHeight(0.9f);
	
		auto rightSideBounds = panelBounds;
		auto leftSideBounds = panelBounds;

		auto presetComboBoxAndArrowsBounds = getLocalBounds().withSizeKeepingCentre((11.5f + 0.2f)* buttonHeight, panelBounds.getHeight());
		rightSideBounds.removeFromLeft(presetComboBoxAndArrowsBounds.getX() + presetComboBoxAndArrowsBounds.getWidth());
		leftSideBounds.removeFromRight(rightSideBounds.getWidth() + presetComboBoxAndArrowsBounds.getWidth());
		presetComboBoxAndArrowsBounds = presetComboBoxAndArrowsBounds.reduced(0.1f * buttonHeight);
		leftSideBounds = leftSideBounds.reduced(0.1f * buttonHeight);
		rightSideBounds = rightSideBounds.reduced(0.1f * buttonHeight);

		lookAndFeel.setCornerSize(0.25f * buttonHeight);

		previousPresetButton.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(.7f * buttonHeight).reduced(0.21f * buttonHeight));
		presetComboBox.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(10.1f * buttonHeight).reduced(0.05f * buttonHeight));
		nextPresetButton.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(.7f * buttonHeight).reduced(0.21f * buttonHeight));

		copyButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.05f * buttonHeight));
		
		leftSideBounds.removeFromRight(2.f * buttonHeight);
		
		redoButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.05f * buttonHeight));
		leftSideBounds.removeFromRight(.25f * buttonHeight);
		undoButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.05f * buttonHeight));

		optionsButton.setBounds(rightSideBounds.removeFromLeft(1.f * buttonHeight).reduced(0.1f * buttonHeight));
		
		rightSideBounds.removeFromLeft(2.f * buttonHeight);
		
		aButton.setBounds(rightSideBounds.removeFromLeft(1.f * buttonHeight));
		copyAtoBButton.setBounds(rightSideBounds.removeFromLeft(.82f * buttonHeight));
		bButton.setBounds(rightSideBounds.removeFromLeft(1.f * buttonHeight));
		
		bypassButton.setBounds(rightSideBounds.removeFromRight(buttonHeight).reduced(0.1f * buttonHeight));
	}

	void PluginPanel::buttonClicked(juce::Button* button) {
		if (button == &undoButton) {
			undoManager.undo();
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
			copyAtoBButton.setButtonText(">");
		}
		else if (button == &bButton) {
			presetManager.switchToConfig("B");
			bButton.setToggleState(true, juce::dontSendNotification);
			aButton.setToggleState(false, juce::dontSendNotification);
			copyAtoBButton.setButtonText("<");
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
			m.addItem("Paste", [this] { presetManager.pastePreset(); });

			m.addSeparator();
			
			juce::PopupMenu guiSizesMenu;
			auto choices = guiSize.getAllValueStrings();

			for (auto i = 0; i < choices.size(); i++) {
				guiSizesMenu.addItem(choices[i], !(i == guiSize.getIndex()), (i == guiSize.getIndex()), [this, i] {
					guiSize.operator=(i);
				});
			}
			m.addSubMenu("GUI Size", guiSizesMenu);
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

	void PluginPanel::configureIconButton(juce::Button& button, std::unique_ptr<juce::Drawable> icon) {
		icon->replaceColour(juce::Colours::black, textBaseColour);
		auto normalImage = icon->createCopy();
		auto overImage = normalImage->createCopy();
		overImage->replaceColour(textBaseColour, textBaseColour.darker());
		auto downImage = normalImage->createCopy();
		downImage->replaceColour(textBaseColour, textBaseColour.brighter());
		auto iconButton = dynamic_cast<juce::DrawableButton*>(&button);
		iconButton->setImages(normalImage.get(), overImage.get(), downImage.get());

		button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
		addAndMakeVisible(button);
		button.addListener(this);
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

	void PluginPanel::changeListenerCallback(juce::ChangeBroadcaster* source) {
		if (source == &undoManager) {
			undoButton.setEnabled(undoManager.canUndo());
			redoButton.setEnabled(undoManager.canRedo());
		}
	}


	// =====================================  Meter  ================================================
	
	LevelMeter::LevelMeter(MeterSource& source, float minDb, float maxDb, Meter::Orientation orientation = Meter::Orientation::Free, bool showClipIndicator) : meterSource(source), minDb(minDb), maxDb(maxDb), setupOrientation(orientation), showClipIndicator(showClipIndicator)
	{
		auto numChannels = meterSource.getNumChannels();
		
		for (auto i = 0; i < numChannels; i++) {
			meterBars.add(new MeterBar());
			addAndMakeVisible(meterBars.getLast());
			
			clipIndicators.add(new ClipIndicator());
			if (showClipIndicator) {
				addAndMakeVisible(clipIndicators.getLast());
			}
		}
	}
    
	void LevelMeter::resized() {
		auto bounds = getLocalBounds();
		orientationToUse = orientation == Free ? bounds.getWidth() > bounds.getHeight() ? Horizontal : Vertical : setupOrientation;

		auto clipIndicatorProportion = showClipIndicator ? 0.1f : 0.0f;
		auto numChannels = meterSource.getNumChannels();

		switch (orientationToUse) {
		case Vertical:
			auto meterWidth = bounds.getWidth() / meterBars.size();

			for (auto i = 0; i < numChannels; i++) {
				auto meterBar = meterBars[i];
				auto clipIndicator = clipIndicators[i];
				auto meterBarBounds = bounds.removeFromLeft(meterWidth);
				if (showClipIndicator) {
					clipIndicator.setBounds(meterBarBounds.removeFromTop(meterBarBounds.getHeight() * clipIndicatorProportion));
				}
				meterBar.setBounds(meterBarBounds);
			}

			break;

		case Horizontal:
	        auto meterHeight = bounds.getHeight() / meterBars.size();

            for (auto i = 0; i < numChannels; i++) {
                auto meterBar = meterBars[i];
                auto clipIndicator = clipIndicators[i];
                auto meterBarBounds = bounds.removeFromTop(meterHeight);
                if (showClipIndicator)
                {
                    clipIndicator.setBounds(meterBarBounds.removeFromRight(meterBarBounds.getWidth() * clipIndicatorProportion));
                }
                meterBar.setBounds(meterBarBounds);
            }

			break;
		}
	}

	void LevelMeter::update() {
		auto numChannels = meterSource.getNumChannels();

		for (auto i = 0; i < numChannels; i++) {
			auto rms = meterSource.getRMS(i);
			auto meterBar = meterBars[i];
			auto clipIndicator = clipIndicators[i];
			meterBar.setBarFill(juce::jmap(rms, minDb, maxDb, 0.0f, 1.0f));
		}
	}

	// =====================================  MeterBar  ================================================

	LevelMeter::MeterBar::MeterBar(Meter::MeterColours colours, Orientation Orientation, warningThreshold = 1.0f, float clipThreshold = 1.0f) : colours(colours), orientation(orientation) {
		setRanges(warningThreshold, clipThreshold);
	}

	LevelMeter::MeterBar::~MeterBar() {}

	LevelMeter::MeterBar::setOrientation(Orientation orientation) {
		this->orientation = orientation;
		repaint();
	}

	LevelMeter::MeterBar::setColours(Meter::MeterColours colours) {
		this->colours = colours;
		repaint();
	}

	LevelMeter::MeterBar::setRanges(float warningThreshold, float clipThreshold) {
		this->warningThreshold = warningThreshold;
		this->clipThreshold = clipThreshold;
	}

	LevelMeter::MeterBar::setBarFill(float fillAmount) {
		fill = fillAmount;
		repaint();
	}

	LevelMeter::MeterBar::paint(juce::Graphics& g) {
		auto bounds = getLocalBounds();
		auto fillBounds = bounds;

		switch (orientation) {
		case Vertical:
			fillBounds.removeFromTop(fillBounds.getHeight() * (1.0f - fill));
			auto normalBounds = fillBounds.removeFromBottom(fillBounds.getHeight() * warningThreshold);
			auto warningBounds = fillBounds.removeFromBottom(fillBounds.getHeight() * (clipThreshold - warningThreshold));
			auto clipBounds = fillBounds;
			break;
		case Horizontal:
			fillBounds.removeFromRight(fillBounds.getWidth() * (1.0f - fill));
			auto normalBounds = fillBounds.removeFromLeft(fillBounds.getWidth() * warningThreshold);
			auto warningBounds = fillBounds.removeFromLeft(fillBounds.getWidth() * (clipThreshold - warningThreshold));
			auto clipBounds = fillBounds;
			break;
		}

		g.setColour(colours.backgroundColour);
		g.fillRect(bounds);
		g.setColour(colours.fillColour);
		g.fillRect(normalBounds);
		g.setColour(colours.warningColour);
		g.fillRect(warningBounds);
		g.setColour(colours.clipColour);
		g.fillRect(clipBounds);
	}

	// =====================================  ClipIndicator  ================================================

	LevelMeter::ClipIndicator::ClipIndicator(juce::Colour colour) : colour(colour) {}

	LevelMeter::ClipIndicator::~ClipIndicator() {}

	LevelMeter::ClipIndicator::paint(juce::Graphics& g) {
		auto bounds = getLocalBounds();
		g.setColour(colour);
		g.fillRect(bounds);
	}

	LevelMeter::ClipIndicator::setClipped(bool clipped) {
		setVisible(clipped);
	}

	LevelMeter::ClipIndicator::mouseDown(const MouseEvent& event) {
		setClipped(false);
	}

}
