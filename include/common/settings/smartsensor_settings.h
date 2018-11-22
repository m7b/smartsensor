#ifndef SMARTSENSOR_SETTINGS_H
#define SMARTSENSOR_SETTINGS_H

#include <vector>

#define ON 1
#define OFF 0

//Debug flag ON or OFF
#define DEBUG ON


/**
 * @brief Declaration of ssid and password
 * 
 */
static const std::vector<std::pair<const char*, const char*>> wifi_access_credentials = {
/*  {"<ssid>"},            "<password>", */
    {"Moto G mjb Hotspot", "Hasso123"},
    {"wurschdsalat",       "0166455478363093"},
    {"wolke",              "5300-00005948-01w"}
};


/**
 * @brief Declaration of NTP serverd
 * 
 */
#define NTP_SERVER "pool.ntp.org"
#define NTP_OFFSET_S 0
#define NTP_UPDATE_INTERVAL_MS 3600000


/**
 * @brief TimeChange rule to define
 * 
 */
#define CEST_ABBREV "CEST"  //Central European Summer Time
#define CEST_WEEK   Last
#define CEST_DOW    Sun
#define CEST_MONTH  Mar
#define CEST_HOUR   2
#define CEST_OFFSET 120
#define CET_ABBREV  "CET"   //Central European Time
#define CET_WEEK    Last
#define CET_DOW     Sun
#define CET_MONTH   Oct
#define CET_HOUR    3
#define CET_OFFSET  60


/**
 * @brief Syslog server connection info
 * 
 */
#define SYSLOG_SERVER "192.168.178.36" //(DSM -> Protocol-Center)
#define SYSLOG_PORT 514
#define DEVICE_HOSTNAME "my-device"
#define APP_NAME "smartsensor"


/**
 * @brief MQTT Connection settings
 * 
 */
#define MQTT_SERVER "broker.hivemq.com"
//#define MQTT_SERVER "test.mosquitto.org"  //bierfass
#define MQTT_PORT   1883


/**
 * @brief Ultrasonic settings
 * 
 * For booting the ESP8266 to operation mode, GPIO0 and GPIO2 has to
 * be at high state at boot time. Therefor only the trigger pin of
 * the ultrasonic sensor can be used to connect to one of those GPIOs.
 * 
 */
#define TRIGGER_PIN                  0   //Trigger-Pin (high impedanz)
#define ECHO_PIN                     3   //Echo-Pin
#define MAX_DISTANCE_CM            200   //Max distance in [cm]
#define MAX_FILL_LEVEL_CM          110   //Max fill level of barrel in [cm]

#define TOP_LEVEL_TOPIC             "WS/RWS/"
#define LOCATION_NAME_SENSOR        "EG/Barrel1/"
#define LEVEL_SENSOR_CM_TOPIC       "Lvl/raw_cm"
#define LEVEL_CM_TOPIC              "Lvl/cm"
#define LEVEL_PERCENT_TOPIC         "Lvl/percent"
#define LEVEL_TIMESTAMP_TOPIC       "Lvl/timestamp"
#define LEVEL_LOCAL_TIMESTAMP_TOPIC "Lvl/local_timestamp"


/**
 * @brief Topics to subscribe
 * 
 */
static const std::vector<std::pair<const int, const char*>> topics_to_subscribe = {
/*  {<idx>}, "<topic>", */
    {0,      "FunctionMode"},
    {1,      "inTopic"}
};

#endif // SMARTSENSOR_SETTINGS_H