#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
String topicString = "Křemíkové zátiší/Shockleyův park";

void mqttPublish(String topic, String message)
{
	if (client.connected())
	{
		topic = topicString + topic;
		int n = topic.length();
		char topic_array[n + 1];
		strcpy(topic_array, topic.c_str());
		n = message.length();
		char message_array[n + 1];
		strcpy(message_array, message.c_str());
		for (uint8_t i = 0; i < 3; i++)
		{
			if (xSemaphoreTake(mqtt_mutex, 200))
			{
				client.publish(topic_array, message_array);
				xSemaphoreGive(mqtt_mutex);
				return;
			}
			taskYIELD();
		}
	}
}

void callback(char *topic, byte *payload, unsigned int length)
{
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	String recieved = "";
	for (int i = 0; i < length; i++)
	{
		recieved += (char)payload[i];
	}
	Serial.println(recieved);
	if (recieved == "true")
	{
		disco_mod = true;
	}
	else if (recieved == "false")
	{
		disco_mod = false;
	}
}

void reconnect()
{
	// Loop until we're reconnected
	while (!client.connected())
	{
		Serial.print("připojení k MQTT...");
		// Create a random client ID
		String clientId = "ChytreLampy";
		clientId += String(random(0xffff), HEX);
		// Attempt to connect
		if (client.connect(clientId.c_str()))
		{
			Serial.println("připojeno");
			String topicSubscribe = topicString + "/lamp/DISCO";
			int n = topicSubscribe.length();
			char topic_array[n + 1];
			strcpy(topic_array, topicSubscribe.c_str());
			client.subscribe(topic_array);
		}
		else
		{
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void mqtt(void *parameters)
{
	client.setServer("10.10.1.13", 1883);
	//client.setServer("10.10.10.11", 1883);
	//client.setServer("192.168.137.1", 1883);
	client.setCallback(callback);

	while (true)
	{
		if (xSemaphoreTake(mqtt_mutex, 100))
		{
			if (!client.connected())
			{
				reconnect();
			}
			client.loop();
			xSemaphoreGive(mqtt_mutex);
			delay(10);
		}
	}
}