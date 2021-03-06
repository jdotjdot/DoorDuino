 /*

 An ethernet web server connected to a door buzzer that receives and processes
 POST requests, with an up-ticked nonce and SHA256 hash based on a pre-determined
 password, to open a door.


 Uses jkiv's fork of Cryptosuite written by Cathedrow
 Uses Ethernet web server written by Tom Igoe

 Written by J.J. Fliegelman at Hacker School W'14
 blog.jdotjdot.com
 www.hackerschool.com

 */


#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "sha256.h"
#include "password2.h"

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(10, 0, 3, 240);

int doorPin = 9;

String password = PASSWORD;  // PASSWORD defined in a header file
unsigned long currentTime = 0; // request index for verification - time is misnomer

String clientMsg = "";
struct clientInput {
  unsigned long time;
  String time_s; // necessary to keep digits for hashing
  String hash_s;
  char hash[65];

};



// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

boolean asyncOpenDoor = false; // for async ending client connection without waiting for door delay

////// END OF DECLARATIONS ////////

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


struct clientInput parseClientInput(String command) {
  struct clientInput holder;

  // To deal with int overflow and no String.toLong command, breaking it up into ints,
  //  making into Longs, then adding together

  holder.time_s = command.substring(6, 15);

  int piece1 = command.substring(6, 9).toInt();
  int piece2 = command.substring(9, 12).toInt();
  int piece3 = command.substring(12, 15).toInt();

  holder.time = (unsigned long) piece1 * (unsigned long) 1000000 +
                (unsigned long) piece2 * (unsigned long) 1000 +
                (unsigned long) piece3;

  holder.hash_s = command.substring(16, 80);
  //Serial.println(holder.time_s);
  //Serial.println(holder.time);
  //Serial.println(holder.hash_s);

  command.substring(16, 80).toCharArray(holder.hash, 65);
  //Serial.println(holder.hash);
  Serial.println(String("Available memory: ") + availableMemory());
  return holder;

 //check the time
}

void openDoor() {
  digitalWrite(doorPin, HIGH);
  delay(10000);
  digitalWrite(doorPin, LOW);
}

void loop() {

  // If door should be opened, do it

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

          struct clientInput parsedInput = parseClientInput(clientMsg);

          clientMsg = "";
          client.flush();
          
          //Serial.println(parsedInput.time);
          //Serial.println(currentTime);

          // Check valid time
          if (parsedInput.time <= currentTime) {
            // Invalid time
            Serial.println(String("client time ") + parsedInput.time +
                           " <= currentTime " + currentTime);

            client.println("HTTP/1.1 400 Bad Request");
            client.println(String("Next-nounce: ") + (currentTime + 1));

            client.stop();
            break;
          } else {

            //update time counter
            currentTime = parsedInput.time;

            // check hash
            uint8_t *hash;
            Sha256.init();
            Sha256.print(parsedInput.time_s + password);
            hash = Sha256.result();

            char hash_c[65];

            // convert from uint8_t to HEX str
            String tmpHash;
            String hash_s = "";

            for (int i=0; i<32; i++) {
              tmpHash = String(hash[i], HEX);
              while (tmpHash.length() < 2) {
                tmpHash = "0" + tmpHash;
              }
              hash_s += tmpHash;
            }
            hash_s.toCharArray(hash_c, 65);


//            Serial.println(String("calculated hash: ") + hash_c);
//            Serial.println(String("parsedInput: ") + parsedInput.hash);
//            Serial.println(String("Comparison: ") + hash_s + " and " + String(parsedInput.hash));

            if (strcmp(parsedInput.hash, hash_c) == 0) {

              // open door
              asyncOpenDoor = true;
              client.println("HTTP/1.1 200 OK");
              Serial.println("Commanded door to open");

            } else {

              // don't open door
              client.println("HTTP/1.1 401 Unauthorized");
              Serial.println("Invalid hash; door not opened");


            }

          }


          client.println("Connection: close");
//          client.println(String("Received: Time=") + parsedInput.time_s +
//                          String(" | Hash: ") + parsedInput.hash_s);
          client.stop();
          break;
        }


//        if (thisChar == '\n') {
//          currentLineIsBlank = true;
//        } else if (thisChar != '\r') {
//          currentLineIsBlank = false;
//        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

int availableMemory() {
  int size = 1024; // Use 2048 with ATmega328
  byte *buf;

  while ((buf = (byte *) malloc(--size)) == NULL)
    ;

  free(buf);

  return size;
}



