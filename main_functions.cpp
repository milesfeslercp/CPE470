#include "main_functions.h"
#include "audio_provider.h"
#include "feature_provider.h"
#include "genre_model_settings.h"
#include "genre_responder.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "model.h"
#include <Arduino.h>

namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  
  constexpr int kTensorArenaSize = 50 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
  
  FeatureProvider* feature_provider = nullptr;
  int8_t feature_buffer[kFeatureSize];
  const char* previous_genre = nullptr;
}

void setup_genre_classifier() {
  // Initialize serial with a timeout
  Serial.begin(9600);
  unsigned long start = millis();
  while (!Serial && (millis() - start < 5000)) {
    ; // Wait up to 5 seconds for serial port to connect
  }
  
  Serial.println("Starting genre classifier setup...");
  
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  Serial.println("Loading model...");
  // Map the model into a usable data structure
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model version mismatch!");
    return;
  }

  Serial.println("Setting up operations...");
  static tflite::MicroMutableOpResolver<6> micro_op_resolver;
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddMaxPool2D();
  micro_op_resolver.AddFullyConnected();
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddSoftmax();
  micro_op_resolver.AddQuantize();

  Serial.println("Creating interpreter...");
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;

  Serial.println("Allocating tensors...");
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  Serial.println(allocate_status);
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed");
    return;
  }

  model_input = interpreter->input(0);
  
  Serial.println("Setting up feature provider...");
  static FeatureProvider static_feature_provider(kFeatureSize, feature_buffer);
  feature_provider = &static_feature_provider;
  
  Serial.println("Setup complete!");
}

void loop_genre_classifier() {
  const int32_t current_time = millis();
  
  int how_many_new_slices = 0;
  TfLiteStatus feature_status = feature_provider->PopulateFeatureData(
      error_reporter, 0, current_time, &how_many_new_slices);
      
  if (feature_status != kTfLiteOk) {
    Serial.println("Feature generation failed");
    return;
  }

  // Run the model on the spectrogram input
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed");
    return;
  }

  // Get the classification results
  TfLiteTensor* output = interpreter->output(0);
  uint8_t top_category_score = 0;
  int top_category_index = 0;
  
  // Find the best score and category
  for (int category_index = 0; category_index < kCategoryCount; ++category_index) {
    const uint8_t score = output->data.uint8[category_index];
    if (score > top_category_score) {
      top_category_score = score;
      top_category_index = category_index;
    }
  }

  const char* new_genre = kCategoryLabels[top_category_index];
  const bool is_new_genre = (new_genre != previous_genre);
  
  // Respond to the results
  RespondToGenre(error_reporter, current_time, new_genre, 
                 top_category_score, is_new_genre);
                 
  // Update previous genre
  previous_genre = new_genre;
  
  // Small delay to prevent overwhelming the system
  delay(50);
}