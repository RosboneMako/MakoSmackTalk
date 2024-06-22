/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "cmath"              //R1.00 Added library.

//==============================================================================
MakoBiteAudioProcessor::MakoBiteAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
    ),
    
    
    //R1.00 Define our VALUE TREE parameter variables. Min val, Max Val, default Val.
    parameters(*this, nullptr, "PARAMETERS", 
      {
        std::make_unique<juce::AudioParameterFloat>("gain","Gain", .0f, 4.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("ngate","Noise Gate", .0f, 1.0f, .0f),
        std::make_unique<juce::AudioParameterFloat>("sense","Sense", .0f, 1.0f, .3f),
        std::make_unique<juce::AudioParameterFloat>("q","Q", .0f, 1.0f, .5f),
        std::make_unique<juce::AudioParameterFloat>("mix","Mix", .0f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterInt>("mode","Mode", 0, 1, 1),
        std::make_unique<juce::AudioParameterInt>("mono","Mono", 0, 1, 1),        
      }
    )   

#endif
{   
}

MakoBiteAudioProcessor::~MakoBiteAudioProcessor()
{
}

//==============================================================================
const juce::String MakoBiteAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MakoBiteAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MakoBiteAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MakoBiteAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MakoBiteAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MakoBiteAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MakoBiteAudioProcessor::getProgramName (int index)
{
    return {};
}

void MakoBiteAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MakoBiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //R1.00 Get our Sample Rate for filter calculations.
    SampleRate = MakoBiteAudioProcessor::getSampleRate();
    if (SampleRate < 21000) SampleRate = 48000;
    if (192000 < SampleRate) SampleRate = 48000;

    //R1.00 Calculate and pre-Run variables/filters/etc.
    Mako_Settings_Update(true);
}

void MakoBiteAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MakoBiteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void MakoBiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //R1.00 Our defined variables.
    float tS;  //R1.00 Temporary Sample.

    //R1.00 Handle any changes to our Paramters.
    //R1.00 Handle any settings changes made in Editor. 
    if (0 < SettingsChanged) Mako_Settings_Update(false);

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        //R1.00 Process the AUDIO buffer data.
        if (Setting[e_Mono] && (channel == 1))
        {
            auto* channel0Data = buffer.getWritePointer(0);

            //R1.00 FORCE MONO - Put CHANNEL 0 data in CHANNEL 1.
            for (int samp = 0; samp < buffer.getNumSamples(); samp++) channelData[samp] = channel0Data[samp];
        }
        else
        {
            // ..do something to the data...
            for (int samp = 0; samp < buffer.getNumSamples(); samp++)
            {
                //R1.00 Get the current sample and put it in tS. 
                tS = buffer.getSample(channel, samp);

                //R1.00 Noise gate. Always call because Signal_AVG is calculated in here.
                tS = Mako_FX_NoiseGate(tS, channel);
                
                //R1.00 Apply one of our world famous effects.
                switch (int(Setting[e_Mode]))
                {
                case 1: tS = Mako_FX_AutoWah(tS, channel); break;
                default: tS = Mako_FX_SynthDrive(tS, channel); break;
                }

                //R1.00 Write our modified sample back into the sample buffer.
                channelData[samp] = tS;
            }
        }
    }
}

//==============================================================================
bool MakoBiteAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MakoBiteAudioProcessor::createEditor()
{
    return new MakoBiteAudioProcessorEditor (*this);
}


//==============================================================================
void MakoBiteAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    //R1.00 Save our parameters to file/DAW.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
   
}

void MakoBiteAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    //R1.00 Read our parameters from file/DAW.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

    //R1.00 Force our variables to get updated.
    Setting[e_Gain] = Mako_GetParmValue_float("gain");
    Setting[e_NGate] = Mako_GetParmValue_float("ngate");
    Setting[e_Sense] = Mako_GetParmValue_float("sense");
    Setting[e_Q] = Mako_GetParmValue_float("q");
    Setting[e_Mix] = Mako_GetParmValue_float("mix");
    Setting[e_Mode] = Mako_GetParmValue_float("mode");
    Setting[e_Mono] = Mako_GetParmValue_float("mono");    
}

//R1.00 Parameter reading helper function.
int MakoBiteAudioProcessor::Mako_GetParmValue_int(juce::String Pstring)
{
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return int(parm->load());
    else
        return 0;
}

//R1.00 Parameter reading helper function.
float MakoBiteAudioProcessor::Mako_GetParmValue_float(juce::String Pstring)
{
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return float(parm->load());
    else
        return 0.0f;
}

//R1.00 Volume envelope based on average Signal volume.
float MakoBiteAudioProcessor::Mako_FX_NoiseGate(float tSample, int channel)
{
    //R1.00 Track our Input Signal Average (Absolute vals). We need this for gate and WAH so always calc.
    //R1.00 NEEDS: This needs to be adjusted for sample rate, but it is not!!! 
    Signal_AVG[channel] = (Signal_AVG[channel] * .995) + (abs(tSample) * .005);

    //R1.00 If not using the Gate, exit out and save a few CPU cycles.
    if (Setting[e_NGate] < .0001f) return tSample;

    //R1.00 Create a volume envelope based on Signal Average.
    Pedal_NGate_Fac[channel] = Signal_AVG[channel] * 10000.0f * (1.1f - Setting[e_NGate]);

    //R1.00 Dont amplify the sound, just reduce when necessary.
    if (1.0f < Pedal_NGate_Fac[channel]) Pedal_NGate_Fac[channel] = 1.0f;

    return tSample * Pedal_NGate_Fac[channel];
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MakoBiteAudioProcessor();
}

//R1.00 Apply filter to a sample.
float MakoBiteAudioProcessor::Filter_Calc_BiQuad(float tSample, int channel, tp_filter* fn)
{
    float tS = tSample;

    fn->xn0[channel] = tS;
    tS = fn->a0 * fn->xn0[channel] + fn->a1 * fn->xn1[channel] + fn->a2 * fn->xn2[channel] - fn->b1 * fn->yn1[channel] - fn->b2 * fn->yn2[channel];
    fn->xn2[channel] = fn->xn1[channel]; fn->xn1[channel] = fn->xn0[channel]; fn->yn2[channel] = fn->yn1[channel]; fn->yn1[channel] = tS;

    return tS;
}

//R1.00 Second order parametric/peaking boost filter with constant-Q
void MakoBiteAudioProcessor::Filter_BP_Coeffs(float Gain_dB, float Fc, float Q, tp_filter* fn)
{    
    float K = pi2 * (Fc * .5f) / SampleRate;
    float K2 = K * K;
    float V0 = pow(10.0, Gain_dB / 20.0);

    float a = 1.0f + (V0 * K) / Q + K2;
    float b = 2.0f * (K2 - 1.0f);
    float g = 1.0f - (V0 * K) / Q + K2;
    float d = 1.0f - K / Q + K2;
    float dd = 1.0f / (1.0f + K / Q + K2);

    fn->a0 = a * dd;
    fn->a1 = b * dd;
    fn->a2 = g * dd;
    fn->b1 = b * dd;
    fn->b2 = d * dd;
    fn->c0 = 1.0f;
    fn->d0 = 0.0f;
}

//R1.00 Second order LOW PASS filter. 
void MakoBiteAudioProcessor::Filter_LP_Coeffs(float fc, tp_filter* fn)
{    
    float c = 1.0f / (tanf(pi * fc / SampleRate));
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = 2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * (1.0f - (c * c));
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}

//F1.00 Second order butterworth High Pass.
void MakoBiteAudioProcessor::Filter_HP_Coeffs(float fc, tp_filter* fn)
{    
    float c = tanf(pi * fc / SampleRate);
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = -2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * ((c * c) - 1.0f);
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}

//R1.00 Create an Envelope Filter based on Signal_AVG value.
float MakoBiteAudioProcessor::Mako_FX_AutoWah(float tSample, int channel)
{
    //R1.00 Exit if not even using Modulate.
    if (Setting[e_Mix] < .001f) return tSample;

    float tS;

    //R2.00 Envelope Filter.
    float tFac = Signal_AVG[channel] * 500.0f * (Setting[e_Sense] * Setting[e_Sense]);
    if (.90f < tFac) tFac = .90f;
    if (tFac < .0001f) tFac = .0001f;

    //R1.00 Adjust the WAH filter. 
    //R1.00 This is an expensive calculation. We are reducing the number of calcs based on signal change.
    //R1.00 The smaller the number the more often it is calc. Not calc enough and it sounds robotic due to the larger steps.
    if (.025 < abs(tFac / Factor_Last))
    {
        Factor_Last = tFac;
        Filter_BP_Coeffs((Setting[e_Q] * 30.0f), 800.0f * (.1f + tFac), 1.4f * (.1f + tFac * 3.0f), &makoF_AutoWah);
    }

    //R1.00 apply our WAH effect filter.
    tS = Filter_Calc_BiQuad(tSample, channel, &makoF_AutoWah);
    
    //Volume/Gain adjust. Reduction only.
    tS *= Setting[e_Gain];

    return (tSample * (1.0f - Setting[e_Mix])) + (tS * Setting[e_Mix]);
}

float MakoBiteAudioProcessor::Mako_FX_SynthDrive(float tSample, int channel)
{
    //R1.00 Exit if not even using SynthDrive.
    if (Setting[e_Mix] < .001f) return tSample;

    //R1.00 Apply our Synth effect filter.
    //R1.00 TFac pushes the frequency of our signal up. We also use it to balance out the gain in volume.
    float tFac = (1.0f + (Setting[e_Sense] * 50));
    float tS = sinf(tSample * tFac) * (1.5f / tFac);

    //R1.00 Volume/Gain adjust.
    tS *= Setting[e_Gain];

    return (tSample * (1.0f - Setting[e_Mix])) + (tS * Setting[e_Mix]);
}


void MakoBiteAudioProcessor::Mako_Settings_Update(bool ForceAll)
{
    //R1.00 We do changes here so we know the vars are not in use while we change them.
    //R1.00 EDITOR sets SETTING flags and we make changes here.
    bool Force = ForceAll;

    //R1.00 This VST has no settings adjusted variables yet.
    
    //R1.00 RESET out settings flags.
    SettingsType = 0;
    SettingsChanged = false;
}

