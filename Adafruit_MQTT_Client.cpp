#include "Adafruit_MQTT_Client.h"


bool Adafruit_MQTT_Client::connectServer() {
  // Grab server name from flash and copy to buffer for name resolution.
  memset(buffer, 0, sizeof(buffer));
  strcpy_P((char *)buffer, servername);
  DEBUG_PRINT(F("Connecting to: ")); DEBUG_PRINTLN((char *)buffer);
  // Connect and check for success (0 result).
  int r = client->connect((char *)buffer, portnum);
  DEBUG_PRINT(F("Connect result: ")); DEBUG_PRINTLN(r);
  return r != 0;
}

bool Adafruit_MQTT_Client::disconnect() {
  // Stop connection and return success (stop has no indication of failure).
  client->stop();
  return true;  
}

uint16_t Adafruit_MQTT_Client::readPacket(uint8_t *buffer, uint8_t maxlen,
                                          int16_t timeout, 
                                          bool checkForValidPubPacket) {
  /* Read data until either the connection is closed, or the idle timeout is reached. */
  uint16_t len = 0;
  int16_t t = timeout;

  while (client->connected() && (timeout >= 0)) {
    //DEBUG_PRINT('.');
    while (client->available()) {
      //DEBUG_PRINT('!');
      char c = client->read();
      timeout = t;  // reset the timeout
      buffer[len] = c;
      //DEBUG_PRINTLN((uint8_t)c, HEX);
      len++;
      if (len == maxlen) {  // we read all we want, bail
        DEBUG_PRINT(F("Read packet:\t"));
        DEBUG_PRINTBUFFER(buffer, len);
        return len;
      }

      // special case where we just one one publication packet at a time
      if (checkForValidPubPacket) {
        if ((buffer[0] == (MQTT_CTRL_PUBLISH << 4)) && (buffer[1] == len-2)) {
          // oooh a valid publish packet!
          DEBUG_PRINT(F("Read PUBLISH packet:\t"));
          DEBUG_PRINTBUFFER(buffer, len);
          return len;
        }
      }
    }
    timeout -= MQTT_CLIENT_READINTERVAL_MS;
    delay(MQTT_CLIENT_READINTERVAL_MS);
  }
  return len;
}

bool Adafruit_MQTT_Client::sendPacket(uint8_t *buffer, uint8_t len) {
  if (client->connected()) {
    uint16_t ret = client->write(buffer, len);
    DEBUG_PRINT(F("sendPacket returned: ")); DEBUG_PRINTLN(ret);
    if (ret != len) {
      DEBUG_PRINTLN("Failed to send complete packet.")
      return false;
    }
  } else {
    DEBUG_PRINTLN(F("Connection failed!"));
    return false;
  }
  return true;
}
