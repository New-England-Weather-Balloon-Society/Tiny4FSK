#include "morse.h"

// Morse code definitions
const char* morseTable[36] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",                    // A-J
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",                      // K-T
  "..-", "...-", ".--", "-..-", "-.--", "--..",                                             // U-Z
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."  // 0-9
};

// Function to send a Morse code character
void sendMorseChar(char c) {
  if (c >= 'A' && c <= 'Z') {
    const char* morse = morseTable[c - 'A'];
    while (*morse) {
      if (*morse == '.') {
        si4063_enable_tx();
        delay(DOT_DURATION);
      } else if (*morse == '-') {
        si4063_enable_tx();
        delay(DASH_DURATION);
      }
      si4063_inhibit_tx();
      morse++;
      if (*morse) delay(SPACE_DURATION);
    }
  } else if (c >= '0' && c <= '9') {
    const char* morse = morseTable[c - '0' + 26];
    while (*morse) {
      if (*morse == '.') {
        si4063_enable_tx();
        delay(DOT_DURATION);
      } else if (*morse == '-') {
        si4063_enable_tx();
        delay(DASH_DURATION);
      }
      si4063_inhibit_tx();
      morse++;
      if (*morse) delay(SPACE_DURATION);
    }
  }
}

// Function to send a Morse code string
void sendMorseString(const char* str) {
  while (*str) {
    if (*str == ' ') {
      delay(WORD_SPACE_DURATION);
    } else {
      sendMorseChar(toupper(*str));
      delay(LETTER_SPACE_DURATION);
    }
    str++;
  }
}