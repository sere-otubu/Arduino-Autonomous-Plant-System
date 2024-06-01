// APSC 200
// Team 12: Erhowvosere Otubu, Carter Moore, Will Pritchard, Tao Nichol, Arta Namjoo

#include <WiFiNINA.h>
#include <math.h>
#define echoPin 2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 3 //attach pin D3 Arduino to pin Trig of HC-SR0
char ssid[] = "Carter";             //  your network SSID (name) between the " "
char pass[] = "Harry623";      // your network password between the " "
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status

// defines variables
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
const int AirValue = 469;   //you need to replace this value with Value_1
const int WaterValue = 310;  //you need to replace this value with Value_2
int soilMoistureValue = 0;
int soilmoisturepercent = 0;

int photoPin = A4;

int heatPin = 8;
int lightPin = 12;
int relayPin = 5;
int light;

int LMT86 = A0;
int A0_Read = 0;
float voltage = 0;
float Temperature = 0;
//0 is chives,  1 is tomatoes. rows 0 & 1 are for temp, rows 2 & 3 are for light, rows 4 & 5 are for moisture, row 4 is for the time it takes to grow
int plants [2][3];
int plantsThreshold [2][7] = {{15, 23, 6, 8, 60, 70, 60}, {15, 34, 6, 8, 60, 80, 90}};
int emptyTank = 6;
int selection = -1;
int tempStatus = 0;
int lightStatus = 0;
int lightCounter = 0;
int moistureStatus = 0;
int tankStatus = 0;
int minutes = 0;
int hours = 0;
int days = 0;
int printTemp;
int printMoisture;
int endProcess = 0;
int lightConstant = 100;
int lightTime;
int finished = 0;
int testnumber = 0;
WiFiServer server(80);            //server socket

WiFiClient client = server.available();
int sensorValue = 8;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(heatPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT

  while (!Serial)
  
  enable_WiFi();
  connect_WiFi();
  server.begin();
  printWifiStatus();
}
int minute = 0;

void loop() {
  if(endProcess == 1){
    exit(0);
  }
  if (selection == 2 || selection == -1){
    Serial.println(testnumber);
    printWifiStatus();
    getTemp();
    getMoisture();
    getLight();
    getWaterDistance();
    decisions();
    delay(6000);
    if(testnumber == 1){
      if(light < 150){
        digitalWrite(lightPin, LOW);
      }
      else{
        digitalWrite(lightPin, HIGH);
      }
    }
    if (testnumber != 1){
      digitalWrite(lightPin, LOW);
    }
    if(testnumber == 2){
      if(soilmoisturepercent < 60){
        
        digitalWrite(relayPin, LOW);
        delay(2000);
        digitalWrite(relayPin, HIGH);
      }
      else{
      
        digitalWrite(relayPin, HIGH);
        delay(6000);
      
      }
    }
    if(testnumber == 3){
      if(distance < 6){
        digitalWrite(relayPin, LOW);
        delay(6000);
        digitalWrite(relayPin, HIGH);
        delay(6000);
      }
      else{
        digitalWrite(relayPin, HIGH);
        delay(6000);
      }

    }
    if(testnumber == 4){
      if(printTemp > 15){
        digitalWrite(heatPin, LOW);
        delay(250);
      }
      else{
        digitalWrite(heatPin, HIGH);
        delay(250);
        }
    }
    if(testnumber != 4){
      digitalWrite(heatPin, LOW);
    }
    client = server.available();
    if(client){
      printWEB();
    }
    delay(500);
  }
  else{
  if(days == plantsThreshold[selection][6]){
    finished = 1;
  }
    Serial.print("The value is printed after: ");
    Serial.print(minute);
    Serial.print(" minute\n");
    minute = minute+1;
    getTemp();
    getMoisture();
    getLight();
    getWaterDistance();
    Serial.println(selection);
    if(selection != -1){
      decisions();
    }
    if(minute == 60){
      //To add: if the light counter isn't 0, subtract 1 from it.
      if(lightTime != 0){
        lightTime = lightTime - 1;
      }
      hours = hours+1;
      //replace threshold for light with readable value
      if(lightStatus == 1){
        lightCounter = lightCounter+1;
      }
      minute = 0;
      if(hours == 24){
        days = days+1;
        if(lightCounter < plantsThreshold[selection][2]){
          lightTime = plantsThreshold[selection][3] - lightCounter;
          if(lightTime < 0){
            lightTime = 0;
          }
        }
        lightCounter = 0;
        hours = 0;
      }
    }
    if(lightTime != 0){
      digitalWrite(lightPin, HIGH);
    }
    else{
      digitalWrite(lightPin, LOW);
    }
    if(tempStatus == 1){
      digitalWrite(heatPin, HIGH);
    }
    else{
      digitalWrite(heatPin, LOW);
    }
    int i = 0;
    if(moistureStatus == 1){
      Serial.println("pump on");
      pinMode(relayPin, OUTPUT);
      digitalWrite(relayPin, HIGH);
      delay(2950);
      digitalWrite(relayPin, LOW);
      i = 50;
    }
    // turn on pins for certain functions
    while(i < 1000){
      client = server.available();
      if(client){
        printWEB();
      }
      i = i+1;
      delay(59);
    }
    }
  // delay(59000);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
}

void printWEB() {

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {

            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
           
            //create the buttons
            // client.print("Click <a href=\"/H\">here</a> turn the LED on<br>");
            // client.print("Click <a href=\"/L\">here</a> turn the LED off<br><br>");

            //only show status if a plant is already growing? Also dont offer to grow something if plant is already growing
            client.print("<body bgcolor=”#f0e926>");

            //put all readings on screen for user to see, temp, need to refil water, etc.
            if(finished == 1){
              client.print("<center>Please harvest plant and press the 'Force End' button.</center><br>");
            }
            else if(selection == -1){
              client.print("<center>What would you like to grow?:</center><br>");
              client.print("<center><a href=\"/Chives\">Chives</a></center><br>");
              client.print("<center><a href=\"/Tomatoes\">Tomatoes</a></center><br>");
              client.print("<center><a href=\"/LiveDemo\">Live Demonstration</a></center><br>");
              //if done, print that the process has finished and the plant must be harvested, else say how much time left on clock.
              //force end button, maybe have like an "all done button when the plant is harvested?"
            }
            else{
              client.print("<center>CURRENT STATUS:</center><br>");
              client.print("<center>");
              client.print("Temperature: ");
              client.print(printTemp);
              client.print("<span>&#176;</span>C");
              client.print("<center/><br>");
              if(tempStatus == 1){
                client.print("<center>");
                client.print("The heater is currently ON");
                client.print("<center/><br>");
              }
              else{
                client.print("<center>");
                client.print("The heater is currently OFF");
                client.print("<center/><br>");
              }
              client.print("<center>");
              client.print("Soil Moisture Percentage: ");
              client.print(printMoisture);
              client.print("%");
              client.print("<center/><br>");
              if(lightTime != 0){
                client.print("<center>");
                client.print("Light is currently ON");
                client.print("<center/><br>");
              }
              else{
                client.print("<center>");
                client.print("Light is currently OFF");
                client.print("<center/><br>");
              }
              if(tankStatus == 1){
                client.print("<center>");
                client.print("The water tank is running low. Please refill as soon as possible.");
                client.print("<center/><br>");
              }
              else{
                  int tankPercent = (distance)/(emptyTank) * 100;
                  client.print("<center>");
                  client.print("The water tank is ");
                  client.print(tankPercent);
                  client.print("% full");
                  client.print("<center/><br>");
                  
              }
              if(selection == 2){
                client.print("<center><a href=\"/testlight\">Test Light</a></center><br>");
                client.print("<center><a href=\"/moisturesensor\">Test Moisture Sensor</a></center><br>");
                client.print("<center><a href=\"/emptytank\">Test Empty Tank</a></center><br>");
                client.print("<center><a href=\"/raiseheat\">Raise Heat Threshold</a></center><br>");
              }
              client.print("<center><a href=\"/END\">Force End</a></center><br><br>");
              client.println();
            }
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        if (currentLine.endsWith("GET /Chives")) {
          digitalWrite(LED_BUILTIN, HIGH);    
          selection = 0;
            
        }
        if (currentLine.endsWith("GET /Tomatoes")) {
          digitalWrite(LED_BUILTIN, LOW);   
          selection = 1;    
        }
        if (currentLine.endsWith("GET /LiveDemo")) {
          digitalWrite(LED_BUILTIN, LOW);   
          selection = 2;    
        }
        if (currentLine.endsWith("GET /Force End")) {
          digitalWrite(LED_BUILTIN, LOW);   
          endProcess = 1;   
        }
        if (currentLine.endsWith("GET /testlight")) {
          digitalWrite(LED_BUILTIN, LOW);   
          testnumber = 1;
        }
        if (currentLine.endsWith("GET /moisturesensor")) {
          digitalWrite(LED_BUILTIN, LOW);   
          testnumber = 2;
        }
        if (currentLine.endsWith("GET /emptytank")) {
          digitalWrite(LED_BUILTIN, LOW);   
          testnumber = 3;
        }
        if (currentLine.endsWith("GET /raiseheat")) {
          digitalWrite(LED_BUILTIN, LOW);   
          testnumber = 4;
        }

      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
//add function that decides what needs to be turned on
void decisions(){
  if(soilmoisturepercent < plantsThreshold[selection][4]){
    if (distance >= 6){
      moistureStatus = 0;
      tankStatus = 1;
      }
    else if(distance < 6){
      tankStatus = 0;
      moistureStatus = 1;
      }
  }
  else {
    moistureStatus = 0;
  }
  if (printTemp < plantsThreshold[selection][0]){
    tempStatus = 1;
  }
  else{
    tempStatus = 0;
  }
  if(light < lightConstant){
    lightStatus = 1;
  }
  else{
    lightStatus = 0;
  }
}


int getTemp(){
  A0_Read = analogRead(LMT86);
  // Serial.println(A0_Read);
  Temperature = (415-A0_Read) / 2.14;
  Serial.print("Temperature: ");
  Serial.print(Temperature, 1);
  Serial.println(" C");
  printTemp = (int)Temperature;
  delay(250);
  return (int)round(Temperature);
}

int getMoisture(){
  soilMoistureValue = analogRead(A2);  //put Sensor insert into soil
  Serial.print("Soil Moisture: ");
  // Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if(soilmoisturepercent >= 100)
  {
    soilmoisturepercent = 100;
    Serial.println("100 %");
  }
  else if(soilmoisturepercent <=0)
  {
    soilmoisturepercent = 0;
    Serial.println("0 %");
  }
  else if(soilmoisturepercent >0 && soilmoisturepercent < 100)
  {
    Serial.print(soilmoisturepercent);
    Serial.println("%");
  }
  printMoisture = soilmoisturepercent;
  delay(250);
  return soilmoisturepercent;
}

int getLight(){
  light = analogRead(photoPin);
  Serial.print("PhotoResistor: " );
  Serial.println(light);
  delay(250);
  return light;
}

int getWaterDistance(){
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(250);
  return (int)distance;
}