#ifndef MODEL_H_
#define MODEL_H_

#include "tensorflow/lite/schema/schema_generated.h"

// Expose model data with extern keyword and proper linkage
#ifdef __cplusplus
extern "C" {
#endif

extern const unsigned char g_model[];
extern const unsigned int g_model_len;

#ifdef __cplusplus
}
#endif

#endif  // MODEL_H_