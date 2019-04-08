#define MAX_SRV_CLIENTS 10
WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

SemaphoreHandle_t telnet_mutex = xSemaphoreCreateMutex();

void writeStringTelnetln(String s)
{
  while (true)
  {
    if (xSemaphoreTake(telnet_mutex, 20) == pdTRUE)
    {
      Serial.println(s);
      for (int i = 0; i < MAX_SRV_CLIENTS; i++)
      {
        if (serverClients[i] && serverClients[i].connected())
        {
          serverClients[i].println(s);
        }
      }
      xSemaphoreGive(telnet_mutex);
      break;
    }
  }
}

void writeStringTelnet(String s)
{
  while (true)
  {
    if (xSemaphoreTake(telnet_mutex, 20) == pdTRUE)
    {
      Serial.print(s);
      for (int i = 0; i < MAX_SRV_CLIENTS; i++)
      {
        if (serverClients[i] && serverClients[i].connected())
        {
          serverClients[i].print(s);
        }
      }
      xSemaphoreGive(telnet_mutex);
      break;
    }
  }
}

void serverHandle(void *parameters)
{
  server.begin();
  server.setNoDelay(true);
  while (true)
  {
    uint8_t i;
    if (xSemaphoreTake(telnet_mutex, 40) == pdTRUE)
    {
      if (WiFi.status() == WL_CONNECTED || WiFi.status() == WL_NO_SHIELD)
      {
        if (server.hasClient())
        {
          for (i = 0; i < MAX_SRV_CLIENTS; i++)
          {
            if (!serverClients[i] || !serverClients[i].connected())
            {
              if (serverClients[i])
                serverClients[i].stop();
              serverClients[i] = server.available();
              break;
            }
          }
          if (i >= MAX_SRV_CLIENTS)
          {
            server.available().stop();
          }
        }
        for (i = 0; i < MAX_SRV_CLIENTS; i++)
        {
          if (serverClients[i] && serverClients[i].connected())
          {
            if (serverClients[i].available())
            {
              while (true)
              {
                if (xSemaphoreTake(telnetWrite_mutex, 20))
                {
                  telnetMessage = "";
                  while (serverClients[i].available())
                  {
                    telnetMessage += (char)serverClients[i].read();
                  }
                  telnetMessage.toLowerCase();
                  telnetNum++;
                  telnetCount = lampCount;
                  break;
                }
              }
            }
          }
          else
          {
            if (serverClients[i])
            {
              serverClients[i].stop();
            }
          }
        }
      }
      else
      {
        Serial.println("WiFi not connected!");
        for (i = 0; i < MAX_SRV_CLIENTS; i++)
        {
          if (serverClients[i])
            serverClients[i].stop();
        }
        delay(1000);
      }
      xSemaphoreGive(telnet_mutex);
    }
    taskYIELD();
  }
}