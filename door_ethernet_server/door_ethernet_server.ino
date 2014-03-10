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

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(10, 0, 3, 240);

int doorPin = 9;

String password = "tuehnoschhrs189072398nthna";
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
  Serial.println(holder.time_s);
  Serial.println(holder.time);
  Serial.println(holder.hash_s);

  command.substring(16, 80).toCharArray(holder.hash, 65);
  Serial.println(holder.hash);
  return holder;
 
 //check the time 
}

void openDoor() {
  digitalWrite(doorPin, HIGH);
  delay(15000);
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
        
        if (thisChar == '\n' && currentLineIsBlank) {
          
          Serial.println("Message from client: ");
          Serial.println(clientMsg);
          
          struct clientInput parsedInput = parseClientInput(clientMsg);
          
          clientMsg = "";
          
          // Check valid time
          if (parsedInput.time <= currentTime) {
            Serial.println(String("client time ") + parsedInput.time +
                           " <= currentTime " + currentTime);
            client.println("Invalid time");
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

              
            Serial.println(String("calculated hash: ") + hash_s);
            Serial.println(String("parsedInput: ") + parsedInput.hash_s);
            Serial.println(String("Comparison: ") + hash_s + " and " + String(parsedInput.hash_s));

            if (hash_s.equalsIgnoreCase(parsedInput.hash_s)) {
            
              // open door
              asyncOpenDoor = true;
              Serial.println("Commanded door to open");
              
            } else {
              
              // don't open door
              Serial.println("Invalid hash; door not opened");

            
            }
        
          }
          
          client.println(String("Received: Time=") + parsedInput.time_s + 
                          String(" | Hash: ") + parsedInput.hash_s);
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


