#pragma once
// Small DSP building blocks for the physical tape model. Double precision,
// no allocation after construction/prepare, no locks, no I/O.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace tape::dsp
{
    constexpr double kPi = 3.14159265358979323846;

    inline double dbToGain(double db) { return std::pow(10.0, db / 20.0); }
    inline double gainToDb(double g) { return 20.0 * std::log10(std::max(g, 1e-30)); }
    inline double finiteOrZero(double x) { return std::isfinite(x) ? x : 0.0; }

    // First-order section  H(s) = g * (1 + s/wz) / (1 + s/wp), bilinear with
    // each corner pre-warped. Swapping zero and pole gives the exact inverse.
    struct FirstOrder
    {
        double b0 = 1.0, b1 = 0.0, a1 = 0.0, z1 = 0.0;

        static double warp(double hz, double fs)
        {
            const double limited = std::min(hz, 0.45 * fs);
            return 2.0 * fs * std::tan(kPi * limited / fs);
        }

        void setShelf(double zeroHz, double poleHz, double gain, double fs)
        {
            const double k = 2.0 * fs;
            const double wz = warp(zeroHz, fs), wp = warp(poleHz, fs);
            const double n0 = 1.0 + k / wz, n1 = 1.0 - k / wz;
            const double d0 = 1.0 + k / wp, d1 = 1.0 - k / wp;
            b0 = gain * n0 / d0;
            b1 = gain * n1 / d0;
            a1 = d1 / d0;
        }

        void setHighpass(double hz, double fs)
        {
            const double k = 2.0 * fs, w = warp(hz, fs);
            const double d0 = k + w;
            b0 = k / d0;
            b1 = -k / d0;
            a1 = (w - k) / d0;
        }

        void setLowpass(double hz, double fs)
        {
            const double k = 2.0 * fs, w = warp(hz, fs);
            const double d0 = k + w;
            b0 = w / d0;
            b1 = w / d0;
            a1 = (w - k) / d0;
        }

        void reset() { z1 = 0.0; }

        inline double process(double x)
        {
            const double y = b0 * x + z1;
            z1 = b1 * x - a1 * y;
            return y;
        }
    };

    // Biquad, transposed direct form II.
    struct Biquad
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0, z1 = 0.0, z2 = 0.0;

        void setPeak(double hz, double gainDb, double q, double fs)
        {
            const double a = std::pow(10.0, gainDb / 40.0);
            const double w = 2.0 * kPi * std::min(hz, 0.45 * fs) / fs;
            const double alpha = std::sin(w) / (2.0 * q), c = std::cos(w);
            const double d0 = 1.0 + alpha / a;
            b0 = (1.0 + alpha * a) / d0;
            b1 = -2.0 * c / d0;
            b2 = (1.0 - alpha * a) / d0;
            a1 = -2.0 * c / d0;
            a2 = (1.0 - alpha / a) / d0;
        }

        void setHighpass(double hz, double q, double fs)
        {
            const double w = 2.0 * kPi * std::min(hz, 0.45 * fs) / fs;
            const double alpha = std::sin(w) / (2.0 * q), c = std::cos(w);
            const double d0 = 1.0 + alpha;
            b0 = (1.0 + c) / 2.0 / d0;
            b1 = -(1.0 + c) / d0;
            b2 = (1.0 + c) / 2.0 / d0;
            a1 = -2.0 * c / d0;
            a2 = (1.0 - alpha) / d0;
        }

        void setLowpass(double hz, double q, double fs)
        {
            const double w = 2.0 * kPi * std::min(hz, 0.45 * fs) / fs;
            const double alpha = std::sin(w) / (2.0 * q), c = std::cos(w);
            const double d0 = 1.0 + alpha;
            b0 = (1.0 - c) / 2.0 / d0;
            b1 = (1.0 - c) / d0;
            b2 = (1.0 - c) / 2.0 / d0;
            a1 = -2.0 * c / d0;
            a2 = (1.0 - alpha) / d0;
        }

        void reset() { z1 = z2 = 0.0; }

        inline double process(double x)
        {
            const double y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }
    };

    // Linear-phase half-band FIR (Kaiser-windowed sinc), used for 2x up and down.
    class HalfbandFir
    {
    public:
        void design(int halfLength, double attenuationDb)
        {
            m = halfLength;
            const int n = 2 * m + 1;
            taps.assign(static_cast<std::size_t>(n), 0.0);
            const double beta = attenuationDb > 50.0 ? 0.1102 * (attenuationDb - 8.7)
                                                      : 0.5842 * std::pow(attenuationDb - 21.0, 0.4) + 0.07886 * (attenuationDb - 21.0);
            double sum = 0.0;
            for (int i = -m; i <= m; ++i)
            {
                const double x = 0.5 * kPi * i;
                const double sinc = i == 0 ? 1.0 : std::sin(x) / x;
                const double r = static_cast<double>(i) / (m + 1);
                const double w = bessel0(beta * std::sqrt(std::max(0.0, 1.0 - r * r))) / bessel0(beta);
                taps[static_cast<std::size_t>(i + m)] = 0.5 * sinc * w;
                sum += taps[static_cast<std::size_t>(i + m)];
            }
            for (auto& t : taps)
                t /= sum;
            history.assign(taps.size(), 0.0);
            pos = 0;
        }

        void reset()
        {
            std::fill(history.begin(), history.end(), 0.0);
            pos = 0;
        }

        int halfLength() const { return m; }

        inline void push(double x)
        {
            history[pos] = x;
            pos = pos + 1 == history.size() ? 0 : pos + 1;
        }

        inline double output() const
        {
            double acc = 0.0;
            std::size_t idx = pos;  // oldest sample
            const std::size_t n = taps.size();
            for (std::size_t k = 0; k < n; ++k)
            {
                acc += taps[n - 1 - k] * history[idx];
                idx = idx + 1 == n ? 0 : idx + 1;
            }
            return acc;
        }

    private:
        static double bessel0(double x)
        {
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 40; ++k)
            {
                term *= (x / (2.0 * k)) * (x / (2.0 * k));
                sum += term;
            }
            return sum;
        }

        int m = 0;
        std::vector<double> taps, history;
        std::size_t pos = 0;
    };

    // 4x oversampling as two half-band stages. Total round-trip latency is
    // an integer number of base-rate samples: latencySamples().
    class Oversampler4x
    {
    public:
        void prepare()
        {
            up1.design(31, 90.0);
            down1.design(31, 90.0);
            up2.design(8, 70.0);
            down2.design(8, 70.0);
        }

        void reset()
        {
            up1.reset();
            down1.reset();
            up2.reset();
            down2.reset();
        }

        static int latencySamples() { return 31 + 4; }

        inline void up(double x, double out[4])
        {
            double mid[2];
            up1.push(x);
            mid[0] = 2.0 * up1.output();
            up1.push(0.0);
            mid[1] = 2.0 * up1.output();
            for (int i = 0; i < 2; ++i)
            {
                up2.push(mid[i]);
                out[2 * i] = 2.0 * up2.output();
                up2.push(0.0);
                out[2 * i + 1] = 2.0 * up2.output();
            }
        }

        inline double down(const double in[4])
        {
            double mid[2];
            for (int i = 0; i < 2; ++i)
            {
                down2.push(in[2 * i]);
                down2.push(in[2 * i + 1]);
                mid[i] = down2.output();
            }
            down1.push(mid[0]);
            down1.push(mid[1]);
            return down1.output();
        }

    private:
        HalfbandFir up1, down1, up2, down2;
    };

    // Fractional delay line with 4-point Lagrange interpolation.
    class DelayLine
    {
    public:
        void prepare(std::size_t sizePowerOfTwo)
        {
            buffer.assign(sizePowerOfTwo, 0.0);
            mask = sizePowerOfTwo - 1;
            write = 0;
        }

        void reset()
        {
            std::fill(buffer.begin(), buffer.end(), 0.0);
            write = 0;
        }

        inline void push(double x)
        {
            buffer[write] = x;
            write = (write + 1) & mask;
        }

        // delay in samples, measured back from the most recently pushed sample; must be >= 1.
        inline double read(double delay) const
        {
            const double whole = std::floor(delay);
            const double f = delay - whole;
            const std::size_t i = (write + mask - static_cast<std::size_t>(whole)) & mask;  // sample at 'whole'
            const double xm1 = buffer[(i + 1) & mask];          // one sample newer
            const double x0 = buffer[i];
            const double x1 = buffer[(i + mask) & mask];         // one older
            const double x2 = buffer[(i + mask - 1) & mask];     // two older
            return xm1 * (-f * (f - 1.0) * (f - 2.0) / 6.0)
                 + x0 * ((f + 1.0) * (f - 1.0) * (f - 2.0) / 2.0)
                 + x1 * (-(f + 1.0) * f * (f - 2.0) / 2.0)
                 + x2 * ((f + 1.0) * f * (f - 1.0) / 6.0);
        }

    private:
        std::vector<double> buffer;
        std::size_t mask = 0, write = 0;
    };

    // Deterministic white noise, roughly Gaussian (sum of uniforms), unit variance.
    struct Noise
    {
        std::uint32_t state = 1;

        inline double uniform()
        {
            state = 1664525u * state + 1013904223u;
            return static_cast<double>((state >> 8) & 0xFFFFFF) / static_cast<double>(0x1000000) - 0.5;
        }

        inline double next() { return (uniform() + uniform() + uniform() + uniform()) * 1.7320508075688772; }
    };
}
