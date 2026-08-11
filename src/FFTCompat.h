/*
 * FFTCompat.h
 *
 * Compatibility shim that exposes the v1.x arduinoFFT API on top of the v2.x
 * ArduinoFFT<T> template class from kosme/arduinoFFT.
 *
 * Background
 * ----------
 * kosme/arduinoFFT v1.x exposed a non-template class `arduinoFFT` with
 * capitalized method names that accepted the sample arrays as arguments:
 *
 *     arduinoFFT FFT = arduinoFFT();
 *     FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
 *     FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
 *     FFT.ComplexToMagnitude(vReal, vImag, samples);
 *
 * kosme/arduinoFFT v2.x renamed the class to `ArduinoFFT<T>` (template),
 * lowercased the method names, and bound the sample arrays to the instance
 * via the constructor or setArrays():
 *
 *     ArduinoFFT<double> FFT(vReal, vImag, samples, samplingFreq);
 *     FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
 *     FFT.compute(FFT_FORWARD);
 *     FFT.complexToMagnitude();
 *
 * The ESP32-DIV project (subghz.cpp line 492/1064-1066 and wifi.cpp line
 * 343/400-402) was written against the v1 API. PIO currently resolves
 * `kosme/arduinoFFT @ ^2.0.0` to v2.x, which breaks the project's source
 * with `'arduinoFFT' does not name a type; did you mean 'ArduinoFFT'?`.
 *
 * This shim defines an `arduinoFFT` class that mirrors the v1 API and
 * delegates to v2's `ArduinoFFT<double>` internally. The project's source
 * files continue to compile unchanged.
 */
#pragma once

#include "arduinoFFT.h"

class arduinoFFT {
private:
    ArduinoFFT<double> _fft;
    double  _samplingFreq = 0.0;

    // Re-bind the v2 instance to the arrays supplied by the caller. v2's
    // setArrays() requires non-null pointers, but Windowing() in v1 was
    // often called without first populating vImag — pass a scratch buffer
    // if the caller didn't supply one.
    static double *_safeImag(double *vImag, double &scratch) {
        return vImag ? vImag : &scratch;
    }

public:
    arduinoFFT() : _fft() {}

    // v1-compatible method: Windowing(vReal, samples, winType, dir)
    //
    // v2's setArrays() signature is `setArrays(T* vReal, T* vImag, uint_fast16_t samples)`
    // — there is no sampling-frequency parameter on this method (it is set
    // separately via the constructor or setSamplingFrequency() if needed).
    //
    // v1's Windowing() did not take a vImag argument, so we pass a scratch
    // double — setArrays() just stores the pointer; the actual complex data
    // is supplied later via Compute().
    void Windowing(double *vReal, uint16_t samples,
                   FFTWindow winType, FFTDirection dir) {
        double scratch = 0.0;
        double *imagPtr = _safeImag(nullptr, scratch);
        _fft.setArrays(vReal, imagPtr, samples);
        _fft.windowing(winType, dir);
    }

    // v1-compatible method: Compute(vReal, vImag, samples, dir)
    void Compute(double *vReal, double *vImag, uint16_t samples,
                 FFTDirection dir) {
        _fft.setArrays(vReal, vImag, samples);
        _fft.compute(dir);
    }

    // v1-compatible method: ComplexToMagnitude(vReal, vImag, samples)
    void ComplexToMagnitude(double *vReal, double *vImag, uint16_t samples) {
        _fft.setArrays(vReal, vImag, samples);
        _fft.complexToMagnitude();
    }

    // Allow the caller to set the sampling frequency if known up-front
    // (not used by the project's v1-style call sites today, but exposed
    // for completeness).
    void setSamplingFrequency(double freq) { _samplingFreq = freq; }
};
