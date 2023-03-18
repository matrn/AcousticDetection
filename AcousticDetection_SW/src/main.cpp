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


//#define ENABLE_WS    // WebSockets
#define ENABLE_SSE   // Server Sent Events
#define ENABLE_SSE_CORS_HEADER

// #define AUDIO_CAPTURE_SERVER_ENABLED

#ifdef AUDIO_CAPTURE_SERVER_ENABLED
	#define AUDIO_CAPTURE_SERVER_URL "http://192.168.0.100:5005/i2s_samples"  //"http://192.168.26.92:5005/i2s_samples"
	// #define AUDIO_CAPTURE_SERVER_URL "http://10.42.0.1:5005/i2s_samples"
	#define SAMPLES_PER_REQ 1024 * 50
	#include <HTTPClient.h>
	unsigned long last_audio_capture_conn_check_time = 0;
	WiFiClient wifiClient;
	HTTPClient httpClient;
	// WiFiClient audio_capture_server_client;
	// CircularBuffer<uint8_t *, 2> data_for_send;
	uint8_t *data_for_send;
#endif

// #define ADC_INPUT1 ADC1_CHANNEL_4	 // pin 32
// #define ADC_INPUT2 ADC1_CHANNEL_5	 // pin 33

// #define SAMPLES_NUM 512

#define CORR_SIZE correlation_window_samples_num
CircularBuffer<audio_sample_t, CORR_SIZE> x1;
CircularBuffer<audio_sample_t, CORR_SIZE> x2;

// ADC adc1(ADC_INPUT1, I2S_NUM_0);
// ADC adc2(ADC_INPUT2, I2S_NUM_1);

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
#endif

OnsetDetector od1;
OnsetDetector od2;


void send_angle(int angle){
	char str[10];
	sprintf(str, "%d", angle);
	#ifdef ENABLE_WS
		ws.textAll(str);
	#endif

	#ifdef ENABLE_SSE
		events.send(str, "angle");
	#endif
}


void dsp_func(void *param) {
	Serial.printf("DSP running on core: %d\n", xPortGetCoreID());
	LRMics *sampler = (LRMics *)(param);

	int qq = 0;
	bool capture_started = false;
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

			if (od1.detect(left) && od2.detect(right) && capture_started == false) {
				Serial.println("Capture started");
				capture_started = true;
			}

			if (capture_started) {
				x1.push(left);
				x2.push(right);
			}
			// if(qq++ < 512) continue;
			// qq = 0;

			if (capture_started && ++qq >= CORR_SIZE) {
				Serial.println("START PROCESSING");
				qq = 0;
				capture_started = false;

				std::pair<int, double> xcorr_peak;
				bool xcorr_peak_found = dsp.xcorr_max<CircularBuffer<audio_sample_t, CORR_SIZE> >(x1, x2, xcorr_peak, CORR_SIZE, max_shift_samples_num + 1, true);

				Serial.println(xcorr_peak_found);
				if (xcorr_peak_found) {
					digitalWrite(LED_BUILTIN, HIGH);

					double tau = xcorr_peak.first * (1. / I2S_SAMPLE_RATE);
					if (tau >= travel_time_for_max_angle)
						Serial.printf("XCORR - shift too big: N: %d, max: %f, tau: %f s\n", xcorr_peak.first, xcorr_peak.second, tau);
					else {
						int angle = dsp.rad2deg(dsp.calculate_angle(tau)) + 0.5;
						
						Serial.printf("N: %d, max: %f, tau: %f s, angle: %d\n", xcorr_peak.first, xcorr_peak.second, tau, angle);

						send_angle(angle);
					}
					digitalWrite(LED_BUILTIN, LOW);
				}
			}
		}
		// ws.textAll("AA");
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
	Serial.setDebugOutput(true);
	Serial.println("ESP32 Acoustic Detection");
	Serial.printf("xcorr window size: %d\n", CORR_SIZE);
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

	SPIFFS.begin();
	
	


	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});

	server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

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

	#ifdef ENABLE_WS
		ws.onEvent(onWsEvent);
		server.addHandler(&ws);
	#endif

	#ifdef ENABLE_SSE
		#ifdef ENABLE_SSE_CORS_HEADER
			DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
		#endif
		events.onConnect([](AsyncEventSourceClient *client){
			// if(client->lastId()){
			// 	Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
			// }
			// //send event with message "hello!", id current millis
			// // and set reconnect delay to 1 second
			client->send("hello!",NULL,millis(),1000);
		});
		server.addHandler(&events);
	#endif

	// server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
	// 	if (!index)
	// 		Serial.printf("UploadStart: %s\n", filename.c_str());
	// 	Serial.printf("%s", (const char *)data);
	// 	if (final)
	// 		Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
	// });
	// server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
	// 	if (!index)
	// 		Serial.printf("BodyStart: %u\n", total);
	// 	Serial.printf("%s", (const char *)data);
	// 	if (index + len == total)
	// 		Serial.printf("BodyEnd: %u\n", total);
	// });
	server.begin();

	// #ifdef AUDIO_CAPTURE_SERVER_ENABLED
	// 	if (!audio_capture_server_client.connect(AUDIO_CAPTURE_SERVER_IP, AUDIO_CAPTURE_SERVER_PORT)) {
	//     	Serial.println("Connection failed.");
	// 	}
	// #endif
	// int count = 2;
	// mics.left_channel_data[0] = 5895;
	// mics.left_channel_data[1] = -5555;
	// mics.right_channel_data[0] = -4206;
	// mics.right_channel_data[1] = 69;
	// httpClient.begin(wifiClient, AUDIO_CAPTURE_SERVER_URL);
	// httpClient.addHeader("content-type", "application/octet-stream");
	// uint8_t data[count*sizeof(int16_t)*2];
	// memcpy(data, mics.left_channel_data, count*sizeof(int16_t));
	// memcpy(data+count*sizeof(int16_t), mics.right_channel_data, count*sizeof(int16_t));
	// for(int i = 0; i < count*2; i ++){
	// 	Serial.printf("%d, ", ((int16_t*)data)[i]);
	// }
	// Serial.println();
	// httpClient.POST(data, count*2*sizeof(int16_t));
	// httpClient.end();
	// while(1);

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
	// vTaskDelay()

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