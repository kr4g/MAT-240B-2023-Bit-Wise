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

struct LineOfC {
  float operator()(int t, int byteBeatEquation = 0) {
    float v = BYTE_BEAT_EQUATIONS.at(byteBeatEquation)(t);

    // auto it = BYTE_BEAT_EQUATIONS.find(byteBeatEquation);
    // if (it != BYTE_BEAT_EQUATIONS.end()) {
    //   v = it->second(t);
    // } else {
    //   v = char(t);
    // }

    v *= 0.078125f;
    return v;
  }
};

using namespace juce;

// http://scp.web.elte.hu/papers/synthesis1.pdf

struct QuasiBandLimited : public AudioProcessor {
  AudioParameterFloat* gain;
  // AudioParameterFloat* note;
  // AudioParameterFloat* filter;
  // AudioParameterFloat* pulseWidth;
  AudioParameterFloat* bitRedux;
  AudioParameterFloat* rateRedux;
  // AudioParameterFloat* alpha;
  AudioParameterInt* byteBeatEquation;
  AudioParameterInt* byteBeatOffsetEquation;
  AudioParameterInt* stereoOffsetAmt;
  AudioParameterFloat* stereoOffsetGain;
  AudioParameterInt* t_n;
  AudioParameterInt* t_refreshRate;

  LineOfC lineOfC;
  /// add parameters here ///////////////////////////////////////////////////
  /// add your objects here /////////////////////////////////////////////////
  // QuasiSaw qSaw;
  // QuasiPulse qPulse;
  
  int t{0};
  int t_offset{0};
  // int count{0};
  QuasiBandLimited()
      : AudioProcessor(BusesProperties()
                           .withInput("Input", AudioChannelSet::stereo())
                           .withOutput("Output", AudioChannelSet::stereo())) {
    addParameter(byteBeatEquation = new AudioParameterInt(
                     {"byteBeatEquation", 1}, "Equation",
                     0, BYTE_BEAT_EQUATIONS.size() - 1, 11));
    addParameter(gain = new AudioParameterFloat(
                     {"gain", 1}, "Gain",
                     NormalisableRange<float>(-64, -1, 0.01f), -64));
    /// add parameters here /////////////////////////////////////////////////
    // addParameter(note = new AudioParameterFloat(
    //                  {"note", 1}, "Pitch (MIDI)",
    //                  NormalisableRange<float>(-2, 129, 0.01f), 33));
    // addParameter(filter = new AudioParameterFloat(
    //                  {"filter", 1}, "Filter",
    //                  NormalisableRange<float>(0, 1.0f, 0.001f), 1.0f));
    // addParameter(pulseWidth = new AudioParameterFloat(
    //                  {"pulseWidth", 1}, "qPulse Width",
    //                  NormalisableRange<float>(0.1f, 0.9f, 0.0001f), 0.1f));
    addParameter(t_n = new AudioParameterInt(
                     {"t_n", 1}, "Position (t_n)",
                     0, 1000000, 0));
    addParameter(t_refreshRate = new AudioParameterInt(
                     {"t_refreshRate", 1}, "Window Size",
                     2, pow(2, 16), 32));
    addParameter(byteBeatOffsetEquation = new AudioParameterInt(
                     {"byteBeatOffsetEquation", 1}, "Offset Equation",
                      0, BYTE_BEAT_EQUATIONS.size() - 1, 11));
    addParameter(stereoOffsetAmt = new AudioParameterInt(
                     {"stereoOffsetAmt", 1}, "Stereo Phase Offset Amt",
                     0, pow(2, 16), 0));
    addParameter(stereoOffsetGain = new AudioParameterFloat(
                     {"stereoOffsetGain", 1}, "Offset Gain",
                     NormalisableRange<float>(-64, -1, 0.01f), -64));
    addParameter(bitRedux = new AudioParameterFloat(
                     {"bitRedux", 1}, "Bit Depth",
                     NormalisableRange<float>(1, 32, 0.001f), 32.f));
    addParameter(rateRedux = new AudioParameterFloat(
                     {"rateRedux", 1}, "Sample Rate Divisor",
                      NormalisableRange<float>(1, 256, 0.001f), 1.f));
    // addParameter(alpha = new AudioParameterFloat(
    //                  {"alpha", 1}, "Alpha",
    //                  NormalisableRange<float>(0, 1, 0.001f), 0.0f));
  }

  /// this function handles the audio ///////////////////////////////////////
  void processBlock(AudioBuffer<float>& buffer, MidiBuffer&) override {
    /// put your own code here instead of this code /////////////////////////
    buffer.clear(0, 0, buffer.getNumSamples());
    buffer.clear(1, 0, buffer.getNumSamples());
    // left[0] = right[0] = dbtoa(gain->get());  // click!
    float rateDivisor = rateRedux->get();
    int start = t_n->get();
    int refreshRate = t_refreshRate->get();
    int end = start + refreshRate;
    int byteOp = byteBeatEquation->get();
    int byteOpOffset = byteBeatOffsetEquation->get();

    bool offset = stereoOffsetAmt->get();
    int t_offsetAmt = stereoOffsetAmt->get();

    AudioSampleBuffer dataOffsetBuffer;
    int numSamples = buffer.getNumSamples();
    dataOffsetBuffer.setSize(2, numSamples, false, true, true); // clears
    dataOffsetBuffer.copyFrom(0, 0, buffer.getReadPointer(0), numSamples);
    if (buffer.getNumChannels() > 1) dataOffsetBuffer.copyFrom(1, 0, buffer.getReadPointer(1), numSamples);
  
    for (int chan = 0; chan < buffer.getNumChannels(); ++chan) {
      float* data = buffer.getWritePointer(chan);
      float* dataOffset = dataOffsetBuffer.getWritePointer(chan);
      for (int i = 0; i < buffer.getNumSamples(); ++i) {
        float A = dbtoa(gain->get())             * 0.05f;
        float B = dbtoa(stereoOffsetGain->get()) * 0.0005f;
        // --------------------------------
        // HANDLE BYTEBEAT GRANULATION
        // --------------------------------
        // if t is out of bounds, reset t to start
        if (t > (end + refreshRate) || t < start) { t = start; }
        // line of c-code
        data[i] = A * lineOfC(t++, byteOp);  // value of t and the equation to sample

        if (offset) {
            // if t_offset is out of bounds, reset t_offset to start + t_offsetAmt
            if (t_offset > (end + t_offsetAmt) || t_offset < start + t_offsetAmt) { t_offset = start + t_offsetAmt; }
          // line of c-code
          dataOffset[i] = B * lineOfC(t_offset++, byteOpOffset);  // value of t and the equation to sample
          buffer.addFrom(chan, 0, dataOffsetBuffer.getReadPointer(chan), numSamples);
        }
        // --------------------------------
        // --------------------------------

        // bit reduction
        float totalQLevels = powf(2, bitRedux->get() - 1);
        int j = (int) (data[i] * totalQLevels);
        data[i] = (float) j / totalQLevels;

        // sample rate reduction
        if (rateDivisor > 1) {
          if (i % static_cast<int>(rateDivisor) != 0) // why do I have to static cast now? I didn't have to before...
            data[i] = data[i - i%static_cast<int>(rateDivisor)];
            // TODO: Bresenham's line algorithm
        }

        // tanh softclip
        data[i] = softclip(data[i]);
      }
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