/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RATAudioProcessor::RATAudioProcessor()
    : parameters(*this, nullptr, Identifier("RAT"),
        {
            std::make_unique<AudioParameterFloat>("dist",            // parameterID
                                                   "Distortion",            // parameter name
                                                    0.0f,              // minimum value
                                                    1.0f,              // maximum value
                                                    0.5f),             // default value
            std::make_unique<AudioParameterFloat>("filter",            // parameterID
                                                   "Filter",            // parameter name
                                                    0.0f,              // minimum value
                                                    1.0f,              // maximum value
                                                    0.5f),             // default value
            std::make_unique<AudioParameterFloat>("volume",            // parameterID
                                                   "Volume",            // parameter name
                                                    0.0f,              // minimum value
                                                    1.0f,              // maximum value
                                                    0.5f),             // default value
        }),
#ifndef JucePlugin_PreferredChannelConfigurations
        AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    distParameter = parameters.getRawParameterValue("dist");
    filterParameter = parameters.getRawParameterValue("filter");
    volParameter = parameters.getRawParameterValue("volume");
}

RATAudioProcessor::~RATAudioProcessor()
{
}

//==============================================================================
const juce::String RATAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RATAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RATAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RATAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RATAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RATAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RATAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RATAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RATAudioProcessor::getProgramName (int index)
{
    return {};
}

void RATAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RATAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    current_sample_rate = static_cast<float>(sampleRate);
    dsp::ProcessSpec spec{ sampleRate, uint32(samplesPerBlock), uint32(getTotalNumOutputChannels()) };

    opamp_filter.reset();
    opamp_filter.prepare(spec);
    
    opamp_gbw.reset();
    opamp_gbw.prepare(spec);
    
    filter.reset();
    filter.prepare(spec);
    
    updateFilterState();

    slew_over_sampler.reset();
    slew_over_sampler.initProcessing(samplesPerBlock);
    diode_over_sampler.reset();
    diode_over_sampler.initProcessing(samplesPerBlock);
}

void RATAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RATAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RATAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    dsp::AudioBlock<float> audio_block(buffer);
    auto audioBlockChannel = audio_block.getSingleChannelBlock(0);
    dsp::ProcessContextReplacing<float> context(audioBlockChannel);
    opamp_filter.process(context);

    // 
    // Slew Rate Code Will Go Here
    //
	float Rdrive = Rf * (*distParameter);
	float gain = 1.0f + Rdrive / Rpar;
    float dt = 1.0f / (current_sample_rate * slew_over_sampler.getOversamplingFactor());
    float vmax = slew_rate * dt;

    /*auto slew_block = slew_over_sampler.processSamplesUp(audio_block);
    auto slew_block_ptr = slew_block.getChannelPointer(0);
    
    float psample = slew_block_ptr[0] < -vcc ? -vcc : (vcc < slew_block_ptr[0]) ? vcc : slew_block_ptr[0];
    for (int i = 0; i < slew_block.getNumSamples(); ++i) {
        float sample = slew_block_ptr[i] < -vcc ? -vcc : (vcc < slew_block_ptr[i]) ? vcc : slew_block_ptr[i];
		float dv = sample - psample;
        if (dv > vmax) {
            dv = vmax;
        }
        if (dv < -vmax) {
            dv = -vmax;
        }
		psample += dv;
        slew_block_ptr[i] = psample;
	}
    slew_over_sampler.processSamplesDown(audio_block);*/

    // Process the gain bandwidth filter
    opamp_gbw.process(context);

    // Simple diode model
    auto diode_block = diode_over_sampler.processSamplesUp(audio_block);
    auto diode_block_ptr = diode_block.getChannelPointer(0);

    for (int i = 0; i < diode_block.getNumSamples(); ++i) {
        auto U = nvt * asinh(diode_block_ptr[i] / (2.0f * Is * 1.0E3f));
        if (std::fabs(U) > std::fabs(diode_block_ptr[i])) {
            U = diode_block_ptr[i];
        }
        diode_block_ptr[i] = U;
    }

    diode_over_sampler.processSamplesDown(audio_block);

    // Tone circuit
    filter.process(context);

    // Volume
    buffer.applyGain(*volParameter); 
    
    // Apply mono to both channels
    buffer.copyFrom(1, 0, buffer.getReadPointer(0), buffer.getNumSamples());

}

//==============================================================================
bool RATAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* RATAudioProcessor::createEditor()
{
    return new RATAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void RATAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RATAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RATAudioProcessor();
}
