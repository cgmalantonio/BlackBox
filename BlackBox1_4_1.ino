// Updated to add Pitch, Roll, and Yaw
// Thanks to: 
// Adafruit.com!
// Arthur "Arne" Spoooner from
// (http://forum.arduino.cc/index.php/topic,8573.0.html)
// Chinotech (accelerometer g-force data)
// (http://chionophilous.wordpress.com/2011/06/20/getting-started-with-accelerometers-and-micro-controllers-arduino-adxl335/)
// GPS Visualizer
// (GPSVisualizer.com)
// Ladyada (logging and gps)
// (http://ladyada.net/make/gpsshield/logging.html)
// March 30, 2014

#include <SPI.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <avr/sleep.h>
#include <Wire.h>
#include <LSM303.h>
#include <Average.h>

#include <Adafruit_NeoPixel.h>
#define PIN 54
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);
int r[9]={0}; // array for each color value
int g[9]={0};
int b[9]={0};

LSM303 compass;

HardwareSerial mySerial = Serial1;
//SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
#define GPSECHO  false
/* set to true to only log to SD when GPS has a fix, for debugging, keep it false */
#define LOG_FIXONLY true  

// Set the pins used
#define chipSelect 10
#define ledPin 13

File logfile;

// read a Hex value and return the decimal equivalent
uint8_t parseHex(char c) {
  if (c < '0')
    return 0;
  if (c <= '9')
    return c - '0';
  if (c < 'A')
    return 0;
  if (c <= 'F')
    return (c - 'A')+10;
}

// blink out an error code
void error(uint8_t errno) {
  while(1) {
    uint8_t i;
    for (i=0; i<errno; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }
    for (i=errno; i<10; i++) {
      delay(200);
    }
  }
}

float alphaAccel = 0.4;
float alphaMagnet = 0.4;

unsigned int xOffset=16;  //grabbed from getAccelOffset(); 16, 708, 699
unsigned int yOffset=708; //used for Pitch and Roll to bring variables
unsigned int zOffset=699; //to zero on a level surface


float Pitch=0;
float Roll=0;
float Yaw=0;
int xRaw=0;
int yRaw=0;
int zRaw=0;
float xFiltered=0;
float yFiltered=0;
float zFiltered=0;
float xFilteredOld=0;
float yFilteredOld=0;
float zFilteredOld=0;
float xAccel=0;
float yAccel=0;
float zAccel=0;

//these variables fine tune the g-force data on the accelerometer
//the calibration data came from BlackBoxCalibrate
int xRawMin = -16240;
int xRawMax = 15616;

int yRawMin = -17216;
int yRawMax = 16992;

int zRawMin = -14560;
int zRawMax = 18448;


void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Wire.begin();
  compass.init();
  compass.enableDefault();
  
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 or 5 Hz update rate
  GPS.sendCommand(PGCMD_NOANTENNA);
  
  strip.begin(); // prep the neopixel
  strip.setBrightness(30);
  strip.show();
  NeoStart();
  
  // make sure that the default chip select pin is set to
  // output, even if you don't use it: -Ultimate GPS Shield and SD logger
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect, 11, 12, 13)) {     
    NeoSDInitErr();
    error(2);
  }
  NeoSDInitOK();
  char filename[15];
  strcpy(filename, "blkbox00.csv");
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }

  logfile = SD.open(filename, FILE_WRITE);

  if( ! logfile ) {
    NeoSDLogErr();
    error(3);
    
  }

  NeoSDLogOK();
  logfile.println("Time, Date, Latitude, Longitude, Elevation, Speed (Knots), Angle, Satellites, Pitch, Roll, G-Force x, y, z, Direction, Orient");
  logfile.flush();

}

void loop() {
  
  char c = GPS.read();
  if (GPSECHO)
    if (c)   Serial.print(c);
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) 
      return; 

    // Sentence parsed! 
    if (LOG_FIXONLY && !GPS.fix) {
      NeoGPSFixErr();
      return;
    }
    NeoGPSFixOK();
    
    compass.read();

    int mx = compass.m.x;
    int my = compass.m.y;
    int mz = compass.m.z;

    // Here is where we handle the G-Force Calculations
    long xRaw = compass.a.x;
    long yRaw = compass.a.y;
    long zRaw = compass.a.z;

    // Convert raw values to 'milli-Gs"
    long xScaled = map(xRaw, xRawMin, xRawMax, -10000, 10000);
    long yScaled = map(yRaw, yRawMin, yRawMax, -10000, 10000);
    long zScaled = map(zRaw, zRawMin, zRawMax, -10000, 10000);

    // re-scale to fractional Gs
    float xAccel = xScaled / 10000.0;
    float yAccel = yScaled / 10000.0;
    float zAccel = zScaled / 10000.0;
    //End G-Force Calc

    //Here is where we calculate Pitch and Roll using Magnetometer
    mx = map(mx,-569 ,717 ,-643,643);
    my = map(my,-727 ,418 ,-572.5,572.5);

    float Pi = 3.14159;
    // Calculate the angle of the vector y,x
    float heading = (atan2(my,mx) * 180) / Pi;
    // Normalize to 0-360
    if (heading < 0)
    {
      heading = 360 + heading;
    }

    char compass[3];
    if (heading >= 0){
      compass[0] = 'N';
      compass[1] = ' ';
    }
    if (heading >= 23){
      compass[0] = 'N';
      compass[1] = 'E';
    }
    if (heading >= 68){
      compass[0] = 'E';
      compass[1] = ' ';
    }
    if (heading >= 113){
      compass[0] = 'S';
      compass[1] = 'E';
    }
    if (heading >= 158){
      compass[0] = 'S';
      compass[1] = ' ';
    }
    if (heading >= 203){
      compass[0] = 'S';
      compass[1] = 'W';
    }
    if (heading >= 248){
      compass[0] = 'W';
      compass[1] = ' ';
    }
    if (heading >= 293){
      compass[0] = 'N';
      compass[1] = 'W';
    }
    if (heading >=338){
      compass[0] = 'N';
      compass[1] = ' ';
    }
    
    if (compass[0] == 'N' && compass[1] == ' '){ NeoN(); }
    if (compass[0] == 'E'){ NeoE(); }
    if (compass[0] == 'S' && compass[1] == ' '){ NeoS(); }
    if (compass[0] == 'W'){ NeoW(); }
    //getAccelOffset(); //used to get offset values
    FilterAD();
    AD2Degree();
    //End Pitch and Roll   

    delay(100);


    if (logfile) {
      NeoLogOK();
 
      logfile.print(GPS.hour, DEC); 
      logfile.print(':');
      logfile.print(GPS.minute, DEC); 
      logfile.print(':');
      logfile.print(GPS.seconds, DEC); 
      logfile.print('.');
      logfile.print(GPS.milliseconds);
      logfile.print(",");

      logfile.print(GPS.month, DEC);  
      logfile.print('/');
      logfile.print(GPS.day, DEC); 
      logfile.print("/20");
      logfile.print(GPS.year, DEC);
      logfile.print(",");

      logfile.print(GPS.latitude, 4); 
      logfile.print(GPS.lat);
      logfile.print(", "); 
      logfile.print(GPS.longitude, 4); 
      logfile.print(GPS.lon);
      logfile.print(","); 
      logfile.print(GPS.altitude);
      logfile.print(","); 
      logfile.print(GPS.speed);
      logfile.print(","); 
      logfile.print(GPS.angle);
      logfile.print(","); 
      logfile.print((int)GPS.satellites);

      logfile.print(",");
      logfile.print(int(Pitch*180/PI));
      logfile.print(",");
      logfile.print(int((Roll*180/PI)+3));

      logfile.print(",");  
      logfile.print(xAccel);
      logfile.print("G, ");
      logfile.print(yAccel);
      logfile.print("G, ");
      logfile.print(zAccel);
      logfile.print("G");

      logfile.print(",");
      logfile.print(heading);
      logfile.print(",");
      logfile.print(compass[0]);
      logfile.print(compass[1]); 

      logfile.print(",");
      logfile.println();
      logfile.flush();    
    } 
    else {
      NeoLogErr();
    } 

  }
}




