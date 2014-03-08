/*
  Web Server
 
 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 */
 
// What needs to be added:
// need to parse the hash
// need to implement the request counter (9 digits), if arduino restarts, allow higher counter
    // from server to up the count b/c arduino will reset to 0
// need to add crypto library https://github.com/Cathedrow/Cryptosuite to do the parsing

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(10, 0, 3, 240);

String password = "tuehnoschhrs189072398nthna";

String clientMsg = "";
struct clientInput {
  int time;
  String hash;
};

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


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


struct clientInput parseClientInput(String command) {
  struct clientInput holder;
  holder.time = command.substring(6, 15).toInt();
  holder.hash = command.substring(16, 80);
 
 //check the time 
}


void loop() {
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
        
        if (thisChar == '\n' && currentLineIsBlank) {
          
          Serial.println("Message from client: ");
          Serial.println(clientMsg);
          
          struct clientInput parsedInput = parseClientInput(clientMsg);

          
          // do stuff to open door
          
          clientMsg = "";
          client.println("Received!");
          client.stop();
          break;
        }
        
        if (thisChar == '\n') {
          currentLineIsBlank = true;
        } else if (thisChar != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

