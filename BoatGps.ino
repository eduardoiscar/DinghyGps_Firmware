
// Include application, user and local libraries
#include "Energia.h"
#include "SPI.h"
//#include "LCD_5110_SPI.h"
#include "LCD_5110.h"
#include <SD.h>

#include <FastDigitalWrite.h>
#include <LCD_documentation.h>
#include <LCD_graphics.h>
#include <LCD_GUI.h>
#include <LCD_screen.h>
#include <LCD_screen_font.h>
#include <LCD_utilities.h>
#include <Screen_K35.h>
#include <Template_screen.h>
#include <Template_screen_font.h>
#include <Terminal12e.h>
#include <Terminal16e.h>
#include <Terminal6e.h>
#include <Terminal8e.h>


boolean result; 
uint8_t chipSelectPin = PB_1;
int8_t SPI_Port = 3; 
uint8_t sckRateID = SPI_QUARTER_SPEED;
int8_t cardDetectionPin = -1;
int8_t level = LOW;

   
File root;
Screen_K35 myScreen;


struct time {
  int hour; 
  int minute; 
  float second; 
};

struct gpsdata {
  float latitude;
  char  latitudeSign[2]; 
  float longitude;
  char  longitudeSign[2];
  float knots;
  int fixquality; 
  int satelites; 
  int horizontalDilution; 
  float altitude; 
  float goid_height; 
  float track_angle; 
  int day;
  int month;
  int year;
  float magnetic_variation;  
  time timestamp; 
}gps,lastgps;

struct imudata {
  float roll; 
  float pitch;
  float yaw;
}imu;


// Variables
/// P._. / PB_4 = SCK (2) = Serial Clock
/// P._. / PB_7 = MOSI (2) = Serial Data

typedef enum displaystate {
  position,
  boatspeed, 
  multipledata,
};
volatile displaystate disp_state = position;
displaystate lastdisp_state =boatspeed;

enum commandstate {
  start_,
  waiting_,
  stop_, 
} command = waiting_;



/*uint8_t pinChipSelect = PA_7;
 uint8_t pinSerialClock = PA_3; 
 uint8_t pinSerialData = PA_4;
 uint8_t pinDataCommand = PA_2;
uint8_t pinReset = PB_5;
uint8_t pinBacklight = PA_6;
uint8_t pinPushButton = PUSH2;


LCD_5110 myScreen (pinChipSelect, 
                   pinSerialClock,  
                   pinSerialData,  
                   pinDataCommand, 
                   pinReset, 
                   pinBacklight,
                   pinPushButton);*/

boolean	backlight = true;
uint8_t k = 0;

uint8_t ledpin = 30;
char incomingByte;

String inputString = ""; // a string to hold incoming data
boolean stringComplete = false; // whether the string is complete
String inputString2 = ""; // a string to hold incoming data
boolean string2Complete = false; // whether the string is complete
String inputString3 = ""; // a string to hold incoming data
boolean string3Complete = false; // whether the string is complete

String latitude= "Initializing";
String longitude = "Initializing";

long timeNow = 0;
long lastTime = 0; 


int button1 = 17 ; 
int button2 = 31; 

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200); //Debug port 
  Serial1.begin(9600);  //GPS 
  Serial2.begin(9600);  //Bluetooth
  Serial1.setPins(UART1_PORTC);
  Serial3.begin(57600); //IMU Port

  pinMode(ledpin,OUTPUT);
  digitalWrite(ledpin,LOW);

   inputString.reserve(200);
  // SPI.begin();
   //SPI.setClockDivider(SPI_CLOCK_DIV128); // for LM4F120H5QR DIV2 = 4 MHz !

   result =  SD.begin(chipSelectPin,sckRateID, SPI_Port,cardDetectionPin,level);
    

    Serial.print("SD card result:"); 
    Serial.println(result);
    
    

    myScreen.setFontSize(myScreen.fontMax());
    myScreen.clear(darkGrayColour);
   initDisplay();
  
  
            

  root = SD.open("/");

  
  
   pinMode(PUSH2, INPUT_PULLUP);
   attachInterrupt(PUSH2, button2interrupt, FALLING); // Interrupt is fired whenever
  
   pinMode(PUSH1, INPUT_PULLUP);
   attachInterrupt(PUSH1, button1interrupt, FALLING); // Interrupt is fired whenever
}

void loop()
{
   
   
  datalog(); 
   
  //updateDisplay();
 
  serialprocessing();

  commandAction();
  
  output(Serial);
 
}


void output(HardwareSerial &Serialdev) {
   timeNow = millis();
   static long lastTimeloc = 0; 
   if (timeNow -lastTimeloc > 100) {

  // if the file is available, write to it:
     Serial.print(millis());
     Serial.print(","); 
     Serial.print(gps.day);
     Serial.print(",");
     Serial.print(gps.month);
     Serial.print(","); 
     Serial.print(gps.year);
     Serial.print(",");
     Serial.print(gps.timestamp.hour);
     Serial.print(",");
     Serial.print(gps.timestamp.minute);
     Serial.print(",");
     Serial.print(gps.timestamp.second);
     Serial.print(",");
     Serial.print(gps.knots);
     Serial.print(",");
     Serial.print(gps.track_angle);
     Serial.print(",");
     Serial.print(gps.altitude);
     Serial.print(",");
     Serial.print(gps.latitude,5);
     Serial.print(",");
     Serial.print(gps.latitudeSign);
     Serial.print(",");
     Serial.print(gps.longitude,5);
     Serial.print(",");
     Serial.print(gps.longitudeSign);
     Serial.print(",");
     Serial.print(gps.fixquality);
     Serial.print(",");
     Serial.print(imu.roll);
     Serial.print(",");
     Serial.print(imu.pitch);
     Serial.print(",");
     Serial.println(imu.yaw);
    // Serial.println("\r\n");
     lastTimeloc = timeNow;
  }
   
}

void datalog() {
   timeNow = millis();
   if (timeNow -lastTime > 1000) {
     File dataFile = SD.open("Logging.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
     dataFile.print(gps.day);
     dataFile.print(",");
     dataFile.print(gps.month);
     dataFile.print(","); 
     dataFile.print(gps.year);
     dataFile.print(",");
     dataFile.print(gps.timestamp.hour);
     dataFile.print(",");
     dataFile.print(gps.timestamp.minute);
     dataFile.print(",");
     dataFile.print(gps.timestamp.second);
     dataFile.print(",");
     dataFile.print(gps.knots);
     dataFile.print(",");
     dataFile.print(gps.track_angle);
     dataFile.print(",");
     dataFile.print(gps.altitude);
     dataFile.print(",");
     dataFile.print(gps.latitude,5);
     dataFile.print(",");
     dataFile.print(gps.latitudeSign);
     dataFile.print(",");
     dataFile.print(gps.longitude,5);
     dataFile.print(",");
     dataFile.print(gps.longitudeSign);
     dataFile.print(",");
     dataFile.print(gps.fixquality);
     dataFile.print(",");
     dataFile.print(imu.roll);
     dataFile.print(",");
     dataFile.print(imu.pitch);
     dataFile.print(",");
     dataFile.println(imu.yaw);
    dataFile.close();
    // print to the serial port too:
   // Serial.println("Printed in file");
  }
  // if the file isn't open, pop up an error:
  else {
    //Serial.println("error opening TEST.txt");
  }
      lastTime = timeNow; 
   }
}


void serialprocessing() {
   if(stringComplete) {
  stringComplete= false; 
  nmeaParser(& inputString);
  //Serial.print(inputString); 
  inputString = ""; 
  }
  
  if(string2Complete) {
  string2Complete= false; 
  processComand(&inputString2); 
  inputString2 = ""; 
  }  
  
  if(string3Complete) {
  string3Complete= false; 
  imuParser(&inputString3); 
  inputString3 = ""; 
  }  
}

void initDisplay() {
    myScreen.begin();
    myScreen.setFontSize(myScreen.fontMax());
    myScreen.clear(darkGrayColour);
   
    myScreen.gText(100 +2, 100 +2, "Hello World");
 
}
/*void initDisplay() {
   myScreen.begin();
   myScreen.setFont(1); 
   myScreen.setBacklight(backlight);
   myScreen.text(0, 1, "Eduboat");
   myScreen.text(3, 3, "GPS"); 
   delay(1000);
   myScreen.clear();
}*/

void commandAction() {

static long now = 0; 
static long lasttime = 0;  
  
now = millis(); 

if(now-lasttime>1000){
  lasttime = now; 
  switch(command) {
   case stop_: 
    command = waiting_; 
   break; 
   case start_:
     Serial2.print(gps.day);
     Serial2.print(",");
     Serial2.print(gps.month);
     Serial2.print(","); 
     Serial2.print(gps.year);
     Serial2.print(",");
     Serial2.print(gps.timestamp.hour);
     Serial2.print(",");
     Serial2.print(gps.timestamp.minute);
     Serial2.print(",");
     Serial2.print(gps.timestamp.second);
     Serial2.print(",");
     Serial2.print(gps.knots);
     Serial2.print(",");
     Serial2.print(gps.track_angle);
     Serial2.print(",");
     Serial2.print(gps.altitude);
     Serial2.print(",");
     Serial2.print(gps.latitude,5);
     Serial2.print(",");
     Serial2.print(gps.latitudeSign);
     Serial2.print(",");
     Serial2.print(gps.longitude,5);
     Serial2.print(",");
     Serial2.print(gps.longitudeSign);
     Serial2.print(",");
     Serial2.println(gps.fixquality);

   break; 
   case waiting_:
     //Do nothing
     break; 
   default: 
     command = waiting_;
     break; 
  }
}
  
} 

void processComand(String *string) {
 if(!string->startsWith("$")) {
    return; 
  }
 
   if(string->startsWith("STOP",1)) {
    command = stop_; 
   }
   if(string->startsWith("START",1)) {
    command = start_;
   }
}

/*void updateDisplay() {
  
static long now = 0; 
static long lasttime = 0;  
displaystate disp_state_temp = disp_state;
now = millis();
if (now -lasttime>500) {
  switch (disp_state){
    case position: 
       
        if (lastdisp_state != disp_state_temp) {
        myScreen.setFont(0);    
        myScreen.clear();  
        myScreen.text(0,0,"Latitude:");
        myScreen.text(0,2,"Longitude");
        myScreen.text(0,4,"Altitude");
        }
        
        myScreen.text(0, 1,  float2string(gps.latitude,7,2));      
        myScreen.text(0, 3,   float2string(gps.longitude,7,2));  
        myScreen.text(0, 5,   float2string(gps.altitude,4,2));
        lastdisp_state = disp_state_temp;
   
      break; 
   
    case boatspeed: 
        if (lastdisp_state != disp_state_temp) {
        myScreen.clear();
        myScreen.setFont(0);  
        myScreen.text(3, 0,  "Speed");
        }
        myScreen.setFont(1);
        myScreen.text(3,2,float2string(gps.knots,4,1));
        lastdisp_state = disp_state_temp;
         
   break;
    case multipledata: 
        if (lastdisp_state != disp_state_temp) {
        myScreen.clear();
        myScreen.setFont(0);  
        myScreen.text(0, 0,  "Speed");
        myScreen.text(0, 1,  "Satelites");
        myScreen.text(0, 2,  "Time");
        myScreen.text(0, 3,  "Date");
        myScreen.text(0, 4,  "Fix Qual");
        myScreen.text(0, 5,  "Mag Var");
        }
        
        myScreen.text(6,0,float2string(gps.knots,4,1));
        myScreen.text(10,1,String(gps.satelites));        
        myScreen.text(5,2,String(gps.timestamp.hour)+":");
        myScreen.text(8,2,String(gps.timestamp.minute)+":");
        myScreen.text(11,2,String((int)gps.timestamp.second));        
        myScreen.text(5,3,String(gps.day)+"/");
        myScreen.text(8,3,String(gps.month)+"/");
        myScreen.text(11,3,String(gps.year));    
        myScreen.text(9,4,String(gps.fixquality));      
        myScreen.text(8,5,float2string(gps.magnetic_variation,5,2));
        lastdisp_state = disp_state_temp;
       
   break;
   default: 
        myScreen.clear();
    
  }
  lasttime = now;
  lastgps = gps; 
   
    
}
}
*/
void serialEvent1() {
  while (Serial1.available()) {
    // get the new byte:
    char inChar = (char)Serial1.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
    stringComplete = true;
    break;
    } 
  }
}

void serialEvent2() {
  while (Serial2.available()) {
    // get the new byte:
    char inChar = (char)Serial2.read(); 
    // add it to the inputString:
    inputString2 += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
    string2Complete = true;
    break;
    } 
  }
}

void serialEvent3() {
  while (Serial3.available()) {
    // get the new byte:
    char inChar = (char)Serial3.read();
    // add it to the inputString:
    inputString3 += inChar;
    //Serial.write(inChar);
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
    string3Complete = true;
    break;
    } 
  }
}

boolean nmeaParser(String* nmeaString) {
  //Check is the string starts with "$"
  if(!nmeaString->startsWith("$")) {
   return false; 
  }
  //Check for message type GPGGA
   if(nmeaString->startsWith("GPGGA",1)) {
    // Serial.println("Valid GPGGA sentence");
     //Get the positions of the comas
     int comaPosition[14] = {0};
     for (int i = 0; i<14;i++){
       if (i==0)  comaPosition[i] = nmeaString->indexOf(','); 
       else       comaPosition[i] = nmeaString->indexOf(',',comaPosition[i-1]+1); 
     }
     
    
    
    gps.timestamp.hour = string2int(nmeaString->substring(comaPosition[0]+1,comaPosition[0]+3));
    gps.timestamp.minute = string2int(nmeaString->substring(comaPosition[0]+3,comaPosition[0]+5));
    gps.timestamp.second = string2float(nmeaString->substring(comaPosition[0]+5,comaPosition[1])); 
    gps.fixquality = string2int(nmeaString->substring(comaPosition[5]+1,comaPosition[6])); 
    gps.satelites = string2int(nmeaString->substring(comaPosition[6]+1,comaPosition[7]));
    gps.horizontalDilution = string2int(nmeaString->substring(comaPosition[7]+1,comaPosition[8]));
    gps.altitude = string2float(nmeaString->substring(comaPosition[8]+1,comaPosition[9]));
    gps.goid_height = string2float(nmeaString->substring(comaPosition[9]+1,comaPosition[10]));


  }
  if(nmeaString->startsWith("GPRMC",1)) {
    // Serial.println("Valid GPRMC sentence");
     //Get the positions of the comas
     int comaPosition[11] = {0};
     for (int i = 0; i<11;i++){
       if (i==0)  comaPosition[i] = nmeaString->indexOf(','); 
       else       comaPosition[i] = nmeaString->indexOf(',',comaPosition[i-1]+1); 
     }
    
    //Parse the Message  
    gps.timestamp.hour = string2int(nmeaString->substring(comaPosition[0]+1,comaPosition[0]+3));
    gps.timestamp.minute = string2int(nmeaString->substring(comaPosition[0]+3,comaPosition[0]+5));
    gps.timestamp.second = string2float(nmeaString->substring(comaPosition[0]+5,comaPosition[1]));  
    gps.latitude = string2float(nmeaString->substring(comaPosition[2]+1,comaPosition[3]));   
    nmeaString->substring(comaPosition[3]+1,comaPosition[4]).toCharArray(gps.latitudeSign,2);
    gps.longitude =string2float(nmeaString->substring(comaPosition[4]+1,comaPosition[5])); 
    nmeaString->substring(comaPosition[5]+1,comaPosition[6]).toCharArray(gps.longitudeSign,2);
    gps.knots = string2float(nmeaString->substring(comaPosition[6]+1,comaPosition[7]));
    gps.track_angle = string2float(nmeaString->substring(comaPosition[7]+1,comaPosition[8]));
    gps.day = string2int(nmeaString->substring(comaPosition[8]+1,comaPosition[8]+3));
    gps.month = string2int(nmeaString->substring(comaPosition[8]+3,comaPosition[8]+5));
    gps.year = string2int(nmeaString->substring(comaPosition[8]+5,comaPosition[9]));
    gps.magnetic_variation = string2float(nmeaString->substring(comaPosition[9]+1,comaPosition[10]));
  
  }
 
}

boolean imuParser(String* imuString) {
  //Check is the string starts with "$"
  if(!imuString->startsWith("#")) {
   return false; 
  }
  //Check for message type GPGGA
   if(imuString->startsWith("YPR=",1)) {
    // Serial.println("Valid IMU sentence sentence"); //Very noisy 
     //Get the positions of the comas
     int comaPosition[2] = {0};
     for (int i = 0; i<2;i++){
       if (i==0)  comaPosition[i] = imuString->indexOf(','); 
       else       comaPosition[i] = imuString->indexOf(',',comaPosition[i-1]+1); 
     }
    
    
    
    imu.yaw = string2float(imuString->substring(5,comaPosition[0]));
    imu.pitch = string2float(imuString->substring(comaPosition[0]+1,comaPosition[1]));
    imu.roll = string2float(imuString->substring(comaPosition[1]+1,imuString->length()-2));



  }
  
 
}


void button2interrupt() {
digitalWrite(ledpin,HIGH);
//disp_state = position;

static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) 
  {
   switch (disp_state) {
     case position:
      disp_state = boatspeed; 
      break; 
     case boatspeed:
       disp_state = multipledata; 
       break;  
     case multipledata: 
       disp_state = position; 
   break; 
  }
  last_interrupt_time = interrupt_time;


  
}
}

void button1interrupt() {
disp_state = boatspeed;
digitalWrite(ledpin, LOW);
}


int string2int(String string) {
 
 char charbuf [20];
 string.toCharArray(charbuf,20);
 return atoi(charbuf);
 
}


float string2float(String string) {
 char charbuf [20];
 string.toCharArray(charbuf,20);
 return atof(charbuf); 
}

String float2string(float var,int minStringWidthIncDecimalPoint,int numVarsAfterDecimal) {
char charbuf[10];
dtostrf(var, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charbuf);
return charbuf;
}

char *dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}


void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}
    




