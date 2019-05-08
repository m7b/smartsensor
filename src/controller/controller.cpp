#include "controller/controller.h"

//Weekly Alarm stuff
extern void callbackPlain(void);

//NTP stuff; Called by time.h handler
extern time_t getNtpTime(void);

controller::controller(rws_wifi *wifi, rws_ntp *ntp, rws_syslog *syslog, rws_pubsubclient *mqtt, rws_webupdate *webUpd, WeeklyAlarm *weekAlarm)
: statemachine(N000_INIT_STEP)
{
    _wifiMulti   = wifi;
    _ntp         = ntp;
    _syslog      = syslog;
    _webUpdate   = webUpd;
    _mqtt        = mqtt;
    _weeklyAlarm = weekAlarm;
    
    _sens_src_online = false;
    _sens_dst_online = false;

    _function_mode_src_req = 0;
    _function_mode_src_ack = 0;
    _function_mode_dst_req = 0;
    _function_mode_dst_ack = 0;

    _light = new rgbled(ONBOARD_LED_RED, ONBOARD_LED_GREEN, ONBOARD_LED_BLUE);
    _pump  = new pump(PUMP_OUTPUT);

    _src_barrel_present = true;
    _dst_barrel_present = true;

    _alarm_occurred = false;
}

controller::~controller()
{
    delete _light;
    delete _pump;
}

void controller::setup(void)
{
    setup_serial();
    setup_wifi();
    setup_ntp();
    setup_syslog();
    setup_mqtt();
    setup_webupdate();
    setup_weeklyAlarms();

    _light->set_delay_ms(333);

    print_stm_steps();
}

void controller::loop(void)
{
    //update time from NTP on demand, see NTP_UPDATE_INTERVAL_MS
    _ntp->loop();
    timeStatus();

    //check and renew WiFi connection
    _wifiMulti->check_connection();

    //check and renew MQTT connection
    _mqtt->check_connection();

    //Web-Update functionality
    _webUpdate->loop();

    //check all conditions are ok
    if (!check_all_conditions())
        return;

    //keep mqtt alive
    _mqtt->loop();

    //test utc time
    //_ntp->test(3000);

    //control rgb led
    _light->loop();

    //Weekly Alarm stuff
    _weeklyAlarm->handler(); //manage time callbacks of all alarms

    //operation
    operating();
}



///////////////////////////////////////////////////////////////////////////////
// PRIVATE
///////////////////////////////////////////////////////////////////////////////
void controller::setup_serial(void)
{
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

#if DEBUG == ON
    // Take time for opening serial window
    delay(10000);
#endif
}

void controller::setup_wifi(void)
{
    // Add WiFi connection credentials
    for(auto entry : wifi_access_credentials) 
        _wifiMulti->addAP(entry.first, entry.second);

    WiFi.mode(WIFI_STA);

    // We start by connecting to a WiFi network
    _wifiMulti->check_connection();
}

void controller::setup_ntp(void)
{
    Serial.println("We start getting time from NTP ...");
    // We start getting time from ntp
    _ntp->setup();
    
    // Wait until time retrieved
    while (!_ntp->update())
        Serial.printf(".");
    
    Serial.printf("\r\nNTP connected\r\n");

    // Setup handler for syncing system time with local time 
    Serial.println("Setup handler for syncing system time with local time");
    setSyncProvider(getNtpTime);
    setSyncInterval(60);

    //Print actual local time with now():
    Serial.printf("Actual local time with now() is: ");
    _weeklyAlarm->prettyPrintTime(now(), Serial);
}

void controller::setup_syslog(void)
{
    //Syslog already usable
}

void controller::setup_mqtt(void)
{
    //Set topics to subscribe
    _mqtt->set_topics_to_subscribe(&topics_to_subscribe);

    //Set callback function
    _mqtt->setCallback([this] (char* topic, uint8_t* payload, unsigned int length) { this->mqtt_callback(topic, payload, length); });

    // We start by connecting to MQTT server
    _mqtt->check_connection();
}


void controller::setup_webupdate(void)
{
    _webUpdate->setup();
}


void controller::setup_weeklyAlarms(void)
{
    _alarm1.setCallback(callbackPlain);
    _alarm1.set(AlarmType::ALL_DAYS, ON, 13, 45);
    _weeklyAlarm->add(_alarm1);
    
    _alarm2.setCallback(callbackPlain);
    _alarm2.set(AlarmType::ALL_DAYS, ON, 13, 50);
    _weeklyAlarm->add(_alarm2);
    
    _alarm3.setCallback(callbackPlain);
    _alarm3.set(AlarmType::ALL_DAYS, ON, 14, 00);
    _weeklyAlarm->add(_alarm3);
    
}


bool controller::check_all_conditions(void)
{
    bool rc = true;
    
    rc = rc & _wifiMulti->connected();
    rc = rc & _mqtt->connected();
    rc = rc & _ntp->update();

    return rc;
}

void controller::mqtt_callback(char* topic, uint8_t* payload, unsigned int length) {
    Serial.print("  - Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.println();

    for(auto el: topics_to_subscribe)
    {
        if (strcmp(topic, std::get<TP_TOP>(el)) == 0)
        {
            switch(std::get<TP_NUM>(el))
            {
                case 0:
                    Serial.printf("  - Function mode ack received from sensor_src: %d\r\n", payload[0]);
                    _function_mode_src_ack = payload[0];
                    break;

                case 1:
                    Serial.printf("  - Function mode ack received from sensor_dst: %d\r\n", payload[0]);
                    _function_mode_dst_ack = payload[0];
                    break;

                case 100:
                    Serial.printf("  - RAW_CM from sensor_src: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 101:
                    Serial.printf("  - CM from sensor_src: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 102:
                    Serial.printf("  - PERCENT from sensor_src: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 103:
                    Serial.printf("  - TIMESTAMP from sensor_src: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 104:
                    Serial.printf("  - LOC_TIMESTAMP from sensor_src: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 105:
                    Serial.printf("  - SENS_LAST_WILL_TOPIC (Status) from sensor_src: %s\r\n", payload_to_string(payload, length).c_str());
                    _sens_src_online = (payload_to_string(payload, length).compare("Online") == 0);
                    break;

                case 200:
                    Serial.printf("  - RAW_CM from sensor_dst: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 201:
                    Serial.printf("  - CM from sensor_dst: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 202:
                    Serial.printf("  - PERCENT from sensor_dst: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 203:
                    Serial.printf("  - TIMESTAMP from sensor_dst: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 204:
                    Serial.printf("  - LOC_TIMESTAMP from sensor_dst: %s\r\n", payload_to_string(payload, length).c_str());
                    break;

                case 205:
                    Serial.printf("  - SENS_LAST_WILL_TOPIC (Status) from sensor_dst: %s\r\n", payload_to_string(payload, length).c_str());
                    _sens_dst_online = (payload_to_string(payload, length).compare("Online") == 0);
                    break;

                case 500:
                    Serial.printf("  - MANUAL_PUMP_REQ from dashboard: %s\r\n", payload_to_string(payload, length).c_str());
                    _light->set_enable(payload[0] == '1');
                    _mqtt->publish(MANUAL_PUMP_ACKNOWLEDGE, payload_to_string(payload, length));
                    break;

                case 501:
                    Serial.printf("  - MANUAL_VALVE_REQ from dashboard: %s\r\n", payload_to_string(payload, length).c_str());
                    _mqtt->publish(MANUAL_VALVE_ACKNOWLEDGE, payload_to_string(payload, length));
                    break;
            }
        }
    }
}

void controller::operating(void)
{
    switch (get_step())
    {
        case N000_INIT_STEP:
            if (_alarm_occurred)
            {
                _alarm_occurred = false;
                set_next_step(N001_START_TIMEOUT_FOR_ACTIVATION);
            }
            break;

        case N001_START_TIMEOUT_FOR_ACTIVATION:
            _start_time = millis();
            set_next_step(N002_WAIT_TIMEOUT_FOR_ACTIVATION);
            break;

        case N002_WAIT_TIMEOUT_FOR_ACTIVATION:
            //wait timeout depending on operation mode
            if (get_duration_ms(_start_time) >= 2500)
            {
                //Test analog input
                _mqtt->publish(VAL_AMBIENT_BRIGHTNESS, analogRead(A0));

                set_next_step(N010_CHECK_PUMP_NOT_ACTIVE);
            }
            break;

        case N010_CHECK_PUMP_NOT_ACTIVE:
            if (true)
                set_next_step(N020_CHECK_DST_LEVEL_BELOW_MAX);
            else
                set_next_step(N080_CHECK_ONLY_START_PUMP);
            break;
            
        case N020_CHECK_DST_LEVEL_BELOW_MAX:
            if (true)
                set_next_step(N030_CHECK_SRC_LEVEL_OVER_MIN);
            else
                set_next_step(N999_END);
            break;
            
        case N030_CHECK_SRC_LEVEL_OVER_MIN:
            if (true)
                set_next_step(N035_CHANGE_MEAS_MODE_SENSORS);
            else
                set_next_step(N999_END);
            break;
            
        case N035_CHANGE_MEAS_MODE_SENSORS:
            set_sens_mode(FunctionModes::FUNCTION_MODE_INTERVAL_MEASURE__5_SEK);
            _start_time = millis();
            set_next_step(N036_WAIT_MEAS_MODE_SENSORS_CHANGED);
    
            break;
            
        case N036_WAIT_MEAS_MODE_SENSORS_CHANGED:
            if (check_sens_mode(FunctionModes::FUNCTION_MODE_INTERVAL_MEASURE__5_SEK))
                set_next_step(N040_CHECK_PUMP_READY);

            //Retry request to change mode after timeout
            if (get_duration_ms(_start_time) >= 5000)
                set_next_step(N035_CHANGE_MEAS_MODE_SENSORS);
          
            break;
            
        case N040_CHECK_PUMP_READY:
            if (true)
                set_next_step(N050_CHECK_START_PUMP);
            else
                set_next_step(N999_END);
            break;
            
        case N050_CHECK_START_PUMP:
            set_next_step(N060_WAIT_PUMP_STARTED);
            break;
            
        case N060_WAIT_PUMP_STARTED:
            set_next_step(N070_STORE_START_TIME);
            break;
            
        case N070_STORE_START_TIME:
            set_next_step(N080_CHECK_ONLY_START_PUMP);
            break;
            
        case N080_CHECK_ONLY_START_PUMP:
            set_next_step(N090_CHECK_SRC_LEVEL_DECREASING);
            break;
            
        case N090_CHECK_SRC_LEVEL_DECREASING:
            if (true)
                set_next_step(N100_CHECK_DST_LEVEL_INCREASING);
            else
                set_next_step(N200_REPORT_ERROR);
            break;
            
        case N100_CHECK_DST_LEVEL_INCREASING:
            if (true)
                set_next_step(N110_CHECK_SRC_LEVEL_OVER_MIN);
            else
                set_next_step(N200_REPORT_ERROR);
            break;
            
        case N110_CHECK_SRC_LEVEL_OVER_MIN:
            if (true)
                set_next_step(N120_CHECK_DST_LEVEL_BELOW_MAX);
            else
                set_next_step(N300_CHECK_STOP_PUMP);
            break;
            
        case N120_CHECK_DST_LEVEL_BELOW_MAX:
            if (true)
                set_next_step(N130_CHECK_NO_STOP_DEMAND_FROM_USER);
            else
                set_next_step(N300_CHECK_STOP_PUMP);
            break;
            
        case N130_CHECK_NO_STOP_DEMAND_FROM_USER:
            if (true)
                set_next_step(N140_CHECK_PUMP_DURATION_BELOW_MAX);
            else
                set_next_step(N300_CHECK_STOP_PUMP);
            break;
            
        case N140_CHECK_PUMP_DURATION_BELOW_MAX:
            if (true)
                set_next_step(N200_REPORT_ERROR);
            else
                set_next_step(N300_CHECK_STOP_PUMP);
            break;
            
        case N200_REPORT_ERROR:
            set_next_step(N300_CHECK_STOP_PUMP);
            break;
            
        case N300_CHECK_STOP_PUMP:
            set_next_step(N310_CHECK_PUMP_STOPPED);
            break;
            
        case N310_CHECK_PUMP_STOPPED:
            set_next_step(N320_STORE_PUMP_DATA);
            break;
            
        case N320_STORE_PUMP_DATA:
            set_next_step(N400_CHANGE_MEAS_MODE_SENSORS);
            break;
            
        case N400_CHANGE_MEAS_MODE_SENSORS:
            set_sens_mode(FunctionModes::FUNCTION_MODE_DEEP_SLEEP_20_SEK);
            _start_time = millis();
            set_next_step(N410_WAIT_MEAS_MODE_SENSORS_CHANGED);
    
            break;
            
        case N410_WAIT_MEAS_MODE_SENSORS_CHANGED:
            if (check_sens_mode(FunctionModes::FUNCTION_MODE_DEEP_SLEEP_20_SEK))
                set_next_step(N999_END);
                
            //Retry request to change mode after timeout
            if (get_duration_ms(_start_time) >= 5000)
                set_next_step(N400_CHANGE_MEAS_MODE_SENSORS);
            
            break;
            
        case N999_END:
            set_next_step(N000_INIT_STEP);
            break;
    }
}



void controller::set_sens_mode(FunctionModes m)
{
    if (_src_barrel_present)
    {
        _function_mode_src_req = m;
        _mqtt->publish(FUNCTION_MODE_SRC_REQUEST, _function_mode_src_req);
    }
    if (_dst_barrel_present)
    {
        _function_mode_dst_req = m;
        _mqtt->publish(FUNCTION_MODE_DST_REQUEST, _function_mode_dst_req);
    }
}


bool controller::check_sens_mode(FunctionModes m)
{
    bool rc = true;

    if (_src_barrel_present)
        rc = rc && (_function_mode_src_ack == m);

    if (_dst_barrel_present)
        rc = rc && (_function_mode_dst_ack == m);

    //Could also check if (_function_mode_src/dst_ack == _function_mode_src/dst_req)

    return rc;
}

void controller::print_stm_steps(void)
{
    Serial.println("");
    print_step_info(N000_INIT_STEP);
    print_step_info(N001_START_TIMEOUT_FOR_ACTIVATION);
    print_step_info(N002_WAIT_TIMEOUT_FOR_ACTIVATION);
    print_step_info(N010_CHECK_PUMP_NOT_ACTIVE);
    print_step_info(N020_CHECK_DST_LEVEL_BELOW_MAX);
    print_step_info(N030_CHECK_SRC_LEVEL_OVER_MIN);
    print_step_info(N035_CHANGE_MEAS_MODE_SENSORS);
    print_step_info(N036_WAIT_MEAS_MODE_SENSORS_CHANGED);
    print_step_info(N040_CHECK_PUMP_READY);
    print_step_info(N050_CHECK_START_PUMP);
    print_step_info(N060_WAIT_PUMP_STARTED);
    print_step_info(N070_STORE_START_TIME);
    print_step_info(N080_CHECK_ONLY_START_PUMP);
    print_step_info(N090_CHECK_SRC_LEVEL_DECREASING);
    print_step_info(N100_CHECK_DST_LEVEL_INCREASING);
    print_step_info(N110_CHECK_SRC_LEVEL_OVER_MIN);
    print_step_info(N120_CHECK_DST_LEVEL_BELOW_MAX);
    print_step_info(N130_CHECK_NO_STOP_DEMAND_FROM_USER);
    print_step_info(N140_CHECK_PUMP_DURATION_BELOW_MAX);
    print_step_info(N200_REPORT_ERROR);
    print_step_info(N300_CHECK_STOP_PUMP);
    print_step_info(N310_CHECK_PUMP_STOPPED);
    print_step_info(N320_STORE_PUMP_DATA);
    print_step_info(N400_CHANGE_MEAS_MODE_SENSORS);
    print_step_info(N410_WAIT_MEAS_MODE_SENSORS_CHANGED);
    print_step_info(N999_END);
    Serial.println("");
}