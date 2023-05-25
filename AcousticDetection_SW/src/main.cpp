#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SPIFFSEditor.h>
#include <WiFi.h>

#include "../include/circular_buffer.hpp"
#include "../include/config.hpp"
#include "../lib/dsp/dsp.hpp"
#include "../lib/i2s_mic/mic.hpp"

// ---------- SETTINGS ---------- //
// #define ENABLE_WS    // WebSockets
#define ENABLE_SSE	// Server Sent Events
// #define ENABLE_SSE_CORS_HEADER   // used when you want to request server from different url (for example if you open index.html file in your browser)

// #define ENABLE_WIFI_AP	// WIFI_STA otherwise
/* settings bellow are not recommended because it cause performance problems */
// #define ENABLE_WIFI_AP_404_REDIRECT
// #define ENABLE_WIFI_AP_DNS
// #define ENABLE_WIFI_AP_CAPTIVE_PORTAL   // not recommended

#define ENABLE_OTA

// #define AUDIO_CAPTURE_SERVER_ENABLED
// ------------------------------ //

#ifdef ENABLE_WIFI_AP
#include <DNSServer.h>
DNSServer dnsServer;
const IPAddress AP_local_ip(192, 168, 1, 1);
const IPAddress AP_gateway(192, 168, 1, 1);
const IPAddress AP_subnet(255, 255, 255, 0);

#ifdef ENABLE_WIFI_AP_CAPTIVE_PORTAL
class CaptiveRequestHandler : public AsyncWebHandler {
   public:
	CaptiveRequestHandler() {}
	virtual ~CaptiveRequestHandler() {}

	bool canHandle(AsyncWebServerRequest *request) {
		// request->addInterestingHeader("ANY");
		return true;
	}

	void handleRequest(AsyncWebServerRequest *request) {
		Serial.printf("Client was trying to reach: http://%s%s\n", request->host().c_str(), request->url().c_str());
		AsyncWebServerResponse *response = request->beginResponse(302);
		response->addHeader("Location", "http://" + WiFi.softAPIP().toString());
		return request->send(response);
	}
};
#endif
#endif

#ifdef ENABLE_OTA
#include <ArduinoOTA.h>
#endif

#ifdef AUDIO_CAPTURE_SERVER_ENABLED
#define AUDIO_CAPTURE_SERVER_URL "http://192.168.0.100:5005/i2s_samples"  //"http://192.168.26.92:5005/i2s_samples"
// #define AUDIO_CAPTURE_SERVER_URL "http://10.42.0.1:5005/i2s_samples"
#define SAMPLES_PER_REQ 1024 * 50
#include <HTTPClient.h>
// unsigned long last_audio_capture_conn_check_time = 0;
WiFiClient wifiClient;
HTTPClient httpClient;
// WiFiClient audio_capture_server_client;
uint8_t *data_for_send;
#endif

#define CORR_SIZE correlation_window_samples_num
CircularBuffer<audio_sample_t, CORR_SIZE> x1;
CircularBuffer<audio_sample_t, CORR_SIZE> x2;

LRMics mics(I2S_NUM_0);

TaskHandle_t dsp_task;

DSP dsp;

// SKETCH BEGIN
AsyncWebServer server(80);
#ifdef ENABLE_WS
AsyncWebSocket ws("/ws");
#endif

#ifdef ENABLE_SSE
AsyncEventSource events("/events");
#define SSE_SEND_ALIVE_MSG_TIME 8 * 1000  // 8 seconds
unsigned long last_sse_alive_msg_time = 0;
#endif

OnsetDetector od1;
OnsetDetector od2;

void send_angle(int angle, int error) {
	/*
		@angle - angle in range [0, 180]
		@error - uncertainty of angle in range [0, 10] - on web, width will be width=2*error
	*/
	char str[12];
	sprintf(str, "%d;%d", angle, error);
#ifdef ENABLE_WS
	ws.textAll(str);
#endif

#ifdef ENABLE_SSE
	events.send(str, "angle");
#endif
}

void sse_send_alive_msg() {
	events.send(String(millis()).c_str(), "alive");
}

#define LAST_CAPTURE_THRESHOLD 100	// 100ms

void dsp_func(void *param) {
	Serial.printf("DSP running on core: %d\n", xPortGetCoreID());
	LRMics *sampler = (LRMics *)(param);

#ifndef AUDIO_CAPTURE_SERVER_ENABLED
	int qq = 0;
	bool capture_started = false;
	unsigned long last_capture = 0;
#endif

	while (true) {
		// mics.read_and_print();
		// continue;
		//
		// delay(10000);

#ifdef AUDIO_CAPTURE_SERVER_ENABLED
		int sampled = 0;
		int ppp = 0;
		while (sampled < SAMPLES_PER_REQ) {
			int count = sampler->read(true);  //[](audio_sample_t left, audio_sample_t right){

			Serial.printf("%d, %d\n", count, sampled);
			// Serial.println("-------LEFT-------");
			// for(int i = 0; i < count; i ++) Serial.printf("%d,", sampler->left_channel_data[i]);
			// Serial.println("------------------");

			// Serial.println("-------RIGHT-------");
			// for(int i = 0; i < count; i ++) Serial.printf("%d,", sampler->right_channel_data[i]);
			// Serial.println("-------------------");
			// if(audio_capture_server_client.connected()){

			memcpy(data_for_send + sampled * sizeof(int16_t), sampler->data, count * sizeof(int16_t));
			sampled += count;
			// for(int i = 0; i < count*2; i ++){
			// 	Serial.printf("%d, ", ((int16_t*)data)[i]);
			// }
			// Serial.println();
			if (ppp == 5) httpClient.begin(wifiClient, AUDIO_CAPTURE_SERVER_URL);
			if (ppp == 6) httpClient.addHeader("content-type", "application/octet-stream");
			ppp++;
		}
		// data_for_send.push(data);

		digitalWrite(LED_BUILTIN, HIGH);

		httpClient.POST(data_for_send, sampled * sizeof(int16_t));
		httpClient.end();
		digitalWrite(LED_BUILTIN, LOW);
		// free(data);

		// for(int i = 0; i < count; i ++){
		// 	audio_sample_t left = sampler->left_channel_data[i];
		// 	audio_sample_t right = sampler->right_channel_data[i];

		// 	audio_capture_server_client.write(sampler->left_channel_data, count*sizeof(audio_sample_t));
		// 	audio_capture_server_client.write(sampler->right_channel_data, count*sizeof(audio_sample_t));
		//}

#else
		int count = sampler->read();  //[](audio_sample_t left, audio_sample_t right){

		for (int i = 0; i < count; i++) {
			audio_sample_t left = sampler->data[i];
			audio_sample_t right = sampler->data[++i];

			if (od1.detect(left) && od2.detect(right) && capture_started == false && millis() - last_capture > LAST_CAPTURE_THRESHOLD) {
				Serial.println("Capture started");
				capture_started = true;
			}

			if (capture_started) {
				x1.push(left);
				x2.push(right);
			}

			if (capture_started && ++qq >= CORR_SIZE) {
				Serial.println("START PROCESSING");
				qq = 0;
				capture_started = false;
				last_capture = millis();

				xcorr_result_t xcorr_peak;
				bool xcorr_peak_found = dsp.xcorr_max<CircularBuffer<audio_sample_t, CORR_SIZE> >(x1, x2, xcorr_peak, CORR_SIZE, max_shift_samples_num + 1, true, true, true);

				Serial.println(xcorr_peak_found);
				if (xcorr_peak_found) {
					digitalWrite(LED_BUILTIN, HIGH);
					int Nshift = xcorr_peak.max_pos;

					if (xcorr_peak.interpolated_max_pos > Nshift + 1 || xcorr_peak.interpolated_max_pos < Nshift - 1) {
						// this should never happen but added just to be sure
						Serial.printf("ERROR: interpolation %f is outside of %d+(-1,1) range", xcorr_peak.interpolated_max_pos, Nshift);
						xcorr_peak.interpolated_max_pos = Nshift;
					}

					double tau = xcorr_peak.interpolated_max_pos * (1. / I2S_SAMPLE_RATE);
					if (-maxN <= Nshift && Nshift <= maxN) {
						// we use interpolated max_pos for angle calculation and normal for error calculation
						int angle = dsp.rad2deg(dsp.theta_from_tau(tau)) + 0.5;
						int error = dsp.rad2deg(dsp.theta_error(Nshift)) + 0.5;

						Serial.printf("Before interpolation: %d, after: %f\n", xcorr_peak.max_pos, xcorr_peak.interpolated_max_pos);
						Serial.printf("N: %d, max: %f, tau: %f s, angle: %d +- %d\n", Nshift, xcorr_peak.max_Rx, tau, angle, error);

						send_angle(angle, error);
					} else {
						Serial.printf("XCORR - shift too big: N: %d, max: %f, tau: %f s\n", Nshift, xcorr_peak.max_Rx, tau);
					}
					digitalWrite(LED_BUILTIN, LOW);
				}
			}
		}
#endif
	}
}

#ifdef ENABLE_WS
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
	if (type == WS_EVT_CONNECT) {
		Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
		client->printf("Hello Client %u :)", client->id());
		client->ping();
	} else if (type == WS_EVT_DISCONNECT) {
		Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
	} else if (type == WS_EVT_ERROR) {
		Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
	} else if (type == WS_EVT_PONG) {
		Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
	} else if (type == WS_EVT_DATA) {
	}
}
#endif

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(115200);
#ifndef RELEASE
	Serial.setDebugOutput(true);
#endif
	Serial.print(F("START, file: " __FILE__ " built on " __DATE__ " " __TIME__));
#ifdef RELEASE
	Serial.println(" - RELEASE version");
#else
	Serial.println(" - DEBUG version");
#endif
	Serial.println("-----------------------------");
	Serial.println("ESP32 Acoustic Detection v0.3");
	Serial.println("-----------------------------");

	Serial.printf("xcorr window size: %d\n", CORR_SIZE);
	Serial.printf("fs: %d, mics d: %f, max N: %d\n", I2S_SAMPLE_RATE, MICS_DISTANCE, maxN);
	Serial.printf("setup running on core: %d\n", xPortGetCoreID());
	mics.init();

#ifdef AUDIO_CAPTURE_SERVER_ENABLED
	data_for_send = (uint8_t *)malloc(SAMPLES_PER_REQ * sizeof(int16_t));
#endif

	// Serial.println(ESP.getFreeHeap());
	// for(int i = 0; i < 2; i ++){
	// 	data_for_send.push((uint8_t*)malloc(SAMPLES_PER_REQ*sizeof(int16_t)));
	// }
	// Serial.println(ESP.getFreeHeap());
	// adc2.init();

	// auto start = micros();
	// for(int i = 0; i < 100; i ++){
	// 	correlation(x1, x2, CORR_SIZE, max_shift_samples_num);
	// }
	// auto end = micros();
	// Serial.printf("Correlation time: %f us\n", (end-start)/100.);
	// delay(10000);
	// while(1);
	/* Core where the task should run */

#ifdef ENABLE_WIFI_AP
	// docs: https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/wifi.html
	Serial.println("---------- AP mode ----------");
	Serial.println(String("AP creation: ") + (WiFi.softAP(wifi_ap_ssid, wifi_ap_pass, 6, 0, 4) ? "true" : "false"));
	Serial.println(String("AP config: ") + (WiFi.softAPConfig(AP_local_ip, AP_gateway, AP_subnet) ? "true" : "false"));
	Serial.println("-----------------------------");
#else
	Serial.println("---------- STA mode ----------");
	Serial.printf("Connecting to: %s\n", wifi_ssid);
	WiFi.mode(WIFI_STA);
	WiFi.begin(wifi_ssid, wifi_password);
	for (int i = 0; i < 3; i++) {
		if (WiFi.waitForConnectResult() == WL_CONNECTED) break;
		Serial.print(".");
	}
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Connection Failed! Rebooting...");
		delay(1000);
		ESP.restart();
	}
	WiFi.hostname(hostname);
	Serial.printf("Connected, ip: %s\n", WiFi.localIP().toString().c_str());

	MDNS.addService("http", "tcp", 80);
	Serial.println("------------------------------");
#endif

#ifdef ENABLE_OTA
	// Send OTA events to the browser
	ArduinoOTA.onStart([]() { events.send("Update Start", "ota"); });
	ArduinoOTA.onEnd([]() { events.send("Update End", "ota"); });
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
#ifdef ENABLE_SSE
		char p[32];
		sprintf(p, "Progress: %u%%\n", (progress / (total / 100)));
		events.send(p, "ota");
#endif
	});
	ArduinoOTA.onError([](ota_error_t error) {
#ifdef ENABLE_SSE
		if (error == OTA_AUTH_ERROR)
			events.send("Auth Failed", "ota");
		else if (error == OTA_BEGIN_ERROR)
			events.send("Begin Failed", "ota");
		else if (error == OTA_CONNECT_ERROR)
			events.send("Connect Failed", "ota");
		else if (error == OTA_RECEIVE_ERROR)
			events.send("Recieve Failed", "ota");
		else if (error == OTA_END_ERROR)
			events.send("End Failed", "ota");
#endif
	});
	// Port defaults to 3232
	// ArduinoOTA.setPort(ota_port);

	// Hostname defaults to esp3232-[MAC]
	ArduinoOTA.setHostname(hostname);

	// No authentication by default
	ArduinoOTA.setPassword(ota_password);

	ArduinoOTA.begin();
#endif

	SPIFFS.begin();

	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});

	// serverStatic supports gzip automatically
	// for GMT date us bash command: LC_TIME=en_US.utf8 TZ=GMT date
	server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html").setLastModified("Tue May 23 11:30:37 AM GMT 2023");  //.setCacheControl("max-age=600");   // Cache responses for 10 minutes (600 seconds)

	server.onNotFound([](AsyncWebServerRequest *request) {
#ifndef RELEASE
		Serial.printf("NOT_FOUND: ");
		if (request->method() == HTTP_GET)
			Serial.printf("GET");
		else if (request->method() == HTTP_POST)
			Serial.printf("POST");
		else if (request->method() == HTTP_DELETE)
			Serial.printf("DELETE");
		else if (request->method() == HTTP_PUT)
			Serial.printf("PUT");
		else if (request->method() == HTTP_PATCH)
			Serial.printf("PATCH");
		else if (request->method() == HTTP_HEAD)
			Serial.printf("HEAD");
		else if (request->method() == HTTP_OPTIONS)
			Serial.printf("OPTIONS");
		else
			Serial.printf("UNKNOWN");
		Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

		if (request->contentLength()) {
			Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
			Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
		}

		int headers = request->headers();
		int i;
		for (i = 0; i < headers; i++) {
			AsyncWebHeader *h = request->getHeader(i);
			Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
		}

		int params = request->params();
		for (i = 0; i < params; i++) {
			AsyncWebParameter *p = request->getParam(i);
			if (p->isFile()) {
				Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
			} else if (p->isPost()) {
				Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
			} else {
				Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
			}
		}
#endif

#ifdef ENABLE_WIFI_AP
#ifdef ENABLE_WIFI_AP_404_REDIRECT
		if (request->getHeader("host")->value() != WiFi.softAPIP().toString()) {
			Serial.printf("Wrong host http://%s%s\n, redirecting\n", request->host().c_str(), request->url().c_str());
			request->redirect("http://" + WiFi.softAPIP().toString());
		} else {
			request->send(404, "text/plain", "404 Not Found");
		}
#else
		request->send(404, "text/plain", "404 Not Found");
#endif
#else
		request->send(404, "text/plain", "404 Not Found");
#endif
	});

#ifdef ENABLE_WS
	ws.onEvent(onWsEvent);
	server.addHandler(&ws);
#endif

#ifdef ENABLE_SSE
#ifdef ENABLE_SSE_CORS_HEADER
	DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
#endif
	events.onConnect([](AsyncEventSourceClient *client) {
		sse_send_alive_msg();
	});
	server.addHandler(&events);
#endif

#ifdef ENABLE_WIFI_AP
#ifdef ENABLE_WIFI_AP_DNS
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(53, "*", WiFi.softAPIP());	// redirect to ESP
#endif
#ifdef ENABLE_WIFI_AP_CAPTIVE_PORTAL
	server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);	 // only when requested from AP
#endif
#endif

	server.begin();

	Serial.println("Starting...");
	xTaskCreatePinnedToCore(
		dsp_func,  /* Function to implement the task */
		"DSP",	   /* Name of the task */
		10000,	   /* Stack size in words */
		&mics,	   /* Task input parameter */
		2,		   /* Priority of the task */
		&dsp_task, /* Task handle. */
		1);
	digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
#ifdef ENABLE_WS
	ws.cleanupClients();
#endif

#ifdef ENABLE_OTA
	ArduinoOTA.handle();
#endif

#ifdef ENABLE_SSE
	if (millis() - last_sse_alive_msg_time > SSE_SEND_ALIVE_MSG_TIME || millis() < last_sse_alive_msg_time) {
		last_sse_alive_msg_time = millis();
		sse_send_alive_msg();
	}
#endif

#ifdef ENABLE_WIFI_AP
#ifdef ENABLE_WIFI_AP_DNS
	dnsServer.processNextRequest();
#endif
#endif

	// #ifdef AUDIO_CAPTURE_SERVER_ENABLED
	// 	if(millis()-last_audio_capture_conn_check_time > 3000){
	// 		last_audio_capture_conn_check_time = millis();
	// 		if(!audio_capture_server_client.connected()){
	// 			if (!audio_capture_server_client.connect(AUDIO_CAPTURE_SERVER_IP, AUDIO_CAPTURE_SERVER_PORT)) {
	// 				Serial.println("Connection failed.");
	// 			}
	// 		}
	// 	}
	// #endif
}