/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MakoBiteAudioProcessorEditor::MakoBiteAudioProcessorEditor (MakoBiteAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{    
    //R1.00 Create SLIDER ATTACHMENTS so our parameter vars get adjusted automatically for Get/Set states.
    ParAtt[e_Gain] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "gain", sldKnob[e_Gain]);
    ParAtt[e_NGate] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "ngate", sldKnob[e_NGate]);
    ParAtt[e_Sense] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "sense", sldKnob[e_Sense]);
    ParAtt[e_Q] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "q", sldKnob[e_Q]);
    ParAtt[e_Mix] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "mix", sldKnob[e_Mix]);
    ParAtt[e_Mode] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "mode", sldKnob[e_Mode]);
    ParAtt[e_Mono] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "mono", sldKnob[e_Mono]);
        
    imgBackground = juce::ImageCache::getFromMemory(BinaryData::smacktalkback_png, BinaryData::smacktalkback_pngSize);

    //****************************************************************************************
    //R1.00 Add GUI CONTROLS
    //****************************************************************************************
    GUI_Init_Large_Slider(&sldKnob[e_Gain], audioProcessor.Setting[e_Gain],0.0f, 4.0f,.01f,"", 1, 0xFF000000);
    GUI_Init_Large_Slider(&sldKnob[e_NGate], audioProcessor.Setting[e_NGate], 0.0f, 1.0f, .01f, "", 1, 0xFF000000);
    GUI_Init_Large_Slider(&sldKnob[e_Sense], audioProcessor.Setting[e_Sense], 0.0f, 1.0f, .01f, "", 1, 0xFF000000);
    GUI_Init_Large_Slider(&sldKnob[e_Q], audioProcessor.Setting[e_Q], 0.0f, 1.0f, .01f, "", 1, 0xFF000000);
    GUI_Init_Large_Slider(&sldKnob[e_Mix], audioProcessor.Setting[e_Mix], 0.0f, 1.0f, .01f, "", 2, 0xFF000000);
   
    GUI_Init_Small_Slider(&sldKnob[e_Mode], audioProcessor.Setting[e_Mode], 0, 1, 1, "");
    GUI_Init_Small_Slider(&sldKnob[e_Mono], audioProcessor.Setting[e_Mono], 0, 1, 1, "");    
    
        
    //R1.00 Update the Look and Feel (Global colors) so drop down menu is the correct color. 
    getLookAndFeel().setColour(juce::DocumentWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::DocumentWindow::textColourId, juce::Colour(255, 255, 255));
    getLookAndFeel().setColour(juce::DialogWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(192, 0, 0));
    getLookAndFeel().setColour(juce::TextButton::buttonOnColourId, juce::Colour(192, 0, 0));
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ListBox::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::Label::backgroundColourId, juce::Colour(32, 32, 32));

    //R1.00 Define our control positions to make drawing easier.
    KNOB_DefinePosition(e_NGate,    10, 20, 60, 60, "Gate");
    KNOB_DefinePosition(e_Gain,     80, 55, 60, 60, "Gain");     
    KNOB_DefinePosition(e_Sense,   150, 55, 60, 60, "Sense");
    KNOB_DefinePosition(e_Q,       220, 55, 60, 60, "Q");
    KNOB_DefinePosition(e_Mix,     290, 20, 60, 60, "Mix");

    KNOB_DefinePosition(e_Mode,   13, 96, 51, 22, "Smack/Talk");
    KNOB_DefinePosition(e_Mono,  293, 96, 51, 22, "Stereo/Mono");
    
    Knob_Cnt = 7;

    //R1.00 Enable/Disable Controls as needed.
    if (audioProcessor.Setting[e_Mode])
        sldKnob[e_Q].setEnabled(true);
    else
        sldKnob[e_Q].setEnabled(false);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
     
    //R1.00 Set the window size.
    setSize(360, 120);
}

MakoBiteAudioProcessorEditor::~MakoBiteAudioProcessorEditor()
{
}

//==============================================================================
void MakoBiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    bool UseImage = true;
    
    if (UseImage)
    {
        g.drawImageAt(imgBackground, 0, 0);        
    }
    else
    {
        //R1.00 Use for getting control positions when creating a background image.
        //g.setColour(juce::Colours::white);
        //g.fillRect(0, 0, 360, 120);

        //R1.00 Draw our GUI.
        //R1.00 Background.
        juce::ColourGradient ColGrad;
        ColGrad = juce::ColourGradient(juce::Colour(0xFFD0D0D0), 0.0f, 0.0f, juce::Colour(0xFFF0F0F0), 0.0f, 50.0f, false);
        g.setGradientFill(ColGrad);
        g.fillRect(0, 0, 360, 50);
        ColGrad = juce::ColourGradient(juce::Colour(0xFFF0F0F0), 0.0f, 50.0f, juce::Colour(0xFF808080), 0.0f, 120.0f, false);
        g.setGradientFill(ColGrad);
        g.fillRect(0, 50, 360, 120);

        //R1.00 Draw LOGO text.
        g.setColour(juce::Colours::black);
        g.fillRect(110, 0, 140, 35);
        g.setFont(18.0f);
        g.setColour(juce::Colours::white);
        g.drawFittedText("Smack Talk", 110, 0, 140, 18, juce::Justification::centred, 1);
        g.setFont(14.0f);
        g.setColour(juce::Colours::orange);
        g.drawFittedText("M * A * K * O", 110, 15, 140, 15, juce::Justification::centred, 1);       
    
        //R1.00 Draw KNOB headers.
        g.setFont(12.0f);
        g.setColour(juce::Colours::black);
        for (int t = 0; t < Knob_Cnt; t++)
        {
            g.drawFittedText(Knob_Name[t], Knob_Pos[t].x, Knob_Pos[t].y - 10, Knob_Pos[t].sizex, 15, juce::Justification::centred, 1);
        }
    }
}

void MakoBiteAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //R1.00 Use RETURN here to have a blank window drawn when creating a GUI image.
    //R1.00 The controls are only visible if thier BOUNDS are defined.
    //return;

    //R1.00 Draw all of the defined KNOBS.
    for (int t = 0; t < Knob_Cnt; t++) sldKnob[t].setBounds(Knob_Pos[t].x, Knob_Pos[t].y, Knob_Pos[t].sizex, Knob_Pos[t].sizey);    
}


//R1.00 Setup the SLIDER control edit values, Text Suffix (if any), UI tick marks, and Indicator Color.
void MakoBiteAudioProcessorEditor::GUI_Init_Large_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix, int TickStyle, int ThumbColor)
{
    //R1.00 Setup the slider edit parameters.
    slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    slider->setTextValueSuffix(Suffix);
    slider->setRange(Vmin, Vmax, Vinterval);
    slider->setValue(Val);
    slider->addListener(this);
    addAndMakeVisible(slider);

    //R1.00 Override the default Juce drawing routines and use ours.
    slider->setLookAndFeel(&myLookAndFeel);

    //R1.00 Setup the type and colors for the sliders.
    slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFC08000));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFF804000));
    slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0x00000000));    //R1.00 Make this SEE THRU. Alpha=0.
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(ThumbColor));

    //R1.00 Cheat: We are using this color as a Tick Mark style selector in our drawing function.
    slider->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(TickStyle));
}

void MakoBiteAudioProcessorEditor::GUI_Init_Small_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix)
{
    //R1.00 Setup the slider edit parameters.
    slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    slider->setRange(Vmin, Vmax, Vinterval);
    slider->setTextValueSuffix(Suffix);
    slider->setValue(Val);
    slider->addListener(this);
    addAndMakeVisible(slider);

    //R1.00 Setup the type and colors for the sliders.
    slider->setSliderStyle(juce::Slider::LinearHorizontal);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFA0A0A0));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF202020));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFF404040));
    slider->setColour(juce::Slider::trackColourId, juce::Colour(0x00000000));
    slider->setColour(juce::Slider::backgroundColourId, juce::Colour(0x00000000));
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(0xFFE0E0E0));
}

void MakoBiteAudioProcessorEditor::KNOB_DefinePosition(int idx,float x, float y, float sizex, float sizey, juce::String name)
{
    Knob_Pos[idx].x = x;
    Knob_Pos[idx].y = y;
    Knob_Pos[idx].sizex = sizex;
    Knob_Pos[idx].sizey = sizey;
    Knob_Name[idx] = name;
}

/*
void MakoBiteAudioProcessorEditor::updateToggleState(juce::Button* button, juce::String name)
{
    auto state = button->getToggleState();
    bool Butt_On = state ? true : false;

}
*/

void MakoBiteAudioProcessorEditor::Setting_UpdateProcessor(int SettingType)
{
    //R1.00 Set flags here that let the PROCESSOR know it needs to update settings.
    //R1.00 If we are changing settings in CODE, we dont want to update the proc until all done changing.
    if (audioProcessor.SettingsType != 100)
    {
        audioProcessor.SettingsType = SettingType;
        audioProcessor.SettingsChanged = true;  //R1.00 We need to update settings in processor.
    }
}


void MakoBiteAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{   
    //R1.00 Check for MODE change and enable/disable controls as needed.
    if (slider == &sldKnob[e_Mode])
    {
        //R1.00 Update the actual processor variable being edited.
        audioProcessor.Setting[e_Mode] = float(sldKnob[e_Mode].getValue());

        if (audioProcessor.Setting[e_Mode])
            sldKnob[e_Q].setEnabled(true);
        else
            sldKnob[e_Q].setEnabled(false);

        //R1.00 We need to update settings in processor.
        //R1.00 Increment changed var to be sure every change gets made. Changed var is decremented in processor.
        audioProcessor.SettingsChanged += 1;

        //R1.00 We have captured the correct slider change, exit this function.
        return;
    }



    //R1.00 When a slider is adjusted, this func gets called. Capture the new edits and flag
    //R1.00 the processor when it needs to recalc things.
    //R1.00 Check which slider has been adjusted.
    for (int t = 0; t < Knob_Cnt; t++)
    {
        if (slider == &sldKnob[t])
        {            
            //R1.00 Update the actual processor variable being edited.
            audioProcessor.Setting[t] = float(sldKnob[t].getValue());

            //R1.00 We need to update settings in processor.
            //R1.00 Increment changed var to be sure every change gets made. Changed var is decremented in processor.
            audioProcessor.SettingsChanged += 1;

            //R1.00 We have captured the correct slider change, exit this function.
            return;
        }
    }
    
    return;
}

