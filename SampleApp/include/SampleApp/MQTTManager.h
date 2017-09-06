#ifndef ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_MQTT_H_
#define ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_MQTT_H_

#include <memory>
#include <mutex>
#include <string>
#include <DefaultClient/DefaultClient.h>
#include "PortAudioMicrophoneWrapper.h"
#include "UIManager.h"

#include "MQTTClient.h"

namespace alexaClientSDK {
namespace sampleApp {

/**
 * A simple class that prints to the screen.
 */
class MQTTManager {
public:
    /**
     * Constructor.
     */
    MQTTManager(std::shared_ptr<defaultClient::DefaultClient> client, 
        std::shared_ptr<sampleApp::PortAudioMicrophoneWrapper> micWrapper,
        capabilityAgents::aip::AudioProvider tapToTalkAudioProvider);



    void connect();
private:
    MQTTClient m_mqttClient;
    void connectionLost(void *context, char *cause);
    void messageDelivered(void *context, MQTTClient_deliveryToken dt);
    int messageArrived(void *context, char *topicName, int topicLen, MQTTClient_message *message);

    std::string m_brokerAddress;
    std::string m_mqttTopic;
    std::string m_mqttclientid;

    /// The default SDK client.
    std::shared_ptr<defaultClient::DefaultClient> m_client;

    /// The microphone managing object.
    std::shared_ptr<sampleApp::PortAudioMicrophoneWrapper> m_micWrapper;
    capabilityAgents::aip::AudioProvider m_tapToTalkAudioProvider;
    avsCommon::utils::threading::Executor m_executor;
};

} // namespace sampleApp
} // namespace alexaClientSDK

#endif // ALEXA_CLIENT_SDK_SAMPLE_APP_INCLUDE_SAMPLE_APP_MQTT_H_