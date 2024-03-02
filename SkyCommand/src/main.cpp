#include <Arduino.h>

#ifdef USE_LCD
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#endif

#ifdef ARDUINO_BLACKPILL_F411CE
#include <f4/bootloader.h>
#ifndef CMD_DFU_TIMEOUT_US
#define CMD_DFU_TIMEOUT_US 10000000UL
#endif // CMD_DFU_TIMEOUT_US
#endif // ARDUINO_BLACKPILL_F411CE

#ifdef USE_THERMOCOUPLE
#include "thermocouple.h"
#endif

#include "roaster.h"
#include "skywalker-protocol.h"

typedef struct {
  uint8_t heat;
  uint8_t vent;
  uint8_t cool;
  uint8_t filter;
  uint8_t drum;
} t_stateRoaster;

uint8_t receiveBuffer[ROASTER_MESSAGE_LENGTH];
SWControllerTx roasterController;
t_stateRoaster roasterState;

/*
 * Until this is replaced by an Class, this structure
 * contains the entire state, status, config, reported
 * and commanded roaster status
 */
t_State state = {
  // t_StateCommanded
  {0, 0, 0, 0, 0},
  // t_StateReported
  {0, 0, 0, 0},
  // t_Config
  {
    // chanMapping
#ifdef USE_THERMOCOUPLE
    {TEMPERATURE_CHANNEL_ROASTER+1, TEMPERATURE_CHANNEL_THERMOCOUPLE+1, 0, 0},
#else
    {0, TEMPERATURE_CHANNEL_ROASTER+1, 0, 0},
#endif // USE_THERMOCOUPLE
    'F',
  },
  // t_Status
  {
    0,      // tc4LastTick
    false,  // isSynchronized
#ifdef USE_THERMOCOUPLE
    1       // tcStatus
#endif
  }
};

void JumpToBootloader(void);

#ifdef USE_LCD
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void setupLCD(void);
void welcomeLCD(void);
void updateLCD(void);

void setupLCD(void) {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
  }
}

void welcomeLCD(void) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("Skyduino Roaster"));
  display.print(F("Version: "));
  display.println(F(SKYDUINO_VERSION));
  display.display();
}

void updateLCD(void) {
  static ustick_t lcd_last_tick = micros();
  static bool invert_text = false;
  ustick_t tick = micros();

  if ((tick - lcd_last_tick) < (unsigned long)(UPDATE_LCD_PERIOD_MS * 1000)) {
    // no need to update yet
    return;
  }

  lcd_last_tick = tick;
  display.clearDisplay();
  display.setCursor(0,0);
  if (!(state.status.isSynchronized)) {
    if (invert_text) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    }
    invert_text = !invert_text;
    display.setTextSize(2);
    display.println(F("Sync Loss"));
    display.display();
    return;
  }
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print(F("Temp: "));
  uint8_t mapping = 0;
  for (uint8_t i = 0; i < 2; i++) {
    mapping = state.cfg.chanMapping[i];
    if (mapping
        && (mapping >= 1)
        && (mapping <= TEMPERATURE_CHANNELS_MAX)) {
        display.print(state.reported.chanTemp[mapping - 1]);
        display.print(F(" "));
    } else {
      display.print(F("0.0 "));
    }
  }
  if (state.cfg.CorF == 'F') {
    display.println(F("F"));
  } else {
    display.println(F("C"));
  }
  

  // New line
  display.print(F("Heat: "));
  display.print(state.commanded.heat, 16);
  display.print(F(" Vent: "));
  display.print(state.commanded.vent, 16);
  display.println();

  // New line
  display.print(F("Fltr: "));
  display.print(state.commanded.filter, 16);
  display.print(F(" Cool: "));
  display.print(state.commanded.cool, 16);
  display.println();

  // New line
  if (state.commanded.drum) {
    display.println(F("Drum is On"));
  } else {
    display.println(F("Drum is Off"));
  }
  display.display();
}
#else
#define setupLCD(x)
#define welcomeLCD(x)
#define updateLCD(x)
#endif


double calculateTemp() {
  /* 
    I really hate this. 
    It seems to work.. but I feel like there must be a better way than 
    using a 4th degree polynomial to model this but I sure can't seem to figure it out. 
  */

  double x = ((receiveBuffer[0] << 8) + receiveBuffer[1]) / 1000.0;
  double y = ((receiveBuffer[2] << 8) + receiveBuffer[3]) / 1000.0;

  DEBUG(x);
  DEBUG(',');
  DEBUGLN(y);

  double v;
  v = 583.1509258523457 
      + x * (-714.0345395202813 + 2238.149675349052 * y
      + x * (413.37964344228334 + y * (-5001.419602972793
      + -6885.682277959339 * y)
      + x * (357.49007607425233 + 3879.431274654493 * y
      + x * (-555.8643213534281))))
      + y * (-196.071718077524
      + y * (-4099.91031297056 + 8242.08618555862 * x
      + y * (247.6124684730026 + 2868.4191998911865 * x
      + y * (-1349.1588373011923))));

  if ( state.cfg.CorF == 'C' ) v = (v - 32) * 5 / 9;

  return v;
}

bool getMessage(int bytes, int pin) {
  unsigned long pulseDuration = 0;
  int bits = bytes * 8;

  // clear buffer and reset checksum
  for (uint8_t i=0; i < ROASTER_MESSAGE_LENGTH; i++) {
    receiveBuffer[i] = 0;
  }

  uint8_t attempts = 0;
  bool preambleDetected = false;
  do {
    pulseDuration = pulseIn(pin, LOW, ROASTER_PREAMBLE_LENGTH_US << 2);
    if ( pulseDuration >= ROASTER_PREAMBLE_LENGTH_US) {
      preambleDetected = true;
      break;
    }
    attempts++;
  } while (attempts <= MESSAGE_PREAMBLE_MAX_ATTEMPTS);

  if (!preambleDetected) {
    WARN(F("Did not detect preamble after "));
    WARN(attempts);
    WARNLN(F(" attempts"));
    return false;
  }

  for (int i = 0; i < bits; i++) {  //Read the proper number of bits..
    pulseDuration = pulseIn(pin, LOW);
    if (pulseDuration == 0) {
      WARN(F("Failed to get bit #"));
      WARNLN(i);
      return false;
    }
    //Bits are received in LSB order..
    if (pulseDuration > ROASTER_ONE_LENGTH_US) {  // we received a 1
      receiveBuffer[i / 8] |= (1 << (i % 8));
    }
  }

  return true;
}

bool calculateRoasterChecksum() {
  uint8_t sum = 0;
  for (unsigned int i = 0; i < (ROASTER_MESSAGE_LENGTH - 1); i++) {
    sum += receiveBuffer[i];
  }

  DEBUG(F("sum: "));
  DEBUG(sum, HEX);
  DEBUG(F(" Checksum Byte: "));
  DEBUGLN(receiveBuffer[ROASTER_MESSAGE_LENGTH - 1], HEX);
  return sum == receiveBuffer[ROASTER_MESSAGE_LENGTH - 1];
}


bool getRoasterMessage() {
  DEBUG(F("R "));

  static unsigned int count = 0;

  if ( !( getMessage(ROASTER_MESSAGE_LENGTH, CONTROLLER_PIN_RX)
          and calculateRoasterChecksum())) {
    // timeout receiving message or receiving it correctly
    count++;
    WARN(F("Failed to get message, attempt #"));
    WARNLN(count);
    return (count < MESSAGE_RX_MAX_ATTEMPTS)
           && (state.status.isSynchronized);
  }

  // received message and passed checksum verification

  if (count > 0) {
    WARN(F("[!] WARN: Took "));
    WARN(count);
    WARNLN(F(" tries to read roaster."));
  }
  count = 0;

  TEMPERATURE_ROASTER(state.reported) = calculateTemp();
  return true;
}

void handleHEAT(uint8_t value) {
  if (value >= 0 && value <= 100) {
    state.commanded.heat = value;
    roasterController.setByte(ROASTER_MESSAGE_BYTE_HEAT, value);
  }
  state.status.tc4LastTick = micros();
}

void handleVENT(uint8_t value) {
  if (value >= 0 && value <= 100) {
    state.commanded.vent = value;
    roasterController.setByte(ROASTER_MESSAGE_BYTE_VENT, value);
  }
  state.status.tc4LastTick = micros();
}

void handleCOOL(uint8_t value) {
  if (value >= 0 && value <= 100) {
    state.commanded.cool = value;
    roasterController.setByte(ROASTER_MESSAGE_BYTE_COOL, value);
  }
  state.status.tc4LastTick = micros();
}

void handleFILTER(uint8_t value) {
  if (value >= 0 && value <= 100) {
    state.commanded.filter = value;
    roasterController.setByte(ROASTER_MESSAGE_BYTE_FILTER, value);
  }
  state.status.tc4LastTick = micros();
}

void handleDRUM(uint8_t value) {
  if (value != 0) {
    state.commanded.drum = 100;
    roasterController.setByte(ROATER_MESSAGE_BYTE_DRUM, 100);
  } else {
    state.commanded.drum = 0;
    roasterController.setByte(ROATER_MESSAGE_BYTE_DRUM, 0);
  }
  state.status.tc4LastTick = micros();
}

void handleREAD() {
  Serial.print(F("0.0"));
  uint8_t mapping = 0;
  for (uint8_t i = 0; i < TEMPERATURE_CHANNELS_MAX; i++) {
    mapping = state.cfg.chanMapping[i];
    if ((mapping >= 1)
        && (mapping <= TEMPERATURE_CHANNELS_MAX)) {
        Serial.print(F(","));
        Serial.print(state.reported.chanTemp[mapping - 1]);
    }
  }
  Serial.print(F(","));
  Serial.print(state.commanded.heat);
  Serial.print(',');
  Serial.print(state.commanded.vent);
  Serial.print(',');
  Serial.println(F("0"));

  state.status.tc4LastTick = micros();
}

bool itsbeentoolong() {
  ustick_t now = micros();
  ustick_t duration = now - state.status.tc4LastTick;
  if (duration > (TC4_COMM_TIMEOUT_MS * 1000)) {
    roasterController.shutdown();  //We turn everything off
  }

  return duration > (TC4_COMM_TIMEOUT_MS * 1000);
}

void handleCHAN(String channels) {
  if (channels.length() != TEMPERATURE_CHANNELS_MAX) {
    WARN(F("Ignoring channels command, as "));
    WARN(channels);
    WARN(F(" does not match the number of supported channels"));
    return;
  }

  WARN(F("Handling CHAN command with "));
  WARN(channels);
  WARNLN(F(" value"));
  Serial.print(F("# Active channels set to "));
  char strbuf[2];
  int chanNum;
  for (uint8_t i = 0; i < TEMPERATURE_CHANNELS_MAX; i++) {
    strbuf[0] = channels.charAt(i);
    strbuf[1] = '\0';
    chanNum = atoi(strbuf);
    if ( (chanNum >= 0)
         && (chanNum <= TEMPERATURE_CHANNELS_MAX)) {
          state.cfg.chanMapping[i] = chanNum;
          Serial.print(chanNum);
    } else {
          Serial.print(F("0"));
    }
  }
  Serial.println();
}

#ifdef ARDUINO_BLACKPILL_F411CE
void handleDFUCommand(int response) {
  static ustick_t lastTick = 0;
  static int challenge = 0;
  ustick_t now = micros();

  if ( response == 0 || challenge == 0) {
    // challenge bootloader trigger command
    challenge = now & 0x0FFF;
    Serial.print(F("DFU challenge: "));
    Serial.println(challenge);
    lastTick = now;
  } else {
    if ((now - lastTick) <= CMD_DFU_TIMEOUT_US && response == challenge) {
      // got challenge respone before the timeout
      JumpToBootloader();
    } else {
      challenge = 0;
      lastTick = 0;
    }
  }

}
#endif // ARDUINO_BLACKPILL_F411CE

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  setupLCD();

  roasterController.begin();
  roasterController.shutdown();

  welcomeLCD();

#ifdef USE_THERMOCOUPLE
  setupThermoCouple();
#endif
}

void loop() {
  //Don't want the roaster be uncontrolled.. By itself, if you don't send a command in 1sec it will shutdown
  //But I also want to ensure the arduino is getting commands from something.
  //I think a safeguard for this might be to ensure we're regularly receiving control messages.
  //If Artisan is on, it should be polling for temps every few seconds. This requires we get a VALID control command.
  //I also want to add some sort of over temp protection. Butthis is the wild west. Don't burn your roaster and/or house down.

  if (itsbeentoolong()) {
    //TODO: Maybe consider moving this logic to the interrupt handler
    //That way if the arduino is having issues, the interrupt handler
    //Will stop sending messages to the roaster and it'll shut down.

    roasterController.shutdown();
  }

  roasterController.loopTick();

  state.status.isSynchronized = getRoasterMessage();

#ifdef USE_THERMOCOUPLE
  state.status.tcStatus = processThermoCouple();
#endif

  while (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');

    uint8_t value = 0;
    input.trim();
    int split = input.indexOf(';');
    String command = "";

    if (split >= 0) {
      command = input.substring(0, split);
      value = input.substring(split + 1).toInt();
    } else {
      command = input;
    }

    if (command == "READ") {
      handleREAD();
    } else if (command == "OT1") { //Set Heater Duty
      handleHEAT(value);
    } else if (command == "OT2") { //Set Fan Duty
      handleVENT(value);
    } else if (command == "OFF") { //Shut it down
      roasterController.shutdown();
    } else if (command == "DRUM") {//Start the drum
      handleDRUM(value);
    } else if (command == "FILTER") { //Turn on the filter fan
      handleFILTER(value);
    } else if (command == "COOL") { //Cool the beans
      handleCOOL(value);
    } else if (command == "CHAN" && (split > 0)) { //Hanlde the TC4 init message
      handleCHAN(input.substring(split+1));
    } else if (command == "UNITS") {
      if (split >= 0) state.cfg.CorF = input.charAt(split + 1);
#ifdef ARDUINO_BLACKPILL_F411CE
    } else if (command == "DFU") {
      if (split < 0) {
        handleDFUCommand(0);
      } else {
        handleDFUCommand(input.substring(split+1).toInt());
      }
#endif // ARDUINO_BLACKPILL_F411CE
    }
  }

  updateLCD();
}
