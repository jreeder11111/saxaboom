# Saxaboom firmware

Based on the 'player-sd-audiokit.ino' example from 'arduino-audio-tools'

Customized to work on the AiThinker ESP-32 Audikit board (https://www.amazon.com/dp/B0B63KZ6C1)

Summary of attempts:

1. Worked: switched from 'CodecMP3Helix' to 'CodecMP3MAD'
1. Tools->PSRAM enabled/disabled - made no difference for either MP3 codecs
1. FS and SD card: FAT32 on 16GB SD card worked; need to retest 128GB card and exFAT
1. File name length: longer than 8.3 worked
1. DIP switches: down, up, up, down, down (as per 'player-sd-audiokit.ino' comments)
1. Board drivers (per https://github.com/pschatzmann/arduino-audio-tools/wiki/Audio-Boards#esp32-a1s-based-audio-boards-audiokit-lyrat):
   1. worked: AudioKitEs8388V1 
   1. didn't work: LyratV43, LyratV42, AudioKitAC101, AudioKitEs8388V2 (listed on above web page)
1. MP3 file that worked: From Chicago II: 1-01 Movin' In.mp3, which I think is one I ripped myself a long time ago; placed in root of 16GB SD card
1. Separate SPI initialization: used in working configuration; tested later with it commented out and it still worked
