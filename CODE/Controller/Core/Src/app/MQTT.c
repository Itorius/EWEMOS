#include "lwesp/apps/lwesp_mqtt_client.h"
#include "mqtt_message.h"
#include "CAN.h"
#include "MQTT.h"


//#define MQTT_SERVER "192.168.1.7"
#define MQTT_SERVER "itorius.com"
#define MQTT_PORT 1883

volatile lwesp_mqtt_client_p mqtt_client;

char TOPIC_NAME[16 + 1 + 4 + 1] = "XXXXXXXXXXXXXXXX/name";
char TOPIC_CONFIG[16 + 1 + 6 + 1] = "XXXXXXXXXXXXXXXX/config";
char TOPIC_STATE[16 + 1 + 5 + 1] = "XXXXXXXXXXXXXXXX/state";
char TOPIC_DATA[16 + 1 + 4 + 1] = "XXXXXXXXXXXXXXXX/data";

static void mqtt_connect_callback(lwesp_mqtt_client_p client, lwesp_mqtt_evt_t* evt);

extern char ID[16];
char id[17];

extern struct DeviceConfig
{
	char Name[64];
} Config __attribute__((aligned(4)));

static lwesp_mqtt_client_info_t mqtt_client_info = {
	.id = "EWEMOS",
	.user="test",
	.pass="test",
	.keep_alive = 15
};

extern void can_init();

void mqtt_init()
{
	mqtt_client = lwesp_mqtt_client_new(256, 128);;

	strcpy(id, ID);
	id[16] = '\0';
	mqtt_client_info.id = id;

	memcpy(TOPIC_NAME, ID, 16);
	memcpy(TOPIC_CONFIG, ID, 16);
	memcpy(TOPIC_STATE, ID, 16);
	memcpy(TOPIC_DATA, ID, 16);

	// bug: shouldnt this be repeated until success?
	lwesp_mqtt_client_connect(mqtt_client, MQTT_SERVER, MQTT_PORT, mqtt_connect_callback, &mqtt_client_info);
}

extern void EEPROM_Write(uint16_t address, uint8_t* data, uint16_t size);

static void mqtt_connect_callback(lwesp_mqtt_client_p client, lwesp_mqtt_evt_t* evt)
{
	lwesp_mqtt_evt_type_t evt_type = lwesp_mqtt_client_evt_get_type(client, evt);
	switch (evt_type)
	{
		case LWESP_MQTT_EVT_CONNECT:
		{
			lwesp_mqtt_conn_status_t status = lwesp_mqtt_client_evt_connect_get_status(client, evt);

			if (status == LWESP_MQTT_CONN_STATUS_ACCEPTED)
			{
				printf("MQTT accepted!\n");


				MQTT_SendName(Config.Name);

				lwesp_mqtt_client_subscribe(client, TOPIC_NAME, LWESP_MQTT_QOS_EXACTLY_ONCE, TOPIC_NAME);
				lwesp_mqtt_client_subscribe(client, TOPIC_CONFIG, LWESP_MQTT_QOS_EXACTLY_ONCE, TOPIC_CONFIG);

				can_init();
			}
			else
			{
				// note: spits out this and doesnt try to reconnect
				printf("MQTT server connection was not successful: %d\n", (int)status);
			}
			break;
		}
		case LWESP_MQTT_EVT_SUBSCRIBE:
		{
			const char* arg = lwesp_mqtt_client_evt_subscribe_get_argument(client, evt);
			lwespr_t res = lwesp_mqtt_client_evt_subscribe_get_result(client, evt);

			if (res == lwespOK)
			{
				printf("Subscribed to topic '%s'\n", arg);
			}
			break;
		}
		case LWESP_MQTT_EVT_PUBLISH_RECV:
		{
			const char* topic = lwesp_mqtt_client_evt_publish_recv_get_topic(client, evt);
			size_t topic_len = lwesp_mqtt_client_evt_publish_recv_get_topic_len(client, evt);
			const uint8_t* payload = lwesp_mqtt_client_evt_publish_recv_get_payload(client, evt);
			size_t payload_len = lwesp_mqtt_client_evt_publish_recv_get_payload_len(client, evt);

			if (strncmp(topic, TOPIC_NAME, topic_len) == 0)
			{
				MqttMessageName mqttMessage;
				memcpy(&mqttMessage, payload, payload_len);

				if (!mqttMessage.ConfigSet) break;

				memcpy(Config.Name, mqttMessage.Name, 64);

				EEPROM_Write(0, (uint8_t*)&Config, 64);

				printf("Received new controller name '%s'\n", mqttMessage.Name);

				MQTT_SendName(Config.Name);
			}
			else if (topic_len == strlen(TOPIC_CONFIG) && strncmp(topic, TOPIC_CONFIG, topic_len) == 0)
			{
				MqttMessageConfig mqttMessage;
				memcpy(&mqttMessage, payload, payload_len);

				if (!mqttMessage.ConfigSet) break;

				CAN_SendSensorConfig(mqttMessage);

				printf("Received new sensor config '%s' '%lu ms'\n", mqttMessage.Name, mqttMessage.Interval);
			}

			break;
		}
		case LWESP_MQTT_EVT_PUBLISH:
		{
			const char* arg = lwesp_mqtt_client_evt_publish_get_argument(client, evt);

			printf("Publish event, user argument on message was: %s\n", arg);
			break;
		}
		case LWESP_MQTT_EVT_DISCONNECT: // todo: reconnect here
		{
			printf("MQTT client disconnected! Reconnecting in 10 seconds.\n");
			osDelay(10000);
			lwesp_mqtt_client_connect(mqtt_client, MQTT_SERVER, MQTT_PORT, mqtt_connect_callback, &mqtt_client_info);
			break;
		}
		default:
			break;
	}
}

void MQTT_SendStatusChange(const Device* device, ConStatus status)
{
	if (mqtt_client == NULL || !lwesp_mqtt_client_is_connected(mqtt_client))
		return;

	if (device->status == status)
		return;

	MqttMessageState message = {
		.ID = device->ID,
		.State = status
	};

	lwesp_mqtt_client_publish(mqtt_client, TOPIC_STATE, &message, sizeof(message), LWESP_MQTT_QOS_AT_MOST_ONCE, 0, 0);
}

void MQTT_SendData(const Device* device, void* data)
{
	if (mqtt_client == NULL || !lwesp_mqtt_client_is_connected(mqtt_client))
		return;

	MqttMessageData message = {
		.ID = device->ID,
	};

	memcpy(message.Data, data, sizeof(data));

	// NOTE: dont send all 56 bytes
	lwesp_mqtt_client_publish(mqtt_client, TOPIC_DATA, &message, sizeof(message), LWESP_MQTT_QOS_AT_MOST_ONCE, 0, 0);
}

void MQTT_SendName(char* name)
{
	if (mqtt_client == NULL || !lwesp_mqtt_client_is_connected(mqtt_client))
		return;

	MqttMessageName mqtt_message;
	memcpy(mqtt_message.Name, &Config.Name, 64);

	lwesp_mqtt_client_publish(mqtt_client, TOPIC_NAME, &mqtt_message, sizeof(mqtt_message), LWESP_MQTT_QOS_EXACTLY_ONCE, 0, TOPIC_NAME);
}

void MQTT_SendConfig(const Device* device)
{
	if (mqtt_client == NULL || !lwesp_mqtt_client_is_connected(mqtt_client))
		return;

	MqttMessageConfig message = {
		.ID = device->ID,
		.Type = device->config.Type,
		.Interval = device->config.Interval
	};
	memcpy(message.Name, (*device).config.Name, 32);

	lwesp_mqtt_client_publish(mqtt_client, TOPIC_CONFIG, &message, sizeof(message), LWESP_MQTT_QOS_EXACTLY_ONCE, 0, 0);
}