#include "sensor/barrel.h"
#include "common/settings/settings.h"
#include "sensor/settings/settings.h"

/**
 * @brief Construct a new smartsensor barrel::smartsensor barrel object
 * 
 * @param ntp 
 * @param mqtt 
 * @param syslog 
 */
barrel::barrel(rws_ntp *ntp, rws_pubsubclient *mqtt, rws_syslog *syslog)
: NewPing(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE_CM), ntp(ntp), mqtt(mqtt), syslog(syslog)
{
    this->max_fill_level_cm = MAX_FILL_LEVEL_CM;
}

/**
 * @brief Destroy the smartsensor barrel::smartsensor barrel object
 * 
 */
barrel::~barrel()
{
}

/**
 * @brief Start meassure
 * 
 * @return true if measured value is in defined area.
 * @return false if measured value is out of area or no obstacle detected.
 */
bool barrel::do_measure(void)
{
    bool rc = false;

    fill_level.clear();
    fill_level.sensor_cm = ping_cm();
    fill_level.utc       = ntp->getEpochTime();

    if ((fill_level.sensor_cm <= 0) || (fill_level.sensor_cm > max_fill_level_cm))
        return rc;
        
    fill_level.cm        = max_fill_level_cm - fill_level.sensor_cm;
    fill_level.percent   = (100.0 * (float)fill_level.cm) / (float)max_fill_level_cm;
    
    rc = true;

    return rc;
}

/**
 * @brief Publish fill level
 * 
 * @return true 
 * @return false 
 */
bool barrel::do_publish(void)
{
    bool rc = true;

    //Publish fill level
    rc = rc & do_publish_mqtt();

#if DEBUG == ON
    rc = rc & do_publish_syslog();
    rc = rc & do_publish_serial();
#endif

    return rc;
}

/**
 * @brief Return fill level struct
 * 
 * @return sFillLevel 
 */
sFillLevel barrel::get_fill_level(void)
{
    return fill_level;
}

/**
 * @brief Return fill level cm
 * 
 * @return unsigned long 
 */
unsigned long barrel::get_fill_level_cm(void)
{
    return fill_level.cm;
}

/**
 * @brief Return fill level in percent
 * 
 * @return float 
 */
float barrel::get_fill_level_percent(void)
{
    return fill_level.percent;
}

/**
 * @brief Return timestamp as local date time
 * 
 * @return std::string 
 */
std::string barrel::get_local_datetime(void)
{
    return ntp->get_local_datetime(fill_level.utc);
}

/**
 * @brief Publish fill level as MQTT
 * 
 * @return true 
 * @return false 
 */
bool barrel::do_publish_mqtt(void)
{
    bool rc = true;

    //Publish fill level
    
    rc = rc & mqtt->publish(LEVEL_SENSOR_CM_TOPIC,       fill_level.sensor_cm);
    rc = rc & mqtt->publish(LEVEL_CM_TOPIC,              fill_level.cm);
    rc = rc & mqtt->publish(LEVEL_PERCENT_TOPIC,         fill_level.percent);
    rc = rc & mqtt->publish(LEVEL_TIMESTAMP_TOPIC,       fill_level.utc);
    rc = rc & mqtt->publish(LEVEL_LOCAL_TIMESTAMP_TOPIC, get_local_datetime());

    return rc;
}

/**
 * @brief Publish fill level to syslog
 * 
 * @return true 
 * @return false 
 */
bool barrel::do_publish_syslog(void)
{
    bool rc;

    rc = syslog->logf(LOG_INFO, "Zeitstempel: %s; Füllstand: %dcm; Prozentual: %.2f%%",
        get_local_datetime().c_str(),
        (int)fill_level.cm,
        fill_level.percent);

    return rc;
}

/**
 * @brief Publish fill level to serial
 * 
 * @return true 
 * @return false 
 */
bool barrel::do_publish_serial(void)
{
    bool rc = false;

    int len = Serial.printf("%s - Füllstand: %dcm; Prozentual: %.2f%%\r\n",
        get_local_datetime().c_str(),
        (int)fill_level.cm,
        fill_level.percent);

    if (len > 0)
        rc = true;
    
    return rc;
}