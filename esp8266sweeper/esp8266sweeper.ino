/* Sweep
  by BARRAGAN <http://barraganstudio.com>
  This example code is in the public domain.
  modified 28 May 2015
  by Michael C. Miller
  modified 8 Nov 2013
  by Scott Fitzgerald
  http://arduino.cc/en/Tutorial/Sweep
*/
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "fd69915f398f4ef78e6b0bef0943c41d";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "mikenet";
char pass[] = "thabo2elise";
boolean initservo = true;

void setup()
{
  Blynk.begin(auth, ssid, pass);

  myservo.attach(2);  // attaches the servo on GIO2 to the servo object
  // myservo.write(10);
}

BLYNK_WRITE(V2)
{
  myservo.write(param.asInt());
}

void loop()
{
  if (initservo) {
    int pos;

    for (pos = 10; pos <= 170; pos += 1) // goes from 0 degrees to 180 degrees
    { // in steps of 1 degree
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }

    initservo = false;
  }
 /*
  for (pos = 180; pos >= 0; pos -= 1) // goes from 180 degrees to 0 degrees
  {
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  */
  Blynk.run();
}


