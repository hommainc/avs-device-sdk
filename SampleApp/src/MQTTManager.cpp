/*
 * ConsolePrinter.cpp
 *
 * Copyright (c) 2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#define MQTTCLIENT_QOS2 1
#define DEFAULT_STACK_SIZE -1
#include "SampleApp/MQTTManager.h"
#include "SampleApp/ConsolePrinter.h"
#include <AVSCommon/Utils/Configuration/ConfigurationNode.h>

static const std::string MQTT_CONFIGURATION_ROOT_KEY = "mqtt";
static const std::string DEFAULT_BROKER_ADDRESS = "tcp://192.168.86.26:1883";
const char* CONFIG_KEY_BROKER_ADDRESS = "broker";
static const std::string DEFAULT_MQTT_TOPIC = "/point/device";
const char* CONFIG_KEY_MQTT_TOPIC = "topic";
static const std::string DEFAULT_MQTT_CLIENT_ID = "AlexaLexSubscriber";
const char* CONFIG_KEY_MQTT_CLIENT_ID = "clientid";

#define QOS         1
#define TIMEOUT     10000L


namespace alexaClientSDK {
namespace sampleApp {

MQTTManager::MQTTManager(
        std::shared_ptr<defaultClient::DefaultClient> client, 
        std::shared_ptr<sampleApp::PortAudioMicrophoneWrapper> micWrapper,
        capabilityAgents::aip::AudioProvider tapToTalkAudioProvider) : 
    m_client{client}, 
    m_micWrapper{micWrapper},
    m_tapToTalkAudioProvider{tapToTalkAudioProvider} {

    auto configuration = avsCommon::utils::configuration::ConfigurationNode::getRoot()[MQTT_CONFIGURATION_ROOT_KEY];
    configuration.getString(CONFIG_KEY_BROKER_ADDRESS, &m_brokerAddress, DEFAULT_BROKER_ADDRESS);
    configuration.getString(CONFIG_KEY_MQTT_TOPIC, &m_mqttTopic, DEFAULT_MQTT_TOPIC);
    configuration.getString(CONFIG_KEY_MQTT_CLIENT_ID, &m_mqttclientid, DEFAULT_MQTT_CLIENT_ID);

    ConsolePrinter::simplePrint("MQTT MANAGER IN CONSTRUCTION");
}

void MQTTManager::messageDelivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    //deliveredtoken = dt;
}

int MQTTManager::messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = (char*)message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');

    //m_client->notifyOfTapToTalk(m_tapToTalkAudioProvider).get();

    m_executor.submit(
        [this] () {
            if (m_client->notifyOfTapToTalk(m_tapToTalkAudioProvider).get()) {
                ConsolePrinter::simplePrint("TAP TO TALK ON");
            }
        }
    );

    // m_interactionManager->tap();

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void MQTTManager::connectionLost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);

}


void MQTTManager::connect() {
    ConsolePrinter::simplePrint("MQTT MANAGER IN CONNECT");
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;   

    MQTTClient_create(&m_mqttClient, m_brokerAddress.c_str(), m_mqttclientid.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(m_mqttClient, NULL, connectionLost, messageArrived, messageDelivered);

    int rc;
    if ((rc = MQTTClient_connect(m_mqttClient, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n", m_mqttTopic.c_str(), m_mqttclientid.c_str(), QOS);
    MQTTClient_subscribe(m_mqttClient, m_mqttTopic.c_str(), QOS);
}



} // namespace sampleApp
} // namespace alexaClientSDK

