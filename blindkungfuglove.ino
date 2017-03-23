#include <CurieBLE.h>
#include <CurieIMU.h>

const int ledPin = 13; // Set ledPin to on-board LED
const int vibratorPin = 3; // Set vibratorPin to analog pin 3

BLEPeripheral blePeripheral; // create peripheral instance
BLEService gloveService("4DB09DD9-E941-4F15-B506-3BA4F524C400"); // This is our glove service

// This is the characteristic that enables haptic feedback
BLECharCharacteristic hapticCharacteristic("4DB09DD9-E941-4F15-B506-3BA4F524C401", BLERead | BLEWrite);
// This is the characteristic that enables listening for punches
BLECharCharacteristic punchCharacteristic("4DB09DD9-E941-4F15-B506-3BA4F524C402", BLERead | BLEWrite | BLENotify);

boolean notify = false;

void setup() {
  pinMode(ledPin, OUTPUT); // Use the LED on pin 13 as an output
  pinMode(vibratorPin, OUTPUT); // Use vibrator pin 3 as an output
  
  Serial.begin(9600);
  //while (!Serial);

  Serial.println("Hello, World!  I'm alive.");

  CurieIMU.begin();
  CurieIMU.attachInterrupt(shockCallback);

  // Shock detection setup
  CurieIMU.setDetectionThreshold(CURIE_IMU_SHOCK, 7500); // Let's start with 1.5 G
  CurieIMU.setDetectionDuration(CURIE_IMU_SHOCK, 50);    // A punch will take no more than 50 ms
  CurieIMU.interrupts(CURIE_IMU_SHOCK);
  
  // Set up the über cool Blind Kung Fu glove peripherale
  blePeripheral.setLocalName("Glove");
  blePeripheral.setDeviceName("Blind Kung Fu glove");
  blePeripheral.setAdvertisedServiceUuid(gloveService.uuid());

  // Now, add services and stuff
  blePeripheral.addAttribute(gloveService);
  blePeripheral.addAttribute(hapticCharacteristic);
  blePeripheral.addAttribute(punchCharacteristic);

  hapticCharacteristic.setValue(0);
  punchCharacteristic.setValue(0);

  // Advertise the service
  blePeripheral.begin();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  BLECentral central = blePeripheral.central();

  if (central) {
    Serial.println("Connected! Yay!");

    while (central.connected()) {
      // Poll peripheral
      blePeripheral.poll();
    
      // Vibrate if requested
      if (hapticCharacteristic.written()) {
        vibrate(hapticCharacteristic.value());
        hapticCharacteristic.setValue(0);
      }
    
      // Notify detected punches
      if (notify) {
        if (punchCharacteristic.setValue(1)) {
          Serial.println("Notified punch");
        } else {
          Serial.println("Couldn't notify");
        }
        
        // Only detetct each punch once. This might not work...
        punchCharacteristic.setValue(0);
        notify = false;
      }
    }

    Serial.print("Disconnected from ");
    Serial.println(central.address());
  }
}

void blink(boolean on) {
  if (on) {
    Serial.println("LED on");
    digitalWrite(ledPin, HIGH);
  } else {
    //Serial.println("LED off");
    digitalWrite(ledPin, LOW);
  }
}

void vibrate(char operation) {
  long startMillis = millis();
  long duration = 0;
  //blink(true);
      
  switch (operation) {
    case 'H': // Yay, we hit an enemy
      Serial.println("We hit an enemy. Vibrate ever so slightly");
      // TODO: analogWrite something to vibratorPin for n time
      analogWrite(vibratorPin, 255);
      duration = 500;
      break;
    case 'D': // Oh noes, we died
      Serial.println("An enemy hit us. Vibrate furiously.");
      // TODO: analogWrite something to vibratorPin for n time
      analogWrite(vibratorPin, 255);
      duration = 2000;
      break;
    default:
      Serial.print("We don't take kindly to that command around here: ");
      Serial.println(operation);
  }
  while (millis() - startMillis < duration) {
  }
  analogWrite(vibratorPin, 0);
  //blink(false);
}

static void shockCallback(void) {
  if (CurieIMU.getInterruptStatus(CURIE_IMU_SHOCK)) {
    Serial.println("MEGA PUNCH!");
    notify = true;
  }
}

