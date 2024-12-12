#include "genre_responder.h"
#include "genre_model_settings.h"
#include <Arduino.h>

// Define LED pins (adjust based on your Arduino board)
const int LED_R = LEDR;  // Red LED pin
const int LED_G = LEDG;  // Green LED pin
const int LED_B = LEDB;  // Blue LED pin

// Color mapping for different genres
struct GenreColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

const GenreColor GENRE_COLORS[kCategoryCount] = {
  {255, 0, 0},    // Blues - Red
  {255, 255, 255}, // Classical - White
  {255, 165, 0},  // Country - Orange
  {148, 0, 211},  // Disco - Purple
  {255, 255, 0},  // Hip Hop - Yellow
  {0, 0, 255},    // Jazz - Blue
  {128, 128, 128}, // Metal - Gray
  {255, 192, 203}, // Pop - Pink
  {0, 255, 0},    // Reggae - Green
  {165, 42, 42}   // Rock - Brown
};

void RespondToGenre(tflite::ErrorReporter* error_reporter,
                    int32_t current_time, const char* genre,
                    uint8_t score, bool is_new_classification) {
  static bool is_initialized = false;
  
  // Initialize LED pins
  if (!is_initialized) {
    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_B, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    is_initialized = true;
    
    // Debug output
    Serial.println("Genre responder initialized");
  }

  // Add periodic heartbeat output
  static unsigned long last_heartbeat = 0;
  if (current_time - last_heartbeat > 1000) {  // Every second
    Serial.println("Heartbeat - Genre classifier running");
    last_heartbeat = current_time;
  }

  // Only respond to new classifications with high confidence
  if (is_new_classification) {
    // Debug output regardless of score
    Serial.print("Classification: ");
    Serial.print(genre);
    Serial.print(" (score: ");
    Serial.print(score);
    Serial.println(")");
    
    if (score > 180) { // Threshold of ~70%
      // Find the genre index
      int genre_idx = 0;
      for (; genre_idx < kCategoryCount; genre_idx++) {
        if (strcmp(genre, kCategoryLabels[genre_idx]) == 0) break;
      }
      
      if (genre_idx < kCategoryCount) {
        // Get color for the genre
        GenreColor color = GENRE_COLORS[genre_idx];
        
        // Set RGB LED color (note: LEDs are active LOW on Nano 33 BLE)
        analogWrite(LED_R, 255 - color.r);
        analogWrite(LED_G, 255 - color.g);
        analogWrite(LED_B, 255 - color.b);
        
        // Log the detection
        Serial.print("High confidence detection - Genre: ");
        Serial.print(genre);
        Serial.print(" Score: ");
        Serial.print(score);
        Serial.print(" Time: ");
        Serial.println(current_time);
      }
      
      // Flash built-in LED to indicate detection
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}