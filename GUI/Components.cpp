
#include "Components.h"
#include "../../ParameterIDs.h"

namespace MyJUCEModules {
	
	// =====================================  ArrowButton  ================================================

	ArrowButton::ArrowButton(const juce::String& buttonName, float arrowDirection, juce::Colour arrowColour) :
		Button(buttonName), colour(arrowColour)
	{
		path.addTriangle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
		path.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::twoPi * arrowDirection, 0.5f, 0.5f));
	}

	ArrowButton::~ArrowButton() {}

	void ArrowButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
	{
		juce::Path p(path);
		juce::Colour colourToUse;

		if (shouldDrawButtonAsDown)
			colourToUse = colour.brighter();
		else if (shouldDrawButtonAsHighlighted)
			colourToUse = colour.darker();
		else
			colourToUse = colour;

		p.applyTransform(path.getTransformToScaleToFit(0.0f, 0.0f, (float)getWidth(), (float)getHeight(), false));

		g.setColour(colourToUse);
		if (!this->isEnabled())
			g.setOpacity(0.5f);
		g.fillPath(p);
	}

	// =====================================  MyTextButton  ================================================

	void MyTextButton::paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) {
		juce::Colour colourToUse = colour;
		auto toggleState = getToggleState();

		if (isMouseOverButton)
			colourToUse = colour.darker();

		g.setColour(colourToUse.withMultipliedAlpha(this->isEnabled() ? 1.0f : 0.5f));

		juce::Font fontTouUSe = font;
		if (isButtonDown || toggleState){
			fontTouUSe = font.boldened();
			g.setFont(fontTouUSe.withHeight(getHeight() * 0.7f));
		}
		else
			g.setFont(fontTouUSe.withHeight(getHeight() * 0.7f));
		

		juce::String buttonText = getButtonText();

		if (buttonText.length() > 0)
			g.drawText(buttonText, getLocalBounds(), juce::Justification::centred);
	}


    // =====================================  PluginPanel  ================================================

	PluginPanel::PluginPanel(PresetManager& pm, juce::UndoManager& uM, juce::AudioProcessorValueTreeState& apvts):
		presetManager(pm), undoManager(uM), pluginApvts(apvts),
		previousPresetButton("Previous", 0.5f, juce::Colours::gainsboro.darker().darker().darker().darker()),
		nextPresetButton("Next", 1.0f, juce::Colours::gainsboro.darker().darker().darker().darker())
	{
		undoManager.addChangeListener(this);
		tooltipWindow->setLookAndFeel(&lookAndFeel);

		undoIcon = juce::Drawable::createFromImageData(BinaryData::arrowgobackline_svg, BinaryData::arrowgobackline_svgSize);
		configureIconButton(undoButton, undoIcon->createCopy());
		undoButton.setTooltip("Undo");
		undoButton.setEnabled(false);
		redoIcon = juce::Drawable::createFromImageData(BinaryData::arrowgoforwardline_svg, BinaryData::arrowgobackline_svgSize);
		configureIconButton(redoButton, redoIcon->createCopy());
		redoButton.setEnabled(false);
		redoButton.setTooltip("Redo");

		copyIcon = juce::Drawable::createFromImageData(BinaryData::filecopyline_svg, BinaryData::filecopyline_svgSize);
		configureIconButton(copyButton, copyIcon->createCopy());
		copyButton.setTooltip("Copy current configuration to clipboard");

		oversamplingIcon = juce::Drawable::createFromImageData(BinaryData::hqline_svg, BinaryData::hqline_svgSize);
		configureIconButton(oversamplingButton, oversamplingIcon->createCopy());
		oversamplingButton.setTooltip("Configure oversampling");
		
		configureArrowButton(previousPresetButton);
		previousPresetButton.setTooltip("Previous preset");
		configureComboBox(presetComboBox, "No preset");
		presetComboBox.setTooltip("Select a preset");
		configureArrowButton(nextPresetButton);
		nextPresetButton.setTooltip("Next preset");
		
		optionsIcon = juce::Drawable::createFromImageData(BinaryData::menuline_svg, BinaryData::menuline_svgSize);
		configureIconButton(optionsButton, optionsIcon->createCopy());
		optionsButton.setTooltip("More...");
		
		configureTextButton(aButton, "A");
		aButton.setClickingTogglesState(true);
		aButton.setToggleState(true, juce::dontSendNotification);
		aButton.setTooltip("Switch to configuration A");
		configureTextButton(copyAtoBButton, ">");
		copyAtoBButton.setTooltip("Copy current configuration to B");
		configureTextButton(bButton, "B");
		bButton.setClickingTogglesState(true);
		bButton.setTooltip("Switch to configuration B");
		
		bypassIcon= juce::Drawable::createFromImageData(BinaryData::shutdownline_svg, BinaryData::shutdownline_svgSize);
		
		bypassIcon->replaceColour(juce::Colours::black, textBaseColour);
		auto normalImage = bypassIcon->createCopy();
		auto overImage = normalImage->createCopy();
		overImage->replaceColour(textBaseColour, textBaseColour.darker());
		auto downImage = normalImage->createCopy();
		downImage->replaceColour(textBaseColour, textBaseColour.brighter());
		auto normalImageDown = normalImage->createCopy();
		normalImageDown->replaceColour(textBaseColour, textBaseColour.brighter());
		bypassButton.setImages(normalImage.get(), overImage.get(), downImage.get(), nullptr, normalImageDown.get(), normalImageDown.get(), normalImageDown.get(), nullptr);
		bypassButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::gainsboro.darker());
		bypassButton.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::gainsboro.darker());
		bypassButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
		addAndMakeVisible(bypassButton);
		bypassButton.setClickingTogglesState(true);
		bypassButton.setTooltip("Toggle plugin bypass");
		bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(pluginApvts, g_bypassID, bypassButton);

		const auto allPresets = presetManager.getAllPresets();
		const auto currentPreset = presetManager.getCurrentPresetName();
		presetComboBox.setJustificationType(juce::Justification::centred);
		presetComboBox.addItemList(allPresets, 1);
		presetComboBox.setSelectedItemIndex(allPresets.indexOf(currentPreset), juce::dontSendNotification);

		presetManager.copyCurrentConfigToOther();
	}

	PluginPanel::~PluginPanel() {
		undoManager.removeChangeListener(this);

		tooltipWindow->setLookAndFeel(nullptr);

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

		juce::Font f1 = font.withHeight(bounds.getHeight() * 0.6f);
		auto pluginNameBounds = bounds.removeFromLeft(f1.getStringWidth(pluginName));
		g.setFont(f1);
		g.drawText(pluginName, pluginNameBounds, juce::Justification::left);
		juce::Font f2 {f1.getHeight() * 0.75f};
		auto pluginVersionBounds = bounds.removeFromLeft(f2.getStringWidth(pluginVersion));
		g.setFont(f2);
		g.drawText(pluginVersion, pluginVersionBounds, juce::Justification::left);

		// draw a line all over the edges of the component
		g.setColour(textBaseColour.brighter());
		g.fillRect(0.0f, 0.0f, (float)getWidth(), 2.0f);
		g.fillRect(0.0f, 0.0f, 2.0f, (float)getHeight());
		g.fillRect(0.0f, (float)getHeight() - 2.0f, (float)getWidth(), 2.0f);
		g.fillRect((float)getWidth() - 2.0f, 0.0f, 2.0f, (float)getHeight());

		g.setColour(textBaseColour.darker());
		g.drawVerticalLine(0, 0.0f, (float)getHeight());
		g.drawVerticalLine(getWidth() - 1, 0.0f, (float)getHeight());
		g.drawHorizontalLine(getHeight() - 1, 0.0f, (float)getWidth());
		g.drawHorizontalLine(0, 0.0f, (float)getWidth());
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

		previousPresetButton.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(.7f * buttonHeight).reduced(0.23f * buttonHeight));
		presetComboBox.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(10.1f * buttonHeight).reduced(0.08f * buttonHeight));
		nextPresetButton.setBounds(presetComboBoxAndArrowsBounds.removeFromLeft(.7f * buttonHeight).reduced(0.23f * buttonHeight));

		copyButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.075f * buttonHeight));
		
		leftSideBounds.removeFromRight(2.f * buttonHeight);
		
		redoButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.075f * buttonHeight));
		leftSideBounds.removeFromRight(.25f * buttonHeight);
		undoButton.setBounds(leftSideBounds.removeFromRight(1.f * buttonHeight).reduced(0.075f * buttonHeight));

		optionsButton.setBounds(rightSideBounds.removeFromLeft(1.f * buttonHeight).reduced(0.15f * buttonHeight));
		
		rightSideBounds.removeFromLeft(2.f * buttonHeight);
		
		aButton.setBounds(rightSideBounds.removeFromLeft(.75f * buttonHeight));
		copyAtoBButton.setBounds(rightSideBounds.removeFromLeft(.6f * buttonHeight));
		bButton.setBounds(rightSideBounds.removeFromLeft(.75f * buttonHeight));

		rightSideBounds.removeFromLeft(3.f * buttonHeight);

		oversamplingButton.setBounds(rightSideBounds.removeFromLeft(1.f * buttonHeight).reduced(0.075f * buttonHeight));
		
		rightSideBounds.removeFromRight(0.1f * buttonHeight);
		bypassButton.setBounds(rightSideBounds.removeFromRight(buttonHeight).reduced(0.075f * buttonHeight));
	}

	void PluginPanel::buttonClicked(juce::Button* button) {
		if (button == &undoButton) {
			undoManager.undo();
		}
		else if (button == &redoButton) {
			undoManager.redo();
		}
		else if (button == &copyButton) {
			presetManager.copyPreset();
		}
		else if (button == &previousPresetButton) {
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
			copyAtoBButton.setTooltip("Copy current configuration to B");
		}
		else if (button == &bButton) {
			presetManager.switchToConfig("B");
			bButton.setToggleState(true, juce::dontSendNotification);
			aButton.setToggleState(false, juce::dontSendNotification);
			copyAtoBButton.setButtonText("<");
			copyAtoBButton.setTooltip("Copy current configuration to A");
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
			auto clipboardText = juce::SystemClipboard::getTextFromClipboard();
			bool isValid = false;
			if (auto xml = juce::parseXML(clipboardText)) {
				if (xml->getStringAttribute("pluginName") == JucePlugin_Name) {
					isValid = true;
				}
			}
			m.addItem("Paste", isValid, false, [this] { presetManager.pastePreset(); });

			m.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(&optionsButton));
			m.setLookAndFeel(nullptr);
		}
		else if (button == &oversamplingButton) {
			juce::PopupMenu m;
			auto oversampling = dynamic_cast<juce::AudioParameterChoice*>(pluginApvts.getParameter(g_osFactorID));
			auto choices = oversampling->getAllValueStrings();
			for (auto i = 0; i < choices.size(); i++) {
				m.addItem((choices[i] == "x1" ? "No" : choices[i]) + " oversampling", !(i == oversampling->getIndex()), (i == oversampling->getIndex()), [this, i, oversampling] {
					oversampling->operator=(i);
				});
			}
			m.setLookAndFeel(&lookAndFeel);

			m.showMenuAsync(juce::PopupMenu::Options{}.withTargetComponent(&oversamplingButton));
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

	void PluginPanel::configureTextButton(MyJUCEModules::MyTextButton& button, const juce::String& buttonText) {
		button.setButtonText(buttonText);
		button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
		button.setFont(font);
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


	// =====================================  LEVEL METER COMPONENT ================================================
	
	LevelMeter::LevelMeter(MeterSource& source, MeterSpecs meterSpecs = MeterSpecs()) : meterSource(source), meterSpecs(meterSpecs)
	{
		auto numChannels = meterSource.getNumChannels();
		
		jassert(numChannels > 0);

		for (auto i = 0; i < numChannels; i++) {
			meterBars.add(new MeterBar(this->meterSpecs, colours));
			addAndMakeVisible(meterBars.getLast());

			barScales.add(new BarScale(this->meterSpecs, colours));
			if (meterSpecs.showScale && meterSpecs.scaleInMeterBar) {
				addAndMakeVisible(barScales.getLast());
			}
			
			clipIndicators.add(new ClipIndicator(this->meterSpecs, colours.clipColour));
			if (meterSpecs.showClipIndicator) {
				addAndMakeVisible(clipIndicators.getLast());
				//addChildComponent(clipIndicators.getLast());
			}
		}
		if (meterSpecs.showScale) {
			meterScale = std::make_unique<MeterScale>(this->meterSpecs, colours);
			//addAndMakeVisible(meterScale.get());
		}
	}
    
	void LevelMeter::resized() {
		auto bounds = getLocalBounds();

		auto clipIndicatorProportion = meterSpecs.showClipIndicator ? meterSpecs.layout.clipIndicatorProportion : 0.0f;

		orientationToUse = meterSpecs.orientation == Free ? bounds.getWidth() > bounds.getHeight() ? Horizontal : Vertical : meterSpecs.orientation;
		numChannels = meterBars.size();

		jassert(numChannels > 0);
		
		switch (orientationToUse) {
			case Vertical:
			{
				juce::Rectangle<int> clipIndicatorBounds;

				if (meterSpecs.showScale) {
					meterScale->setBounds(bounds);
					auto scaleWidth = bounds.getWidth() * meterSpecs.layout.scaleProportion;
					auto paddingWidth = bounds.getWidth() * meterSpecs.layout.barToScalePaddingProportion;
					auto scaleBounds = bounds.removeFromRight(scaleWidth);
					scaleBounds.removeFromLeft(paddingWidth);
				}

				if (meterSpecs.showClipIndicator) {
					clipIndicatorBounds = bounds.removeFromTop(bounds.getHeight() * clipIndicatorProportion);
				}

				auto meterWidth = bounds.getWidth() / numChannels;
				for (auto ch = 0; ch < numChannels; ch++) {
					auto meterBar = meterBars[ch];
					auto clipIndicator = clipIndicators[ch];
					auto meterBarBounds = bounds.removeFromLeft(meterWidth);
					if (meterSpecs.showClipIndicator) {
						auto paddedClipIndicatorBounds = clipIndicatorBounds.removeFromLeft(meterWidth).reduced(getHeight() * meterSpecs.layout.barPaddingProportion);
						clipIndicator->setBounds(paddedClipIndicatorBounds);
					}
					meterBar->setOrientation(orientationToUse);
					auto paddedMeterBarBounds = meterBarBounds.reduced(getHeight() * meterSpecs.layout.barPaddingProportion);
					meterBar->setBounds(paddedMeterBarBounds);
					barScales[ch]->setBounds(paddedMeterBarBounds);
				}

				break;
			}
			case Horizontal:
			{
				auto meterHeight = bounds.getHeight() / meterBars.size();
				for (auto ch = 0; ch < numChannels; ch++) {
					auto meterBar = meterBars[ch];
					auto clipIndicator = clipIndicators[ch];
					auto meterBarBounds = bounds.removeFromTop(meterHeight);
					if (meterSpecs.showClipIndicator)
					{
						clipIndicator->setBounds(meterBarBounds.removeFromRight(meterBarBounds.getWidth() * clipIndicatorProportion));
					}
					meterBar->setOrientation(orientationToUse);
					meterBar->setBounds(meterBarBounds);
				}
				break;
			}
		}
	}

	void LevelMeter::update() {
		auto numChannels = meterSource.getNumChannels();

		for (auto channel = 0; channel < numChannels; channel++) {
			auto dBLevel = jlimit(meterSpecs.meterRange.start, meterSpecs.meterRange.end, juce::Decibels::gainToDecibels(meterSource.getRMS(channel), meterSpecs.meterRange.start));
			auto meterBar = meterBars[channel];
			meterBar->setBarFill(meterSpecs.meterRange.convertTo0to1(dBLevel));
		}
	}

	void LevelMeter::setScaleValues(std::vector<float> scaleValues) {
		meterScale->setScaleValues(scaleValues);
	}

	void LevelMeter::setColours(MeterColours colours) {
		
		for (auto i = 0; i < meterBars.size(); i++) {
			meterBars[i]->setColours(colours);
		}

		repaint();
	}

	// =====================================  MeterBar  ================================================
	
	LevelMeter::MeterBar::MeterBar(MeterSpecs& meterSpecs, MeterColours& colours) : meterSpecs(meterSpecs), colours(colours){
		setSpecs(meterSpecs);
	}

	LevelMeter::MeterBar::~MeterBar() {}

	void LevelMeter::MeterBar::setOrientation(Orientation orientation) {
		meterSpecs.orientation = orientation;
		repaint();
	}

	void LevelMeter::MeterBar::setColours(MeterColours colours) {
		this->colours = colours;
		repaint();
	}

	void LevelMeter::MeterBar::setSpecs(MeterSpecs meterSpecs) {
		this->meterSpecs = meterSpecs;
		warningThresholdLinear = meterSpecs.meterRange.convertTo0to1(meterSpecs.warningThreshold);
		clipThresholdLinear = meterSpecs.meterRange.convertTo0to1(meterSpecs.clipThreshold);
		repaint();
	}

	void LevelMeter::MeterBar::setBarFill(float fillAmount) {
		if (fill != fillAmount) {
			fill = fillAmount;
			repaint();
		}
	}

	void LevelMeter::MeterBar::paint(juce::Graphics& g) {
		auto bounds = getLocalBounds();
		auto fillBounds = bounds;

		juce::Path backgroundPath;
		backgroundPath.addRoundedRectangle(bounds.toFloat(), bounds.getWidth()/8.f);
		g.reduceClipRegion(backgroundPath);

		juce::Rectangle<int> normalBounds, warningBounds, clipBounds;

		switch (meterSpecs.orientation) {
		case Vertical:
			fillBounds.removeFromTop(fillBounds.getHeight() * (1.0f - fill));
			normalBounds = fillBounds.removeFromBottom(bounds.getHeight() * warningThresholdLinear);
			warningBounds = fillBounds.removeFromBottom(bounds.getHeight() * (clipThresholdLinear - warningThresholdLinear));
			clipBounds = fillBounds;
			break;
		case Horizontal:
			fillBounds.removeFromRight(fillBounds.getWidth() * (1.0f - fill));
			normalBounds = fillBounds.removeFromLeft(bounds.getWidth() * warningThresholdLinear);
			warningBounds = fillBounds.removeFromLeft(bounds.getWidth() * (clipThresholdLinear - warningThresholdLinear));
			clipBounds = fillBounds;
			break;
		}

		g.setColour(colours.backgroundColour);
		g.fillPath(backgroundPath);
		g.setColour(colours.normalColour);
		g.fillRect(normalBounds);
		g.setColour(colours.warningColour);
		g.fillRect(warningBounds);
		g.setColour(colours.clipColour);
		g.fillRect(clipBounds);
	}

	// =====================================  ClipIndicator  ================================================

	LevelMeter::ClipIndicator::ClipIndicator(MeterSpecs& meterSpecs, juce::Colour colour) : meterSpecs(meterSpecs), colour(colour) {
		setMouseCursor(juce::MouseCursor::PointingHandCursor);
	}

	LevelMeter::ClipIndicator::~ClipIndicator() { }

	void LevelMeter::ClipIndicator::paint(juce::Graphics& g) {
		auto bounds = getLocalBounds();
		g.setColour(colour);
		g.fillRoundedRectangle(bounds.toFloat(), getWidth()/8.f);
	}

	void LevelMeter::ClipIndicator::setClipped(bool clipped) {
		setVisible(clipped);
	}

	void LevelMeter::ClipIndicator::setColour(juce::Colour colour) {
		this->colour = colour;
		repaint();
	}

	void LevelMeter::ClipIndicator::mouseDown(const MouseEvent& event) {
		setClipped(false);
	}

	// =====================================  MeterScale  ================================================

	LevelMeter::MeterScale::MeterScale(MeterSpecs& meterSpecs, MeterColours& colours) : meterSpecs(meterSpecs), colours(colours) { }

	LevelMeter::MeterScale::~MeterScale() { }

	void LevelMeter::MeterScale::setScaleValues(std::vector<float> scaleValues) {
		this->scaleValues = scaleValues;
		repaint();
	}

	void LevelMeter::MeterScale::paint(juce::Graphics& g) {
		auto yStart = getHeight() * (meterSpecs.layout.clipIndicatorProportion * meterSpecs.showClipIndicator);
		auto padding = getParentHeight() * meterSpecs.layout.barPaddingProportion;

		for (auto i = 0; i < scaleValues.size(); i++) {
			auto value = scaleValues[i];
			auto y = yStart + padding + (getHeight() - (yStart + 2 * padding))  * (1.0f - meterSpecs.meterRange.convertTo0to1(value));
			auto fontSize = getHeight() * meterSpecs.layout.scaleFontSizeProportion;
			g.setColour(colours.scaleColour);
			g.drawHorizontalLine(y, 0.0f, (float)getWidth() * 0.2f);
			g.setFont(fontSize);
			g.drawText(juce::String(abs(value)), getWidth() * 0.4f, y - fontSize * 0.5f, (float)getWidth() * 0.6f, fontSize, juce::Justification::left);
		}
	}

	// =====================================  BarScale  ================================================

	void LevelMeter::BarScale::paint(juce::Graphics& g) {
		
		for (auto i = 1; i < scaleValues.size() - 1; i++) {
			juce::Path path;
			auto bounds = getLocalBounds();
			path.addRoundedRectangle(bounds.toFloat().reduced(1.f), bounds.getWidth() / 8.f);
			auto value = scaleValues[i];
			auto y = getHeight() * (1.0f - meterSpecs.meterRange.convertTo0to1(value));
			g.setColour(colours.scaleColour);
			//g.drawHorizontalLine(y, 0.0f, (float)getWidth());
			g.strokePath(path, juce::PathStrokeType{ 1.f });
			//g.fillPath(path);
		}
	}
}
