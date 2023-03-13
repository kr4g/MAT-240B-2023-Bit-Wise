#include <cmath>

const float SAMPLERATE = 48000.0f;

class QuasiOscillator {
protected:
    float phase = 0;
    float w = 0;
    float scaling = 0;
    float DC = 0;
    float norm = 0;
    float in_hist = 0;
    float filter = 1.0f;
    float output = 0;

public:
    void set(float freq) {
        float lastW = w;
        w = freq / SAMPLERATE;
        if (std::fabs(w - lastW) < 1e-6) {
            return;
        }
        float n = 0.5f - w;
        scaling = filter * 13.0f * std::pow(n, 4);
        DC = 0.376f - w * 0.752f;
        norm = 1.0f - 2.0f * w;
    }

    void updateFilter(float amt) {
        filter = amt;
        float n = 0.5f - w;
        scaling = filter * 13.0f * std::pow(n, 4);
    }
};

class QuasiSaw : public QuasiOscillator {
public:
    float osc = 0;
    const float a0 = 2.5f;
    const float a1 = -1.5f;

public:
    float operator()() {
        phase += 2.0f * w;
        if (phase >= 1.0f) {
            phase -= 2.0f;
        }
        osc = (osc + std::sin(M_PI * (phase + osc * scaling))) * 0.5f;
        float out = a0 * osc + a1 * in_hist;
        in_hist = osc;
        out += DC;
        output = out * norm;
        return output;
    }
};

class QuasiPulse : public QuasiOscillator {
public:
    float osc = 0;
    float osc2 = 0;
    float pw = 0.5f;
    const float a0 = 1.9f;
    const float a1 = -0.9f;

public:
    float operator()() {
        phase += 2.0f * w;
        if (phase >= 1.0f) {
            phase -= 2.0f;
        }
        osc = (0.45f * osc + 0.55f * std::sin(M_PI * (phase - osc * osc * scaling)));
        osc2 = (0.45f * osc2 + 0.55f * std::sin(M_PI * (phase - osc2 * osc2 * scaling + 2.0f * pw)));
        float out = osc - osc2;
        out = a0 * out + a1 * in_hist;
        in_hist = osc - osc2;
        output = out * norm;
        return output;
    }
};
