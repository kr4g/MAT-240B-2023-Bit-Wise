// Ryan Millett
// 
// From starter code by Karl Yerkes
// MAT-240B-2023


#include <juce_audio_processors/juce_audio_processors.h>

#include "quasi.hpp"
#include "Bitwise.h"

#include <iostream>

template <typename T>
T mtof(T m) {
  return T(440) * pow(T(2), (m - T(69)) / T(12));
}

template <typename T>
T dbtoa(T db) {
  return pow(T(10), db / T(20));
}

template <class T>
inline T sine(T n) {
  T nn = n*n;
	return n * (T(3.138982) + nn * (T(-5.133625) +
                          nn * (T(2.428288) - nn * T(0.433645))));
}

float softclip(float x) {
  if (x >= 1.0f) x = 1.0f;
  if (x <= -1.0f) x = -1.0f;
  return (3 * x - x * x * x) / 2.0f;
}

struct Cycle
{
  float phase = 0;
  float operator()(float hertz) {
    float value = sine(phase); // output sample;
    phase += hertz / 48000;
    if (phase > 1.0f) {
      phase -= 2.0f;
    }
    return value;
  }
};

// t<<((t>>8&t)|(t>>14&t)) //strange rhythms

struct LineOfC {
  int t = 0;
  int N, M, H; // SHIFTS (0, 32)
  int Q; // MASK power of 2 minus 1 (63, 127, 7, 4095)
  float operator()() {
    // float v = char(t<<((t>>8&t)|(t>>14&t)));
    // float v = char((t>>8&t)*(t>>15&t));
    // float v = char((t>>13&t)*(t>>8));
    float v = char(((t>>8&t)-(t>>3&t>>8|t>>16))&128);
    v *= 0.003921568627450980392156862745098f; // 1/255
    ++t;
    return v;
  }
};

using namespace juce;

// http://scp.web.elte.hu/papers/synthesis1.pdf

struct QuasiBandLimited : public AudioProcessor {
  AudioParameterFloat* gain;
  AudioParameterFloat* note;
  AudioParameterFloat* oscMix;
  AudioParameterFloat* filter;
  AudioParameterFloat* pulseWidth;
  AudioParameterFloat* bitRedux;
  AudioParameterFloat* rateRedux;
  AudioParameterFloat* alpha;
  AudioParameterInt* bitOp;
  AudioParameterInt* sampleOffset;
  Cycle carrier, modulator;
  LineOfC lineOfC;
  /// add parameters here ///////////////////////////////////////////////////
  /// add your objects here /////////////////////////////////////////////////
  QuasiSaw qSaw;
  QuasiPulse qPulse;

  // int32_t t{0};
  int count{0};
  QuasiBandLimited()
      : AudioProcessor(BusesProperties()
                           .withInput("Input", AudioChannelSet::stereo())
                           .withOutput("Output", AudioChannelSet::stereo())) {
    addParameter(gain = new AudioParameterFloat(
                     {"gain", 1}, "Gain",
                     NormalisableRange<float>(-128, -1, 0.01f), -128));
    /// add parameters here /////////////////////////////////////////////////
    addParameter(note = new AudioParameterFloat(
                     {"note", 1}, "Pitch (MIDI)",
                     NormalisableRange<float>(-2, 129, 0.01f), 33));
    addParameter(filter = new AudioParameterFloat(
                     {"filter", 1}, "Filter",
                     NormalisableRange<float>(0, 1.0f, 0.001f), 1.0f));
    addParameter(pulseWidth = new AudioParameterFloat(
                     {"pulseWidth", 1}, "qPulse Width",
                     NormalisableRange<float>(0.1f, 0.9f, 0.0001f), 0.1f));
    addParameter(oscMix = new AudioParameterFloat(
                     {"quasiMix", 1}, "qSaw <--> qPulse",
                     NormalisableRange<float>(0, 1, 0.001f), 0.5f));
    addParameter(bitRedux = new AudioParameterFloat(
                     {"bitRedux", 1}, "Bit Depth",
                     NormalisableRange<float>(1, 32, 0.001f), 32.f));
    addParameter(rateRedux = new AudioParameterFloat(
                     {"rateRedux", 1}, "Sample Rate Divisor",
                      NormalisableRange<float>(1, 500, 0.001f), 1.f));
    addParameter(bitOp = new AudioParameterInt(
                     {"bitOp", 1}, "Bit Operation",
                     0, 5, 0));
    addParameter(sampleOffset = new AudioParameterInt(
                     {"sampleOffset", 1}, "Sample Offset",
                     0, 1000, 0));
    addParameter(alpha = new AudioParameterFloat(
                     {"alpha", 1}, "Alpha",
                     NormalisableRange<float>(0, 1, 0.001f), 0.0f));
  }

  /// this function handles the audio ///////////////////////////////////////
  void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) override {
    /// put your own code here instead of this code /////////////////////////
    buffer.clear(0, 0, buffer.getNumSamples());
    buffer.clear(1, 0, buffer.getNumSamples());
    // auto left = buffer.getWritePointer(0, 0);
    // auto right = buffer.getWritePointer(1, 0);
    // left[0] = right[0] = dbtoa(gain->get());  // click!
    float rateDivisor = rateRedux->get();
    for (int chan = 0; chan < buffer.getNumChannels(); ++chan) {
      float* data = buffer.getWritePointer(chan);
      for (int i = 0; i < buffer.getNumSamples(); ++i) {
        // quasi synth
        float A = dbtoa(gain->get());
        float freq = mtof(note->get());
        
        // qSaw.set(freq);
        // qSaw.updateFilter(filter->get());

        // qPulse.set(freq);
        // qPulse.updateFilter(filter->get());
        // qPulse.pw = pulseWidth->get();
        
        // mix between QuasiPulse and QuasiSaw
        // data[i] = A * (oscMix->get() * qPulse() + (1 - oscMix->get()) * qSaw());
        data[i] = A * lineOfC();

        // bit reduction
        float totalQLevels = powf(2, bitRedux->get() - 1);
        int j = (int) (data[i] * totalQLevels);
        data[i] = (float) j / totalQLevels;

        if (rateDivisor > 1) {
          if (i % static_cast<int>(rateDivisor) != 0) // why do I have to static cast now? I didn't have to before...
            data[i] = data[i - i%static_cast<int>(rateDivisor)];
            // TODO: Bresenham's line algorithm
        }

        // bit operation -- TESTING - NOT WORKING PROPERLY
        if (bitOp->get() != 0) {
          BitwiseOp op;
          switch (bitOp->get()) {
            case 1: // AND
              op = static_cast<BitwiseOp>(BitwiseOp::AND);
              break;
            case 2: // OR
              op = static_cast<BitwiseOp>(BitwiseOp::OR);
              break;
            case 3: // XOR
              op = static_cast<BitwiseOp>(BitwiseOp::XOR);
              break;
            case 4: // NOT
              op = static_cast<BitwiseOp>(BitwiseOp::NOT);
              break;
            case 5: // SHIFT_LEFT
              op = static_cast<BitwiseOp>(BitwiseOp::ROTATE_LEFT);
              break;
          }
          int next_i = (i + sampleOffset->get()) % buffer.getNumSamples();
          data[i] = bitwise(data[i], data[next_i], op);
        }
        data[i] = softclip(data[i]);
        // data[i] = data[i] * (1 - alpha->get()) + original[i] * alpha->get();
      }
      // if (count++ % 6 == 0) { ++t; }
    }
  }

  /// start and shutdown callbacks///////////////////////////////////////////
  void prepareToPlay(double, int) override {}
  void releaseResources() override {}

  /// maintaining persistant state on suspend ///////////////////////////////
  void getStateInformation(MemoryBlock& destData) override {
    MemoryOutputStream(destData, true).writeFloat(*gain);
    /// add parameters here /////////////////////////////////////////////////
  }

  void setStateInformation(const void* data, int sizeInBytes) override {
    gain->setValueNotifyingHost(
        MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false)
            .readFloat());
    /// add parameters here /////////////////////////////////////////////////
  }

  /// do not change anything below this line, probably //////////////////////

  /// general configuration /////////////////////////////////////////////////
  const String getName() const override { return "bitQuant"; }
  double getTailLengthSeconds() const override { return 0; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }

  /// for handling presets //////////////////////////////////////////////////
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const String getProgramName(int) override { return "None"; }
  void changeProgramName(int, const String&) override {}

  /// ?????? ////////////////////////////////////////////////////////////////
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override {
    const auto& mainInLayout = layouts.getChannelSet(true, 0);
    const auto& mainOutLayout = layouts.getChannelSet(false, 0);

    return (mainInLayout == mainOutLayout && (!mainInLayout.isDisabled()));
  }

  /// automagic user interface //////////////////////////////////////////////
  AudioProcessorEditor* createEditor() override {
    return new GenericAudioProcessorEditor(*this);
  }
  bool hasEditor() const override { return true; }

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuasiBandLimited)
};

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new QuasiBandLimited();
}