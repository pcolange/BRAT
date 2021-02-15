#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class RATAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RATAudioProcessor();
    ~RATAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void updateFilterState() {
            
        float Fs = current_sample_rate;
        float Fsx2 = 2.0f * Fs;
        float Fs2x4 = 4.0f * powf(Fs, 2);
        float Fs3 = powf(Fs, 3);
		float Rdrive = Rf * (*distParameter);
        
        {

            // 3rd order op-amp feedback
            float bz3 = 8.0f * C1 * C2 * Cf * R1 * R2 * Rdrive * Fs3 + C1 * C2 * R1 * R2 * Fs2x4 + C1 * C2 * R1 * Rdrive * Fs2x4 + C1 * C2 * R2 * Rdrive * Fs2x4 + C1 * Cf * R1 * Rdrive * Fs2x4 + C1 * R1 * Fsx2 + C1 * Rdrive * Fsx2 + C2 * Cf * R2 * Rdrive * Fs2x4 + C2 * R2 * Fsx2 + C2 * Rdrive * Fsx2 + Cf * Rdrive * Fsx2 + 1.0f;
            float bz2 = -24.0f * C1 * C2 * Cf * R1 * R2 * Rdrive * Fs3 - C1 * C2 * R1 * R2 * Fs2x4 - C1 * C2 * R1 * Rdrive * Fs2x4 - C1 * C2 * R2 * Rdrive * Fs2x4 - C1 * Cf * R1 * Rdrive * Fs2x4 + C1 * R1 * Fsx2 + C1 * Rdrive * Fsx2 - C2 * Cf * R2 * Rdrive * Fs2x4 + C2 * R2 * Fsx2 + C2 * Rdrive * Fsx2 + Cf * Rdrive * Fsx2 + 3.0f;
            float bz1 = 24.0f * C1 * C2 * Cf * R1 * R2 * Rdrive * Fs3 - C1 * C2 * R1 * R2 * Fs2x4 - C1 * C2 * R1 * Rdrive * Fs2x4 - C1 * C2 * R2 * Rdrive * Fs2x4 - C1 * Cf * R1 * Rdrive * Fs2x4 - C1 * R1 * Fsx2 - C1 * Rdrive * Fsx2 - C2 * Cf * R2 * Rdrive * Fs2x4 - C2 * R2 * Fsx2 - C2 * Rdrive * Fsx2 - Cf * Rdrive * Fsx2 + 3.0f;
            float bz0 = -8.0f * C1 * C2 * Cf * R1 * R2 * Rdrive * Fs3 + C1 * C2 * R1 * R2 * Fs2x4 + C1 * C2 * R1 * Rdrive * Fs2x4 + C1 * C2 * R2 * Rdrive * Fs2x4 + C1 * Cf * R1 * Rdrive * Fs2x4 - C1 * R1 * Fsx2 - C1 * Rdrive * Fsx2 + C2 * Cf * R2 * Rdrive * Fs2x4 - C2 * R2 * Fsx2 - C2 * Rdrive * Fsx2 - Cf * Rdrive * Fsx2 + 1.0f;

            float az3 = 8.0f * C1 * C2 * Cf * R1 * R2 * Rdrive * Fs3 + C1 * C2 * R1 * R2 * Fs2x4 + C1 * Cf * R1 * Rdrive * Fs2x4 + C1 * R1 * Fsx2 + C2 * Cf * R2 * Rdrive * Fs2x4 + C2 * R2 * Fsx2 + Cf * Rdrive * Fsx2 + 1.0f;
            float az2 = -24.0f * C1 * C2 * Cf * R1 * R2 * Rdrive * Fs3 - C1 * C2 * R1 * R2 * Fs2x4 - C1 * Cf * R1 * Rdrive * Fs2x4 + C1 * R1 * Fsx2 - C2 * Cf * R2 * Rdrive * Fs2x4 + C2 * R2 * Fsx2 + Cf * Rdrive * Fsx2 + 3.0f;
            float az1 = 24.0f * C1 * C2 * Cf * R1 * R2 * Rdrive * Fs3 - C1 * C2 * R1 * R2 * Fs2x4 - C1 * Cf * R1 * Rdrive * Fs2x4 + C1 * R1 * Fsx2 - C2 * Cf * R2 * Rdrive * Fs2x4 - C2 * R2 * Fsx2 - Cf * Rdrive * Fsx2 + 3.0f;
            float az0 = -8.0f * C1 * C2 * Cf * R1 * R2 * Rdrive * Fs3 + C1 * C2 * R1 * R2 * Fs2x4 + C1 * Cf * R1 * Rdrive * Fs2x4 - C1 * R1 * Fsx2 + C2 * Cf * R2 * Rdrive * Fs2x4 - C2 * R2 * Fsx2 - Cf * Rdrive * Fsx2 + 1.0f;

            bz3 /= az3;
            bz2 /= az3;
            bz1 /= az3;
            bz0 /= az3;
            az2 /= az3;
            az1 /= az3;
            az0 /= az3;
            az3 = 1;

            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz3, bz2, bz1, bz0, az3, az2, az1, az0);
            opamp_filter.coefficients = *newCoefficients;
        }
        {
            // Modeling the op amp gain bandwidth as a LPF
            float gain = 1.0f + Rdrive / Rpar;
            float RCgbw = 1.0f / (2.0f * MathConstants<float>::pi * gain_bw / gain);
            float bz1 = 1.0f;
            float bz0 = 1.0f;
            float az1 = 2.0f * RCgbw * Fs + 1.0f;
            float az0 = -2.0f * RCgbw * Fs + 1.0f;
            bz1 /= az1;
            bz0 /= az1;
            az0 /= az1;
            az1 = 1;

            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz1, bz0, az1, az0);
            opamp_gbw.coefficients = *newCoefficients;
        }
        {
            // The filter circuit is yet another LPF 
            float Rtotal = Rfilter * (*filterParameter) + R3;
            float bz1 = 1.0f;
            float bz0 = 1.0f;
            float az1 =  2.0f * Rtotal * Cfilter * Fs + 1.0f;
            float az0 = -2.0f * Rtotal * Cfilter * Fs + 1.0f;
            bz1 /= az1;
            bz0 /= az1;
            az0 /= az1;
            az1 = 1;

            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz1, bz0, az1, az0);
            filter.coefficients = *newCoefficients;
        }

    }

private:
    float current_sample_rate = 441000.0f;

    dsp::Oversampling<float> slew_over_sampler{ 2, 2, dsp::Oversampling<float>::filterHalfBandFIREquiripple };
    dsp::Oversampling<float> diode_over_sampler{ 2, 2, dsp::Oversampling<float>::filterHalfBandFIREquiripple };

    // Opamp parameters
    float gain_bw = 1E9f;
    float vcc = 4.95f;
    float slew_rate = 235E3f;

    // Opamp filter section 
    float R1 = 47;
    float R2 = 560;
    float Rf = 100E3;
    float Rpar = (R1 * R2 / (R1 + R2));
    float C1 = 2.2E-6;
    float C2 = 4.7E-6;
    float Cf = 100E-12;

    // Filter section
    float Rfilter = 100.0E3f;
    float R3 = 1.5E3f;
    float Cfilter = 3.3E-9f;

    // Diode parameters
    float Is = 1E-14f;
    float nvt = 26.E-3f;

    std::atomic<float>* distParameter = nullptr;
    std::atomic<float>* filterParameter = nullptr;
    std::atomic<float>* volParameter = nullptr;
    
    dsp::IIR::Filter<float> opamp_filter;
    dsp::IIR::Filter<float> opamp_gbw;
    dsp::IIR::Filter<float> filter;

    AudioProcessorValueTreeState parameters;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RATAudioProcessor)
};
