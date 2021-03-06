#ifndef RWS_NTP_H
#define RWS_NTP_H

#include <rws_common.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Timezone.h>

class rws_ntp : public NTPClient
{
    public:
        rws_ntp(const char* poolServerName, int timeOffset, int updateInterval, Timezone *tz);
        ~rws_ntp();

        void setup(void);
        void loop(void);

        void setRules(TimeChangeRule dstStart, TimeChangeRule stdStart);

        void test(void);
        void test(unsigned long delay_ms);
        std::string get_local_datetime(void);
        time_t get_local_datetime_t();
        std::string get_local_datetime(time_t utc);

    private:
        WiFiUDP ntpUDP;
        Timezone *tz;

        void printDateTime(time_t t, const char *tz);
};

#endif // RWS_NTP_H