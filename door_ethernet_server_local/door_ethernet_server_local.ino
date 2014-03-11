

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(10, 0, 3, 240);

boolean asyncOpenDoor = false;
String clientMsg = "";

int doorPin = 9;

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  pinMode(doorPin, OUTPUT);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void openDoor() {
    digitalWrite(doorPin, HIGH);
    delay(15000);
    digitalWrite(doorPin, LOW);
}


void loop() {

    // if door should be opened, do it

    if (asyncOpenDoor) {
        asyncOpenDoor = false;
        openDoor();
    }

    // listen for incoming clients
    EthernetClient client = server.available();
    if (client) {
      Serial.println("new client");
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {

          // read incoming bytes from client
          char thisChar = client.read();

          clientMsg += thisChar;

          // flush buffer if goes above 100
          if (clientMsg.length() >= 100) {
            clientMsg = "";
            client.flush();

            client.println("HTTP/1.1 418 I'm a teapot");
          }

          if (clientMsg.endsWith("HTTP")) {

            Serial.println("Message from client: ");
            Serial.println(clientMsg);

            // struct clientInput parsedInput = parseClientInput(clientMsg);

            clientMsg = "";
            client.flush();

            if (clientMsg.substring(6,14) == String("opendoor")) {
                // open door
                asyncOpenDoor = true;
                client.println("HTTP/1.1 200 OK");
                Serial.println("Commanded door to open");
            } else {
                // don't open door
                client.println("HTTP/1.1 401 Unauthorized");
                Serial.println("Invalid URL; door not opened.");
            }

         client.println("Connection: close");
         client.stop();
         break;

         }
        }
      }

      delay(1);
      // close the connection
      client.stop();
      Serial.println("client disconnected");
    }
}
