
#include "Olson2POSIX.h"

/* Olson2POSIX by GuruSR (https://www.github.com/GuruSR/Olson2POSIX)
 * Version 1.0, January   2, 2022
 * Version 1.1, January  12, 2022 - Fix issues with TZ strings with quoted <+-nn> names
 * Version 1.2, March    28, 2023 - Converted to a threaded system for main thread performance.
 * Version 1.3, January  12, 2024 - Added WebError and methods to get the last HTTP error.
 * Version 1.4, July     17, 2024 - Migrated away from getString to improve task performance.
 * Version 1.5, December  5, 2024 - Migrated from Arduino_JSON to ArduinoJson.
 * Version 1.6, April     6, 2025 - Fix mistake on WiFi pointer usage.
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

static const char TZURL[] PROGMEM = "http://ip-api.com/json/?fields=timezone";
static const char P_LOC[] PROGMEM = "LOC";
RTC_DATA_ATTR char POSIX[64];
RTC_DATA_ATTR String OlsonFromWeb;
RTC_DATA_ATTR bool Inited;
RTC_DATA_ATTR TaskHandle_t OlsHandle = NULL;
RTC_DATA_ATTR BaseType_t OlsRet;
RTC_DATA_ATTR bool ODone;
bool Obegan;
volatile int WebError;
WiFiClient oWiFiC;
HTTPClient oHTTP;              // Tz

// Data from:  https://github.com/pgurenko/tzinfo
static const char *sPOSIX[] = {"WET0WEST,M3.5.0/1,M10.5.0","AEST-10AEDT,M10.1.0,M4.1.0/3","<-09>9","<-08>8","<-03>3","<-02>2","<-01>1","GMT0","EST5EDT,M3.2.0,M11.1.0","<-06>6","<-05>5","<-03>3<-02>,M3.2.0,M11.1.0","<+12>-12","MST7","<+07>-7","<+06>-6","AST4","EET-2","<-07>7","MST7MDT,M4.1.0,M10.5.0","CET-1CEST,M3.5.0,M10.5.0/3","GMT0BST,M3.5.0/1,M10.5.0","<+12>-12<+13>,M11.1.0,M1.3.0/3","<+13>-13<+14>,M9.5.0/3,M4.1.0/4","<-04>4","SST11","CST6CDT,M4.1.0,M10.5.0","<+04>-4","<+0630>-6:30","WET0WEST,M3.5.0,M10.5.0/3","<+11>-11","AST4ADT,M3.2.0,M11.1.0","<+0330>-3:30<+0430>,J80/0,J264/0","<-01>1<+00>,M3.5.0/0,M10.5.0/1","<+03>-3","PST8PDT,M3.2.0,M11.1.0","CST6","<+05>-5","JST-9","<+14>-14","EET-2EEST,M3.5.0/3,M10.5.0/4","<+09>-9","SAST-2","MST7MDT,M3.2.0,M11.1.0","<+0430>-4:30","AEST-10","EAT-3","CST6CDT,M3.2.0,M11.1.0","ACST-9:30ACDT,M10.1.0,M4.1.0/3","<-04>4<-03>,M10.3.0/0,M2.3.0/0","WIT-9","CET-1","CAT-2","WAT-1","<-12>12","<+13>-13","<-10>10","<+08>-8","<-04>4<-03>,M10.1.0/0,M3.4.0/0","AKST9AKDT,M3.2.0,M11.1.0","KST-9","HST10HDT,M3.2.0,M11.1.0","MSK-3","<+0530>-5:30","<+1245>-12:45<+1345>,M9.5.0/2:45,M4.1.0/3:45","PKT-5","<+1030>-10:30<+11>-11,M10.1.0,M4.1.0","<+01>-1","<+02>-2","EST5","EET-2EEST,M3.5.4/24,M10.5.5/1","WAT-1WAST,M9.1.0,M4.1.0","<-03>3<-02>,M10.3.0/0,M2.3.0/0","AWST-8","WIB-7","KST-8:30","<+00>0<+02>-2,M3.5.0/1,M10.5.0/3","CST5CDT,M3.2.0/0,M11.1.0/1","ChST-10","<+0845>-8:45","CST-8","EET-2EEST,M3.5.0/0,M10.5.0/0","EET-2EEST,M3.5.5/0,M10.5.5/0","UTC0","IST-5:30","HST10","<+10>-10","WITA-8","EET-2EEST,M3.5.0,M10.5.0/3","NST3:30NDT,M3.2.0,M11.1.0","UCT0","<-11>11","EET-2EEST,M3.5.6/1,M10.5.6/1","<-00>0","<-0930>9:30","ACST-9:30","<+0545>-5:45","HKT-8","NZST-12NZDT,M9.5.0,M4.1.0/3","<+13>-13<+14>,M11.1.0,M1.3.0/3","MET-1MEST,M3.5.0,M10.5.0/3","GMT0IST,M3.5.0/1,M10.5.0"};

static const String sOLSON = "\x01|ATLANTIC/CANARY|\x02|AUSTRALIA/MELBOURNE|\x03|ETC/GMT+9|\x04|ETC/GMT+8|\x01|EUROPE/LISBON|\x05|ETC/GMT+3|\x06|ETC/GMT+2|\x07|ETC/GMT+1|\x08|ETC/GMT+0|\x09|AMERICA/NIPIGON|\x0A|ETC/GMT+6|\x0B|ETC/GMT+5|\x0C|AMERICA/MIQUELON|\x0D|PACIFIC/WALLIS|\x0E|AMERICA/FORT_NELSON|\x0F|ANTARCTICA/DAVIS|\x10|ASIA/DHAKA|\x11|AMERICA/ST_LUCIA|\x0E|AMERICA/PHOENIX|\x12|EUROPE/KALININGRAD|\x13|ETC/GMT+7|\x14|AMERICA/MAZATLAN|\x15|ARCTIC/LONGYEARBYEN|\x16|EUROPE/GUERNSEY|\x05|ANTARCTICA/ROTHERA|\x15|EUROPE/STOCKHOLM|\x17|PACIFIC/FIJI|\x18|PACIFIC/APIA|\x19|ETC/GMT+4|\x1A|PACIFIC/PAGO_PAGO|\x1B|AMERICA/MEXICO_CITY|\x11|AMERICA/PUERTO_RICO|\x1C|INDIAN/MAURITIUS|\x1D|ASIA/YANGON|\x15|EUROPE/BERLIN|\x15|EUROPE/ZURICH|\x1E|AFRICA/CASABLANCA|\x1F|ANTARCTICA/MACQUARIE|\x15|EUROPE/WARSAW|\x0F|ASIA/KRASNOYARSK|\x20|ATLANTIC/BERMUDA|\x05|AMERICA/ARAGUAINA|\x21|ASIA/TEHRAN|\x1C|ASIA/BAKU|\x11|AMERICA/ST_BARTHELEMY|\x05|AMERICA/SANTAREM|\x08|AMERICA/DANMARKSHAVN|\x22|AMERICA/SCORESBYSUND|\x0B|AMERICA/EIRUNEPE|\x19|AMERICA/CARACAS|\x23|ASIA/BAGHDAD|\x08|AFRICA/MONROVIA|\x11|AMERICA/ST_VINCENT|\x24|AMERICA/VANCOUVER|\x0F|ASIA/HO_CHI_MINH|\x15|EUROPE/BUSINGEN|\x08|ETC/GMT0|\x10|ASIA/THIMPHU|\x08|AFRICA/OUAGADOUGOU|\x25|AMERICA/BELIZE|\x11|AMERICA/PORT_OF_SPAIN|\x23|ASIA/FAMAGUSTA|\x26|ASIA/TASHKENT|\x27|ASIA/TOKYO|\x28|PACIFIC/KIRITIMATI|\x02|AUSTRALIA/SYDNEY|\x29|EUROPE/RIGA|\x2A|ASIA/DILI|\x2B|AFRICA/MBABANE|\x26|ASIA/ORAL|\x23|ASIA/ADEN|\x2C|MST7MDT|\x23|EUROPE/ISTANBUL|\x1F|ASIA/MAGADAN|\x2D|ASIA/KABUL|\x2E|AUSTRALIA/LINDEMAN|\x0A|PACIFIC/GALAPAGOS|\x0B|AMERICA/BOGOTA|\x2F|AFRICA/ASMARA|\x30|AMERICA/CHICAGO|\x0D|PACIFIC/KWAJALEIN|\x31|AUSTRALIA/BROKEN_HILL|\x32|AMERICA/CUIABA|\x0F|INDIAN/CHRISTMAS|\x33|ASIA/JAYAPURA|\x15|EUROPE/BRUSSELS|\x05|AMERICA/ARGENTINA/CORDOBA|\x06|AMERICA/NORONHA|\x15|EUROPE/PODGORICA|\x34|AFRICA/ALGIERS|\x35|AFRICA/HARARE|\x36|AFRICA/NDJAMENA|\x25|AMERICA/COSTA_RICA|\x15|EUROPE/LJUBLJANA|\x2F|INDIAN/MAYOTTE|\x0F|ASIA/PHNOM_PENH|\x25|AMERICA/MANAGUA|\x09|AMERICA/PANGNIRTUNG|\x37|ETC/GMT+12|\x24|AMERICA/TIJUANA|\x38|PACIFIC/FAKAOFO|\x11|AMERICA/MARTINIQUE|\x11|AMERICA/ANTIGUA|\x09|AMERICA/INDIANA/INDIANAPOLIS|\x05|AMERICA/ARGENTINA/LA_RIOJA|\x39|PACIFIC/TAHITI|\x3A|ASIA/BRUNEI|\x15|EUROPE/ZAGREB|\x3B|AMERICA/ASUNCION|\x15|EUROPE/VIENNA|\x02|AUSTRALIA/HOBART|\x3C|AMERICA/JUNEAU|\x2C|AMERICA/INUVIK|\x2C|AMERICA/OJINAGA|\x1C|EUROPE/ASTRAKHAN|\x1C|EUROPE/SARATOV|\x3D|ASIA/SEOUL|\x2F|INDIAN/COMORO|\x15|EUROPE/PARIS|\x29|EUROPE/TALLINN|\x1C|INDIAN/MAHE|\x05|AMERICA/ARGENTINA/JUJUY|\x0E|AMERICA/CRESTON|\x3E|AMERICA/ADAK|\x3A|ASIA/SINGAPORE|\x2F|AFRICA/NAIROBI|\x05|AMERICA/MACEIO|\x12|AFRICA/CAIRO|\x3F|EUROPE/MOSCOW|\x05|ANTARCTICA/PALMER|\x3A|ASIA/ULAANBAATAR|\x30|AMERICA/RAINY_RIVER|\x2F|AFRICA/KAMPALA|\x40|ASIA/COLOMBO|\x31|AUSTRALIA/ADELAIDE|\x2C|AMERICA/CAMBRIDGE_BAY|\x36|AFRICA/LUANDA|\x41|PACIFIC/CHATHAM|\x09|AMERICA/INDIANA/WINAMAC|\x1C|ASIA/TBILISI|\x15|EUROPE/GIBRALTAR|\x42|ASIA/KARACHI|\x43|AUSTRALIA/LORD_HOWE|\x2A|ETC/GMT-9|\x3A|ETC/GMT-8|\x1B|AMERICA/BAHIA_BANDERAS|\x44|ETC/GMT-1|\x08|ETC/GMT-0|\x23|ETC/GMT-3|\x45|ETC/GMT-2|\x26|ETC/GMT-5|\x1C|ETC/GMT-4|\x0F|ETC/GMT-7|\x10|ETC/GMT-6|\x19|AMERICA/BOA_VISTA|\x12|AFRICA/TRIPOLI|\x1C|INDIAN/REUNION|\x05|ATLANTIC/STANLEY|\x1F|ASIA/SREDNEKOLYMSK|\x15|CET|\x38|ETC/GMT-13|\x11|AMERICA/BLANC-SABLON|\x23|ANTARCTICA/SYOWA|\x46|AMERICA/JAMAICA|\x29|EUROPE/KIEV|\x15|EUROPE/BUDAPEST|\x1A|PACIFIC/MIDWAY|\x20|AMERICA/GOOSE_BAY|\x47|ASIA/AMMAN|\x1F|ASIA/SAKHALIN|\x48|AFRICA/WINDHOEK|\x3C|AMERICA/SITKA|\x19|AMERICA/GUYANA|\x1F|PACIFIC/POHNPEI|\x49|AMERICA/SAO_PAULO|\x11|AMERICA/LOWER_PRINCES|\x4A|AUSTRALIA/PERTH|\x2F|AFRICA/DJIBOUTI|\x4B|ASIA/JAKARTA|\x4C|ASIA/PYONGYANG|\x09|EST5EDT|\x2B|AFRICA/JOHANNESBURG|\x4D|ANTARCTICA/TROLL|\x23|ASIA/ISTANBUL|\x3A|ASIA/IRKUTSK|\x36|AFRICA/NIAMEY|\x05|AMERICA/BELEM|\x09|AMERICA/INDIANA/MARENGO|\x08|AFRICA/NOUAKCHOTT|\x29|EUROPE/VILNIUS|\x05|AMERICA/CAYENNE|\x2F|AFRICA/MOGADISHU|\x09|AMERICA/KENTUCKY/MONTICELLO|\x0B|AMERICA/RIO_BRANCO|\x46|AMERICA/CANCUN|\x4E|AMERICA/HAVANA|\x4F|PACIFIC/GUAM|\x1F|PACIFIC/KOSRAE|\x22|ATLANTIC/AZORES|\x50|AUSTRALIA/EUCLA|\x51|ASIA/SHANGHAI|\x30|AMERICA/RANKIN_INLET|\x52|ASIA/BEIRUT|\x35|AFRICA/MAPUTO|\x23|ASIA/BAHRAIN|\x26|ASIA/ASHGABAT|\x23|ASIA/RIYADH|\x16|EUROPE/LONDON|\x05|AMERICA/MONTEVIDEO|\x11|AMERICA/ANGUILLA|\x53|ASIA/DAMASCUS|\x30|AMERICA/NORTH_DAKOTA/CENTER|\x09|AMERICA/INDIANA/VEVAY|\x08|ATLANTIC/ST_HELENA|\x11|AMERICA/BARBADOS|\x15|EUROPE/VATICAN|\x09|AMERICA/INDIANA/VINCENNES|\x10|ASIA/ALMATY|\x11|AMERICA/SANTO_DOMINGO|\x36|AFRICA/BRAZZAVILLE|\x3C|AMERICA/NOME|\x51|ASIA/TAIPEI|\x3C|AMERICA/YAKUTAT|\x05|AMERICA/ARGENTINA/MENDOZA|\x02|AUSTRALIA/CURRIE|\x54|ETC/UTC|\x15|EUROPE/VADUZ|\x2F|AFRICA/ASMERA|\x26|ANTARCTICA/MAWSON|\x55|ASIA/KOLKATA|\x2B|AFRICA/MASERU|\x46|AMERICA/ATIKOKAN|\x3A|ASIA/KUCHING|\x36|AFRICA/LIBREVILLE|\x08|AFRICA/FREETOWN|\x08|AFRICA/BISSAU|\x1C|EUROPE/SAMARA|\x15|EUROPE/AMSTERDAM|\x15|EUROPE/TIRANE|\x4F|PACIFIC/SAIPAN|\x08|AFRICA/ABIDJAN|\x29|EUROPE/ZAPOROZHYE|\x56|HST|\x25|AMERICA/EL_SALVADOR|\x15|EUROPE/MADRID|\x2F|AFRICA/JUBA|\x05|AMERICA/ARGENTINA/BUENOS_AIRES|\x05|AMERICA/ARGENTINA/SAN_LUIS|\x15|EUROPE/SKOPJE|\x01|WET|\x26|ASIA/ATYRAU|\x11|AMERICA/ARUBA|\x25|AMERICA/REGINA|\x57|PACIFIC/CHUUK|\x2A|ASIA/KHANDYGA|\x0D|PACIFIC/FUNAFUTI|\x1B|AMERICA/MERIDA|\x25|AMERICA/GUATEMALA|\x08|AFRICA/SAO_TOME|\x0F|ASIA/TOMSK|\x58|ASIA/MAKASSAR|\x35|AFRICA/BUJUMBURA|\x59|EUROPE/CHISINAU|\x1B|AMERICA/MONTERREY|\x26|ASIA/YEKATERINBURG|\x1F|ANTARCTICA/CASEY|\x38|PACIFIC/ENDERBURY|\x20|AMERICA/THULE|\x5A|AMERICA/ST_JOHNS|\x20|AMERICA/MONCTON|\x29|EUROPE/HELSINKI|\x07|ATLANTIC/CAPE_VERDE|\x25|AMERICA/TEGUCIGALPA|\x1D|INDIAN/COCOS|\x2C|AMERICA/BOISE|\x11|AMERICA/GUADELOUPE|\x09|AMERICA/NASSAU|\x15|EUROPE/PRAGUE|\x20|AMERICA/HALIFAX|\x0F|ASIA/HOVD|\x19|AMERICA/MANAUS|\x5B|ETC/UCT|\x08|ETC/GREENWICH|\x30|AMERICA/NORTH_DAKOTA/BEULAH|\x14|AMERICA/CHIHUAHUA|\x09|AMERICA/IQALUIT|\x05|AMERICA/ARGENTINA/RIO_GALLEGOS|\x03|PACIFIC/GAMBIER|\x23|EUROPE/VOLGOGRAD|\x08|AFRICA/BAMAKO|\x0F|ASIA/NOVOKUZNETSK|\x29|EUROPE/UZHGOROD|\x08|AFRICA/BANJUL|\x26|ASIA/AQTAU|\x2A|PACIFIC/PALAU|\x36|AFRICA/MALABO|\x23|EUROPE/MINSK|\x24|PST8PDT|\x01|ATLANTIC/MADEIRA|\x1F|PACIFIC/NOUMEA|\x36|AFRICA/KINSHASA|\x15|EUROPE/MALTA|\x05|AMERICA/ARGENTINA/USHUAIA|\x0F|ASIA/BANGKOK|\x5C|PACIFIC/NIUE|\x2E|AUSTRALIA/BRISBANE|\x05|AMERICA/RECIFE|\x0E|MST|\x1C|ASIA/YEREVAN|\x19|AMERICA/LA_PAZ|\x10|ASIA/URUMQI|\x35|AFRICA/LUSAKA|\x1F|PACIFIC/GUADALCANAL|\x2C|AMERICA/YELLOWKNIFE|\x0F|ASIA/VIENTIANE|\x23|ASIA/KUWAIT|\x08|AFRICA/CONAKRY|\x05|AMERICA/ARGENTINA/TUCUMAN|\x2A|ASIA/CHITA|\x15|EUROPE/OSLO|\x11|AMERICA/ST_KITTS|\x46|AMERICA/PANAMA|\x35|AFRICA/GABORONE|\x5D|ASIA/HEBRON|\x0B|AMERICA/GUAYAQUIL|\x3A|ASIA/KUALA_LUMPUR|\x30|AMERICA/MENOMINEE|\x0D|ASIA/KAMCHATKA|\x5E|FACTORY|\x57|ASIA/VLADIVOSTOK|\x30|AMERICA/MATAMOROS|\x23|ASIA/QATAR|\x1C|ASIA/DUBAI|\x2A|ASIA/YAKUTSK|\x10|ASIA/OMSK|\x36|AFRICA/BANGUI|\x05|AMERICA/PARAMARIBO|\x1F|ETC/GMT-11|\x57|ETC/GMT-10|\x35|AFRICA/LUBUMBASHI|\x0D|ETC/GMT-12|\x28|ETC/GMT-14|\x5F|PACIFIC/MARQUESAS|\x15|EUROPE/BRATISLAVA|\x0D|ASIA/ANADYR|\x09|AMERICA/NEW_YORK|\x1F|PACIFIC/NORFOLK|\x30|CST6CDT|\x39|PACIFIC/RAROTONGA|\x11|AMERICA/DOMINICA|\x36|AFRICA/PORTO-NOVO|\x26|ASIA/SAMARKAND|\x26|ASIA/DUSHANBE|\x09|AMERICA/KENTUCKY/LOUISVILLE|\x09|AMERICA/TORONTO|\x05|AMERICA/BAHIA|\x26|INDIAN/MALDIVES|\x08|AFRICA/ACCRA|\x1C|ASIA/MUSCAT|\x2C|AMERICA/EDMONTON|\x0D|PACIFIC/WAKE|\x30|AMERICA/INDIANA/TELL_CITY|\x60|AUSTRALIA/DARWIN|\x24|AMERICA/WHITEHORSE|\x25|AMERICA/SWIFT_CURRENT|\x15|EUROPE/COPENHAGEN|\x05|AMERICA/ARGENTINA/SALTA|\x11|AMERICA/MONTSERRAT|\x3F|EUROPE/SIMFEROPOL|\x35|AFRICA/BLANTYRE|\x09|AMERICA/DETROIT|\x11|AMERICA/GRENADA|\x01|ATLANTIC/FAROE|\x09|AMERICA/INDIANA/PETERSBURG|\x61|ASIA/KATHMANDU|\x4B|ASIA/PONTIANAK|\x29|EUROPE/ATHENS|\x09|AMERICA/PORT-AU-PRINCE|\x46|AMERICA/CAYMAN|\x54|ETC/UNIVERSAL|\x2F|AFRICA/DAR_ES_SALAAM|\x11|AMERICA/CURACAO|\x26|INDIAN/KERGUELEN|\x2F|AFRICA/KHARTOUM|\x3A|ASIA/MANILA|\x08|AFRICA/LOME|\x36|AFRICA/DOUALA|\x29|EET|\x05|AMERICA/ARGENTINA/SAN_JUAN|\x30|AMERICA/NORTH_DAKOTA/NEW_SALEM|\x11|AMERICA/KRALENDIJK|\x57|PACIFIC/PORT_MORESBY|\x16|EUROPE/JERSEY|\x15|EUROPE/ANDORRA|\x15|EUROPE/LUXEMBOURG|\x56|PACIFIC/HONOLULU|\x11|AMERICA/ST_THOMAS|\x0D|PACIFIC/MAJURO|\x62|ASIA/HONG_KONG|\x51|ASIA/MACAU|\x16|EUROPE/ISLE_OF_MAN|\x15|EUROPE/BELGRADE|\x3A|ASIA/CHOIBALSAN|\x29|EUROPE/MARIEHAMN|\x63|ANTARCTICA/MCMURDO|\x09|AMERICA/THUNDER_BAY|\x24|AMERICA/LOS_ANGELES|\x05|AMERICA/PUNTA_ARENAS|\x2F|INDIAN/ANTANANARIVO|\x15|EUROPE/SARAJEVO|\x08|ATLANTIC/REYKJAVIK|\x29|ASIA/NICOSIA|\x5C|ETC/GMT+11|\x39|ETC/GMT+10|\x0F|ASIA/BARNAUL|\x64|PACIFIC/TONGATAPU|\x11|AMERICA/MARIGOT|\x04|PACIFIC/PITCAIRN|\x06|ATLANTIC/SOUTH_GEORGIA|\x1E|AFRICA/EL_AAIUN|\x24|US/PACIFIC-NEW|\x23|EUROPE/KIROV|\x32|AMERICA/CAMPO_GRANDE|\x0E|AMERICA/DAWSON_CREEK|\x10|ANTARCTICA/VOSTOK|\x29|EUROPE/BUCHAREST|\x19|AMERICA/PORTO_VELHO|\x15|EUROPE/MONACO|\x10|ASIA/BISHKEK|\x15|AFRICA/CEUTA|\x15|EUROPE/ROME|\x30|AMERICA/WINNIPEG|\x26|ASIA/AQTOBE|\x08|AFRICA/DAKAR|\x05|AMERICA/FORTALEZA|\x0D|PACIFIC/TARAWA|\x24|AMERICA/DAWSON|\x2F|AFRICA/ADDIS_ABABA|\x1F|PACIFIC/EFATE|\x08|GMT|\x10|ASIA/QYZYLORDA|\x15|EUROPE/SAN_MARINO|\x65|MET|\x63|PACIFIC/AUCKLAND|\x3C|AMERICA/METLAKATLA|\x11|AMERICA/TORTOLA|\x54|ETC/ZULU|\x2C|AMERICA/DENVER|\x10|INDIAN/CHAGOS|\x20|AMERICA/GLACE_BAY|\x1F|PACIFIC/BOUGAINVILLE|\x0E|AMERICA/HERMOSILLO|\x34|AFRICA/TUNIS|\x1C|EUROPE/ULYANOVSK|\x57|ASIA/UST-NERA|\x30|AMERICA/RESOLUTE|\x08|ETC/GMT|\x5D|ASIA/GAZA|\x66|EUROPE/DUBLIN|\x57|ANTARCTICA/DUMONTDURVILLE|\x05|AMERICA/ARGENTINA/CATAMARCA|\x30|AMERICA/INDIANA/KNOX|\x29|EUROPE/NICOSIA|\x08|AFRICA/TIMBUKTU|\x0F|ASIA/NOVOSIBIRSK|\x46|EST|\x35|AFRICA/KIGALI|\x11|AMERICA/GRAND_TURK|\x36|AFRICA/LAGOS|\x29|EUROPE/SOFIA|\x0B|AMERICA/LIMA|\x3C|AMERICA/ANCHORAGE|\x0D|PACIFIC/NAURU|";

// Gets the POSIX equiv to Olson, if not, shows TZMISSING.
String Olson2POSIX::getPOSIX(String inOlson){
    int I = 0;
    byte B = 255;
    String S = inOlson;
    S.toUpperCase();
    String T = "|#|";
    T.replace("#",S);
    if (!Inited) init();
    I = sOLSON.indexOf(T);
    if (I > 0) B = sOLSON.charAt(I - 1);
    if (B != 255) { OlsonFromWeb = inOlson; return sPOSIX[B - 1]; }
    return TZMISSING;
}

// setOlsonTimeZone sets the timezone currently stored.
bool Olson2POSIX::setCurrentTimeZone(){
    if (!Inited) init();
    if (POSIX[0] != 0){
        setTZInternal();
        tzset();
        return true;
    }
    return false;
}

// setOlsonTimeZone sets the timezone to the Olson given.
bool Olson2POSIX::setOlsonTimeZone(String inOlson){
    String P = getPOSIX(inOlson);
    if (!Inited) init();
    if (P.length() > 0 && P != TZMISSING){
        strcpy(POSIX,P.c_str());
        OlsonFromWeb = inOlson;
        setTZInternal();
        tzset();
        return true;
    }
    return false;
}

String Olson2POSIX::getCurrentPOSIX(){
    if (!Inited) init();
    String C = POSIX;
    if (C.length() == 0 || POSIX[0] == 0) return TZMISSING;
    return C;
}

String Olson2POSIX::getCurrentOlson() { return OlsonFromWeb; }

// Set a POSIX from possibly an NVS value?
bool Olson2POSIX::setCurrentPOSIX(String NewPOSIX){
    if (!Inited) init();
    if (NewPOSIX.length() > 0){
        strcpy(POSIX,NewPOSIX.c_str());
        setTZInternal();
        tzset();
        return true;
    }
    return false;
}

// Sets the TimeZone with fix for static timezones.
void Olson2POSIX::setTZInternal(){
if (POSIX[0] == 0x3C){     // check if first char is '<'
    String _tz(POSIX);
    String _tzfix((char *)0);
    _tzfix.reserve(sizeof(POSIX)) ;
    _tzfix += FPSTR(P_LOC);
    if (_tz.indexOf('<',1) > 0){  // there might be two <> quotes
        _tzfix += _tz.substring(_tz.indexOf('>')+1, _tz.indexOf('<',1));
        _tzfix += FPSTR(P_LOC);
    }
    _tzfix += _tz.substring(_tz.lastIndexOf('>')+1, _tz.length());
    setenv("TZ", _tzfix.c_str(), 1);
    } else setenv("TZ", POSIX, 1);
}

// Asks for the response, not currently Async...
bool Olson2POSIX::beginOlsonFromWeb(WiFiClient &client){
    oWiFiC = client;
    if (!Inited) init();
    if (WiFi.status() != WL_CONNECTED) return false;
    ODone = false;
    if (OlsHandle == NULL) {
      WebError = 0;
      OlsRet = xTaskCreate(Olson2POSIX::OlsonGet,"Olson2POSIX_Get",3072,NULL,(tskIDLE_PRIORITY + 1),&OlsHandle);
      Obegan = (OlsHandle != NULL);
    }
    return Obegan;
}

void Olson2POSIX::endOlsonFromWeb(){ if (Obegan){ ODone = false; Obegan = false; } }

int Olson2POSIX::getOlsonWebError() { return WebError; }

// Has the response happened?
bool Olson2POSIX::gotOlsonFromWeb(){
    if (!Inited) init();
	return ODone;
}

// Does the function to retrieve the Olson from the web (on a task).
void Olson2POSIX::OlsonGet(void * parameter){
int itmp = 0;
int tmp = 0;
int len = 0;
int cnt = 0;
size_t size = 0;
uint8_t buff[128] = {0};
WiFiClient *netstream;
String stmp, payload;
bool Good = (WiFi.status() == WL_CONNECTED);
JsonDocument root;
bool Sent = false;
unsigned long Stay = millis() + 1000;
vTaskDelay(5/portTICK_PERIOD_MS);
    while (Good && millis() < Stay){
        if (Obegan){
            if (!Sent) { Sent = true; oHTTP.setConnectTimeout(3000); Stay = millis() + 3500; Good = oHTTP.begin(oWiFiC, TZURL); } // Call it and leave.
            else itmp = oHTTP.GET();
            if (itmp == HTTP_CODE_OK) {
                payload = "";
                len = oHTTP.getSize();
                netstream = oHTTP.getStreamPtr();
                while (oHTTP.connected() && Good && (len > 0 || len == -1) && millis() < Stay) {
                    size = netstream->available();
                    if (size) {
                        cnt = netstream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        if (cnt){
                            payload += String((const char*)buff).substring(0,cnt);
                            Stay += 250;
                        }
                        if (len == -1) {
                            Good = !(size > 0 && !(size - cnt));
                        } else if (len > 0) {
                            len -= cnt;
                        }
                    }
                    vTaskDelay(10/portTICK_PERIOD_MS);    // 10ms pauses.
                }
                if (len == -1) {
                  cnt = payload.indexOf("\x0a");
                  if (cnt && cnt + 1 < payload.length()) {
                      payload = payload.substring(cnt + 1);
                  }
                }
			if (!deserializeJson(root, payload)){
                stmp = root["timezone"].as<String>();
                stmp.replace('"',' ');
                stmp.trim();
	                OlsonFromWeb = stmp;
			}
                Good = false;
            }else if (itmp && Sent) { WebError = itmp; Good = false; } // Hit a web error.
        }
        if (Good) vTaskDelay(100/portTICK_PERIOD_MS);    // 100ms pauses.
    }
    oHTTP.end();
    ODone = true;
    OlsHandle = NULL;
    vTaskDelete(OlsHandle);
}

void Olson2POSIX::init(){
    int I;
    for (I = 0; I < 64; I++) POSIX[I] = 0;
    Inited = true;
    Obegan = false;
    OlsonFromWeb = "";
}
