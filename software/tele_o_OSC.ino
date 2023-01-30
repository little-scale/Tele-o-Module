// CS2 should be 32 but it was 34. 34 is only an input

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <SPI.h>

#define BUILTIN_LED 2
#define DAC_CHANNELS 4
#define DAC_STEPS 4096
#define GATE_CHANNELS 2

#define LED1 13
#define LED2 12

char msgbuffer[1024];

float DAC_values[DAC_CHANNELS];
int gate_values[GATE_CHANNELS];

const byte gate_pins[] = { 2, 15 };
const byte cs_pins[] = { 26, 32 };


unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 100;

char ssid[] = "OliveNet New";     // your network SSID (name)
char pass[] = "chummyteapot589";  // your network password

IPAddress local_IP(10, 0, 0, 100);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);    //optional
IPAddress secondaryDNS(8, 8, 4, 4);  //optional

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const IPAddress outIp(10, 40, 10, 14);  // remote IP (not needed for receive)
const unsigned int outPort = 3001;      // remote port (not needed for receive)
const unsigned int localPort = 3000;    // local port to listen for UDP packets (here's where we send the packets)

OSCErrorCode error;

unsigned int ledState = HIGH;  // LOW means led is *on*

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  for (int i = 0; i < sizeof(gate_pins); i++) {
    pinMode(gate_pins[i], OUTPUT);
  };

  for (int i = 0; i < sizeof(cs_pins); i++) {
    pinMode(cs_pins[i], OUTPUT);
  };

  digitalWrite(BUILTIN_LED, LOW);  // turn *on* led

  Serial.begin(115200);
  SPI.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");


  digitalWrite(LED1, HIGH);

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(localPort);
}

void msg_CV(OSCMessage &msg, int addressOffset) {
  int DAC_ch;
  float CV_val;
  int DAC_raw;

  msg.getAddress(msgbuffer, 0);                // Get address
  DAC_ch = msgbuffer[addressOffset + 1] - 48;  // Get first element after intiial address match

  if (DAC_ch >= 0 && DAC_ch < DAC_CHANNELS) {         // Is the address range within the number of DACs?
    if (msg.isFloat(0)) {                             // Is the message a float?
      CV_val = constrain(msg.getFloat(0), 0.0, 1.0);  // Constrain to normalised value
      if (DAC_values[DAC_ch] != CV_val) {             // Has the value changed?
        digitalWrite(LED2, HIGH);
        previousMillis = millis();
        DAC_values[DAC_ch] = CV_val;
        DAC_raw = CV_val * (float)(DAC_STEPS - 1.0);
        writeDAC(DAC_ch, DAC_raw);
      }
    }
  }
}

void msg_GATE(OSCMessage &msg, int addressOffset) {
  int GATE_ch;
  int GATE_val;

  msg.getAddress(msgbuffer, 0);                 // Get address
  GATE_ch = msgbuffer[addressOffset + 1] - 48;  // Get first element after intiial address match

  if (GATE_ch >= 0 && GATE_ch < GATE_CHANNELS) {  // Is the address range within the number of gates?
    if (msg.isInt(0)) {                           // Is the message a int?
      GATE_val = msg.getInt(0);
      if (gate_values[GATE_ch] != GATE_val) {  // Has the value changed?
        digitalWrite(LED2, HIGH);
        previousMillis = millis();
        gate_values[GATE_ch] = GATE_val;
        writegate(GATE_ch, GATE_val);
      }
    }
  }
}

void loop() {
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.route("/cv", msg_CV);
      msg.route("/gate", msg_GATE);
    }
  }
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    digitalWrite(LED2, LOW);
  }
}

void writeDAC(int ch, int val) {
  Serial.print("Write CV... ");
  Serial.print(ch);
  Serial.print("\t");
  Serial.println(val);

  int cs = cs_pins[ch / 2];
  ch = ch & 1;
  ch = ch << 7;

  byte low = val & 0xff;
  byte high = (val >> 8) & 0x0f;

  digitalWrite(cs, LOW);
  SPI.transfer(ch | 0x10 | high);
  SPI.transfer(low);
  digitalWrite(cs, HIGH);
}

void writegate(int ch, int val) {
  Serial.print("Write gate... ");
  Serial.print(ch);
  Serial.print("\t");
  Serial.println(val);
  Serial.println(gate_pins[ch]);
  digitalWrite(gate_pins[ch], val);
}
