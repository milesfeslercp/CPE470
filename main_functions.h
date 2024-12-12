#ifndef MAIN_FUNCTIONS_H_
#define MAIN_FUNCTIONS_H_

#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initializes all data needed for the example
void setup_genre_classifier();

// Runs one iteration of data gathering and inference
void loop_genre_classifier();

#ifdef __cplusplus
}
#endif

#endif  // MAIN_FUNCTIONS_H_