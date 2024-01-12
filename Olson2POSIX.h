#ifndef WATCHY_OLSON2POSIX_H
#define WATCHY_OLSON2POSIX_H

/* Olson2POSIX by GuruSR (https://www.github.com/GuruSR/Olson2POSIX)
 * Version 1.0, January  2, 2022
 * Version 1.1, January 12, 2022 - Fix issues with TZ strings with quoted <+-nn> names
 * Version 1.2, March   28, 2023 - Converted to a threaded system for main thread performance.
 * Version 1.3, January 12, 2024 - Added WebError and methods to get the last HTTP error.
 *
 * This library offers the ability to convert from Olson to POSIX timezones as well as it will store the
 * Olson and POSIX in RTC memory to survive Deep Sleep.
 *
 * This library is adapted to work with the Arduino ESP32 and any other project that has similar libraries.
 *
 * MIT License
 *
 * Copyright (c) 2022 GuruSR
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>

class Olson2POSIX{
    public:
        String getPOSIX(String inOlson);
        bool setCurrentTimeZone();
        bool setOlsonTimeZone(String inOlson);
        String getCurrentPOSIX();
        String getCurrentOlson();
        bool setCurrentPOSIX(String NewPOSIX);
        bool beginOlsonFromWeb();
        void endOlsonFromWeb();
        bool gotOlsonFromWeb();
        int getOlsonWebError();
        void init();
        const String TZMISSING = "--MISSING--";
    private:
        void setTZInternal();
		static void OlsonGet(void * parameter);
};
#endif
