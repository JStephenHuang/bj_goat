#include <BjGoat.h>
#include <Arduino.h>

// Hardware configuration
const uint8_t JOY_X_PIN = A0;
const uint8_t JOY_Y_PIN = A1;
const uint8_t JOY_BTN_PIN = 2;
const uint8_t BUZZER_PIN = 8;

// Constants
const uint16_t JOY_THRESHOLD = 200;
const uint16_t JOY_CENTER = 512;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long LONG_PRESS_TIME = 1000;
const unsigned long DOUBLE_PRESS_TIME = 300;

// Card representations (A=1, J=11, Q=12, K=13)
const char* CARD_NAMES[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
const uint8_t CARD_VALUES[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
const uint8_t NUM_CARDS = 10;

// Game state (now encapsulated in BjGoat)
BjGoat bj;

// Input state
bool joyBtnState = HIGH;
bool lastJoyBtnState = HIGH;
bool joyMoved = false;
bool btnPressed = false;
bool waitingForDoubleClick = false;
unsigned long singleClickTime = 0;

unsigned long btnPressTime = 0;
unsigned long lastBtnReleaseTime = 0;
unsigned long lastActionTime = 0;

void updateDisplay();
void confirmCardSelection();
void startNewHand();
void requestDecision();

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for serial connection
  
  pinMode(JOY_BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println(F("Blackjack Strategy Advisor"));
  Serial.println(F("Up/Down: Select card value"));
  Serial.println(F("Left/Right: Toggle between dealer/player"));
  Serial.println(F("Button Press: Confirm card"));
  Serial.println(F("Long Press: New hand"));
  Serial.println(F("Double Press: Request decision"));

  updateDisplay();
}

void loop() {
  int joyX = analogRead(JOY_X_PIN);
  int joyY = analogRead(JOY_Y_PIN);
  unsigned long currentTime = millis();

  // Handle button input with improved double-click detection
  handleButtonInput();

  // Joystick movement
  if (!joyMoved && (currentTime - lastActionTime) > DEBOUNCE_DELAY) {
    if (joyY < JOY_CENTER - JOY_THRESHOLD) {
      bj.set_current_card_index((bj.get_current_card_index() + 1) % NUM_CARDS);
      updateDisplay();
      joyMoved = true;
    } else if (joyY > JOY_CENTER + JOY_THRESHOLD) {
      bj.set_current_card_index((bj.get_current_card_index() - 1 + NUM_CARDS) % NUM_CARDS);
      updateDisplay();
      joyMoved = true;
    }

    if (joyX < JOY_CENTER - JOY_THRESHOLD) {
      bj.set_side(Side::PLAYER);
      updateDisplay();
      joyMoved = true;
    } else if (joyX > JOY_CENTER + JOY_THRESHOLD) {
      bj.set_side(Side::DEALER);
      updateDisplay();
      joyMoved = true;
    }
  }

  // Reset movement flag
  if (joyMoved && 
      joyX > JOY_CENTER - JOY_THRESHOLD && joyX < JOY_CENTER + JOY_THRESHOLD &&
      joyY > JOY_CENTER - JOY_THRESHOLD && joyY < JOY_CENTER + JOY_THRESHOLD) {
    joyMoved = false;
  }
}

void handleButtonInput() {
  bool currentBtnState = digitalRead(JOY_BTN_PIN);
  unsigned long currentTime = millis();
  
  // Handle button press detection
  if (currentBtnState != lastJoyBtnState) {
    if (currentTime - lastActionTime > DEBOUNCE_DELAY) {
      lastActionTime = currentTime;
      
      if (currentBtnState == LOW && lastJoyBtnState == HIGH) {
        // Button just pressed
        btnPressed = true;
        btnPressTime = currentTime;
        
        // Check if this is a double-click
        if (waitingForDoubleClick && (currentTime - singleClickTime) < DOUBLE_PRESS_TIME) {
          // Double-click detected!
          waitingForDoubleClick = false;
          requestDecision();
          btnPressed = false;
          lastJoyBtnState = currentBtnState;
          return;
        }
      }
      else if (currentBtnState == HIGH && lastJoyBtnState == LOW) {
        // Button just released
        if (btnPressed) {
          unsigned long pressDuration = currentTime - btnPressTime;
          
          if (pressDuration >= LONG_PRESS_TIME) {
            // Long press detected
            startNewHand();
            waitingForDoubleClick = false;
          }
          else {
            // Short press - start waiting for potential double-click
            waitingForDoubleClick = true;
            singleClickTime = currentTime;
          }
          
          btnPressed = false;
        }
      }
      
      lastJoyBtnState = currentBtnState;
    }
  }
  
  // Handle ongoing long press while button is held down
  if (btnPressed && (currentTime - btnPressTime) >= LONG_PRESS_TIME) {
    // Long press triggered while button is still held
    startNewHand();
    waitingForDoubleClick = false;
    btnPressed = false;
    
    // Wait for button release to avoid multiple triggers
    while (digitalRead(JOY_BTN_PIN) == LOW) {
      delay(10);
    }
    lastJoyBtnState = HIGH;
    return;
  }
  
  // Handle double-click timeout (confirm single click)
  if (waitingForDoubleClick && (currentTime - singleClickTime) >= DOUBLE_PRESS_TIME) {
    // Time expired - it was a single click
    confirmCardSelection();
    waitingForDoubleClick = false;
  }
}

void confirmCardSelection() {
  uint8_t cardValue = CARD_VALUES[bj.get_current_card_index()];
  const char* cardName = CARD_NAMES[bj.get_current_card_index()];
  const char* side = bj.get_current_side() == Side::DEALER ? "DEALER" : "PLAYER";
  
  Serial.print(side);
  Serial.print(F(" received: "));
  Serial.println(cardName);

  bj.add_card(cardValue);
}

void startNewHand() {
  Serial.println(F("\n--- NEW HAND ---"));
  bj.reset();
  updateDisplay();
}

void updateDisplay() {
  const char* side = bj.get_current_side() == Side::DEALER ? "DEALER" : "PLAYER";
  Serial.print(F("\nSelected: "));
  Serial.print(side);
  Serial.print(F(" - Card: "));
  Serial.println(CARD_NAMES[bj.get_current_card_index()]);
}

void requestDecision() {
  Decision d = bj.decide();
  Serial.println(F("\n=== DECISION ==="));
  Serial.print(F("Recommended action: "));
  Serial.println(bj.get_decision_string());

  playDecisionFeedback(d);
  
  // Show player's cards with total
  bj.print_player_cards();
  Serial.println();
  
  // Show dealer's visible cards (only first card shown)
  bj.print_dealer_cards();
  Serial.println(F(" (First card shown)"));
  
  Serial.println(F("================"));
}

void playDecisionFeedback(Decision decision) {
  switch(decision) {
    case DOUBLE_DOWN:
      // Two high-pitched beeps for double down
      for (int i = 0; i < 2; i++) {
        tone(BUZZER_PIN, 1000, 200); // 1000Hz for 200ms
        delay(300); // 300ms total (200ms tone + 100ms silence)
      }
      break;
      
    case HIT:
      // Medium-pitched beep for hit
      tone(BUZZER_PIN, 800, 300); // 800Hz for 300ms
      break;
      
    case STAND:
      // Low-pitched long beep for stand
      tone(BUZZER_PIN, 400, 500); // 400Hz for 500ms
      break;
      
    case SPLIT:
      // Two alternating tones for split
      tone(BUZZER_PIN, 600, 150);
      delay(150);
      tone(BUZZER_PIN, 900, 150);
      break;
      
    case INVALID:
      // Error buzz
      tone(BUZZER_PIN, 200, 100);
      delay(100);
      tone(BUZZER_PIN, 200, 100);
      break;
  }
}