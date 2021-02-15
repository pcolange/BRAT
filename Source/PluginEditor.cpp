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
    
    setSize (650, 600);
    
    addAndMakeVisible(dist_slider);
    dist_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    dist_slider.setLookAndFeel(&knobLookAndFeel);
    dist_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    dist_slider.setRange(0.001, 1.0);
    dist_slider.setValue(0.5);
    dist_slider.setDoubleClickReturnValue(true, 0.5);
    dist_slider.onValueChange = [this] {
        dist_value_label.setText(String::toDecimalStringWithSignificantFigures(dist_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };

    distAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(parameters, "dist", dist_slider));

    addAndMakeVisible(dist_label);
    dist_label.setText("DISTORTION", juce::NotificationType::dontSendNotification);
    dist_label.setFont(Font("Segoe UI", 28.0f, 1));
    dist_label.setColour(Label::ColourIds::textColourId, Colours::white);
    dist_label.setJustificationType(Justification::centred);

    addAndMakeVisible(dist_value_label);
    dist_value_label.setText(String::toDecimalStringWithSignificantFigures(dist_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
    dist_value_label.setFont(Font("Segoe UI", 28.0f, 1));
    dist_value_label.setColour(Label::ColourIds::textColourId, Colours::white);
    dist_value_label.setJustificationType(Justification::centred);

    addAndMakeVisible(filter_slider);
    filter_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    filter_slider.setLookAndFeel(&knobLookAndFeel);
    filter_slider.setTextValueSuffix("");
    filter_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    filter_slider.setRange(0.001, 1.0);
    filter_slider.setValue(0.5);
    filter_slider.setDoubleClickReturnValue(true, 0.5);
    filter_slider.onValueChange = [this] {
        filter_value_label.setText(String::toDecimalStringWithSignificantFigures(filter_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };

    filterAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(parameters, "filter", filter_slider));

    addAndMakeVisible(filter_label);
    filter_label.setText("FILTER", juce::NotificationType::dontSendNotification);
    filter_label.setFont(Font("Segoe UI", 28.0f, 1));
    filter_label.setColour(Label::ColourIds::textColourId, Colours::white);
    filter_label.setJustificationType(Justification::centred);

    addAndMakeVisible(filter_value_label);
    filter_value_label.setText(String::toDecimalStringWithSignificantFigures(filter_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
    filter_value_label.setFont(Font("Segoe UI", 18.0f, 1));
    filter_value_label.setColour(Label::ColourIds::textColourId, Colours::white);
    filter_value_label.setJustificationType(Justification::centred);

    addAndMakeVisible(vol_slider);
    vol_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    vol_slider.setLookAndFeel(&knobLookAndFeel);
    vol_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    vol_slider.setRange(0.001, 1.0);
    vol_slider.setValue(0.5);
    vol_slider.setDoubleClickReturnValue(true, 0.5);
    vol_slider.onValueChange = [this] {
        vol_value_label.setText(String::toDecimalStringWithSignificantFigures(vol_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };

    volAttachment.reset(new AudioProcessorValueTreeState::SliderAttachment(parameters, "volume", vol_slider));

    addAndMakeVisible(vol_label);
    vol_label.setText("VOLUME", juce::NotificationType::dontSendNotification);
    vol_label.setFont(Font("Segoe UI", 28.0f, 1));
    vol_label.setColour(Label::ColourIds::textColourId, Colours::white);
    vol_label.setJustificationType(Justification::centred);

    addAndMakeVisible(vol_value_label);
    vol_value_label.setText(String::toDecimalStringWithSignificantFigures(vol_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
    vol_value_label.setFont(Font("Segoe UI", 20.0f, 1));
    vol_value_label.setColour(Label::ColourIds::textColourId, Colours::white);
    vol_value_label.setJustificationType(Justification::centred);

}

RATAudioProcessorEditor::~RATAudioProcessorEditor()
{
}

//==============================================================================
void RATAudioProcessorEditor::paint (juce::Graphics& g)
{
    Font logo_font = Font("Saira", 164.0f, Font::plain);
    logo_font.setExtraKerningFactor(0.10f);
    String NAME("RAT");

	g.fillAll(juce::Colour::fromRGB(34, 34, 34));

    g.setColour(juce::Colours::white);

    g.setFont(logo_font);
    float border_w = getWidth() * 1.0f;
    float border_h = getHeight() * 1.0f;
    float border_x = 0;
    float border_y = 0;
    Rectangle<float> border(border_x, border_y, border_w, border_h);

    float text_width = logo_font.getStringWidthFloat(NAME);
    float ts_x = (getWidth() - text_width) / 2.0f;
    float nine_x = ts_x + logo_font.getStringWidthFloat(NAME);

    float text_area_w = getWidth() * 0.80f;
    float text_area_h = getHeight() * 0.25f;
//    float text_area_y = getHeight() * 0.025f;
    float text_area_y = getHeight() * 0.5f;

    Rectangle<float> logo_border(ts_x-20.0f, text_area_y, text_width+20.0f, text_area_h);

    g.setOpacity(0.9f);
	g.drawRoundedRectangle(border, 10.0f, 15.0f);
    g.setColour(Colours::white);
    g.drawText(NAME, ts_x, text_area_y, text_area_w, text_area_h, Justification::left);
    g.drawRect(logo_border, 5.0f);
}

void RATAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto r = getLocalBounds();

    int knob_y_offset = 175;
    int knob_x_outer = 25;

    dist_slider.setSize(150, 150);
    dist_slider.setCentrePosition(r.getWidth() / 4 - knob_x_outer, knob_y_offset);

    dist_label.setSize(150, 50);
    dist_label.setCentrePosition(dist_slider.getX() + dist_slider.getWidth() / 2,
        dist_slider.getY() - 25);

    /*dist_value_label.setSize(50, 50);
    dist_value_label.setCentrePosition(dist_slider.getX() + dist_slider.getWidth() / 2,
        dist_slider.getY() - 10);*/

    filter_slider.setSize(150, 150);
    filter_slider.setCentrePosition(r.getWidth() / 2, knob_y_offset);

    filter_label.setSize(100, 50);
    filter_label.setCentrePosition(filter_slider.getX() + filter_slider.getWidth() / 2,
        filter_slider.getY() - 25);

    /*filter_value_label.setSize(50, 50);
    filter_value_label.setCentrePosition(filter_slider.getX() + filter_slider.getWidth() / 2,
        filter_slider.getY() - 10);*/

    vol_slider.setSize(150, 150);
    vol_slider.setCentrePosition(r.getWidth() * 3 / 4 + knob_x_outer, knob_y_offset);

    vol_label.setSize(100, 50);
    vol_label.setCentrePosition(vol_slider.getX() + vol_slider.getWidth() / 2,
        vol_slider.getY() - 25);

  /*  vol_value_label.setSize(50, 50);
    vol_value_label.setCentrePosition(vol_slider.getX() + vol_slider.getWidth() / 2,
        vol_slider.getY() - 10);*/
}
