/*

  Udp NTP Client

  Get the time from a Network Time Protocol (NTP) time server
  Demonstrates use of UDP sendPacket and ReceivePacket
  For more on NTP time servers and the messages needed to communicate with them,
  see http://en.wikipedia.org/wiki/Network_Time_Protocol

  created 4 Sep 2010
  by Michael Margolis
  modified 9 Apr 2012
  by Tom Igoe
  updated for the ESP8266 12 Apr 2015
  by Ivan Grokhotkov

  This code is in the public domain.

*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

char ssid[] = "mikenet";  //  your network SSID (name)
char pass[] = "thabo2elise";       // your network password


unsigned int localPort = 2390;      // local port to listen for UDP packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

/* ----------------------------------------------------------------------------------- */
// When in calibration mode, adjust the following factor until the servos move exactly 90 degrees
#define SERVOFAKTORLEFT 650
#define SERVOFAKTORRIGHT 650

// Zero-position of left and right servo
// When in calibration mode, adjust the NULL-values so that the servo arms are at all times parallel
// either to the X or Y axis
#define SERVOLEFTNULL 2250
#define SERVORIGHTNULL 920

#define SERVOPINLIFT  2 // 2
#define SERVOPINLEFT  3 // 3
#define SERVOPINRIGHT 4 // 4 

// lift positions of lifting servo
#define LIFT0 1180 // on drawing surface
#define LIFT1 925  // between numbers
#define LIFT2 725  // going towards sweeper

// speed of liftimg arm, higher is slower
#define LIFTSPEED 1500

// length of arms
#define L1 35
#define L2 55.1
#define L3 13.2

// origin points of left and right servo
#define O1X 22
#define O1Y -25
#define O2X 47
#define O2Y -25

// mkb #include <Time.h> // see http://playground.arduino.cc/Code/time
#include <Servo.h>

#ifdef REALTIMECLOCK
// for instructions on how to hook up a real time clock,
// see here -> http://www.pjrc.com/teensy/td_libs_DS1307RTC.html
// DS1307RTC works with the DS1307, DS1337 and DS3231 real time clock chips.
// Please run the SetTime example to initialize the time on new RTC chips and begin running.

#include <Wire.h>
#include <DS1307RTC.h> // see http://playground.arduino.cc/Code/time    
#endif

int servoLift = 1500;

Servo servo1;  //
Servo servo2;  //
Servo servo3;  //

volatile double lastX = 75;
volatile double lastY = 47.5;

int last_min = 0;
int ghour = 0;
int gminute = 0;


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());


#ifdef REALTIMECLOCK

  //while (!Serial) { ; } // wait for serial port to connect. Needed for Leonardo only

  // Set current time only the first to values, hh,mm are needed
  tmElements_t tm;
  if (RTC.read(tm))
  {
    setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);
    Serial.println("DS1307 time is set OK.");
  }
  else
  {
    if (RTC.chipPresent())
    {
      Serial.println("DS1307 is stopped.  Please run the SetTime example to initialize the time and begin running.");
    }
    else
    {
      Serial.println("DS1307 read error!  Please check the circuitry.");
    }
    // Set current time only the first to values, hh,mm are needed
    setTime(19, 38, 0, 0, 0, 0);
  }
#else
  // Set current time only the first to values, hh,mm are needed
  // mkb setTime(19,38,0,0,0,0);
#endif

  drawTo(75.2, 47);
  lift(0);
  servo1.attach(SERVOPINLIFT);  //  lifting servo
  servo2.attach(SERVOPINLEFT);  //  left servo
  servo3.attach(SERVOPINRIGHT);  //  right servo
  delay(1000);
}

void loop()
{


  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP);

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);

  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    ghour = (epoch  % 86400L) / 3600;
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    gminute = (epoch  % 3600) / 60;
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second

    // wait ten seconds before asking for the time again
    delay(10000);

#ifdef CALIBRATION

    // Servohorns will have 90° between movements, parallel to x and y axis
    drawTo(-3, 29.2);
    delay(500);
    drawTo(74.1, 28);
    delay(500);

#else


    int i = 0;
    if (last_min != minute()) {


      if (!servo1.attached()) servo1.attach(SERVOPINLIFT);
      if (!servo2.attached()) servo2.attach(SERVOPINLEFT);
      if (!servo3.attached()) servo3.attach(SERVOPINRIGHT);

      lift(0);

      hour();
      while ((i + 1) * 10 <= hour())
        // while ((i+1)*10 <= 1)
      {
        i++;
      }

      number(3, 3, 111, 1);  // erase board
      if (i != 0) {
         number(5, 25, i, 0.9);
      }
      number(19, 25, (hour() - i * 10), 0.9);
      number(28, 25, 11, 0.9); // draw colon

      i = 0;
      while ((i + 1) * 10 <= minute())
      {
        i++;
      }
      number(34, 25, i, 0.9);
      number(48, 25, (minute() - i * 10), 0.9);
      lift(2);   // lift pen all the way up.
      // drawTo(74.2, 47.5);  // go to top right.
      drawTo(70.2, 49.5);  // go to top right.
      lift(1);
      last_min = minute();

      servo1.detach();
      servo2.detach();
      servo3.detach();
    }

#endif
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}



void bogenUZS(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = -0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
           radius * sin(start + count) + by);
    count += inkr;
  }
  while ((start + count) > ende);

}



void bogenGZS(float bx, float by, float radius, int start, int ende, float sqee) {
  float inkr = 0.05;
  float count = 0;

  do {
    drawTo(sqee * radius * cos(start + count) + bx,
           radius * sin(start + count) + by);
    count += inkr;
  }
  while ((start + count) <= ende);
}


void drawTo(double pX, double pY) {
  double dx, dy, c;
  int i;

  // dx dy of new point
  dx = pX - lastX;
  dy = pY - lastY;
  //path lenght in mm, times 4 equals 4 steps per mm
  c = floor(4 * sqrt(dx * dx + dy * dy));

  if (c < 1) c = 1;

  for (i = 0; i <= c; i++) {
    // draw line point by point
    set_XY(lastX + (i * dx / c), lastY + (i * dy / c));

  }

  lastX = pX;
  lastY = pY;
}

double return_angle(double a, double b, double c) {
  // cosine rule for angle between c and a
  return acos((a * a + c * c - b * b) / (2 * a * c));
}

void set_XY(double Tx, double Ty)
{
  delay(1);
  double dx, dy, c, a1, a2, Hx, Hy;

  // calculate triangle between pen, servoLeft and arm joint
  // cartesian dx/dy
  dx = Tx - O1X;
  dy = Ty - O1Y;

  // polar lemgth (c) and angle (a1)
  c = sqrt(dx * dx + dy * dy); //
  a1 = atan2(dy, dx); //
  a2 = return_angle(L1, L2, c);

  servo2.writeMicroseconds(floor(((a2 + a1 - M_PI) * SERVOFAKTORLEFT) + SERVOLEFTNULL));

  // calculate joinr arm point for triangle of the right servo arm
  a2 = return_angle(L2, L1, c);
  Hx = Tx + L3 * cos((a1 - a2 + 0.621) + M_PI); //36,5°
  Hy = Ty + L3 * sin((a1 - a2 + 0.621) + M_PI);

  // calculate triangle between pen joint, servoRight and arm joint
  dx = Hx - O2X;
  dy = Hy - O2Y;

  c = sqrt(dx * dx + dy * dy);
  a1 = atan2(dy, dx);
  a2 = return_angle(L1, (L2 - L3), c);

  servo3.writeMicroseconds(floor(((a1 - a2) * SERVOFAKTORRIGHT) + SERVORIGHTNULL));

}



// Writing numeral with bx by being the bottom left originpoint. Scale 1 equals a 20 mm high font.
// The structure follows this principle: move to first startpoint of the numeral, lift down, draw numeral, lift up
void number(float bx, float by, int num, float scale) {

  switch (num) {

    case 0:
      drawTo(bx + 12 * scale, by + 6 * scale);
      lift(0);
      bogenGZS(bx + 7 * scale, by + 10 * scale, 10 * scale, -0.8, 6.7, 0.5);
      lift(1);
      break;
    case 1:

      drawTo(bx + 3 * scale, by + 15 * scale);
      // lift(0);
      drawTo(bx + 10 * scale, by + 20 * scale); // top part on 1
      lift(0);
      drawTo(bx + 10 * scale, by + 0 * scale);
      lift(1);
      break;
    case 2:
      drawTo(bx + 2 * scale, by + 12 * scale);
      lift(0);
      bogenUZS(bx + 8 * scale, by + 14 * scale, 6 * scale, 3, -0.8, 1);
      drawTo(bx + 1 * scale, by + 0 * scale);
      drawTo(bx + 12 * scale, by + 0 * scale);
      lift(1);
      break;
    case 3:
      drawTo(bx + 2 * scale, by + 17 * scale);
      lift(0);
      bogenUZS(bx + 5 * scale, by + 15 * scale, 5 * scale, 3, -2, 1);
      bogenUZS(bx + 5 * scale, by + 5 * scale, 5 * scale, 1.57, -3, 1);
      lift(1);
      break;
    case 4:
      drawTo(bx + 10 * scale, by + 0 * scale);
      lift(0);
      drawTo(bx + 10 * scale, by + 20 * scale);
      drawTo(bx + 2 * scale, by + 6 * scale);
      drawTo(bx + 12 * scale, by + 6 * scale);
      lift(1);
      break;
    case 5:
      drawTo(bx + 2 * scale, by + 5 * scale);
      lift(0);
      bogenGZS(bx + 5 * scale, by + 6 * scale, 6 * scale, -2.5, 2, 1);
      drawTo(bx + 5 * scale, by + 20 * scale);
      drawTo(bx + 12 * scale, by + 20 * scale);
      lift(1);
      break;
    case 6:
      drawTo(bx + 2 * scale, by + 10 * scale);
      lift(0);
      bogenUZS(bx + 7 * scale, by + 6 * scale, 6 * scale, 2, -4.4, 1);
      drawTo(bx + 11 * scale, by + 20 * scale);
      lift(1);
      break;
    case 7:
      drawTo(bx + 2 * scale, by + 20 * scale);
      lift(0);
      drawTo(bx + 12 * scale, by + 20 * scale);
      drawTo(bx + 2 * scale, by + 0);
      lift(1);
      break;
    case 8:
      drawTo(bx + 5 * scale, by + 10 * scale);
      lift(0);
      bogenUZS(bx + 5 * scale, by + 15 * scale, 5 * scale, 4.7, -1.6, 1);
      bogenGZS(bx + 5 * scale, by + 5 * scale, 5 * scale, -4.7, 2, 1);
      lift(1);
      break;

    case 9:
      drawTo(bx + 9 * scale, by + 11 * scale);
      lift(0);
      bogenUZS(bx + 7 * scale, by + 15 * scale, 5 * scale, 4, -0.5, 1);
      drawTo(bx + 5 * scale, by + 0);
      lift(1);
      break;

    case 111:

      lift(0);
      drawTo(70, 46);
      drawTo(65, 43);

      drawTo(65, 49);
      drawTo(5, 49);
      drawTo(5, 45);
      drawTo(65, 45);
      drawTo(65, 40);

      drawTo(5, 40);
      drawTo(5, 35);
      drawTo(65, 35);
      drawTo(65, 30);

      drawTo(5, 30);
      drawTo(5, 25);
      drawTo(65, 25);
      drawTo(65, 20);

      drawTo(5, 20);
      drawTo(60, 44);

      // drawTo(75.2, 47);  // purhaps top right
      drawTo(72.5, 47.5);  // go to top right.
      lift(2);

      break;

    case 11:
      drawTo(bx + 5 * scale, by + 15 * scale);
      lift(0);
      bogenGZS(bx + 5 * scale, by + 15 * scale, 0.1 * scale, 1, -1, 1);
      lift(1);
      drawTo(bx + 5 * scale, by + 5 * scale);
      lift(0);
      bogenGZS(bx + 5 * scale, by + 5 * scale, 0.1 * scale, 1, -1, 1);
      lift(1);
      break;

  }
}



void lift(char lift) {
  switch (lift) {
    // room to optimize  !

    case 0: //850

      if (servoLift >= LIFT0) {
        while (servoLift >= LIFT0)
        {
          servoLift--;
          servo1.writeMicroseconds(servoLift);
          delayMicroseconds(LIFTSPEED);
        }
      }
      else {
        while (servoLift <= LIFT0) {
          servoLift++;
          servo1.writeMicroseconds(servoLift);
          delayMicroseconds(LIFTSPEED);

        }

      }

      break;

    case 1: //150

      if (servoLift >= LIFT1) {
        while (servoLift >= LIFT1) {
          servoLift--;
          servo1.writeMicroseconds(servoLift);
          delayMicroseconds(LIFTSPEED);

        }
      }
      else {
        while (servoLift <= LIFT1) {
          servoLift++;
          servo1.writeMicroseconds(servoLift);
          delayMicroseconds(LIFTSPEED);
        }

      }

      break;

    case 2:

      if (servoLift >= LIFT2) {
        while (servoLift >= LIFT2) {
          servoLift--;
          servo1.writeMicroseconds(servoLift);
          delayMicroseconds(LIFTSPEED);
        }
      }
      else {
        while (servoLift <= LIFT2) {
          servoLift++;
          servo1.writeMicroseconds(servoLift);
          delayMicroseconds(LIFTSPEED);
        }
      }
      break;
  }
}








/*
    Time stub functions return random numbers between 1 and 20
*/
int hour() {
  // return (random(1, 10));

  // convert hours to EDT
  int localtime = ghour - 4;

  // concert localtime from millitary to civilian
  if (localtime > 12) {
    localtime = localtime - 12;
  }
  
  return localtime;
}

int minute() {
  // return (random(1, 10));
  return gminute;
}





