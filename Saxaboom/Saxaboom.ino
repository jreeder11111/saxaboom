#include <AudioFileSourceSD.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>
#include <SPI.h>
#include <SD.h>

// Requires the ESP8266Audio library (https://github.com/earlephilhower/ESP8266Audio)
// Files are loaded on the microSD card, named [1-8].wav

// These pins correspond to the Sonatino board (https://sonatino.com):
#define DAC_BCLK_PIN          15
#define DAC_LRCLK_PIN         16
#define DAC_DATA_PIN          17
#define ADC_BCLK_PIN          38
#define ADC_LRCLK_PIN         39
#define ADC_DATA_PIN          40
#define MICROSD_SPI_SS_PIN    41
#define MICROSD_SPI_SCK_PIN   42
#define MICROSD_SPI_MOSI_PIN  43
#define MICROSD_SPI_MISO_PIN  44


#define NUM_BUTTONS 8
#define LOOP_SAMPLES true
#define VOL_EVERY_MS 100
#define VOLUME_PIN 13
#define STOP_PIN 11

const uint8_t buttonPins[NUM_BUTTONS] = {1, 2, 3, 4, 5, 6, 7, 8};
uint8_t lastPlayed = 0;
unsigned long lastVolAt = 0;

AudioGeneratorWAV *wav;
AudioFileSourceSD *files[NUM_BUTTONS];
AudioOutputI2S *out;


void setup() {
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  pinMode(STOP_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  SPI.begin(MICROSD_SPI_SCK_PIN, MICROSD_SPI_MISO_PIN, MICROSD_SPI_MOSI_PIN, MICROSD_SPI_SS_PIN);
  SD.begin(MICROSD_SPI_SS_PIN, SPI, 4000000, "/sd", 10, false);

  out = new AudioOutputI2S();
  out->SetPinout(DAC_BCLK_PIN, DAC_LRCLK_PIN, DAC_DATA_PIN);
  out->SetGain(0.3);
  out->begin();

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    String fileName = String("/") + (i + 1) + ".wav";
    files[i] = new AudioFileSourceSD(fileName.c_str());
  }

  wav = new AudioGeneratorWAV();
  Serial.println("Ready");
}

void loop() {
  if (wav->isRunning()) {
    wav->loop();
  } else {
    // Loop sample
    stop();
    if (LOOP_SAMPLES && lastPlayed > 0) {
      playWAV(lastPlayed);
    }
  }
  
  unsigned long now = millis();
  if (now - lastVolAt > VOL_EVERY_MS) {
    lastVolAt = now;
    out->SetGain(((float)analogRead(VOLUME_PIN)/4096.0) * 0.5);
  }

  checkForButtonPress();
}

void stop() {
  if (wav->isRunning()) {
    wav->stop();
  }
  // Reopen to be ready
  if (lastPlayed > 0 && !files[lastPlayed - 1]->isOpen()) {
    Serial.printf("Reopening file: %d\n", lastPlayed);
    String fileName = String("/") + lastPlayed + ".wav";
    files[lastPlayed - 1] = new AudioFileSourceSD(fileName.c_str());
  }
}

void playWAV(uint8_t fileNumber) {
  lastPlayed = fileNumber;
  stop();

  if (!wav->begin(files[fileNumber - 1], out)) {
    Serial.printf("Error playing %s\n", String(fileNumber) + ".wav");
    return;
  }
}

void checkForButtonPress() {
  if (digitalRead(STOP_PIN) == LOW) {
    lastPlayed = 0;
    stop();
    return;
  }
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      Serial.printf("Playing: %d\n", i+1);
      if (i + 1 != lastPlayed) {
        playWAV(i + 1);
      }
    }
  }
}
