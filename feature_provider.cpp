#include "feature_provider.h"
#include "audio_provider.h"
#include "genre_model_settings.h"
#include <Arduino.h>

// Pre-calculated constants and buffers
namespace {
  const float MY_PI = 3.14159265358979323846;
  float window_buffer[kFeatureWindowSize];  // Using kFeatureWindowSize from settings
  float mel_energies[kFeatureSliceSize];
}

FeatureProvider::FeatureProvider(int feature_size, int8_t* feature_data)
    : feature_size_(feature_size),
      feature_data_(feature_data) {
      
  // Initialize Hanning window
  for (int i = 0; i < kFeatureWindowSize; i++) {
    window_buffer[i] = 0.5f * (1.0f - cos(2.0f * MY_PI * i / (kFeatureWindowSize - 1)));
  }
  
  // Initialize mel filterbank
  initMelFilterbank();
}

float FeatureProvider::hzToMel(float hz) {
  return 2595.0f * log10f(1.0f + hz / 700.0f);
}

float FeatureProvider::melToHz(float mel) {
  return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f);
}

void FeatureProvider::initMelFilterbank() {
  float mel_low = hzToMel(kFeatureLowerHz);
  float mel_high = hzToMel(kFeatureUpperHz);
  float mel_step = (mel_high - mel_low) / (kFeatureSliceSize + 1);
  
  // Generate center frequencies
  float mel_centers[kFeatureSliceSize];
  for (int i = 0; i < kFeatureSliceSize; i++) {
    mel_centers[i] = mel_low + mel_step * (i + 1);
  }
  
  // Generate triangular filters
  for (int i = 0; i < kFeatureSliceSize; i++) {
    float left_mel = mel_centers[i] - mel_step;
    float center_mel = mel_centers[i];
    float right_mel = mel_centers[i] + mel_step;
    
    for (int j = 0; j < kFeatureWindowSize/2; j++) {
      float freq = j * kAudioSampleFrequency / kFeatureWindowSize;
      float mel = hzToMel(freq);
      
      if (mel < left_mel || mel > right_mel) {
        mel_filterbank_[i][j] = 0.0f;
      } else if (mel <= center_mel) {
        mel_filterbank_[i][j] = (mel - left_mel) / (center_mel - left_mel);
      } else {
        mel_filterbank_[i][j] = (right_mel - mel) / (right_mel - center_mel);
      }
    }
  }
}

void FeatureProvider::computePowerSpectrum(const int16_t* input, float* output) {
  // Apply window and compute power
  for (int i = 0; i < kFeatureWindowSize/2; i++) {
    float windowed = input[i] * window_buffer[i];
    output[i] = windowed * windowed;
  }
}

TfLiteStatus FeatureProvider::PopulateFeatureData(
    tflite::ErrorReporter* error_reporter, int32_t last_time_in_ms,
    int32_t time_in_ms, int* how_many_new_slices) {
    
  int16_t* audio_samples = nullptr;
  int audio_samples_size = 0;

  // Get audio samples
  GetAudioSamples(error_reporter, time_in_ms - kAudioSampleDuration,
                  kAudioSampleDuration, &audio_samples_size, &audio_samples);

  if (audio_samples_size < kFeatureWindowSize) {
    return kTfLiteError;
  }

  // Temporary buffer for power spectrum
  float power_spectrum[kFeatureWindowSize/2];
  computePowerSpectrum(audio_samples, power_spectrum);

  // Apply mel filterbank
  for (int i = 0; i < kFeatureSliceSize; i++) {
    float mel_energy = 0;
    for (int j = 0; j < kFeatureWindowSize/2; j++) {
      mel_energy += power_spectrum[j] * mel_filterbank_[i][j];
    }
    mel_energies[i] = mel_energy;
  }

  // Convert to log scale and quantize
  for (int i = 0; i < kFeatureSliceSize; i++) {
    float mel_energy = mel_energies[i];
    if (mel_energy < 1e-10) mel_energy = 1e-10;
    float log_energy = 10.0f * log10f(mel_energy);
    
    // Normalize to int8 range (-128 to 127)
    int8_t quantized = (int8_t)constrain(
      map(log_energy, -40, 40, -128, 127),
      -128, 127
    );
    feature_data_[i] = quantized;
  }

  *how_many_new_slices = 1;
  return kTfLiteOk;
}