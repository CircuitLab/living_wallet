#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#include <SFEMP3Shield.h>

SdFat sd;
SFEMP3Shield MP3player;

int debugPin = 13;

int delayTime = 500;

int command;

// Da Vinciとのコミュニケーション用
#define START_BYTE 0x7E
#define END_BYTE 0x88
#define COMMAND_NEUTRAL 0x00
#define COMMAND_ENABLE_CONSUME 0x10
#define COMMAND_DISABLE_CONSUME 0x20
#define COMMAND_AVOID 0x30
#define COMMAND_CAUGHT 0x40

volatile byte prevCommand;
volatile byte currentCommand;

char* trackNamesOnEnableConsume[] = { "track1_1.mp3" };
char* trackNamesOnCaught[] = { "track2_1.mp3", "track3_1.mp3" };

uint8_t currentStatus;
volatile byte state;

void setup()
{
  Serial.begin(57600);  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) sd.initErrorHalt();
  if(!sd.chdir("/")) sd.errorHalt("sd.chdir");
  
  currentStatus = MP3player.begin();
  if (0 != currentStatus) {
    Serial.print(F("Error code: "));
    Serial.print(currentStatus);
    Serial.println(F(" when trying to start MP3 player"));
    
    delayTime = 100;
    
    if(6 == currentStatus) {
      Serial.println(F("Warning: patch file not found, skipping."));
      Serial.println(F("Use the \"d\" command to verify SdCard can be read"));
    }
  }
  
  union twobyte mp3_vol;
  mp3_vol.word = MP3player.getVolume();
  mp3_vol.byte[1] = 2;
  MP3player.setVolume(mp3_vol.byte[1], mp3_vol.byte[1]);
  
  randomSeed(analogRead(0));
  
  pinMode(debugPin, OUTPUT);
}

void loop()
{
  if (0 != currentStatus) {
    debugBlink();
  }
  
  delay(10);
}

void debugBlink()
{
  digitalWrite(debugPin, HIGH);
  delay(delayTime);
  digitalWrite(debugPin, LOW);
  delay(delayTime);
}

void serialEvent()
{  
  while (Serial.available()) {
    if (START_BYTE == (char)Serial.read()) {
      byte command =  (char)Serial.read();
      
      if (COMMAND_ENABLE_CONSUME == command) {
        currentCommand = COMMAND_ENABLE_CONSUME;
      } else if (COMMAND_DISABLE_CONSUME == command) {
        currentCommand = COMMAND_DISABLE_CONSUME;
      } else if (COMMAND_AVOID == command) {
        currentCommand = COMMAND_AVOID;
      } else if (COMMAND_CAUGHT == command) {
        currentCommand = COMMAND_CAUGHT;
      } else {
        currentCommand = COMMAND_NEUTRAL;
      }
      
      if (prevCommand != currentCommand) {
        uint8_t result;
        
        if (MP3player.isPlaying()) {
          MP3player.stopTrack();
          delay(10);
        }
        
        if (COMMAND_ENABLE_CONSUME == currentCommand) {
          result = MP3player.playMP3(trackNamesOnEnableConsume[0], 0);
        } else if (COMMAND_DISABLE_CONSUME == currentCommand) {
          result = 0;
        } else if (COMMAND_AVOID == currentCommand) {
          result = 0;
        } else if (COMMAND_CAUGHT == currentCommand) {
          result = MP3player.playMP3(trackNamesOnCaught[random(2)], 0);
        } else {
          MP3player.stopTrack();
          result = 0;
        }
        
        if (0 != result) {
          Serial.print((char)result);
        }
      }
      
      prevCommand = currentCommand;
      
      Serial.print((char)currentCommand);
      Serial.flush();
      break;
    }
  }
}
