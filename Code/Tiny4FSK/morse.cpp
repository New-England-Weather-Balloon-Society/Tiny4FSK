/*
morse.cpp, part of Tiny4FSK, for a high-altitude tracker.
Copyright (C) 2024 Maxwell Kendall

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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