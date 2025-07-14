/*
This is program for the Arduino Uno which controls the pumps. It takes commands from either the keyboard of a connected PC,
or from the other Arduino uno fitted with buttons and connected through a serial connection. 

Code by Jack Samson, Washington State University, spring 2025.
Permanent email address: Jacksbuilds@gmail.com

Feel free to reach out with any questions regarding the microprocessor code or electrical systems of the refreshers pump prototype

Resources for troubleshooting and further information:
    HW-039 Motor Controller information: https://www.handsontec.com/dataspecs/module/BTS7960%20Motor%20Driver.pdf
    Tutorial on Arduino serial communication: https://forum.arduino.cc/t/serial-input-basics-updated/382007

Troubleshooting (add more to this section as issues and fixes are discovered):
    -When uploading any program to the arduino, the program will not upload if the serial cable is connected. Unplug the serial from
        ports 0 and 1 on the Arduino, then reupload and plug back in when done. 
*/

//arduino functionality
#include <Arduino.h>

//pins for motors and solenoids
const int pump1_pin = 11;
const int pump2_pin = 10;
const int pump3_pin = 9;
const int pump4_pin = 8;

/*currenty not implemented. A relay could be added to control a solenoid to
turn on water flow from a tap water supply and add water to the drinks. Some of the functionality
to do this is included in the drink ordering and size ordering states below.*/
//const int water_valve_pin 

//const int cleaner_pump_pin = 73; //currently unused because no cleaner reservior is currently used

//some variables used later
bool initial_activation = true;
bool selected = false;
bool size_selected = false;

//arduino pin outputs for each flavor pump
const int strawb_acai_pump = 3;
const int mango_dragon_pump = 4;
const int lemon_pump = 5; 
const int passion_pump = 6; 

const float flow_rate = 2.5; //oz per sec that pumps provide

const int pump_power = 255; /*pwm level for proper flow rate. 
We ended up using the power supply voltage to adjust pump power 
but this can be changed to do it a different way.*/

int input; //drink ordering input

//an enum that defines all the possible states of the state machine
enum StateMachineState {
    STANDBY = 0,
    TEST = 1,
    CLEANING = 2,
    DRINK_ORDERING = 3, 
    SIZE_ORDERING = 4
};

/*Structs are similar to classes, and contain varibales for different characteristics. In this case,
the struct defining the drink recipes contain a variable for the name of the drink and an array for the quantity
of each concentrate in the drink*/
struct baseRecipe {
    String name; //name of the recipe. used for reference and printing
    float quantities[5][2]; //quantities of each ingredient. one row is an index and one is the amount
};

/*array that conatins an index, the first number in the pair {0, 12} for example
and the size of each drink size in ounces. Tall, grande, venti, and trenta are included*/
int drink_sizes[4][2] = {{0, 12}, {1, 16}, {2, 20}, {3, 30}};

/*Recipes are stored in an array of recipe structs, defined earlier. Theoretically, any number of recipes could be 
added to this array, although there are only enough buttons in the current configuration for four.*/
baseRecipe recipe_array[4] {
    //quantities contained in the arrays should be in ounces of concentrate per ounce of finished drink
    {"test drink", {{0, 0.25}, {1, 0.25}, {2, 0.25}, {3, 0.25}, {4, 0}}}, //currently this recipe will run all pumps sequentially
    {"lemonade", {{0, 0.5}, {1, 0}, {2, 0}, {3, 0.5}, {4, 0}}},
    {"passionfruit lemonade", {{0, 0}, {1, 0.25}, {2, 0.25}, {3, 0}, {4, 0}}},
    {"mango dragonfruit", {{0, 0}, {1, 0}, {2, 0}, {3, 0.5}, {4, 0}}}
};


StateMachineState state; //initialize the state machine 

//function prototypes. functions are declared here and then fully coded later, after the void loop()
void runPumps(int, int, int, int, int);
void pumpsOn(bool, bool, bool, bool, bool);
//state function prototypes
void runStandbyState(void);
void runTestState(void);
void runCleaningState(void);
void runOrderingState(void);
void runSizeOrderingState(void);

void setup(){

    /*initialize the serial connection. Used to optionally send messages to the computer terminal, 
    and to communicate with the separate arduino currently used to take button inputs.*/
    Serial.begin(9600);

    /*Setting the mode of pump control pins to output. This helps avoid errors 
    if the arduino was accidentally programmed to read from these pins*/
    pinMode(pump1_pin, OUTPUT);
    pinMode(pump2_pin, OUTPUT);
    pinMode(pump3_pin, OUTPUT);
    pinMode(pump4_pin, OUTPUT);

    //write all devices low at the beginning of the program to guard against running out of control
    digitalWrite(pump1_pin, LOW);
    digitalWrite(pump2_pin, LOW);
    digitalWrite(pump3_pin, LOW);
    digitalWrite(pump4_pin, LOW);
}

void loop(){

    /*The core of a state machine-based arduino program, and in this case the only thing contained in the built in loop() function,
    is the switch that executes the different state functions depending on the value of the state variable. The state variable begins 
    in this case in the standby state, which contains logic to change the state variable and thus cause the switch() to execute a different
    state function on the next loop. Each state function should thus also switch the state to a different value. The logic of doing so
    governs the overall flow of the state machine operation.*/

    switch(state){
        case(STANDBY):
        runStandbyState();
        break;

        case(TEST):
        runTestState();
        break; 

        case(CLEANING):
        runCleaningState();
        break;

        case(DRINK_ORDERING):
        selected = false; //order hasn not been made yet
        runOrderingState();
        break;

        case(SIZE_ORDERING):
        runSizeOrderingState();
        break;
    }
}

/*The two following functions runPumps() and pumpsOn(), both serve to turn the pumps on and off, but in different ways. 
The runPumps() function takes inputs of time for each pump to run, and then shuts the pumps off after the commanded time has elapsed.
The pumps are ran one at a time to ensure consistent flow rate and for simplicity. If desired,  */

void runPumps(int pump1time, int pump2time, int pump3time, int pump4time, int pump5time){
    Serial.print("..."); //these printouts just serve to indicate that the corresponding pumps are supposed to be working
    analogWrite(pump1_pin, pump_power);
    delay(pump1time * 1000);
    analogWrite(pump1_pin, 0);

    Serial.print("...");
    analogWrite(pump2_pin, pump_power);
    delay(pump2time * 1000);
    analogWrite(pump2_pin, 0);

    Serial.print("...");
    analogWrite(pump3_pin, pump_power);
    delay(pump3time * 1000);
    analogWrite(pump3_pin, 0);

    Serial.print("...");
    analogWrite(pump4_pin, pump_power);
    delay(pump4time * 1000);
    analogWrite(pump4_pin, 0);

    //pump 5 currently unused

}

void pumpsOn(bool p1_on, bool p2_on, bool p3_on, bool p4_on, bool p5_on){
    if(p1_on){
        analogWrite(pump1_pin, pump_power); //pump on if true
        Serial.println("pump 1 on"); //for testing
    }
    else{
        analogWrite(pump1_pin, 0); //turn pump off if false
        Serial.println("pump 1 off"); //for testing
    }

    if(p2_on){
        analogWrite(pump2_pin, pump_power); //pump on if true
        Serial.println("pump 2 on");
    }
    else{
        analogWrite(pump2_pin, 0); //turn pump off if false
        Serial.println("pump 2 off");
    }

    if(p3_on){
        analogWrite(pump3_pin, pump_power); //pump on if true
        Serial.println("pump 3 on");
    }
    else{
        analogWrite(pump3_pin, 0); //turn pump off if false
        Serial.println("pump 3 off");
    }

    if(p4_on){
        analogWrite(pump4_pin, pump_power); //pump on if true
        Serial.println("pump 4 on");
    }
    else{
        analogWrite(pump4_pin, 0); //turn pump off if false
        Serial.println("pump 4 off");
    }

    ////currently unused. could be used if a cleaner reservoir and corresponding pump was added
    // if(p5_on){
    //     analogWrite(cleaner_pump_pin, pump_power); //pump on if true
    // }
    // else{
    //     analogWrite(cleaner_pump_pin, 0); //turn pump off if false
    // }
}

void printouts(String input){

    /*Function to make printouts to the serial monitor more efficient. These currently do not do anything if the device is
    not connected by USB to a computer, as they only print to the serial monitor in the Arduino IDE or similar programming environment*/

    if (input == "STANDBY"){
        Serial.println("------------------Awaiting instructions------------------------");
        Serial.println("Press P to purge cycle, or press O for drink ordering");
    }
    else if (input == "TEST"){

    }
    else if (input == "ORDERING"){
        Serial.println("------------------DRINK ORDERING--------------------------------");
        Serial.print("press A for ");
        Serial.print(recipe_array[0].name);
        Serial.print(", B for ");
        Serial.println(recipe_array[1].name);
        Serial.print(", C for ");
        Serial.println(recipe_array[2].name);
        Serial.print(", or D for ");
        Serial.println(recipe_array[3].name);
    }

    else if (input == "sizing"){
        Serial.println("press W for tall, X for grande, Y for venti, or Z for trenta");
    }
}

void runStandbyState(){
    /*This state is the base mode to handle commands that send the machine into other modes.
    Eventually, all other states will lead back to this after completion. Code for each state is
    contained in its respective state function. This void runStandbyState() is the state function
    for the standby state.*/

    if (initial_activation == true){
        printouts("STANDBY");
        initial_activation = false;
    }

    char input = Serial.read();

    if (input == 'V'){ 
        /*Currently unused state. It is retained in the code because the state function
        could be modified for testing or troubleshooting as desired. The character 'V' is
        not sent by the button controller from any button press, so to use this state a
        USB connection with a PC could be used for keyboard input. */
        state = TEST;
    }
    else if (input == 'P'){
        //Send the device into the purging state
        state = CLEANING;
    }
    else if (input == 'O'){
        //drink ordering state. after drink ordering the program will proceed to the next state to specify drink size
        printouts("ORDERING");
        state = DRINK_ORDERING;
    }
    else{
        state = STANDBY; //loop back to standby if input isnt either
    }
}

void runTestState(){

    Serial.println("empty placeholder state, sending back to standby");
    printouts("STANDBY");
    state = STANDBY;
}

void runCleaningState(){

/*This state serves to either prime the pumps by eliminating air from the tubing, or to pump cleaner through
the system with all pumps. All 4 concentrate pumps will run simultaneously until the button is pressed again,
which will turn them off. */

  Serial.print("."); //only used to display on computer terminal to show it is running. optional
  delay(50); //not technically necessary just makes the terminal output look nicer
  bool pressed;
  char input = Serial.read();

  pumpsOn(true, true, true, true, false); //turn all concentrate pumps on

  if (input == 'P'){
    pressed = true;
  }
  else{
    pressed = false;
  }

  if (pressed == true){ //if the purging button is pressed
    pumpsOn(false, false, false, false, false); //shut pumps off
    state = STANDBY; //send back to standby
    Serial.println("done");
  }
}

void runOrderingState(){

    char drink_input = Serial.read();

    //inputs to select recipe
    if (drink_input == 'A'){
        input = 0;
        selected = true; 
    }
    if (drink_input == 'B'){
        input = 1; 
        selected = true;
    }
    if (drink_input == 'C'){
        input = 2;
        selected = true;
    }
    if (drink_input == 'D'){
        input = 3;
        selected = true;
    }
    else {
        state = DRINK_ORDERING; //loop back to drink ordering state if nothing received yet
    }

    if (selected){
        Serial.print(recipe_array[input].name);
        Serial.println(" selected");

        for (int i = 0; i <= 4; i++){
            Serial.println(recipe_array[input].quantities[i][1]);
        }

        size_selected = false; 

        printouts("sizing");
        state = SIZE_ORDERING;
    }
}

void runSizeOrderingState(){
        
    bool size_selected = false;

    int size;
    int ounces;

    //read from serial (button input arduino or usb connection to PC) for input
    char size_input = Serial.read();

    //inputs to select recipe size
    if (size_input == 'W'){
        Serial.println("one selected");
        size_selected = true;
        size = 0;
    }
    else if (size_input == 'X'){
        size_selected = true;
        size = 1;
    }
    else if (size_input == 'Y'){
        size_selected = true;
        size = 2; 
    }
    else if (size_input == 'Z'){
        size_selected = true;
        size = 3;
    }

    if (size_selected == true){
        ounces = drink_sizes[size][1];

        // Serial.print("ounces:"); //for troubleshooting
        // Serial.println(ounces);

        float pump_1_oz = recipe_array[input].quantities[0][1] * ounces;
        float pump_2_oz = recipe_array[input].quantities[1][1] * ounces; 
        float pump_3_oz = recipe_array[input].quantities[2][1] * ounces; 
        float pump_4_oz = recipe_array[input].quantities[3][1] * ounces; 
        float water_oz = recipe_array[input].quantities[4][1] * ounces;

        //calculate run times from quantity and flow rate
        float pump_1_time = pump_1_oz / flow_rate;
        float pump_2_time = pump_2_oz / flow_rate;
        float pump_3_time = pump_3_oz / flow_rate;
        float pump_4_time = pump_4_oz / flow_rate;
        //float water_flow_time = water_oz / flow_rate; //unused for possible later water dispensing implementation

        //for troubleshooting
        Serial.println("pump oz:");
        Serial.println(pump_1_oz); 
        Serial.println(pump_2_oz); 
        Serial.println(pump_3_oz); 
        Serial.println(pump_4_oz); 
        Serial.println(water_oz);

        Serial.println("run times:");
        Serial.println(pump_1_time);
        Serial.println(pump_2_time);
        Serial.println(pump_3_time);
        Serial.println(pump_4_time);

        //running the pumps for the calculated neccesary time
        runPumps(pump_1_time, pump_2_time, pump_3_time, pump_4_time, 0); //last pump zero (cleaning pump)

        Serial.println("finished");

        //return to standby after dispensing is done
        printouts("STANDBY");
        state = STANDBY;
    }
}