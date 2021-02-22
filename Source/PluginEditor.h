/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class KnobLookAndFeel : public LookAndFeel_V4
{
public:
	KnobLookAndFeel() {
		File desktop = File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory);
		
		File knob_bg_file = desktop.getFullPathName() + "/JUCEProjects/BRAT/Media/BRATKnobBG.png";
		knobBgImg = ImageCache::getFromFile(knob_bg_file);
		
		File knob_image_file = desktop.getFullPathName() + "/JUCEProjects/BRAT/Media/BRATKnob.png";
		knobImg = ImageCache::getFromFile(knob_image_file);
	}

	void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
		const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override {

		if (knobImg.isValid())
		{
			const double rotation = (slider.getValue() - slider.getMinimum()) / (slider.getMaximum() - slider.getMinimum());

			const int   frames  = knobImg.getHeight() / knobImg.getWidth();
			const int   frameId = (int)ceil(rotation * ((double)frames - 1.0));
			const float radius  = jmin(width / 2.0f, height / 2.0f);
			const float centerX = x + width * 0.5f;
			const float centerY = y + height * 0.5f;
			const float rx      = centerX - radius - 1.0f;
			const float ry      = centerY - radius;

			float min = -145.0f * MathConstants<float>::pi / 180.0f;
			float max = 145.0f * MathConstants<float>::pi / 180.0f;
			float rad = jmap<float>(static_cast<float>(slider.getValue() / slider.getMaximum()), min, max);

			AffineTransform rotator;

			g.drawImage(knobBgImg,
				(int)rx,
				(int)ry,
				2 * (int)radius,
				2 * (int)radius,
				0,
				frameId * knobImg.getWidth(),
				knobImg.getWidth(),
				knobImg.getWidth());

			g.addTransform(rotator.rotated(rad, (float)(centerX), (float)(centerY)));

			g.drawImage(knobImg,
				(int)rx,
				(int)ry,
				2 * (int)radius,
				2 * (int)radius,
				0,
				frameId * knobImg.getWidth(),
				knobImg.getWidth(),
				knobImg.getWidth());

		}
		else
		{
			static const float textPpercent = 0.35f;
			Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f,
				0.5f * height, width * textPpercent, 0.5f * height);

			g.setColour(Colours::white);

			g.drawFittedText(String("No Image"), text_bounds.getSmallestIntegerContainer(),
				Justification::horizontallyCentred | Justification::centred, 1);
		}
	}

	//==============================================================================
	void drawBubble (Graphics& g, BubbleComponent& comp,
									 const Point<float>& tip, const Rectangle<float>& body)
	{
		Path p;
		
	/*	p.addBubble (body.reduced (0.5f), 
					 body.getUnion (Rectangle<float> (tip.x, tip.y, 1.0f, 1.0f)),
					 tip, 5.0f, jmin (15.0f, body.getWidth() * 0.2f, body.getHeight() * 0.2f));*/
		
		// I think the body has to do where the text is
		int rectWidth = body.getWidth();
		int rectHeight = body.getHeight();
		int rectX = body.getX();
		int rectY = body.getY();
	    
		Rectangle<int> rect(rectX, rectY, rectWidth, rectHeight);		
		
		p.addRoundedRectangle(rect, 1.0f);

		g.setColour (comp.findColour (BubbleComponent::backgroundColourId));
		g.fillPath (p);

		g.setColour (comp.findColour (BubbleComponent::outlineColourId));
		g.strokePath (p, PathStrokeType (1.0f));
	}

	int getSliderPopupPlacement(Slider& s) {
		return BubbleComponent::below;
	}

	Image knobImg;
	Image knobBgImg;

};

//==============================================================================
/**
*/
class RATAudioProcessorEditor  : public juce::AudioProcessorEditor, LookAndFeel_V4
{
public:
    RATAudioProcessorEditor (RATAudioProcessor&, AudioProcessorValueTreeState&);
    ~RATAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
	KnobLookAndFeel knobLookAndFeel{};
    
	Slider distSlider;
    Label distLabel;
    Label distValueLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> distAttachment;

    Slider filterSlider;
    Label filterLabel;
    Label filterValueLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> filterAttachment;

    Slider volSlider;
    Label volLabel;
    Label volValueLabel;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> volAttachment;
    
    AudioProcessorValueTreeState& parameters;
    
	RATAudioProcessor& audioProcessor;
	
	TooltipWindow tooltipWindow;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RATAudioProcessorEditor)
};
