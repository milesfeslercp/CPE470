#ifndef GENRE_MODEL_SETTINGS_H_
#define GENRE_MODEL_SETTINGS_H_

// Audio sampling configuration
constexpr int kAudioSampleFrequency = 16000;
constexpr int kAudioSampleDuration = 3000;
constexpr int kAudioSampleSize = kAudioSampleFrequency * 2;  // Reduced from 3 to 2 seconds

// Reduced feature sizes
constexpr int kFeatureSliceSize = 32;  // Reduced from 64
constexpr int kFeatureSize = 32 * 32;  // Reduced from 64 * 49

// Model settings
constexpr int kCategoryCount = 10;
extern const char* kCategoryLabels[kCategoryCount];

// Feature extraction settings
constexpr int kFeatureWindowSize = 256;  // Reduced from 512
constexpr int kFeatureHopSize = 128;     // Reduced from 256
constexpr float kFeatureLowerHz = 0;
constexpr float kFeatureUpperHz = 8000;

#endif  // GENRE_MODEL_SETTINGS_H_