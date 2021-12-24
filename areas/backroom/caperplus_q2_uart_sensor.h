#include "esphome.h"

#include "esphome.h"

#define bytes_to_uint16(MSB,LSB) (((unsigned int) ((unsigned char) MSB)) & 255)<<8 | (((unsigned char) LSB)&255)

/**
 * Rough documentation
 *
 * This is a rough documentation representing half the communication protocol
 * in the Caperplus Q2 pH/TDS/Temp meter. Specifically, it is the data coming
 * from the onboard MCU to the ESP8266. I haven't figured out how to communicate
 * back to the MCU to make things like the WiFi indicator light turn on.
 *
 * This is also a rough understanding of the protocol as I can't explain many
 * parts of it. I'm sure some of it is a checksum.
 *
 * Everything is in Big Endian
 *
 * General structure:
 *
 * HEADER (4 bytes)
 * ?????? (1 byte)
 * Constant (?) (42 bytes)
 * ?????? (1 byte)
 * pH * 100 (2 bytes) (e.g. 605 / 100 = 6.05pH)
 * ?????? (2 bytes)
 * TDS (2 bytes)
 * Temp in C * 100 (2 bytes) (e.g. 1830 / 100 = 18.3C)
 *
 * So given a packet
 *
 * 00000000:  ffff 003a 0560 0000 0400 0002 0000 0001 0000 0e8e 9001 2c03
 * 00000018:  e800 0001 b801 5100 0003 e807 d00d ac00 0001 0001 0002 1000
 * 00000030:  0002 5d08 5d00 0107 2600 0000 0177
 *
 * 0xffff003A = Header
 * 0x0560 = ?????
 * 0x0000040000020000000100000E8E90012C03E8000001B80151000003
 * E807D00DAC000001000000021000 = ???? Seems constant in every packet
 * 0x00 = ?? Perhaps just a NULL spacer?
 * 0x025D = 2 byte word for pH (605) = 6.05pH
 * 0x085D = ?????
 * 0x0001 = 2 byte word for TDS (1 in this case)
 * 0x0726 = 2 byte word for temp in C (1830) = 18.3C
 */
class CaperPlusQ2UartSensor : public Component, public UARTDevice
{
public:
  CaperPlusQ2UartSensor(UARTComponent *parent) : UARTDevice(parent) {}

  Sensor *temperature_sensor = new Sensor();
  Sensor *ph_sensor = new Sensor();
  Sensor *tds_sensor = new Sensor();

protected:
  float pH = 0.0;
  float temperature = 0.0;
  uint16_t tds = 0;

  void clearSerialBuffer()
  {
    uint16_t availableBytes = available();

    ESP_LOGD("caperplus-q2", "Clearing Serial Buffer");

    if (availableBytes > 0)
    {
      uint8_t b[availableBytes];
      read_array(b, availableBytes);
      delay(100);
    }

  }

  void setup() override
  {
    // This space intentionally left blank
    ESP_LOGD("caperplus-q2", "Initialized Caperplus Q2 UART");
  }

  void loop() override
  {
    uint8_t data[62];
    uint16_t value;

    memset(&data[0], 0x00, 62);

    if (available() <= 61)
    {
      return;
    }

    ESP_LOGD("caperplus-q2", "Available bytes: %d", available());

    read_array(data, 62);

    if ((data[0] == 0xFF) && (data[1] == 0xFF) && (data[2] == 0x00) && (data[3] == 0x3A))
    {
      ESP_LOGD("caperplus-q2", "Found header!");
    }
    else
    {
      return;
    }

    value = bytes_to_uint16(data[49], data[50]);
    pH = value / 100.0;

    tds = bytes_to_uint16(data[53], data[54]);

    value = bytes_to_uint16(data[55], data[56]);
    temperature = value / 100.0;

    ESP_LOGD("caperplus-q2", "pH: %.2f", pH);
    ESP_LOGD("caperplus-q2", "TDS: %d", tds);
    ESP_LOGD("caperplus-q2", "Temp: %.1f", temperature);

    temperature_sensor->publish_state(temperature);
    ph_sensor->publish_state(pH);
    tds_sensor->publish_state(tds);

    clearSerialBuffer();

  }

};
