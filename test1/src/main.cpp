#include <Arduino.h>
#include <Notecard.h>

#define productUID "app:ba104549-63da-4bfd-96e0-bab93c7ffb06"
#define usbSerial Serial
#define BUTTON_PIN USER_BTN

Notecard notecard;
bool locationRequested = false;

void RequestLocation(void) {
  usbSerial.println("Button pressed");
  notecard.logDebug("Button pressed\n");
  locationRequested = true;
}

// the setup function runs once when you press reset or power the board
void setup()
{
  //pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), RequestLocation, FALLING);
  
  //while (!usbSerial)
    usbSerial.begin(115200);

  notecard.begin();
  notecard.setDebugOutputStream(usbSerial);

  J *req1 = notecard.newRequest("hub.set");
  JAddStringToObject(req1, "product", productUID);
  JAddStringToObject(req1, "mode", "periodic");
  JAddNumberToObject(req1, "outbound", 5);
  JAddStringToObject(req1, "sn", "Ola-la");
  notecard.sendRequest(req1);

  usbSerial.println("Setup complete");

  J *req2 = notecard.newRequest("card.location.mode");
  JAddStringToObject(req2, "mode", "periodic");
  JAddNumberToObject(req2, "seconds", 320);
  notecard.sendRequest(req2);

  J *req3 = notecard.newRequest("card.location.track");
  JAddBoolToObject(req3, "start", true);
  JAddBoolToObject(req3, "heartbeat", true);
  JAddNumberToObject(req3, "hours", 2);
  notecard.sendRequest(req3);
}



// the loop function runs over and over again forever
void loop()
{
  //digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  //delay(1000);                     // wait for a second
  //digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
  //delay(1000);                     // wait for a second

  if (!locationRequested) {
    return;
  }

  J *req = notecard.newRequest("card.location");
  J *resp = notecard.requestAndResponse(req);
  usbSerial.println("Location:");
  double lat = JGetNumber(resp, "lat");
  double lon = JGetNumber(resp, "lon");
  usbSerial.println(lat, 10);
  usbSerial.println(lon, 10);

  // if (lat == 0) {
  //   usbSerial.println("The Notecard does not yet have a location");
  //   // Wait a minute before trying again.
  //   delay(1000 * 60);
  //   return;
  // }

  // http://maps.google.com/maps?q=<lat>,<lon>
  char buffer[100];
    snprintf(
     buffer,
     sizeof(buffer),
     "Your kids are requesting you. https://maps.google.com/maps?q=%.8lf,%.8lf",
     lat,
     lon
    );
  usbSerial.println(buffer);

  J *req2 = notecard.newRequest("note.add");
  JAddStringToObject(req2, "file", "twilio.qo");
  JAddBoolToObject(req2, "sync", true);
  J *body = JCreateObject();
  JAddStringToObject(body, "message", buffer);
  JAddItemToObject(req2, "body", body);
  notecard.sendRequest(req2);

  locationRequested = false;
  usbSerial.println("Location sent successfully.");
}