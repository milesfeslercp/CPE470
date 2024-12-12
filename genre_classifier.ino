// genre_classifier.ino
#include <PDM.h>
#include <TensorFlowLite.h>

#include "main_functions.h"
#include "audio_provider.h"
#include "feature_provider.h"
#include "genre_model_settings.h"
#include "genre_responder.h"
#include "model.h"

extern "C" char* sbrk(int incr);
int freeRam() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void setup() {
  // Initialize serial with a timeout
  Serial.begin(9600);
  unsigned long startTime = millis();
  while (!Serial && (millis() - startTime) < 5000) {
    ;  // Wait up to 5 seconds for serial connection
  }

  Serial.println(freeRam());

  Serial.println("Starting TEMPO initialization...");

  // Initialize PDM
  if (!PDM.begin(1, kAudioSampleFrequency)) {
    Serial.println("Failed to initialize PDM!");
    while (1)
      ;  // Halt if PDM fails
  }

  Serial.println("PDM initialized successfully");

  Serial.println(freeRam());


  // Call the main setup function
  setup_genre_classifier();
  Serial.println("Genre classifier initialization complete");

  // Print some diagnostic info
  Serial.print("Sample Rate: ");
  Serial.println(kAudioSampleFrequency);
  Serial.print("Sample Duration: ");
  Serial.println(kAudioSampleDuration);
  Serial.print("Feature Size: ");
  Serial.println(kFeatureSize);
}

void loop() {
  static unsigned long lastPrint = 0;
  static unsigned long loopCount = 0;

  // Run the classifier
  loop_genre_classifier();
  loopCount++;

  // Print diagnostic info every 5 seconds
  if (millis() - lastPrint > 5000) {
    Serial.print("Loops completed: ");
    Serial.println(loopCount);
    Serial.print("Time running (ms): ");
    Serial.println(millis());
    lastPrint = millis();
  }

  // Small delay to prevent overwhelming the serial output
  delay(10);
}
