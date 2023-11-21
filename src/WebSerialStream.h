#ifndef _H_WEBSERVER_TEE_LOG_PLUS
#define _H_WEBSERVER_TEE_LOG_PLUS

#include <TLogPlus.h>

#  if defined(ESP32)
#    include <WebServer.h>
#  elif defined(ESP8266)
#    include <ESP8266WebServer.h>
     typedef ESP8266WebServer WebServer;
#  else
#    error "Must be ESP32 or ESP8266"
#  endif
namespace TLogPlusStream {
    class WebSerialStream : public TLogPlus::TLog {
    public:
        WebSerialStream(const uint16_t webPort = 80) : _webPort(webPort) {};

        ~WebSerialStream();

        virtual size_t write(uint8_t c);

        virtual void begin();

        virtual void loop();

        virtual void stop();

    private:
        uint16_t _webPort;
        WebServer *_server = NULL;
        uint8_t _buff[1024];
        unsigned long _at = 0;
    protected:
    };
}
#endif