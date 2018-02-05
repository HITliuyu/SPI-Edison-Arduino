// Written by Yu Liu
// Sept. 2017


#include <SPI.h>
#include "DHT.h"
#include <Wire.h> //I2C
#include "MutichannelGasSensor.h"
#include <SoftwareSerial.h>
#include "Ultrasonic.h"
#include "Arduino.h"
#include "SI114X.h" //sunlight

#define DHTPIN A0 
#define DHTTYPE DHT11   // DHT 11 
#define sensor s_serial
#define SS 10 //slave select pin
#define BUFFSIZE 30
//union definition
union cvtfloat {
    float val;
    unsigned char bytes[4];
} myfloat;
union cvtint {
    int val;
    unsigned char bytes[2];
} myint;
union cvtlong {
    long val;
    unsigned char bytes[4];
} mylong;

//temperature and humidity
DHT dht(DHTPIN, DHTTYPE);

//CO2 sensor
const unsigned char cmd_get_sensor[] =
{
    0xff, 0x01, 0x86, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x79
};
unsigned char dataRevice[9];
int CO2PPM;
SoftwareSerial s_serial(5, 6);      // RX, TX

//ultrasonic sensor
Ultrasonic ultrasonic(7);
long RangeInCM;

//PM sensor
int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 15000;//sampe 30s&nbsp;;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

//sunlight sensor
SI114X sunlight = SI114X();

//buffer used to store sensor data
//|0.humidity|1.temperature|2.3.4.5.ratio0|6.7.8.9.ratio1|10.11.12.13.ratio2|14.15.CO2PPM|16.17.18.19.RangInCM|20.21.22.23.concentration|
//|24.25.light.visible|26.27.light.IR|28.29.light.UV*100|
uint8_t sensorData [BUFFSIZE];

byte command = 0;
unsigned long cycle;
int i;
byte checksum;

void setup (void)
{
    Serial.begin (9600);   // debugging
   
     //tempurature and humidity sensor
    dht.begin();
    
    //multichannel gas sensor
    gas.begin(0x04);//the default I2C address of the slave is 0x04
    gas.powerOn();
    Serial.println(gas.getVersion());
  
    //CO2 sensor
    sensor.begin(9600);
  
    //ultrasonic range sensor
    //None
  
    //PM sensor
    pinMode(pin,INPUT);
    starttime = millis();//get the current time;

    //sunlight sensor
    if(!sunlight.Begin())
    {
      sunlight.Begin();
    }
    //variable initialization
    cycle = 0;
    i = 0;
    checksum = 0;
    // turn on SPI in slave mode
    SPCR |= bit (SPE);
  
    // have to send on master in, *slave out*
    pinMode(MISO, OUTPUT);
  
    // now turn on interrupts
    //  SPI.attachInterrupt();
     SPCR |= _BV(SPIE);
  
     // interrupt for SS falling edge
//    attachInterrupt (0, resetParam, FALLING);
    attachInterrupt (0, resetParam, RISING);// we reset parameters when SS is released
    

}  // end of setup


// SPI interrupt routine
ISR (SPI_STC_vect)
{
    byte c = SPDR;  // grab byte from SPI Data Register
    switch (command)
    {
    // no command? then this is the command
    case 0:
      command = c;
      SPDR = 0;
      break;
      
    // transmit buffered data
    case 'a':
      if (i < BUFFSIZE)
      {
          SPDR = sensorData[i];
          checksum += sensorData[i];
          i++;
      }
      else
      {
          SPDR = (checksum & 0xff);      
      } 
      break;
  
    } // end of switch
    // cycle++;
        
}  // end of interrupt routine SPI_STC_vect

// main loop - wait for flag set in interrupt routine
void loop (void)
{
    Serial.print("enter ISR: ");Serial.println(cycle);
    //temperature and humidity
    sensorData[0] = dht.readHumidity();
    sensorData[1] = dht.readTemperature();
    Serial.print("Real sensor data: ");
    Serial.print(sensorData[0]);
    Serial.print("   ");
    Serial.println(sensorData[1]);

     //multichannel sensor
    float c;
    int j; //convert float to char counter
/*    
    c = gas.measure_NH3();
    Serial.print("The concentration of NH3 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
  
    c = gas.measure_CO();
    Serial.print("The concentration of CO is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
  
    c = gas.measure_NO2();
    Serial.print("The concentration of NO2 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
  
    c = gas.measure_C3H8();
    Serial.print("The concentration of C3H8 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
  
    c = gas.measure_C4H10();
    Serial.print("The concentration of C4H10 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
  
    c = gas.measure_CH4();
    Serial.print("The concentration of CH4 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
  
    c = gas.measure_H2();
    Serial.print("The concentration of H2 is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
  
    c = gas.measure_C2H5OH();
    Serial.print("The concentration of C2H5OH is ");
    if(c>=0) Serial.print(c);
    else Serial.print("invalid");
    Serial.println(" ppm");
*/
    //display ratio values
    c = gas.measure_RATIO_0();
    Serial.print("The ratio0 is ");
    if(c>=0) 
    {
        Serial.print(c);
        myfloat.val = c;
        for(j = 0; j < 4; j++)
        {
            sensorData[j+2] = myfloat.bytes[j];
        }
    }
    else Serial.print("invalid");
    Serial.println(" ");

    c = gas.measure_RATIO_1();
    Serial.print("The ratio1 is ");
    if(c>=0) 
    {
        Serial.print(c);
        myfloat.val = c;
        for(j = 0; j < 4; j++)
        {
            sensorData[j+6] = myfloat.bytes[j];
        }
    }
    else Serial.print("invalid");
    Serial.println(" ");
    
    
    c = gas.measure_RATIO_2();
    Serial.print("The ratio2 is ");
    if(c>=0) 
    {
        Serial.print(c);
        myfloat.val = c;
        for(j = 0; j < 4; j++)
        {
            sensorData[j+10] = myfloat.bytes[j];
        }
    }
    else Serial.print("invalid");
    Serial.println(" ");
    
    //CO2 sensor
    if(CO2dataRecieve())
    {
        Serial.print("  CO2: ");
        Serial.print(CO2PPM);
        Serial.println("");
        myint.val = CO2PPM;
        sensorData[14] = myint.bytes[0];
        sensorData[15] = myint.bytes[1];
    }
  
    //ultrasonic sensor
    RangeInCM = ultrasonic.MeasureInCentimeters();
    Serial.print("Range is ");
    Serial.print(RangeInCM);
    Serial.print(" cm\n");
    mylong.val = RangeInCM;
    for(j = 0; j < 4; j++)
    {
      sensorData[j+16] = mylong.bytes[j];
    }
    
    //PM sensor
    duration = pulseIn(pin, LOW);
    lowpulseoccupancy = lowpulseoccupancy+duration;
  
    if ((millis()-starttime) >= sampletime_ms)//if the sampel time = = sampletime_ms
    {
        ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=&gt;100
        concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
        myfloat.val = concentration;
        for(j = 0; j < 4; j++)
        {
          sensorData[j+20] = myfloat.bytes[j];
        }
        Serial.print("ratio = ");
        Serial.print(ratio);
        Serial.print("\t");
        Serial.print("concentration = ");
        Serial.print(concentration);
        Serial.println(" pcs/0.01cf");
        Serial.println("\n");
        lowpulseoccupancy = 0;
        starttime = millis();
    }

    //sunlight sensor
    myint.val = sunlight.ReadVisible();
    sensorData[24] = myint.bytes[0];
    sensorData[25] = myint.bytes[1];
    Serial.print("Vis: "); Serial.println(myint.val);
        
    myint.val = sunlight.ReadIR();
    sensorData[26] = myint.bytes[0];
    sensorData[27] = myint.bytes[1];
    Serial.print("IR: "); Serial.println(myint.val);
    
    //the real UV value must be div 100 from the reg value , datasheet for more information.
    myint.val = sunlight.ReadUV();
    sensorData[28] = myint.bytes[0];
    sensorData[29] = myint.bytes[1];
    Serial.print("UV: "); Serial.println((float)myint.val/100);
    
    delay(3000);
 }  // end of loop

 // start of transaction, no command yet
void resetParam ()
{
  command = 0;
  i = 0;
  checksum = 0;
}  // end of interrupt service routine (ISR) resetParam

//CO2 sensor data
bool CO2dataRecieve(void)
{
    byte data[9];
    int i = 0;

    //transmit command data
    for(i=0; i<sizeof(cmd_get_sensor); i++)
    {
        sensor.write(cmd_get_sensor[i]);
    }
    delay(10);
    //begin reveiceing data
    if(sensor.available())
    {
        while(sensor.available())
        {
            for(int i=0;i<9; i++)
            {
                data[i] = sensor.read();
            }
        }
    }

    for(int j=0; j<9; j++)
    {
        Serial.print(data[j]);
        Serial.print(" ");
    }
    Serial.println("");

    if((i != 9) || (1 + (0xFF ^ (byte)(data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7]))) != data[8])
    {
        return false;
    }

    CO2PPM = (int)data[2] * 256 + (int)data[3];

    return true;
}
