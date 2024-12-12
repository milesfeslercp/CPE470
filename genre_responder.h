// genre_responder.h
#ifndef GENRE_RESPONDER_H_
#define GENRE_RESPONDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

// Called every time a new genre classification result is available
void RespondToGenre(tflite::ErrorReporter* error_reporter,
                   int32_t current_time, const char* genre,
                   uint8_t score, bool is_new_classification);

#endif  // GENRE_RESPONDER_H_