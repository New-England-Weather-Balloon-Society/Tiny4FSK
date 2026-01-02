/*
sd_card.cpp, part of Tiny4FSK, for a high-altitude tracker.
Copyright (C) 2026 Maxwell Kendall

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

#include "sd_card.h"

bool sd_card_begin() {
    //SPI.begin();

    if (!SD.begin(SD_CS)) {
        SPI.end();
        delay(5);
        SPI.begin();
        return false;
    }
    return true;
}

bool sd_card_write_line(const char* filename, const char* data) {
    File dataFile = SD.open(filename, FILE_WRITE);

    if (dataFile) {
        dataFile.println(data);
        dataFile.close();
        return true;
    } else {
        return false;
    }
}

bool sd_card_read_line(const char* filename, char* buffer, size_t bufferSize) {
    File dataFile = SD.open(filename);

    if (dataFile) {
        if (dataFile.available()) {
            int bytesRead = dataFile.readBytesUntil('\n', buffer, bufferSize - 1);
            buffer[bytesRead] = '\0';
            dataFile.close();
            return true;
        } else {
            dataFile.close();
            return false;
        }
    } else {
        return false;
    }
}
