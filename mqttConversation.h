#ifndef MQTT_CONVERSATION_H
#define MQTT_CONVERSATION_H

#include <Arduino.h>
#include <PubSubClient.h>

#define MQTT_PORT 1883

#ifdef ESP32
    #include "WiFi.h"
#elif defined(ESP8266)
    #include <ESP8266WiFi.h>
#endif

typedef void (* MQTTPayloadParserCallBack)(const String & topic, const String & package);

/**
 * @brief MQTT configuration type
 * 
 */
struct MQTTConfigs{
    String serverName;
    uint16_t port;
    String id;
    String userName;
    String password;

    /**
     * @brief Construct a new MQTTConfigs object.
     * 
     * @param _serverName 
     * @param _port 
     * @param _id 
     * @param _userName 
     * @param _password 
     */
    MQTTConfigs(const String & _serverName, const uint16_t & _port, const String & _id, 
                const String & _userName, const String & _password){
                    serverName = _serverName;
                    port = _port;
                    id = _id;
                    userName = _userName;
                    password = _password;
                }
    /**
     * @brief Construct a new MQTTConfigs object with empty values.
     * 
     */
    MQTTConfigs(){
                    serverName = "";
                    port = MQTT_PORT;
                    id = "";
                    userName = "";
                    password = "";
    }
};


void mqttSetup(MQTTConfigs _mqttConfig, MQTTPayloadParserCallBack mqttPayloadParserCallBack = NULL, Print* _ptrLog = NULL);
bool mqttloop();
bool mqttPublish(const String& topic, const String& value);
bool mqttSubscribe(const String& topic);
bool mqttConnected();



#endif