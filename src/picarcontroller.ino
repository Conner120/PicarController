/*
  Software serial multple serial test

  Receives from the hardware serial, sends to software serial.
  Receives from software serial, sends to hardware serial.

  The circuit:
   RX is digital pin 10 (connect to TX of other device)
   TX is digital pin 11 (connect to RX of other device)

  Note:
  Not all pins on the Mega and Mega 2560 support change interrupts,
  so only the following can be used for RX:
  10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

  Not all pins on the Leonardo support change interrupts,
  so only the following can be used for RX:
  8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

  created back in the mists of time
  modified 25 May 2012
  by Tom Igoe
  based on Mikal Hart's example

  This example code is in the public domain.

*/
#include <SD.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Button.h>
#include <Dial.h>
#include <Display.h>
#include <Gauge.h>
#include <Numkey.h>
#include <Popup.h>

Display disp = Display(); // Memory used: (storage/ram: 484/37)    12,240/392
Canvas canvas = Canvas(); // Memory used: (storage/ram: 1,676/36)  3,372/228Popup popup = Popup();
Popup popup = Popup();
Dial dialRight = Dial();       // Memory used: (storage/ram: 4,760/64)  11756/355
Dial dialLeft = Dial();      // Memory used: (storage/ram: 4,760/64)  11756/355
Button menuButton = Button(); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button backButton = Button(); // Memory used: (storage/ram: 3,624/63)  6,996/291
Gauge gauge = Gauge();    // Memory used: (storage/ram: 1,470/52)  13710/444
Button monitorButton = Button(); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button settingsButton = Button(); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button otherButton = Button(); // Memory used: (storage/ram: 3,624/63)  6,996/291
Button erase = Button();
Button emergancyButton = Button();
int prevScreen = 0;
Sd2Card card;
const int chipSelect = 4;
bool ESTOP = false;
bool EModePush = false;
//SoftwareSerial Serial1(52, 53); // RX, TX
int curScreen = 0;
bool HLPush = false;
int rVal = 0;
int lVal = 0;
#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2
bool headLight = false;
int menu = 0;
struct settings {
  float ver = 2.4;
  bool sound = true;
  bool usbDebug = false;
  bool codeLock = false;
  bool fingerLock = false;
  int robotType = 0; //aka wild thumper
  bool secUSB = false;
  bool commandUSB = false;
  char deviceName[20] = "Conner's Remote";
  float lastGPSLon = 0.0000;
  float lastGPSLat = 0.0000;
  int lGPSRTMo = 0;
  int lGPSRTD = 0;
  int lGPSRTY = 0;
  int lGPSRTH = 0;
  int lGPSRTM = 0;
  int lGPSRTS = 0;
  bool APPUSB = false;
};
bool SDCard = true;
int fn = 0;
int sn = 0;
int tn = 0;
settings setr = {};
#define LNA 0
void setup() {
  Serial.begin(57600);

  fn = EEPROM.read(LNA);
  sn = EEPROM.read(LNA + 1);
  tn = EEPROM.read(LNA + 2);
  if (fn == 255) {
    Serial.println("reseting launch ids");
    EEPROM.update(LNA + 2, 0);
    EEPROM.update(LNA + 1, 0);
    EEPROM.update(LNA, 0);
  }
  if (sn == 255) {
    EEPROM.update(LNA + 1, 0);
    EEPROM.update(LNA, fn + 1);
  }
  if (tn == 255) {
    EEPROM.update(LNA + 2, 0);
    EEPROM.update(LNA + 1, sn + 1);
  } else {
    EEPROM.update(LNA + 2, tn + 1);
  }
  Serial.println(String(fn) + "." + String(sn) + "." + String(tn));
  //  pinMode(11, INPUT);
  //  pinMode(12, INPUT);
  //  pinMode(13, INPUT);
  SPI.begin();
  Tft.TFTinit();

  TFT_CS_HIGH;
  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);
  canvas.init(TFT_LANDSCAPE);
  //   Tft.setCol(0, 239);
  //   Tft.setPage(0, 319);
  Tft.sendCMD(0x2c);//start to write to display ram
  if (!SD.begin(chipSelect)) //SPI_QUARTER_SPEED,
  { //53 is used as chip select pin
    Serial.println("failed!");
    SDCard = false;

  } else {
    Serial.println("SD OK!");

  }
  if (!SDCard) {
    popup.setText("SD Failed, Reboot");
    popupSDError();
  }
  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  Serial1.begin(57600);
  Serial1.println("Hello, world?");
  drawHome();

}
void sendAnalogStick(int right, int left) {
  //  int mm1 = 0;
  //  if (analogRead(right)-514 > 0) {
  //    mm1 = 1;
  //  } else {
  //    mm1 = 2;
  //  }
  //  int mm2 = 0;
  //  if (analogRead(left)-514 > 0) {
  //    mm2 = 1;
  //  } else {
  //    mm2 = 2;
  //  }
  int d1;
  int d2;
  if ((analogRead(right) - 515) > 3) {
    d1 = 1;
  } else if ((analogRead(right) - 515) < -3) {
    d1 = 2;
  } else {

    d1 = 0;
  }
  if ((analogRead(left) - 515) > 3 ) {
    d2 = 1;
  } else if ((analogRead(left) - 515) < -3) {
    d2 = 2;
  } else {
    d2 = 0;
  }
  if (d1 == 0) {
    if (d2 == 0) {
      Serial1.println("d," + String(d1) + "," + String(d2) + ",0," + String("0") + ",");
      Serial.println("d,"  + String(d1) + "," + String(d2) + ",0,0,");
    } else {
      Serial1.println("d," + String(d1) + "," + String(d2) + ",0," + String((analogRead(left) - 515) ) + ",");
      Serial.println("d,"  + String(d1) + "," + String(d2) + ",0," + String((analogRead(left) - 515 )) + ",");
    }
  } else if (d2 == 0) {
    if (d1 == 0) {
      Serial1.println("d," + String(d1) + "," + String(d2) + ",0,0,");
      Serial.println("d,"  + String(d1) + "," + String(d2) + ",0,0,");
    } else {
      Serial1.println("d," + String(d1) + "," + String(d2) + "," + String((analogRead(right) - 515)) + ",0,");
      Serial.println("d,"  + String(d1) + "," + String(d2) + "," + String((analogRead(right) - 515 )) + ",0,");
    }
  } else {
    Serial1.println("d," + String(d1) + "," + String(d2) + "," + String((analogRead(right) - 515)) + "," + String((analogRead(left) - 515) ) + ",");
    Serial.println("d,"  + String(d1) + "," + String(d2) + "," + String((analogRead(right) - 515 )) + "," + String((analogRead(left) - 515 )) + ",");
  }

  rVal = analogRead(right);
  lVal = analogRead(left);
}
void loop() { // run over and over
  canvas.scan();
  switch (curScreen) {
    case 0:
      dialRight.setCV(analogRead(A12));
      //      if (analogRead(A12) > 514) {
      //
      //        dialRight.setCV(analogRead(A12) - 514);
      //      } else if (analogRead(A12) == 514) {
      //        dialRight.setCV(0);
      //      } else {
      //        dialRight.setCV((analogRead(A12) - 514) * -1);
      //      }
      //      if (analogRead(A15) > 514) {
      //        dialLeft.setCV(analogRead(A15) - 514);
      //      } else if (analogRead(A15) == 514) {
      //        dialLeft.setCV(0);
      //      } else {
      //        dialLeft.setCV((analogRead(A15) - 514) * -1);
      //      }
      dialLeft.setCV(analogRead(A15));
      break;
  }
  //if (!ESTOP) {
  int analog15 = analogRead(A15);
  int analog13 = analogRead(A12);
    if ((millis()%10 )< 1) {
// if (analog15 == rVal && analog13 == lVal) {
//
//    } else {
      sendAnalogStick(A15, A12);
   // }
    }
  if (Serial1.available()) {
    remoteCommand();
    //Serial.write(Serial1.read());
  }
  if (Serial.available()) {
    localCommand();
    Serial1.write(Serial.read());
    //      if (SDCard) {
    //        File dataFile = SD.open("datalog.txt", FILE_WRITE);
    //        dataFile.println(buf);
    //        dataFile.close();
    //      }
  }
  if (menu == 0) {
  } else if ( menu == 1) {
    // }
  }
  //  } else {
  //    Serial1.println("d" + String(0) + "," + String(0) + ",");
  //    Serial.println("d" + String(0) + "," + String(0) + ",");
  //  }
  //delay(100);
}
void drawHome() {
  curScreen = 0;
  Serial.println("draw home");
  dialRight.setSize(50);
  dialRight.setColors(GRAY2, YELLOW, GRAY1);
  dialRight.setLimits(0, 514, 999);
  dialRight.init();
  dialRight.setHiLimit(515, GREEN);
  dialRight.setLowLimit(513, RED);

  dialLeft.setSize(50);
  dialLeft.setColors(GRAY2, YELLOW, GRAY1);
  dialLeft.setLimits(0, 514, 999);
  dialLeft.init();
  dialLeft.setHiLimit(515, GREEN);
  dialLeft.setLowLimit(513, RED);
  menuButton.setSize(80, 40);
  menuButton.setColors(GRAY1, BLACK, WHITE);
  menuButton.setText("Menu");
  menuButton.setEventHandler(&drawMenuButton);
  menuButton.init();
  menuButton.visible = true;
  menuButton.setDebounce(1000);
  canvas.add(&menuButton, 0, 0);
  canvas.add(&dialLeft, 65, 180);
  canvas.add(&dialRight, 260, 180);

}
void drawMenuButton(Button *btn) {
  clearhome();
  drawMenu(1);
}
void clearmenu(int p) {
  backButton.visible = false;
  monitorButton.visible = false;
  settingsButton.visible = false;
  otherButton.visible = false;
  disp.clear();
  Tft.fillRectangle(0, 0, 320, 240, BLACK);
  prevScreen = 1;

}
void clearhome() {
  menuButton.visible = false;
  disp.clear();
  Tft.fillRectangle(0, 0, 320, 240, BLACK);
  prevScreen = 0;
}
void drawMenu(int p) {
  Tft.fillRectangle(0, 0, 320, 240, WHITE);
  curScreen = 1;
  //if (menuButton.visible) {
  //clearhome();
  backButton.setEventHandler(&drawHomeBtn);
  drawTopBar(1);
  Serial.println("draw menu");
  monitorButton.setSize(140, 75);
  monitorButton.setColors(GRAY1, BLACK, WHITE);
  monitorButton.setText("Monitor");
  monitorButton.setEventHandler(&drawMonitorButton);
  monitorButton.init();
  monitorButton.visible = true;
  settingsButton.setSize(140, 75);
  settingsButton.setColors(GRAY1, BLACK, WHITE);
  settingsButton.setText("Settings");
  settingsButton.setEventHandler(&drawSettingsButton);
  settingsButton.init();
  settingsButton.visible = true;
  otherButton.setSize(140, 75);
  otherButton.setColors(GRAY1, BLACK, WHITE);
  otherButton.setText("Other");
  otherButton.setEventHandler(&drawOtherButton);
  otherButton.init();
  otherButton.visible = true;
  backButton.setSize(140, 75);
  backButton.setColors(GRAY1, BLACK, WHITE);
  backButton.visible = true;
  backButton.setText("Home");
  backButton.setEventHandler(&drawHomeBtn);
  backButton.init();
  backButton.setDebounce(1000);
  canvas.add(&monitorButton, 15, 150);
  canvas.add(&settingsButton, 165, 65);
  canvas.add(&otherButton, 165, 150);
  canvas.add(&backButton, 15, 65);

  //}

}
void drawTopBar(int p) {
  Tft.fillRectangle(0, 0, 320, 40, GRAY2);
  switch (p) {
    case 1:
      Tft.drawString("Menu", 135, 7, 2, WHITE);  backButton.setSize(80, 40);
      backButton.setColors(RED, WHITE, GRAY2);
      backButton.visible = true;
      backButton.setText("back");
      backButton.init();
      backButton.setDebounce(1000);
      backButton.setEventHandler(&drawHomeBtn);
      break;
    case 111:
      Tft.drawString("SD Card", 115, 7, 2, WHITE);
      backButton.setSize(80, 40);
      backButton.setColors(RED, WHITE, GRAY2);
      backButton.visible = true;
      backButton.setText("back");
      backButton.init();
      backButton.setDebounce(1000);
      backButton.setEventHandler(&drawSettingsButton);
      break;
    case 11:
      Tft.drawString("Settings", 110, 7, 2, WHITE);
      menuButton.setSize(80, 40);
      menuButton.setColors(RED, WHITE, GRAY2);
      menuButton.setText("Menu");
      menuButton.setEventHandler(&drawMenuButton);
      menuButton.init();
      menuButton.visible = true;
      menuButton.setDebounce(1000);
      canvas.add(&menuButton, 0, 0);
      break;
  }

  if (curScreen != 1 || curScreen != 111) {
    canvas.add(&backButton, 0, 0);
  }
}
void drawHomeBtn(Button * btn) {
  if (backButton.visible) {
    clearmenu(0);

    drawHome();
  }
}
void popupSDError() {
  popup.setSize(220, 100);
  popup.setColors(RED, BLACK, WHITE);
  popup.setEventHandler(&clearSC);
  popup.type = POPUP_OK;
  popup.init();
  popup.visible = true;
  // popup.setDebounce(100);

  canvas.add(&popup, 50, 70);
}
void clearSC(Popup * pup, unsigned char val) {
  canvas.redraw();
}
void drawMonitor(int p ) {

}
void drawOtherButton(Button *btn) {
  clearmenu(1);
  drawMenu(2);
}
void drawSettingsButton(Button *btn) {
  Serial.print("draw menu");
  clearmenu(1);
  drawSettings(1);
}
void drawSettings(int p) {
  switch (p) {
    case 1:
      Tft.fillRectangle(0, 0, 320, 240, WHITE);
      curScreen = 11;
      //if (menuButton.visible) {
      //clearhome();
      backButton.setEventHandler(&drawHomeBtn);
      drawTopBar(11);
      Serial.println("draw menu");
      monitorButton.setSize(140, 75);
      monitorButton.setColors(GRAY1, BLACK, WHITE);
      monitorButton.setText("About");
      monitorButton.setEventHandler(&drawAboutButton);
      monitorButton.init();
      monitorButton.visible = true;
      settingsButton.setSize(140, 75);
      settingsButton.setColors(GRAY1, BLACK, WHITE);
      settingsButton.setText("Connections");
      settingsButton.setEventHandler(&drawConnectionsButton);
      settingsButton.init();
      settingsButton.visible = true;
      otherButton.setSize(140, 75);
      otherButton.setColors(GRAY1, BLACK, WHITE);
      otherButton.setText("More");
      otherButton.setEventHandler(&drawMoreButton);
      otherButton.init();
      otherButton.visible = true;
      emergancyButton.setSize(140, 75);
      emergancyButton.setColors(GRAY1, BLACK, WHITE);
      emergancyButton.setText("Emergancy");
      emergancyButton.setEventHandler(&drawMoreButton);
      emergancyButton.init();
      emergancyButtonvisible = true;
      backButton.setSize(140, 75);
      backButton.setColors(GRAY1, BLACK, WHITE);
      backButton.visible = true;
      backButton.setText("SD");
      backButton.setEventHandler(&drawSDButton);
      backButton.init();
      backButton.setDebounce(1000);
      canvas.add(&monitorButton, 15, 150);
      canvas.add(&settingsButton, 165, 65);
      canvas.add(&otherButton, 165, 150);
      canvas.add(&backButton, 15, 65);
      canvas.add(&emergancyButton, 110, 90);
      break;
    case 3:

      break;
  }
}
void drawAboutButton(Button *btn) {

}
void drawConnectionsButton(Button *btn) {

}
void drawMoreButton(Button *btn) {

}
void drawSDButton(Button *btn) {
  curScreen = 111;
  clearSettings();
  drawSD();
}
void drawSD() {

  Tft.fillRectangle(0, 0, 320, 240, BLACK);
  drawTopBar(111);
  gauge.setSize(50, 120);
  gauge.setColors(GRAY2, YELLOW, WHITE);
  gauge.setLimits(0, 50, 100);
  gauge.init();
  gauge.setHiLimit(75, GREEN);
  gauge.setLowLimit(25, RED);

  erase.setSize(190, 50);
  erase.setColors(RED, WHITE, BLACK);
  erase.setText("ERASE");
  erase.setEventHandler(&SDErase);
  erase.init();
  erase.setDebounce(1000);
  canvas.add(&gauge, 15, 65);
  canvas.add(&erase, 120, 165);
}
void clearSettings() {
  backButton.visible = false;
  monitorButton.visible = false;
  settingsButton.visible = false;
  otherButton.visible = false;
  disp.clear();
  Tft.fillRectangle(0, 0, 320, 240, BLACK);
  prevScreen = 11;
}
void drawMonitorButton(Button *btn) {

}
void SDErase(Button *btn) {

}
int getSDInfo(int i) {

}

void remoteCommand(){
  char mode=Serial1.read();
  if (mode=='c'){
    int side = Serial1.parseInt();
    int dist = Serial1.parseInt();
    updateGuides(side,dist);
  }else if(mode=='w'){

  }
}


void localCommand(){

}
void updateGuides(int side,int dist){

}
