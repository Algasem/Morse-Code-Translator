#include <LiquidCrystal.h>

/*
 * 
 * Title: Morse code translator
 * 
 * Name: Algasem Zabarah
 * Date: 2025/05/23
 * 
 * Description: This project is a morse code translator. It can translate from english to morse code, and vice versa.
 * It uses an array called alphabet that contains the morse code for each associated letter. When the 
 * user inputs a word into the serial monitor, it is then displayed onto the lcd as morse code (dots and dashes).
 * When translating from morse code to english, it uses a button to allow the user to input their morse code, and then 
 * redisplays the english translation onto the LCD.
 * 
 * 
 * algasem_zabarah_summative_project1:
 * Created array for morse code
 * Checked if serial input is avaliable
 * When it is, it starts going through the word and translating it into morse code
 * It checks each letter, and uses the array alphabet to print the according morse code letter
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
 * algasem_zabarah_summative_project3:
 * Tuned the logic so a dash is any press under 1000ms (1 second), instead of 1500ms
 * Finished commenting my code (this was done as I was coding)
 * Reviewed my code, for any bugs and errors
 * Formatted and indented my code
 * Removed unnecessary variables
 *
 */

#include <LiquidCrystal_I2C.h>

// Constants for morse code array elements
const char CLEAR = 0;
const char DOT = 1;
const char DASH = 2;

// 2D array storing morse code patterns for each letter A-Z
// Each row contains letter followed by morse pattern (DOT/DASH/CLEAR)
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

// Pin assignments for hardware components
const int buttonPin = 6;
const int startButtonPin = 8;
const int buzzerPin = 7;
const int redLEDPin = 2;
const int blueLEDPin = 3;

// LCD cursor position tracking
int col = 0;
int row = 0;

// State variables for button handling and mode switching
bool morseMode = false;
bool lastStartButtonState = HIGH;

// Timing variables for button press duration detection
int pressStartTime = 0;
int lastInputTime = 0;

// Variables for morse code input processing
bool wasButtonDown = false;
String morseSequence = "";
String englishOutput = "";

// Function declarations
char translateMorse(String morse);
void updateLCDOutput(String output);

// Custom character arrays for LCD display
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

// LCD object initialization
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // Configure pin modes for all hardware components
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);

  // Initialize  serial monitor for user input
  Serial.begin(9600);

  // Initialize LCD display and create custom characters
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, smiley);
  lcd.createChar(1, dot);

  // Display welcome message on startup
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("The morse code");
  lcd.setCursor(0, 1);
  lcd.print("translator!!");
  lcd.write(0);
  delay(3000);

  // Provide user instructions via serial monitor
  Serial.println("Enter an english WORD to translate to Morse code (it will display on LCD):");
  Serial.println("Or press the start (yellow) button to enter Morse input mode.");
}

void loop() {

  // English to Morse Code Translation Section
  if (Serial.available() && !morseMode) {
    delay(100);
    lcd.clear();

    // Read and prepare input string for processing
    String input = Serial.readString();
    input.trim();
    input.toUpperCase();

    // Display header message before showing morse code
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Morse Code:");
    delay(2000);
    lcd.clear();

    // Reset cursor position for morse code display
    row = 0;
    col = 0;
    lcd.setCursor(col, row);

    // Main translation loop, process each character in input
    for (int i = 0; i < input.length(); i++) {
      char c = input.charAt(i);

      // Handle spaces between words with double spacing
      if (c == ' ') {
        // Check if double space fits on current line
        if (col + 2 > 16) {
          if (row == 0) {
            row = 1;
          } else {
            lcd.clear();
            row = 0;
          }
          col = 0;
          lcd.setCursor(col, row);
        }
        lcd.print("  ");
        col += 2;
      }
      // Process alphabetic characters for morse translation
      else if (c >= 'A' && c <= 'Z') {
        int index = c - 'A';

        // Calculate length of morse code for current letter
        int morseLen = 0;
        for (int j = 1; j < 6; j++) {
          if (alphabet[index][j] == DOT || alphabet[index][j] == DASH) {
            morseLen++;
          } else {
            break;
          }
        }

        // Determine if space needed after letter
        bool addSpace = (i < input.length() - 1 && input.charAt(i + 1) != ' ');
        int totalLen = morseLen;
        if (addSpace) {
          totalLen++;
        }

        // Handle line wrapping and screen overflow
        if (col + totalLen > 16) {
          if (row == 0) {
            row = 1;
          } else {
            // Pause to let user read content before clearing screen
            delay(4000);
            lcd.clear();
            row = 0;
          }
          col = 0;
          lcd.setCursor(col, row);
        }

        // Display morse code pattern for current letter
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

        // Add single space between letters within same word
        if (addSpace) {
          lcd.print(" ");
          col++;
        }
      }
    }

    Serial.println("Morse code printed. Enter another word:");
  }

  // Start Button Handling
  bool currentStartButtonState = digitalRead(startButtonPin);

  // Detect falling edge (button just pressed)
  if (lastStartButtonState == HIGH && currentStartButtonState == LOW) {
    if (!morseMode) {
      // Enter morse input mode
      morseMode = true;
      morseSequence = "";
      englishOutput = "";
      lastInputTime = millis();

      // Display mode change message
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Start typing in");
      lcd.setCursor(0, 1);
      lcd.print("Morse Code!");

      delay(2000);
    } else {

      // Exit morse mode and return to English input mode
      morseMode = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Morse mode");
      lcd.setCursor(0, 1);
      lcd.print("deactivated!");
      delay(3000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ready for");
      lcd.setCursor(0, 1);
      lcd.print("English input");
      delay(2000);
    }
  }

  lastStartButtonState = currentStartButtonState;


  // Morse Code Input Processing Section
  if (morseMode) {
    bool buttonState = digitalRead(buttonPin) == LOW;
    int currentTime = millis();

    // Detect button press start
    if (buttonState && !wasButtonDown) {
      pressStartTime = currentTime;
      wasButtonDown = true;
    }

    // Detect button release and process press duration
    if (!buttonState && wasButtonDown) {
      int pressDuration = currentTime - pressStartTime;
      wasButtonDown = false;

      // Short press = dot (300ms to 1000ms)
      if (pressDuration >= 300 && pressDuration < 1000) {
        morseSequence += ".";
        tone(buzzerPin, 1000, 100);
        digitalWrite(redLEDPin, HIGH);
        delay(100);
        digitalWrite(redLEDPin, LOW);
      }
      // Long press = dash (1000ms to 4000ms)
      else if (pressDuration >= 1000 && pressDuration <= 4000) {
        morseSequence += "-";
        tone(buzzerPin, 1000, 300);
        digitalWrite(blueLEDPin, HIGH);
        delay(300);
        digitalWrite(blueLEDPin, LOW);
      }

      // Update display with current morse sequence
      lastInputTime = currentTime;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Morse: ");
      lcd.print(morseSequence);
    }

    // After 2 seconds of no input, translate current morse sequence to letter
    if (!wasButtonDown && morseSequence.length() > 0 && currentTime - lastInputTime >= 2500 && currentTime - lastInputTime < 6000) {
      char translatedChar = translateMorse(morseSequence);
      if (translatedChar != '?') {
        englishOutput += translatedChar;
        updateLCDOutput(englishOutput);
      }
      morseSequence = "";
    }



    // After 8 seconds of no input, add space to separate words
    if (!wasButtonDown && (currentTime - lastInputTime >= 8000)) {
      // Process any remaining morse input first
      if (morseSequence.length() > 0) {
        char translatedChar = translateMorse(morseSequence);
        if (translatedChar != '?') {
          englishOutput += translatedChar;
          updateLCDOutput(englishOutput);
        }
        morseSequence = "";
      }

      // Add space for word separation
      if (englishOutput.length() > 0 && englishOutput.charAt(englishOutput.length() - 1) != ' ') {
        englishOutput += ' ';
        updateLCDOutput(englishOutput);
      }

      lastInputTime = millis() + 1000;
    }
  }
}


// Morse Code to Letter Translation Function
char translateMorse(String morse) {
  // Search through alphabet array for matching morse pattern
  for (int i = 0; i < 26; i++) {
    String current = "";
    // Build morse string from array pattern
    for (int j = 1; j < 6; j++) {
      if (alphabet[i][j] == DOT) {
        current += ".";
      } else if (alphabet[i][j] == DASH) {
        current += "-";
      } else {
        break;
      }
    }
    // Return letter if morse pattern matches
    if (current == morse) {
      return alphabet[i][0];
    }
  }
  // Display error message for invalid morse code
  lcd.clear();
  lcd.print("Invalid Morse");
  lcd.setCursor(0, 1);
  lcd.print("Code.");
  return '?';
}


// Displays translated English text with scrolling for long strings
void updateLCDOutput(String output) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("English: ");
  lcd.setCursor(0, 1);
  int len = output.length();
  if (len <= 16) {
    lcd.print(output);
  } else {
    // Display only last 16 characters for scrolling effect
    lcd.print(output.substring(len - 16));
  }
}