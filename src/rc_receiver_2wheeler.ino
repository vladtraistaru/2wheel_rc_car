
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define CH1 4
#define CH2 5 //PWM
#define CH3 6 //PWM
#define CH4 7
#define CH5 8
#define CH6 9 //PWM

int maxSpeed = 200;

int dcMotor1En = CH2; //pwm
int dcMotor1pin1 = CH1;
int dcMotor1pin2 = CH3;

int dcMotor2pin1 = CH4;
int dcMotor2pin2 = CH5;
int dcMotor2En = CH6;  //PWM

RF24 radio(3, 2);   // nRF24L01 (CE, CSN)
const byte address[6] = "00001";
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

// Max size of this struct is 32 bytes
struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j1Button;
  byte j2PotX;
  byte j2PotY;
  byte j2Button;
  byte pot1;
  byte pot2;
  byte tSwitch1;
  byte tSwitch2;
  byte button1;
  byte button2;
  byte button3;
  byte button4;
};

Data_Package data;

void setup() {

  Serial.begin(9600);

  pinMode(dcMotor1pin1, OUTPUT);
  pinMode(dcMotor1pin2, OUTPUT);
  pinMode(dcMotor1En, OUTPUT);

  pinMode(dcMotor2pin1, OUTPUT);
  pinMode(dcMotor2pin2, OUTPUT);
  pinMode(dcMotor2En, OUTPUT);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
  resetData();
}

void loop() {
  
  // Check whether we keep receving data, or we have a connection between the two modules
  currentTime = millis();
  if ( currentTime - lastReceiveTime > 1000 ) { // If current time is more then 1 second since we have recived the last data, that means we have lost connection
    resetData(); // If connection is lost, reset the data. It prevents unwanted behavior, for example if a drone jas a throttle up, if we lose connection it can keep flying away if we dont reset the function
  }
  // Check whether there is data to be received
  if (radio.available()) {
    radio.read(&data, sizeof(Data_Package)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
  }
  
  logData(data);

  if(data.tSwitch2 == 0) //up
  {
    //separate wheel drive
    driveMotorX(data.j2PotY, dcMotor1pin1, dcMotor1pin2, dcMotor1En, data.pot2);
    driveMotorX(data.j1PotY, dcMotor2pin2, dcMotor2pin1, dcMotor2En, data.pot2);
  }
  else
  {
    //both wheels
    driveMotorX(data.j2PotY, dcMotor1pin1, dcMotor1pin2, dcMotor1En, data.pot2);
    driveMotorX(data.j2PotY, dcMotor2pin2, dcMotor2pin1, dcMotor2En, data.pot2);
  }

  
}

void driveMotorX(int position, int motorPin1, int motorPin2, int motorPinEnable, int maxSpeed)
{
  int throttle = position;  
  
  if(throttle > 127 && throttle < 132) //middle is stop. Why is this middle ??
  {
    analogWrite(motorPinEnable, 0);
  }
  else if(throttle >= 131)
  {
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    analogWrite(motorPinEnable, map(throttle-127, 0, 128, 0, maxSpeed));
  }
  else if(throttle <= 127)
  {
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    analogWrite(motorPinEnable, map(127-throttle, 0, 127, 0, maxSpeed));
  }
}
void logData(Data_Package data)
{ 
  Serial.println("data.pot2:  " + String(data.pot2, DEC));
  //Serial.println("throttle: " + String(throttle, DEC));
}


void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  data.j1PotX = 127;
  data.j1PotY = 127;
  data.j2PotX = 127;
  data.j2PotY = 127;
  data.j1Button = 1;
  data.j2Button = 1;
  data.pot1 = 1;
  data.pot2 = 1;
  data.tSwitch1 = 1;
  data.tSwitch2 = 1;
  data.button1 = 1;
  data.button2 = 1;
  data.button3 = 1;
  data.button4 = 1;
}
