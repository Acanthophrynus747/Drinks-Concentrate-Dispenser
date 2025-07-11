/*
Program for a secondary Arduino Uno that sends serial characters
to the motor control arduino when buttons are pressed. The Arduino
on which this program runs should be powered through a connection
to the Vin and GND (ground) pins from the 5V supply and GND pins on 
the motor controlling arduino. Serial data is connected through the 
Tx and Rx pins. The series of buttons are connected to ground by 
the wire included on the custom button circuit board, and each button
also wired to the pin included in the below code. 
*/

#include <Arduino.h> //arduino features

//pins for controller buttons
const int purge = 2;
const int order = 3;

const int rec1 = 4;
const int rec2 = 5;
const int rec3 = 6;
const int rec4 = 7;

const int tall = 8;
const int grande = 9;
const int venti = 10;
const int trenta = 11;

//messages that will be sent over serial
char write_msg;
char read_msg; 

bool sent = false;

//function prototypes
bool pressed(int);
char getMsg(void);

void setup() {
  //initialize serial communication with motor controlling arduino
  Serial.begin(9600); 

  //all buttons are input pullups
  //the pins will read LOW when buttons are pressed
  pinMode(purge, INPUT_PULLUP);
  pinMode(order, INPUT_PULLUP);

  pinMode(rec1, INPUT_PULLUP);
  pinMode(rec2, INPUT_PULLUP);
  pinMode(rec3, INPUT_PULLUP);
  pinMode(rec4, INPUT_PULLUP);

  pinMode(tall, INPUT_PULLUP);
  pinMode(grande, INPUT_PULLUP);
  pinMode(venti, INPUT_PULLUP);
  pinMode(trenta, INPUT_PULLUP);
}

void loop() {
  
  if (sent == false){
    write_msg = getMsg(); //read which control button has been pressed
    Serial.write(write_msg); //write the message to the serial connection
    sent = true;
    delay(200);
  }

  write_msg = getMsg(); 
  Serial.write(write_msg);


  read_msg = Serial.read();
  delay(200);
}

//function to make reading buttons more efficient to put in code
bool pressed(int pin){
  if (digitalRead(pin) == LOW){
    //buttons read LOW when pressed
    return true;
  }
  else{
    return false; 
  }
}

//function to read which button is pressed and decide the corresponding character to send by serial
char getMsg(){
  char msg;

  if (pressed(purge)){
    msg = 'P';
  }
  else if (pressed(order)){
    msg = 'O';
  }

  else if (pressed(rec1)){
    msg = 'A';
  }
  else if (pressed(rec2)){
    msg = 'B';
  }
  else if (pressed(rec3)){
    msg = 'C';
  }
  else if (pressed(rec4)){
    msg = 'D';
  }

  else if (pressed(tall)){
    msg = 'W';
  }
  else if (pressed(grande)){
    msg = 'X';
  }
  else if (pressed(venti)){
    msg = 'Y';
  }
  else if (pressed(trenta)){
    msg = 'Z';
  }

  return msg;
}