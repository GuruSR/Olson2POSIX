#ifndef WATCHY_OLSON2POSIX_H
#define WATCHY_OLSON2POSIX_H

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
        void setTZInternal();
        bool beginOlsonFromWeb();
        void endOlsonFromWeb();
        bool gotOlsonFromWeb();
        void init();
        const String TZMISSING = "--MISSING--";
};
#endif
