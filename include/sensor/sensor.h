#ifndef SENSOR_H
#define SENSOR_H

#include <rws_wifi.h>
#include <rws_ntp.h>
#include <rws_syslog.h>
#include <rws_pubsubclient.h>
#include <rws_webupdate.h>
#include <statemachine.h>

#include "sensor/settings/settings.h"
#include "sensor/barrel.h"
#include "sensor/topics.h"

#include "common/mqtt_comm.h"

class sensor : public statemachine
{
    public:
        sensor(rws_wifi *wifi, rws_ntp *ntp, rws_syslog *syslog, rws_pubsubclient *mqtt, rws_webupdate *webUpd);
        ~sensor();

        void setup(void);
        void loop(void);

    private:
        rws_wifi *_wifiMulti;
        rws_ntp *_ntp;
        rws_syslog *_syslog;
        rws_webupdate *_webUpdate;
        rws_pubsubclient *_mqtt;

        barrel *_barrel;
        
        unsigned long _start_time;
        uint64_t _ds_time;

        bool _mqtt_online;
    
        //Function mode
        uint8_t _FunctionModeRequest;
        uint8_t _FunctionModeAck;
        uint8_t _FunctionMode;

        void setup_serial(void);

        void setup_wifi(void);
        void setup_ntp(void);
        void setup_syslog(void);
        void setup_mqtt(void);
        void setup_webupdate(void);

        bool check_all_conditions(void);

        void mqtt_callback(char* topic, uint8_t* payload, unsigned int length);
        
        void operating(void);

        void mqtt_offline_demand(void);
        void do_deep_sleep(uint64_t ds_time);
        void action_wlan_con_timeout(void);
        void action_mqtt_con_timeout(void);
        
        void print_stm_steps(void);
};


STEP_DEF(N000_INIT_STEP,                      "N000: Init step");
STEP_DEF(N010_START_TIMEOUT,                  "N010: Start timeout");
STEP_DEF(N020_START_MEASSURE,                 "N020: Start meassure");
STEP_DEF(N030_REPORT_MEASSURE,                "N030: Report meassure");
STEP_DEF(N040_CHECK_FUNCTION_MODE_CHANGE_REQ, "N040: Check function mode change request");
STEP_DEF(N050_CHANGE_TO_REQ_FUNCTION_MODE,    "N050: Change to requested function mode");
STEP_DEF(N060_ACK_NEW_FUNCTION_MODE,          "N060: Acknowledge new function mode");
STEP_DEF(N070_WAIT_TIMEOUT,                   "N070: Wait timeout, depending on operation mode");
STEP_DEF(N080_PUBLISH_SENSOR_OFFLINE,         "N080: Publish sensor offline");
STEP_DEF(N090_START_TIMEOUT_DS,               "N090: Start timeout for Deep-Sleep");
STEP_DEF(N100_WAIT_TIMEOUT_DS,                "N100: Wait timeout for Deep-Sleep");
STEP_DEF(N110_ENTER_DS,                       "N110: Enter Deep-Sleep");

#endif // SENSOR_H