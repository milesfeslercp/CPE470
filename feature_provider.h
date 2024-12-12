// feature_provider.h
#ifndef FEATURE_PROVIDER_H_
#define FEATURE_PROVIDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "genre_model_settings.h"

class FeatureProvider {
 public:
  FeatureProvider(int feature_size, int8_t* feature_data);
  
  TfLiteStatus PopulateFeatureData(tflite::ErrorReporter* error_reporter,
                                  int32_t last_time_in_ms, int32_t time_in_ms,
                                  int* how_many_new_slices);

 private:
  void initMelFilterbank();
  void computePowerSpectrum(const int16_t* input, float* output);
  float hzToMel(float hz);
  float melToHz(float mel);
  
  int feature_size_;
  int8_t* feature_data_;
  
  // Mel filterbank coefficients
  float mel_filterbank_[kFeatureSliceSize][kFeatureWindowSize/2];
};

#endif  // FEATURE_PROVIDER_H_