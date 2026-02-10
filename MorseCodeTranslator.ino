/*
 * 
 * Title: Morse code translator
 * 
 * Name: Algasem Zabarah
 * Date: 2025/05/23
 * 
 * Description: This project is a morse code translator. It can translate from english to morse code, and vice versa.
 * It uses an array called alphabet that contains the morse ode for each associated letter in the alphabet. When the 
 * user inputs a word into the serial monitor, it is then displayed onto the lcd as morse code (dots and dashes).
 * When translating from morse code to english, it uses a button to allow the user to input their morse code, and then 
 * redisplay the english translation onto the LCD.
 * 
 * 
 * algasem_zabarah_summative_project1:
 * Created array for morse code
 * Checked if serial input is avaliable
 * When it is, it starts going through the word and translating it into morse code
 * It checks each letter, and uses my array alphabet to print the according morse code letter
 * 
 * 
 * 
 * algasem_zabarah_summative_project2:
 * Added a button to be pressed when the user wants to start typing in morse code
 * Coded the release and press of the button
 * Added according logic to detect the length of the button press
 * Flashed blue led for dashes and red led for dots
 * Added a short/long beep to go along with the dots and dashes
 * Added morse mode, which would be activated when user wants to type in morse language
 * Allowed user to exit morse mode, but pressing the start button again
 * 
 * 
 * 
 */

#include <LiquidCrystal_I2C.h>

const char CLEAR = 0;
const char DOT = 1;
const char DASH = 2;

const char alphabet[26][6] = {
  { 'A', DOT, DASH, CLEAR, CLEAR, CLEAR },
  { 'B', DASH, DOT, DOT, DOT, CLEAR },
  { 'C', DASH, DOT, DASH, DOT, CLEAR },
  { 'D', DASH, DOT, DOT, CLEAR, CLEAR },
  { 'E', DOT, CLEAR, CLEAR, CLEAR, CLEAR },
  { 'F', DOT, DOT, DASH, DOT, CLEAR },
  { 'G', DASH, DASH, DOT, CLEAR, CLEAR },
  { 'H', DOT, DOT, DOT, DOT, CLEAR },
  { 'I', DOT, DOT, CLEAR, CLEAR, CLEAR },
  { 'J', DOT, DASH, DASH, DASH, CLEAR },
  { 'K', DASH, DOT, DASH, CLEAR, CLEAR },
  { 'L', DOT, DASH, DOT, DOT, CLEAR },
  { 'M', DASH, DASH, CLEAR, CLEAR, CLEAR },
  { 'N', DASH, DOT, CLEAR, CLEAR, CLEAR },
  { 'O', DASH, DASH, DASH, CLEAR, CLEAR },
  { 'P', DOT, DASH, DASH, DOT, CLEAR },
  { 'Q', DASH, DASH, DOT, DASH, CLEAR },
  { 'R', DOT, DASH, DOT, CLEAR, CLEAR },
  { 'S', DOT, DOT, DOT, CLEAR, CLEAR },
  { 'T', DASH, CLEAR, CLEAR, CLEAR, CLEAR },
  { 'U', DOT, DOT, DASH, CLEAR, CLEAR },
  { 'V', DOT, DOT, DOT, DASH, CLEAR },
  { 'W', DOT, DASH, DASH, CLEAR, CLEAR },
  { 'X', DASH, DOT, DOT, DASH, CLEAR },
  { 'Y', DASH, DOT, DASH, DASH, CLEAR },
  { 'Z', DASH, DASH, DOT, DOT, CLEAR }
};

const int buttonPin = 6;
const int startButtonPin = 8;
const int buzzerPin = 7;

const int redLEDPin = 2;
const int blueLEDPin = 3;

int col = 0;
int row = 0;

const int dotDuration = 1000;
const int dashDuration = 3 * dotDuration;
const int shortGap = dotDuration;
const int mediumGap = 3 * dotDuration;

bool buttonPressed = false;
bool morseMode = false;

bool lastStartButtonState = HIGH;

unsigned long pressStartTime = 0;
unsigned long releaseStartTime = 0;
unsigned long lastInputTime = 0;

bool wasButtonDown = false;
String morseSequence = "";
String englishOutput = "";

int buttonPressStart = 0;
int buttonReleaseStart = 0;

char translateMorse(String morse);
void updateLCDOutput(String output);

byte dot[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

byte smiley[8] = {
  0b00000,
  0b01010,
  0b00000,
  0b00000,
  0b10001,
  0b01010,
  0b00100,
  0b00000
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, smiley);
  lcd.createChar(1, dot);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("The morse code");
  lcd.setCursor(0, 1);
  lcd.print("translator!!");
  lcd.write(0);
  delay(3000);

  Serial.println("Enter an english WORD to translate to Morse code (it will display on LCD):");
  Serial.println("Or press the start (yellow) button to enter Morse input mode.");
}

void loop() {
  // Handle English to Morse (only when NOT in morse mode)
  if (Serial.available() && !morseMode) {
    delay(100);
    lcd.clear();

    String input = Serial.readString();
    input.trim();
    input.toUpperCase();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Morse Code:");
    delay(2000);
    lcd.clear();

    row = 0;
    col = 0;
    lcd.setCursor(col, row);

    for (int i = 0; i < input.length(); i++) {
      char c = input.charAt(i);
      if (c >= 'A' && c <= 'Z') {
        int index = c - 'A';

        int morseLen = 0;
        for (int j = 1; j < 6; j++) {
          if (alphabet[index][j] == DOT || alphabet[index][j] == DASH) {
            morseLen++;
          } else {
            break;
          }
        }

        bool addSpace = (i < input.length() - 1);
        int totalLen = morseLen;
        if (addSpace) totalLen++;

        if (col + totalLen > 16) {
          row = (row == 0) ? 1 : 0;
          if (row == 0) lcd.clear();
          col = 0;
          lcd.setCursor(col, row);
        }

        for (int j = 1; j < 6; j++) {
          if (alphabet[index][j] == DOT) {
            lcd.write(byte(1));
            col++;
          } else if (alphabet[index][j] == DASH) {
            lcd.print("-");
            col++;
          } else {
            break;
          }
        }

        if (addSpace) {
          lcd.print(" ");
          col++;
        }
      }
    }

    Serial.println("Morse code printed. Enter another word:");
  }

  // Handle start button with proper edge detection
  bool currentStartButtonState = digitalRead(startButtonPin);
  
   if (lastStartButtonState == HIGH && currentStartButtonState == LOW) {
    if (!morseMode) {
      // Button just pressed - enter morse mode
      morseMode = true;
      morseSequence = "";
      englishOutput = "";
      lastInputTime = millis();
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Start typing in");
      lcd.setCursor(0, 1);
      lcd.print("Morse Code!");
      
      delay(2000);
      Serial.println("Morse mode activated! Press start button again to exit.");
    } else {
      // Exit morse mode
      morseMode = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Morse mode");
      lcd.setCursor(0, 1);
      lcd.print("deactivated!");
      delay(2000);
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ready for");
      lcd.setCursor(0, 1);
      lcd.print("English input");
      delay(2000);
      
      Serial.println("Morse mode deactivated. Enter English words to translate:");
    }
  }
  
  lastStartButtonState = currentStartButtonState;

  // Handle Morse input mode
  if (morseMode) {
    bool buttonState = digitalRead(buttonPin) == LOW;
    unsigned long currentTime = millis();

    if (buttonState && !wasButtonDown) {
      pressStartTime = currentTime;
      wasButtonDown = true;
    }

    if (!buttonState && wasButtonDown) {
      unsigned long pressDuration = currentTime - pressStartTime;
      wasButtonDown = false;


      if (pressDuration >= 300 && pressDuration < 1500) {
        morseSequence += ".";
        tone(buzzerPin, 1000, 100);
        digitalWrite(redLEDPin, HIGH);
        delay(100);
        digitalWrite(redLEDPin, LOW);
      } else if (pressDuration >= 1500 && pressDuration <= 4000) {
        morseSequence += "-";
        tone(buzzerPin, 1000, 300);
        digitalWrite(blueLEDPin, HIGH);
        delay(300);
        digitalWrite(blueLEDPin, LOW);
      }

      lastInputTime = currentTime;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Morse: ");
      lcd.print(morseSequence);
    }

    // Letter translation (after 2 seconds of no input)
    if (!wasButtonDown && morseSequence.length() > 0 && currentTime - lastInputTime >= 2000 && currentTime - lastInputTime < 6000) {
      char translatedChar = translateMorse(morseSequence);
      if (translatedChar != '?') {  // Fixed: was checking for ' ' instead of '?'
        englishOutput += translatedChar;
        updateLCDOutput(englishOutput);
      }
      morseSequence = "";
    }

    // Word break (after 6 seconds of no input) - MOVED INSIDE morse mode check
    if (!wasButtonDown && (currentTime - lastInputTime >= 6000)) {
      // First, process any remaining morse input
      if (morseSequence.length() > 0) {
        char translatedChar = translateMorse(morseSequence);
        if (translatedChar != '?') {  // Fixed: was checking for ' ' instead of '?'
          englishOutput += translatedChar;
          updateLCDOutput(englishOutput);
        }
        morseSequence = "";
      }

      // Then add space to signify word break
      if (englishOutput.length() > 0 && englishOutput.charAt(englishOutput.length() - 1) != ' ') {
        englishOutput += ' ';
        updateLCDOutput(englishOutput);
      }

      lastInputTime = millis() + 1000;
    }
  }
}

char translateMorse(String morse) {
  for (int i = 0; i < 26; i++) {
    String current = "";
    for (int j = 1; j < 6; j++) {
      if (alphabet[i][j] == DOT) {
        current += ".";
      } else if (alphabet[i][j] == DASH) {
        current += "-";
      } else {
        break;
      }
    }
    if (current == morse) {
      return alphabet[i][0];
    }
  }
  lcd.clear();
  lcd.print("Invalid Morse");
  lcd.setCursor(0,1);
  lcd.print("Code.");
  return '?';
}

void updateLCDOutput(String output) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("English: ");
  lcd.setCursor(0, 1);
  int len = output.length();
  if (len <= 16) {
    lcd.print(output);
  } else {
    lcd.print(output.substring(len - 16));
  }
}