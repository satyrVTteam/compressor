#include "daisysp.h"
#include "daisy_petal.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyPetal Petal;
Compressor comp;

Parameter threshParam, ratioParam, attackParam, releaseParam;

bool      effectOn = false;

void UpdateControls();

void AudioCallback(float **in, float **out, size_t size)
{
    float sig;
    float dry_in;
    float volume;
    UpdateControls();
    effectOn ^= Petal.switches[0].RisingEdge();
    volume = Petal.knob[0].Process();//the simplest version how to process knobs

    // Scales input by 2 and then the output by 0.5
    // This is because there are 6dB of headroom on the daisy
    // and currently no way to tell where '0dB' is supposed to be
    for(size_t i = 0; i < size; i++)
    {
        dry_in = in[0][i] * 2.0f;
        sig    = comp.Process(dry_in);
        out[0][i] = out[1][i] = in[0][i]*volume*2.0f; // to avoid if else with footswitch 
                                                 // volume to check compression vs just volume raise, volume goes 0...1 thats why 2

        if(effectOn)
        {
            out[0][i] = out[1][i] = sig * 0.5f;
        }
    }
}

int main(void)
{
    float samplerate = Petal.AudioSampleRate();
    Petal.Init(); // Initialize hardware


    comp.Init(samplerate);

    //parameter parameters
    threshParam.Init(Petal.knob[Petal.KNOB_3], -80.0f, 0.f, Parameter::LINEAR);
    ratioParam.Init(Petal.knob[Petal.KNOB_4], 1.2f, 40.f, Parameter::LINEAR);
    attackParam.Init(Petal.knob[Petal.KNOB_5], 0.01f, 1.f, Parameter::EXPONENTIAL);
    releaseParam.Init(Petal.knob[Petal.KNOB_6], 0.01f, 1.f, Parameter::EXPONENTIAL);

    Petal.StartAdc();
    Petal.StartAudio(AudioCallback);

    while(1)
    {
        Petal.DelayMs(6);
        Petal.ClearLeds();
        Petal.SetFootswitchLed((DaisyPetal::FootswitchLed)0, effectOn);
        Petal.UpdateLeds();
    }
}

void UpdateControls()
{
    Petal.ProcessAllControls();

    //controls
    comp.SetThreshold(threshParam.Process());
    comp.SetRatio(ratioParam.Process());
    comp.SetAttack(attackParam.Process());
    comp.SetRelease(releaseParam.Process());
}
