# Olson2POSIX
This library offers the ability to convert from Olson to POSIX TimeZones as well as it will store the Olson and POSIX in RTC memory to survive Deep Sleep.  This library is adapted to work with the Arduino ESP32 and any other project that has similar libraries.

**String getPOSIX(String inOlson):** Returns the POSIX value from the Olson TimeZone, if it cannot find the TimeZone, "--MISSING--" (Olson2POSIX.TZMISSING) will be returned.

**bool setCurrentTimeZone():**  Sets the current POSIX TimeZone to the environment variable, must be done after a Deep Sleep, usually just before using `localtime`.

**bool setOlsonTimeZone(String inOlson):**  Sets the current POSIX TimeZone to the environment variable once found, if not found will return `false`.

**String getCurrentPOSIX():**  Returns the current POSIX TimeZone in a String.

**String getCurrentOlson():**  Returns the current Olson TimeZone in a String.

**bool setCurrentPOSIX(String NewPOSIX):**  Sets the new POSIX as the current POSIX TimeZone and environment variable.

**bool beginOlsonFromWeb():**  Starts the operation to ask for a TimeZone based on IP address (WiFi needs to be on and connected).  This is not ASYNC currently.

**endOlsonFromWeb():**  This tells the operation to end, do so once the function `gotOlsonFromWeb()` returns `true`.

**bool gotOlsonFromWeb():**  Returns `true` if it recieved an Olson TimeZone.

**init():**  Do this from the `switch (wakeup_reason)` in `default:` or not, the above functions do it if you don't.

The current Olson and POSIX TimeZones will remain after a Deep Sleep, but not a reboot, so be sure to record them if you're wanting them to stay.
