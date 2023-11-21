#include "WebSerialStream.h"
namespace TLogPlusStream {
    size_t WebSerialStream::write(uint8_t c) {
        _buff[_at % sizeof(_buff)] = c;
        _at++;
        return 1;
    }

    WebSerialStream::~WebSerialStream() {
        stop();
    }

    void WebSerialStream::begin() {
        if (_server != NULL)
            return;
        _server = new WebServer(_webPort);
        _server->on("/", [this]() {
            _server->send(200, "text/html", "<html><head><title>log</title></head>"
                                            "<style>"
                                            "#log { font-family: 'Courier New', monospace; white-space: pre; }"
                                            "</style>"
                                            "<script language=javascript>"
                                            "var at = 0;"
                                            "function st() { setTimeout(750, f()); };"
                                            "function f() { fetch('log?at='+at)."
                                            "then("
                                            "r => { return r.json(); }"
                                            ").then( "
                                            "j => { "
                                            /* we do rouhgly 'at the end' as Macs and Edge are a few pixels off. */
                                            " isAtEnd = (window.innerHeight + window.pageYOffset) >= document.body.offsetHeight - 4; "
                                            " document.getElementById('log').innerHTML += j.buff; "
                                            " at= j.at; "
                                            " if (isAtEnd) window.scrollTo(0,document.body.scrollHeight); "
                                            " st(); "
                                            "}"
                                            ").catch( e => { console.log(e); st(); } "
                                            ");"
                                            "};"
                                            "window.onLoad = f();"
                                            "</script>"
                                            "<body><div id=log></div></body></html>");
        });
        _server->on("/log", [this]() {
            if (!_server->hasArg("at")) {
                _server->send(400, "text/plain", "Missing at argument.");
                return;
            };
            unsigned long prevAt = _server->arg("at").toInt();
            String out = "{\"at\":" + String(_at) + ",\"buff\":\"";

            // reset browsers from the future (e.g. after a reset)
            if (prevAt > _at) {
                out += "<font color=red><hr><i>.. log reset..</i></font><hr>";
                prevAt = _at;
            };
            if (_at > sizeof(_buff) && prevAt < _at - sizeof(_buff)) {
                out += "<font color=red><hr><i>.. skipping " +
                       String(_at - sizeof(_buff) - prevAt) +
                       " bytes of log - no longer in buffer ..</i><hr></font>";
                prevAt = _at - sizeof(_buff);
            };
            for (; prevAt != _at; prevAt++) {
                char c = _buff[prevAt % sizeof(_buff)];
                switch (c) {
                    case '<':
                        out += "&lt;";
                        break;
                    case '>':
                        out += "&gt;";
                        break;
                    case '\b':
                        out += "\\b";
                        break;
                    case '\n':
                        out += "\\n";
                        break;
                    case '\r':
                        out += "\\r";
                        break;
                    case '\f':
                        out += "\\f";
                        break;
                    case '\t':
                        out += "\\t";
                        break;
                    case '"' :
                        out += "\\\"";
                        break;
                    case '\\':
                        out += "\\\\";
                        break;
                    default  :
                        out += c;
                        break;
                };
            };
            out += "\"}";
            _server->send(200, "application/json", out);
        });
        _server->begin();

        TLogPlus::Log.printf("Opened serial web server on http://%s:%d\n", WiFi.localIP().toString().c_str(), _webPort);
    };

    void WebSerialStream::stop() {
        if (!_server)
            return;
        _server->stop();
        delete _server;
        _server = NULL;
    }

    void WebSerialStream::loop() {
        if (_server)
            _server->handleClient();
    }
}