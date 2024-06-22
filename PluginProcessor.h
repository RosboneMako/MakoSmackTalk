/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class MakoBiteAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MakoBiteAudioProcessor();
    ~MakoBiteAudioProcessor() override;

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

    //R1.00 Add a Parameters variable.
    juce::AudioProcessorValueTreeState parameters;                           
    
    //R1.00 Our settings variables. These must match the KNOB controls in the editor. 
    //R1.00 The e_ enum is used to keep them synced.
    int SettingsChanged = 0;
    int SettingsType = 0;
    float Setting[10] = {};
    float Setting_Last[10] = {};

    //R1.00 Our public variables.
    float Pedal_NGate_Fac[2] = {};    //R1.00 Noise Gate.
    float Signal_AVG[2] = {};       

    //R1.00 These are the indexes into our Settings var.
    enum { e_Gain, e_NGate, e_Sense, e_Q, e_Mix, e_Mode, e_Mono, };


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MakoBiteAudioProcessor)
   
    //R1.00 Clean up the parameter reading code.
    int Mako_GetParmValue_int(juce::String Pstring);
    float Mako_GetParmValue_float(juce::String Pstring);

    //R1.00 Handle parameter changes made in editor.
    void Mako_Settings_Update(bool ForceAll);

    //R1.00 Our actual AUDIO adjusting functions.
    float Mako_FX_NoiseGate(float tSample, int channel);
    float Mako_FX_AutoWah(float tSample, int channel);
    float Mako_FX_SynthDrive(float tSample, int channel);
    
    //R1.00 Some Constants and vars.
    const float pi = 3.14159265f;
    const float pi2 = 6.2831853f;
    const float sqrt2 = 1.4142135f;
    float SampleRate = 48000.0f;

    //R1.00 OUR FILTER VARIABLES
    struct tp_coeffs {
        float a0;
        float a1;
        float a2;
        float b1;
        float b2;
        float c0;
        float d0;
    };

    struct tp_filter {
        float a0;
        float a1;
        float a2;
        float b1;
        float b2;
        float c0;
        float d0;
        float xn0[2];
        float xn1[2];
        float xn2[2];
        float yn1[2];
        float yn2[2];
        float offset[2];
    };

    //R1.00 FILTER FUNCTIONS
    float Filter_Calc_BiQuad(float tSample, int channel, tp_filter* fn);
    void Filter_BP_Coeffs(float Gain_dB, float Fc, float Q, tp_filter* fn);
    void Filter_LP_Coeffs(float fc, tp_filter* fn);
    void Filter_HP_Coeffs(float fc, tp_filter* fn);    

    //R1.00 Our pedal filters and function def.
    tp_filter makoF_AutoWah = {};
    float Factor_Last = 1.0f;       //R1.00 The last filter calc factor. Track to reduce calculations.
    

};
