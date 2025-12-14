/* For Ai Thinker Audiokit board:
*  1. Install 'esp32' board library
*  2. Select esp32->ESP32 Dev Module 
*/

#define WAV 1
#define MP3 2
#define FORMAT WAV

#include "AudioTools.h"
#include "AudioTools/AudioLibs/AudioBoardStream.h"
#include "AudioTools/Disk/AudioSourceSD.h"  // or AudioSourceIdxSD.h

#if (FORMAT == WAV)
#include "AudioTools/AudioCodecs/CodecWAV.h"
#elif (FORMAT == MP3)
// #include "AudioTools/AudioCodecs/CodecMP3Helix.h" // doesn't work with AI Thinker AudioKit board - crashes board, sometimes with a 'arduino guru meditation error storeprohibited exception was unhandled' messages, something without anything
#include "AudioTools/AudioCodecs/CodecMP3MAD.h"
#endif

#if (FORMAT == WAV)
const char* ext = "wav";
#elif (FORMAT == MP3)
const char* ext = "mp3";
#endif
const char* startFilePath = "/";

class MyAudioSourceSD : public AudioSourceSD {
public:
  MyAudioSourceSD(const char *startFilePath = "/", const char *ext = ".mp3", int chipSelect = PIN_CS, bool setupIndex=true)
    : AudioSourceSD(startFilePath, ext, chipSelect, setupIndex) {
      
  }

  virtual bool isAutoNext() { return false; }

  const char* name(int index) {
    return idx[index];
  }

  float readVolumeSettingFile() {
    float rval = -1.0;
    File file = SD.open("/volume.txt");
    if (!file) {
      LOGW("No volume file")
    } else {
      String line = file.readStringUntil('\n');
      line.trim();
      float volume = line.toFloat();
      if (volume >= 0 && volume <= 1.0) {
        LOGW("Read volume %f from file", volume)
        rval = volume;
      } else {
        LOGW("Invalid volume in file: %s", line.c_str())
      }
      file.close();
    }

    return rval;
  }

  void writeVolumeSettingFile(float volume) {
    File file = SD.open("/volume.txt", FILE_WRITE);
    if (!file) {
      LOGW("Error opening volume file for writing")
    } else {
      file.print(volume, 2);
      file.close();
    } 
  }
};

MyAudioSourceSD source(startFilePath, ext, PIN_AUDIO_KIT_SD_CARD_CS);


class MyPinsAudioKitEs8388v1Class : public DriverPins {
 public:
  MyPinsAudioKitEs8388v1Class() {
    // sd pins
    addSPI(ESP32PinsSD);
    // add i2c codec pins: scl, sda, port, frequency
    addI2C(PinFunction::CODEC, 32, 33);
    // add i2s pins: mclk, bck, ws,data_out, data_in ,(port)
    addI2S(PinFunction::CODEC, 0, 27, 25, 26, 35);

    // add other pins
    addPin(PinFunction::KEY, 36, PinLogic::InputActiveLow, 1);
    addPin(PinFunction::KEY, 13, PinLogic::InputActiveLow, 2);
    addPin(PinFunction::KEY, 19, PinLogic::InputActiveLow, 3);
    addPin(PinFunction::KEY, 23, PinLogic::InputActiveLow, 4);
    addPin(PinFunction::KEY, 18, PinLogic::InputActiveLow, 5);
    addPin(PinFunction::CODEC_ADC, 5, PinLogic::Input);
    addPin(PinFunction::AUXIN_DETECT, 12, PinLogic::InputActiveLow);
    addPin(PinFunction::HEADPHONE_DETECT, 39, PinLogic::InputActiveLow);
    addPin(PinFunction::PA, 21, PinLogic::Output);
    addPin(PinFunction::LED, 22, PinLogic::Output);
  }
};
static MyPinsAudioKitEs8388v1Class MyPinsAudioKitEs8388v1;
static AudioBoard MyAudioKitEs8388V1{AudioDriverES8388, MyPinsAudioKitEs8388v1};
// AudioBoardStream kit(MyAudioKitEs8388V1);
AudioBoardStream kit(AudioKitEs8388V1);

#if (FORMAT == WAV)
WAVDecoder decoder;
#elif (FORMAT == MP3)
// MP3DecoderHelix decoder;  // or change to MP3DecoderMAD
MP3DecoderMAD decoder;  // or change to MP3DecoderMAD
#endif
AudioPlayer player(source, kit, decoder);

// AnalogAudioStream adc;
// void adcBegin() {
//     // Get default configuration for RX_MODE (input)
//   auto cfgRx = adc.defaultConfig(RX_MODE);

//   // Set desired sample rate and channels
//   cfgRx.sample_rate = 1000; // Or your desired sample rate
//   cfgRx.channels = 1;

//   // Crucially, set the input pin
//   cfgRx.setInputPin1(ADC_INPUT_LINE2);

//   // Begin the ADC stream with the configured settings
//   adc.begin(cfgRx);
// }

void info() {
  LOGW("Number of files: %d", source.size())
  for (int i = 0; i < source.size(); ++i) {
    LOGW("  %d: %s", i, source.name(i))
  }
}

void sound0(bool, int, void*) {
  LOGW("sound0");
  // info();
  // player.setIndex(0);
  player.setPath("/sbLoop1.wav");
}

void sound1(bool, int, void*) {
  LOGW("sound1");
  player.setPath("/sbLoop2.wav");
}

void sound2(bool, int, void*) {
  LOGW("sound2");
  player.setPath("/sbLoop3.wav");
}

void sound3(bool, int, void*) {
  LOGW("sound3");
  player.setPath("/sbLoop4.wav");
}

void sound4(bool, int, void*) {
  LOGW("sound4");
  player.setPath("/sbLoop5.wav");
}

void sound5(bool, int, void*) {
  LOGW("sound5");
  player.setPath("/sbLoop6.wav");
}

// void sound6(bool, int, void*) {
//   LOGW("sound6");
//   player.setIndex(6);
// }

// void sound7(bool, int, void*) {
//   LOGW("sound7");
//   player.setIndex(7);
// }

void next(bool, int, void*) {
  player.next();
}

void previous(bool, int, void*) {
  player.previous();
}

void startStop(bool, int, void*) {
  player.setActive(!player.isActive());
}

float configureVolume() {
  // Read the stored volume
  float storedVolume = source.readVolumeSettingFile();
  
  // Determine the key volume by which key is pressed
  float keyVolume = 1.0;
  float step = (keyVolume - 0.1) / 6.0;
  bool keyPressed = false;
  for (int key = 1; key <= 6; key++) {
    if (kit.isKeyPressed(key)) {
      keyPressed = true;
      break;
    }
    keyVolume -= step;
  }

  float retVolume = -1.0;
  if (keyPressed) {
    retVolume = keyVolume;
    source.writeVolumeSettingFile(retVolume);
  } else {
    retVolume = storedVolume;
  }

  if (retVolume < 0.0 || retVolume > 1.0) {
    LOGW("retVolume out of bounds: %f");
    retVolume = 0.5;
  }

  return retVolume;
}

void setup() {
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

  // setup output
  auto cfg = kit.defaultConfig(RXTX_MODE);
  // Initialize SD card
  // SPI.begin(PIN_AUDIO_KIT_SD_CARD_CLK, PIN_AUDIO_KIT_SD_CARD_MISO,
  //           PIN_AUDIO_KIT_SD_CARD_MOSI, PIN_AUDIO_KIT_SD_CARD_CS);
  cfg.sd_active = true;
  cfg.input_device = ADC_INPUT_ALL;
  kit.begin(cfg);

  // Read the files from the SD card
  source.begin();
  // source.readVolumeSettingFile();
  
  // for (int key = 1; key <= 6; key++) {
  //   LOGW("key %d pressed %d", key, kit.isKeyPressed(key));
  // }
  float volume = configureVolume();


  // setup additional buttons
  kit.addDefaultActions();
  // kit.addAction(kit.getKey(1), startStop);
  // kit.addAction(kit.getKey(4), next);
  // kit.addAction(kit.getKey(3), previous);
  kit.addAction(kit.getKey(1), sound0);
  kit.addAction(kit.getKey(2), sound1);
  kit.addAction(kit.getKey(3), sound2);
  kit.addAction(kit.getKey(4), sound3);
  kit.addAction(kit.getKey(5), sound4);
  kit.addAction(kit.getKey(6), sound5);

  // // Read the files from the SD card
  // source.begin();

  // setup player
  player.setAutoFade(false);
  // player.setVolume(0.7);
  LOGW("Set volume to %f", volume);
  player.setVolume(volume);
  // player.setActive(false);
  player.begin(0, false);

  // select file with setPath() or setIndex()
  // player.setPath("/1-01 Movin' In.mp3");
  // player.setIndex(0); 

  // adcBegin();
}

void loop() {
  player.copy();
  kit.processActions();

}