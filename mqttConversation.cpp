#include "mqttConversation.h"


WiFiClient espClient;
PubSubClient pubSubClient(espClient);
Print* ptrLog = NULL;
MQTTConfigs mqttConfig;
MQTTPayloadParserCallBack mqttOuterCallBack = NULL;

String * topicList = NULL;
uint8_t topicListCount = 0;

bool mqttReconnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
bool subscribe(const String& topic);
void reSubscribe();


void pushTopicList(const String & topic){
    topicListCount++;
    if(topicList != NULL){
            String * tmp = new String[topicListCount];
        for(uint8_t i = 0; i < topicListCount - 1; i++){
            tmp[i] = topicList[i];
        }
        tmp[topicListCount - 1] = topic;
        delete[] topicList;
        topicList = tmp;
    } else {
        topicList = new String[topicListCount];
        topicList[0] = topic;
    }
}

/**
 * Sets UP a MQTT connection. 
 * @param _ptrConfing refrence to global config object with const type use
 * @param mqttPayloadParserCallBack pointer to outer callback for mqtt package parse
 * @param _ptrLog pointer to stream for loging
 */
void mqttSetup(MQTTConfigs _mqttConfig, MQTTPayloadParserCallBack mqttPayloadParserCallBack, Print* _ptrLog) {
    mqttOuterCallBack = mqttPayloadParserCallBack;
    ptrLog = _ptrLog;
    mqttConfig = _mqttConfig;
    if (mqttConfig.serverName != "") {
        pubSubClient.setServer(mqttConfig.serverName.c_str(), mqttConfig.port);
        pubSubClient.setCallback(mqttCallback);
    }
}

/**
 * Loop function, need to set in main loop()
 * @return a state of connection to MQTT server. true - is connected
 */
bool mqttloop() {
    if(!pubSubClient.loop()){
        return mqttReconnect();
    } else {
        return false;
    }
    return true;
}

/**
 * reconnect function to MQTT server. It call then connection lost
 * @return a state of connection to MQTT server. true - is connected
 */
bool mqttReconnect() {
    const uint32_t timeout = 30000;
    static uint32_t nextTime;
    bool result = false;

    if ((int32_t)(millis() - nextTime) >= 0) {
        ptrLog->print(F("Attempting MQTT connection..."));

        if (mqttConfig.userName != "")
            result = pubSubClient.connect(mqttConfig.id.c_str(), mqttConfig.userName.c_str(), mqttConfig.password.c_str());
        else
            result = pubSubClient.connect(mqttConfig.id.c_str());
        
        if (result) {
            if(ptrLog != NULL)
                ptrLog->println(F(" connected"));
            reSubscribe();
            
        } else if(ptrLog != NULL) {
            ptrLog->print(F(" failed, rc="));
            ptrLog->println(pubSubClient.state());
        }
        nextTime = millis() + timeout;
    }
    return result;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if(ptrLog != NULL){
        ptrLog->print(F("MQTT message arrived ["));
        ptrLog->print(topic);
        ptrLog->print(F("] "));
    }
    String package = "";
    for (int i = 0; i < length; ++i) {
        package += (char)payload[i];
    }
    ptrLog->println(package);
    if(mqttOuterCallBack != NULL && length)
        mqttOuterCallBack(topic, package);
}

bool mqttSubscribe(const String& topic) {
    pushTopicList(topic);
    return subscribe(topic);
}

bool subscribe(const String& topic) {
    if(ptrLog != NULL){
        ptrLog->print(F("MQTT subscribe to topic \""));
        ptrLog->print(topic);
        ptrLog->println('\"');
    }
    if(pubSubClient.subscribe(topic.c_str())){
        if(ptrLog != NULL)
            ptrLog->println(F("succes"));
        return true;
    } else {
        if(ptrLog != NULL)
            ptrLog->println(F("faild"));
        return false;
    }
}

void reSubscribe(){
    if(pubSubClient.connected()){
        for(uint8_t i = 0; i < topicListCount; i++){
            subscribe(topicList[i]);
        }
    }
}

bool mqttPublish(const String& topic, const String& value) {
    if(pubSubClient.connected() && pubSubClient.publish(topic.c_str(), value.c_str())){
        ptrLog->print(F("MQTT publish topic \""));
        ptrLog->print(topic);
        ptrLog->print(F("\" with value \""));
        ptrLog->print(value);
        ptrLog->println('\"');
        return true;
    }
    return false;
}

bool mqttConnected(){
    return pubSubClient.connected();
}
