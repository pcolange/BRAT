/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RATAudioProcessorEditor::RATAudioProcessorEditor (RATAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), parameters(vts), LookAndFeel_V4()
{
    
    setSize (400, 500);
   
    // Defines the sliders used for knobs
    distSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    distSlider.setLookAndFeel(&knobLookAndFeel);
    distSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    distSlider.setRange(0.001, 1.0);
    distSlider.setValue(0.5);
    distSlider.setPopupDisplayEnabled(true, false, this);
    distSlider.setDoubleClickReturnValue(true, 0.5);
    distSlider.onValueChange = [this] {
        audioProcessor.updateFilterState();
    };

    filterSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    filterSlider.setLookAndFeel(&knobLookAndFeel);
    filterSlider.setTextValueSuffix("");
    filterSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    filterSlider.setRange(0.001, 1.0);
    filterSlider.setValue(0.5);
    filterSlider.setPopupDisplayEnabled(true, false, this); 
    filterSlider.setDoubleClickReturnValue(true, 0.5);
    filterSlider.onValueChange = [this] {
        audioProcessor.updateFilterState();
    };

    volSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    volSlider.setLookAndFeel(&knobLookAndFeel);
    volSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    volSlider.setRange(0.001, 1.0);
    volSlider.setValue(0.5);
    volSlider.setPopupDisplayEnabled(true, false, this); 
    volSlider.setDoubleClickReturnValue(true, 0.5);
    volSlider.onValueChange = [this] {
        audioProcessor.updateFilterState();
    };

    // Labels for knob discription
    distLabel.setText("DISTORTION", juce::NotificationType::dontSendNotification);
    distLabel.setFont(Font("Segoe UI", 20.0f, 1));
    distLabel.setColour(Label::ColourIds::textColourId, Colours::white);
    distLabel.setJustificationType(Justification::centred);

    filterLabel.setText("FILTER", juce::NotificationType::dontSendNotification);
    filterLabel.setFont(Font("Segoe UI", 20.0f, 1));
    filterLabel.setColour(Label::ColourIds::textColourId, Colours::white);
    filterLabel.setJustificationType(Justification::centred);

    volLabel.setText("VOLUME", juce::NotificationType::dontSendNotification);
    volLabel.setFont(Font("Segoe UI", 20.0f, 1));
    volLabel.setColour(Label::ColourIds::textColourId, Colours::white);
    volLabel.setJustificationType(Justification::centred);

    // Attaches slider to value treee state 
    filterAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(parameters, "filter", filterSlider));
    distAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(parameters, "dist", distSlider));
    volAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(parameters, "volume", volSlider));

    addAndMakeVisible(distSlider);
    addAndMakeVisible(distLabel);
    addAndMakeVisible(distValueLabel);
    addAndMakeVisible(filterSlider);
    addAndMakeVisible(filterLabel);
    addAndMakeVisible(filterValueLabel);
    addAndMakeVisible(volSlider);
    addAndMakeVisible(volLabel);
    addAndMakeVisible(volValueLabel);
}

RATAudioProcessorEditor::~RATAudioProcessorEditor()
{
}

//==============================================================================
void RATAudioProcessorEditor::paint (juce::Graphics& g)
{
    Font logo_font = Font("Saira", 118.0f, Font::plain);
    logo_font.setExtraKerningFactor(0.10f);
    String NAME("BRAT");
    
	File desktop = File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory);
	File textureImageFile = desktop.getFullPathName() + "/JUCEProjects/BRAT/Media/MetalTexture.png";
	Image bgImg = ImageCache::getFromFile(textureImageFile);
    

	g.fillAll(juce::Colour::fromRGB(34, 34, 34));

    Rectangle<float> targetArea(getWidth(), getHeight());
    g.setOpacity(0.25f);
    g.drawImage(bgImg, targetArea);

    g.setColour(juce::Colours::white);

    g.setFont(logo_font);
    float border_w = getWidth() * 1.0f;
    float border_h = getHeight() * 1.0f;
    float border_x = 0.0f;
    float border_y = 0.0f;
    Rectangle<float> border(border_x, border_y, border_w, border_h);

    float text_width = logo_font.getStringWidthFloat(NAME);
    float ts_x = (1.0f * getWidth() - text_width) / 2.0f;

    float text_area_w = getWidth() * 0.80f;
    float text_area_h = getHeight() * 0.15f;
    float text_area_y = getHeight() * 0.5f + 20;

    Rectangle<float> logo_border(ts_x-2.5f, text_area_y, text_width, text_area_h);

    g.setOpacity(0.9f);
	g.drawRoundedRectangle(border, 10.0f, 15.0f);
    g.setColour(Colours::white);
    g.drawText(NAME, ts_x+2.5f, text_area_y, text_area_w, text_area_h, Justification::left);
    g.drawRect(logo_border, 3.0f);
}

void RATAudioProcessorEditor::resized()
{
    auto r = getLocalBounds();

    int knob_y_offset = 175;
    int knob_x_outer = 25;

    distSlider.setSize(100, 100);
    distSlider.setCentrePosition(r.getWidth() / 4 - knob_x_outer, knob_y_offset);
    distSlider.setColour(Slider::ColourIds::backgroundColourId, Colours::red);

    filterSlider.setSize(100, 100);
    filterSlider.setCentrePosition(r.getWidth() / 2, knob_y_offset);
    
    volSlider.setSize(100, 100);
    volSlider.setCentrePosition(r.getWidth() * 3 / 4 + knob_x_outer, knob_y_offset);
    
    distLabel.setSize(100, 50);
    distLabel.setCentrePosition(distSlider.getX() + distSlider.getWidth() / 2,
        distSlider.getY() - 25);

    filterLabel.setSize(100, 50);
    filterLabel.setCentrePosition(filterSlider.getX() + filterSlider.getWidth() / 2,
        filterSlider.getY() - 25);

    volLabel.setSize(100, 50);
    volLabel.setCentrePosition(volSlider.getX() + volSlider.getWidth() / 2,
        volSlider.getY() - 25);

}
