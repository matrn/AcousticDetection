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




#define AUDIO_CAPTURE_SERVER_ENABLED

#ifdef AUDIO_CAPTURE_SERVER_ENABLED
	#define AUDIO_CAPTURE_SERVER_URL "http://192.168.0.100:8888/i2s_data_stereo"
	#define SAMPLES_PER_REQ 20480
	#include <HTTPClient.h>
	unsigned long last_audio_capture_conn_check_time = 0;
	WiFiClient wifiClient;
	HTTPClient httpClient;
	WiFiClient audio_capture_server_client;
#endif

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
		

		#ifdef AUDIO_CAPTURE_SERVER_ENABLED
			// int sampled = 0;
			// while(sampled < SAMPLES_PER_REQ){
				int count = sampler->read(true); //[](audio_sample_t left, audio_sample_t right){
				//sampled += count;
				
			// Serial.println("-------LEFT-------");
			// for(int i = 0; i < count; i ++) Serial.printf("%d,", sampler->left_channel_data[i]);
			// Serial.println("------------------");

			// Serial.println("-------RIGHT-------");
			// for(int i = 0; i < count; i ++) Serial.printf("%d,", sampler->right_channel_data[i]);
			// Serial.println("-------------------");
			//if(audio_capture_server_client.connected()){
				digitalWrite(LED_BUILTIN, HIGH);
				httpClient.begin(wifiClient, AUDIO_CAPTURE_SERVER_URL);
				httpClient.addHeader("content-type", "application/octet-stream");
				uint8_t data[count*sizeof(int16_t)*2];
				memcpy(data, sampler->left_channel_data, count*sizeof(int16_t));
				memcpy(data+count*sizeof(int16_t), sampler->right_channel_data, count*sizeof(int16_t));

				// for(int i = 0; i < count*2; i ++){
				// 	Serial.printf("%d, ", ((int16_t*)data)[i]);
				// }
				// Serial.println();
				httpClient.POST(data, count*2*sizeof(int16_t));
				httpClient.end();
				digitalWrite(LED_BUILTIN, LOW);
				// for(int i = 0; i < count; i ++){
				// 	audio_sample_t left = sampler->left_channel_data[i];
				// 	audio_sample_t right = sampler->right_channel_data[i];

				// 	audio_capture_server_client.write(sampler->left_channel_data, count*sizeof(audio_sample_t));
				// 	audio_capture_server_client.write(sampler->right_channel_data, count*sizeof(audio_sample_t));
			//}
		
		#else
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
		#endif
	}
}



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
		AwsFrameInfo *info = (AwsFrameInfo *)arg;
		String msg = "";
		if (info->final && info->index == 0 && info->len == len) {
			// the whole message is in a single frame and we got all of it's data
			Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

			if (info->opcode == WS_TEXT) {
				for (size_t i = 0; i < info->len; i++) {
					msg += (char)data[i];
				}
			} else {
				char buff[3];
				for (size_t i = 0; i < info->len; i++) {
					sprintf(buff, "%02x ", (uint8_t)data[i]);
					msg += buff;
				}
			}
			Serial.printf("%s\n", msg.c_str());

			if (info->opcode == WS_TEXT)
				client->text("I got your text message");
			else
				client->binary("I got your binary message");
		} else {
			// message is comprised of multiple frames or the frame is split into multiple packets
			if (info->index == 0) {
				if (info->num == 0)
					Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
				Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
			}

			Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

			if (info->opcode == WS_TEXT) {
				for (size_t i = 0; i < len; i++) {
					msg += (char)data[i];
				}
			} else {
				char buff[3];
				for (size_t i = 0; i < len; i++) {
					sprintf(buff, "%02x ", (uint8_t)data[i]);
					msg += buff;
				}
			}
			Serial.printf("%s\n", msg.c_str());

			if ((info->index + len) == info->len) {
				Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
				if (info->final) {
					Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
					if (info->message_opcode == WS_TEXT)
						client->text("I got your text message");
					else
						client->binary("I got your binary message");
				}
			}
		}
	}
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
	ws.cleanupClients();
	//vTaskDelay()

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