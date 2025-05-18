#ifndef BJGOAT_H
#define BJGOAT_H

#include <Arduino.h>

enum Decision { HIT, STAND, DOUBLE_DOWN, SPLIT, INVALID };
enum Side { PLAYER, DEALER };

class BjGoat {
public:
    BjGoat();
    void reset();
    void add_card(uint8_t card);
    void switch_side();
    void set_side(Side side);
    Side get_current_side() const;
    int get_player_value() const;
    int get_dealer_value() const;
    int get_dealer_up_card() const;
    Decision decide() const;
    const char* get_decision_string() const;
    void print_player_cards() const;
    void print_dealer_cards(bool showAll = false) const;
    const char* get_card_name(uint8_t card) const;
    
    // New methods for state management
    uint8_t get_current_card_index() const;
    void set_current_card_index(uint8_t index);
    bool get_is_dealer() const;
    void set_is_dealer(bool dealer);


private:
    uint8_t normalize_card(uint8_t card);
    int calculate_hand_value(const uint8_t* cards, uint8_t count) const;
    bool is_soft_hand(const uint8_t* cards, uint8_t count) const;
    bool is_pair() const;

    // Game state variables
    uint8_t currentCardIndex = 0;
    bool isDealer = false;
    
    // Card tracking
    uint8_t player_cards[10];
    uint8_t dealer_cards[10];
    uint8_t player_card_count = 0;
    uint8_t dealer_card_count = 0;
    Side current_side;
    
    // Strategy charts
    Decision strategy_chart[18][10];
    Decision pair_chart[10][10];
};

#endif

BjGoat::BjGoat() : current_side(Side::PLAYER) {
    // Initialize strategy chart (basic Blackjack strategy)
    Decision temp_strategy[18][10] = {
        // 2    3    4    5    6    7    8    9   10    A
        {HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT},       // 5
        {HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT},       // 6
        {HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT},       // 7
        {HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT, HIT},       // 8
        {HIT, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, HIT, HIT, HIT, HIT, HIT}, // 9
        {DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, HIT, HIT}, // 10
        {DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, HIT}, // 11
        {HIT, HIT, STAND, STAND, STAND, HIT, HIT, HIT, HIT, HIT},  // 12
        {STAND, STAND, STAND, STAND, STAND, HIT, HIT, HIT, HIT, HIT}, // 13
        {STAND, STAND, STAND, STAND, STAND, HIT, HIT, HIT, HIT, HIT}, // 14
        {STAND, STAND, STAND, STAND, STAND, HIT, HIT, HIT, HIT, HIT}, // 15
        {STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND}, // 16
        {STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND}, // 17
        {STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND}, // 18
        {STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND}, // 19
        {STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND}, // 20
        {STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND}  // 21
    };

    // Initialize pair splitting chart
    Decision temp_pair[10][10] = {
        // 2    3    4    5    6    7    8    9   10    A
        {SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, HIT, HIT, HIT, HIT},       // A,A
        {SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, HIT, HIT, HIT, HIT},       // 2,2
        {HIT, HIT, HIT, SPLIT, SPLIT, HIT, HIT, HIT, HIT, HIT},               // 3,3
        {HIT, HIT, HIT, SPLIT, SPLIT, HIT, HIT, HIT, HIT, HIT},               // 4,4
        {DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, DOUBLE_DOWN, HIT, HIT}, // 5,5
        {SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, HIT, HIT, HIT, HIT, HIT},         // 6,6
        {SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, HIT, HIT, HIT, HIT},       // 7,7
        {SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, SPLIT}, // 8,8
        {SPLIT, SPLIT, SPLIT, SPLIT, SPLIT, STAND, SPLIT, SPLIT, STAND, STAND}, // 9,9
        {STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND, STAND}  // 10,10
    };

    // Copy strategy charts to class members
    memcpy(strategy_chart, temp_strategy, sizeof(temp_strategy));
    memcpy(pair_chart, temp_pair, sizeof(temp_pair));
}

void BjGoat::reset() {
    player_card_count = 0;
    dealer_card_count = 0;
    currentCardIndex = 0;
    current_side = Side::PLAYER;
}

void BjGoat::add_card(uint8_t card) {
    if (current_side == Side::PLAYER && player_card_count < 10) {
        player_cards[player_card_count++] = normalize_card(card);
    } else if (current_side == Side::DEALER && dealer_card_count < 10) {
        dealer_cards[dealer_card_count++] = normalize_card(card);
    }
}

void BjGoat::switch_side() {
    current_side = (current_side == Side::PLAYER) ? Side::DEALER : Side::PLAYER;
}

void BjGoat::set_side(Side side) {
    current_side = side;
}

Side BjGoat::get_current_side() const {
    return current_side;
}

uint8_t BjGoat::get_current_card_index() const {
  return currentCardIndex;
}

void BjGoat::set_current_card_index(uint8_t index) {
  currentCardIndex = index;
}

const char* BjGoat::get_card_name(uint8_t card) const {
  switch(card) {
      case 1: return "A";
      case 11: return "A"; // Normalized ace
      default:
          if (card >= 2 && card <= 10) {
              static char num[3];
              itoa(card, num, 10);
              return num;
          }
          return "?";
  }
}

void BjGoat::print_player_cards() const {
  Serial.print(F("Player cards: ["));
  for (uint8_t i = 0; i < player_card_count; i++) {
      Serial.print(get_card_name(player_cards[i]));
      if (i < player_card_count - 1) {
          Serial.print(F(", "));
      }
  }
  Serial.print(F("] (Total: "));
  Serial.print(get_player_value());
  Serial.print(F(")"));
}

void BjGoat::print_dealer_cards(bool showAll) const {
  Serial.print(F("Dealer cards: ["));
  if (dealer_card_count == 0) {
      Serial.print(F("none"));
  } else {
      // Show first card always
      Serial.print(get_card_name(dealer_cards[0]));
      
      // Show remaining cards if showAll is true, otherwise show as hidden
      for (uint8_t i = 1; i < dealer_card_count; i++) {
          Serial.print(F(", "));
          if (showAll) {
              Serial.print(get_card_name(dealer_cards[i]));
          } else {
              Serial.print(F("?"));
          }
      }
  }
  Serial.print(F("]"));
  
  if (showAll) {
      Serial.print(F(" (Total: "));
      Serial.print(get_dealer_value());
      Serial.print(F(")"));
  }
}

uint8_t BjGoat::normalize_card(uint8_t card) {
    if (card > 10) return 10; // Face cards (J,Q,K) are worth 10
    if (card == 1) return 11; // Ace is worth 11 by default
    return card;
}

int BjGoat::calculate_hand_value(const uint8_t* cards, uint8_t count) const {
    int total = 0;
    uint8_t aces = 0;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t card = cards[i];
        if (card == 11) { // Ace (normalized)
            aces++;
            total += 11;
        } else {
            total += card;
        }
    }

    // Adjust for aces if total is over 21
    while (total > 21 && aces > 0) {
        total -= 10;
        aces--;
    }

    return total;
}

bool BjGoat::is_soft_hand(const uint8_t* cards, uint8_t count) const {
    int total = 0;
    bool has_ace = false;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t card = cards[i];
        if (card == 11) {
            has_ace = true;
            total += 11;
        } else {
            total += card;
        }
    }

    return has_ace && (total <= 21);
}

bool BjGoat::is_pair() const {
    if (player_card_count != 2) return false;
    uint8_t card1 = (player_cards[0] > 10) ? 10 : player_cards[0];
    uint8_t card2 = (player_cards[1] > 10) ? 10 : player_cards[1];
    return card1 == card2;
}

int BjGoat::get_player_value() const {
    return calculate_hand_value(player_cards, player_card_count);
}

int BjGoat::get_dealer_value() const {
    return calculate_hand_value(dealer_cards, dealer_card_count);
}

int BjGoat::get_dealer_up_card() const {
    if (dealer_card_count == 0) return 0;
    uint8_t card = dealer_cards[0];
    return (card > 10) ? 10 : card;
}

Decision BjGoat::decide() const {
    if (player_card_count == 0 || dealer_card_count == 0) return Decision::INVALID;

    int dealer_up = get_dealer_up_card();
    if (dealer_up == 0) return Decision::INVALID; // No dealer card

    // Adjust dealer index (2-10,A -> 0-9)
    int dealer_index = constrain(dealer_up - 2, 0, 9);

    // Check for pairs first
    if (is_pair()) {
        uint8_t pair_value = player_cards[0];
        if (pair_value > 10) pair_value = 10; // Face cards
        if (pair_value == 11) pair_value = 1; // Aces

        int pair_index = (pair_value == 1) ? 0 : pair_value - 1;
        if (pair_index >= 0 && pair_index < 10) {
            return pair_chart[pair_index][dealer_index];
        }
    }

    int player_total = get_player_value();

    // Soft hand strategy (Ace counted as 11)
    if (is_soft_hand(player_cards, player_card_count)) {
        if (player_total >= 13 && player_total <= 21) {
            int soft_index = player_total - 13;
            if (soft_index >= 0 && soft_index < 8) {
                return strategy_chart[soft_index + 10][dealer_index];
            }
        }
    }
    // Hard hand strategy
    else {
        if (player_total >= 5 && player_total <= 21) {
            int hard_index = constrain(player_total - 5, 0, 16);
            return strategy_chart[hard_index][dealer_index];
        }
    }

    return Decision::INVALID;
}

const char* BjGoat::get_decision_string() const {
    switch (decide()) {
        case HIT: return "HIT";
        case STAND: return "STAND";
        case DOUBLE_DOWN: return "DOUBLE DOWN";
        case SPLIT: return "SPLIT";
        case INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}