#include <Arduino.h>
#include <SPIFFS.h>
#include "constCollection.h"
#include "espIoTWiFi.h"
// #include <ESPAsyncWebServer.h>
#include "cfgStore.h"
#include "sensors.h"
#include "realTime.h"
#ifdef USE_MQTT
	#include "mqttConversation.h"
#endif

#include <ArduinoJson.h>

/**
 * Sets up WiFi connection
 * @param _Config pointer to config base type
 */
void startsWiFi(ThermometerConfing  * Config, espIoTWiFi * _espIoTWiFi){
	Config->wifiMode = WIFI_MODE_STA;
	Config->ssid = "Faberlic WH";
	Config->password = "A4B05E35D1";
	Config->mqttServer = "10.15.2.90";
	Config->mqttUser = "fadmin";
	Config->mqttPassword = "Qwer1234";
	// Config->ssid = "AdvPara";
	// Config->password = "50245468";
	// Config->mqttServer = "192.168.0.134";
	// Config->mqttUser = "mdclient";
	// Config->mqttPassword = "Qwer1234";
	Config->writeConfig();
	configWiFi cofgWiFi;
	cofgWiFi.ssid = Config->ssid;
	cofgWiFi.password = Config->password;
	cofgWiFi.wifiMode = Config->wifiMode;
    cofgWiFi.mDNS = Config->mDNS;
    cofgWiFi.ipaddr = Config->ipaddr;
    cofgWiFi.netmask = Config->netmask;
    cofgWiFi.gateway = Config->gateway;
    cofgWiFi.dns1 = Config->dns1;
    cofgWiFi.dns2 = Config->dns2;

	_espIoTWiFi->begin(cofgWiFi, &Serial);
}

#ifdef USE_MQTT
	/**
	 * @brief Callback function for MQTT pakage parse, need to send MQTT conversation module
	 * 		
	 * 
	 * @param topic MQTT topic
	 * @param package received notification
	 */
	void parseMQTTPakage(const String & topic, const String & package){
		Serial.println("Parse topic:"+topic+", package:"+package);
	}
#endif

void setup() {
	Serial.begin(115200);
	Serial.println("Starting");

	sensors.begin();

	thermometerConfing.setupEEPROM(&Serial);
	startsWiFi(&thermometerConfing, &wifiConnection);

	realTimeClock.begin(thermometerConfing.ntpServer1, thermometerConfing.ntpServer2, 
						thermometerConfing.ntpServer3, thermometerConfing.ntpTimeZone, &Serial);
	#ifdef USE_MQTT
		mqttSetup(MQTTConfigs(thermometerConfing.mqttServer, 
							thermometerConfing.mqttPort,
							thermometerConfing.boardId,
							thermometerConfing.mqttUser,
							thermometerConfing.mqttPassword), 
							parseMQTTPakage, &Serial);
	#endif

}

void loop() {
	static uint32_t timerMLS = 0;
	wifiConnection.loop();
	#ifdef USE_MQTT
		static bool b = false;
		if(thermometerConfing.mqttServer != ""){
			if(mqttloop() && !b){
				mqttSubscribe("test1");	
				delay(100);
				mqttSubscribe("test2");	
				delay(100);
				mqttSubscribe("test3");	
				b = true;
			}		
		}
	#endif
	if(timerMLS < millis()){
		#ifdef USE_MQTT
			if(mqttConnected())
				mqttPublish("testT", String(sensors.getT()));
        #endif
		timerMLS = millis() + 20000;
	}
}

