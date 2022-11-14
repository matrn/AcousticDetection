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
#include "../lib/i2s_mic/mic.hpp"
#include "../lib/dsp/dsp.hpp"


// ----------SETTINGS----------
const double sound_speed = 343;		 // [m/s]
const double mics_distance = 0.145;	 // [m]  = distance between microphones in meters
// ----------SETTINGS----------

const double Ts = 1. / I2S_SAMPLE_RATE;	 // sample time

const double travel_time_for_max_angle = mics_distance / sound_speed;	// how long does it take for sound wave to travel from one microphone to the other

const double correlation_window_time = travel_time_for_max_angle * 50.0;					// 140% of maximum time difference between audio signals
const int correlation_window_samples_num = correlation_window_time / Ts + 0.5;	// how many samples should we take for cross correlation computation

const int max_shift_samples_num = travel_time_for_max_angle/Ts+0.99;   // number of samples needed for maximum time, 0.99 = round up
// #define ADC_INPUT1 ADC1_CHANNEL_4	 // pin 32
// #define ADC_INPUT2 ADC1_CHANNEL_5	 // pin 33



// #define SAMPLES_NUM 512

CircularBuffer<audio_sample_t, correlation_window_samples_num> x1;
CircularBuffer<audio_sample_t, correlation_window_samples_num> x2;

// ADC adc1(ADC_INPUT1, I2S_NUM_0);
// ADC adc2(ADC_INPUT2, I2S_NUM_1);

LRMics mics(I2S_NUM_0);

TaskHandle_t dsp_task;

DSP dsp;

// SKETCH BEGIN
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void dsp_func(void *param) {
	Serial.printf("DSP running on core: %d\n", xPortGetCoreID());
	LRMics *sampler = (LRMics *)(param); 

	while (true) {
		//Serial.println("f");
		// mics.read_and_print();
		// continue;
		// 
		// delay(10000);
		int count = sampler->read(); //[](audio_sample_t left, audio_sample_t right){
		for(int i = 0; i < count; i ++){
			audio_sample_t left = sampler->left_channel_data[i];
			audio_sample_t right = sampler->right_channel_data[i];

			x1.push(left);
			x2.push(right);

			//if(count%100 != 0) continue;
			int N = -1;

			auto corr1 = dsp.correlation<correlation_window_samples_num>(x1, x2, correlation_window_samples_num, max_shift_samples_num+1);
			auto corr2 = dsp.correlation<correlation_window_samples_num>(x2, x1, correlation_window_samples_num, max_shift_samples_num+1);
			
			if(corr1.first != -1 && corr2.first != -1){
				if(corr1.second > corr2.second) N = corr1.first;
				else N = -corr2.first;
			}
			else if(corr1.first != -1) N = corr1.first;
			else if(corr2.first != -1) N = corr2.first;
			if(N != -1 && N != 0){
				digitalWrite(LED_BUILTIN, HIGH);
				
				double tau = N*(1./I2S_SAMPLE_RATE);
				int angle = dsp.rad2deg( acos((tau*sound_speed)/mics_distance) )+0.5;
				char str[10];
				
				sprintf(str, "%d", angle);
				Serial.printf("%d, %f s, %d\n", N, tau, angle);
				ws.textAll(str);
				digitalWrite(LED_BUILTIN, LOW);
			}
		}
		//ws.textAll("AA");
	}
}



void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
	
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	Serial.println("ESP32 Acoustic Detection");
	Serial.printf("xcorr window size: %d\n", correlation_window_samples_num);
	Serial.printf("setup running on core: %d\n", xPortGetCoreID());
	mics.init();
	// adc2.init();

	// auto start = micros();
	// for(int i = 0; i < 100; i ++){
	// 	correlation(x1, x2, correlation_window_samples_num, max_shift_samples_num);
	// }
	// auto end = micros();
	// Serial.printf("Correlation time: %f us\n", (end-start)/100.);
	// delay(10000);
	// while(1);
	   /* Core where the task should run */

	Serial.printf("Connecting to: %s\n", wifi_ssid);
	WiFi.mode(WIFI_STA);
	WiFi.begin(wifi_ssid, wifi_password);
	for(int i = 0; i < 3; i ++){
		if(WiFi.waitForConnectResult() == WL_CONNECTED) break;
		Serial.print(".");		
	}
	if(WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Connection Failed! Rebooting...");
		delay(1000);
		ESP.restart();
	}
	WiFi.hostname(hostname);
	Serial.printf("Connected, ip: %s\n", WiFi.localIP().toString().c_str());

	MDNS.addService("http", "tcp", 80);

	SPIFFS.begin();

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);

	events.onConnect([](AsyncEventSourceClient *client) {
		client->send("hello!", NULL, millis(), 1000);
	});
	server.addHandler(&events);

	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});

	server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

	server.onNotFound([](AsyncWebServerRequest *request) {
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

		request->send(404);
	});
	server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
		if (!index)
			Serial.printf("UploadStart: %s\n", filename.c_str());
		Serial.printf("%s", (const char *)data);
		if (final)
			Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
	});
	server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
		if (!index)
			Serial.printf("BodyStart: %u\n", total);
		Serial.printf("%s", (const char *)data);
		if (index + len == total)
			Serial.printf("BodyEnd: %u\n", total);
	});
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
	ws.cleanupClients();
	//vTaskDelay()
}