#if defined(ARDUINO) && !defined(ARDUINO_ARDUINO_NANO33BLE)
#define ARDUINO_EXCLUDE_CODE
#endif  // defined(ARDUINO) && !defined(ARDUINO_ARDUINO_NANO33BLE)

#ifndef ARDUINO_EXCLUDE_CODE

#include "audio_provider.h"
#include "genre_model_settings.h"
#include <PDM.h>

namespace {
bool g_is_audio_initialized = false;
// Increased buffer size to handle PDM data more reliably
constexpr int kAudioCaptureBufferSize = DEFAULT_PDM_BUFFER_SIZE * 8;
int16_t g_audio_capture_buffer[kAudioCaptureBufferSize];
int16_t g_audio_output_buffer[kAudioSampleSize];
volatile int32_t g_latest_audio_timestamp = 0;
volatile bool pdm_data_ready = false;
}  // namespace

void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();
  
  // Read into capture buffer
  PDM.read(g_audio_capture_buffer, bytesAvailable);
  pdm_data_ready = true;
  
  // Update timestamp
  g_latest_audio_timestamp = millis();
}

TfLiteStatus InitAudioRecording(tflite::ErrorReporter* error_reporter) {
  // Initialize PDM with:
  // - One channel (mono)
  // - 16 kHz sample rate
  if (!PDM.begin(1, kAudioSampleFrequency)) {
    Serial.println("Failed to start PDM!");
    return kTfLiteError;
  }

  // Configure the data receive callback
  PDM.onReceive(onPDMdata);
  
  // Increase PDM buffer size (helps with stability)
  PDM.setBufferSize(kAudioCaptureBufferSize);
  
  // Set gain
  PDM.setGain(80);  // Increased from 20 to 80 for better sensitivity

  Serial.println("PDM initialized successfully");
  
  // Wait for first audio samples
  int timeout = 0;
  while (!pdm_data_ready && timeout < 10) {
    delay(100);
    timeout++;
  }
  
  if (!pdm_data_ready) {
    Serial.println("Timeout waiting for PDM data!");
    return kTfLiteError;
  }
  
  Serial.println("Audio recording initialized successfully");
  return kTfLiteOk;
}

TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                             int start_ms, int duration_ms,
                             int* audio_samples_size, int16_t** audio_samples) {
  // Set everything up to start receiving audio
  if (!g_is_audio_initialized) {
    TfLiteStatus init_status = InitAudioRecording(error_reporter);
    if (init_status != kTfLiteOk) {
      Serial.println("Failed to initialize audio recording!");
      return init_status;
    }
    g_is_audio_initialized = true;
  }

  // Ensure we have fresh data
  if (!pdm_data_ready) {
    Serial.println("Waiting for PDM data...");
    return kTfLiteError;
  }

  // Clear the ready flag
  pdm_data_ready = false;

  // Determine the index, in the history of all samples, of the first sample we want
  const int start_offset = start_ms * (kAudioSampleFrequency / 1000);
  
  // Determine how many samples we want in total
  const int duration_sample_count = duration_ms * (kAudioSampleFrequency / 1000);
  
  // Copy samples to output buffer
  for (int i = 0; i < duration_sample_count; ++i) {
    const int capture_index = (start_offset + i) % kAudioCaptureBufferSize;
    g_audio_output_buffer[i] = g_audio_capture_buffer[capture_index];
  }

  // Output debug info occasionally
  static int debug_counter = 0;
  if (++debug_counter >= 100) {
    debug_counter = 0;
    Serial.print("Audio buffer stats - Max value: ");
    int16_t max_val = 0;
    for (int i = 0; i < duration_sample_count; i++) {
      if (abs(g_audio_output_buffer[i]) > max_val) {
        max_val = abs(g_audio_output_buffer[i]);
      }
    }
    Serial.println(max_val);
  }

  *audio_samples_size = kAudioSampleSize;
  *audio_samples = g_audio_output_buffer;

  return kTfLiteOk;
}

int32_t LatestAudioTimestamp() {
  return g_latest_audio_timestamp;
}

#endif  // ARDUINO_EXCLUDE_CODE