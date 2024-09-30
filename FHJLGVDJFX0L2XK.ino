
#include <SD.h>                   //this library included to could use SD card module
#include <TMRpcm.h>               //this library included to play sounds".wav" on speaker
#include <Wire.h>                 //this library included to could use I2C interface with LCD
#include <LiquidCrystal_I2C.h>    //this library included to work on LCD which depend on I2C interface in transmitting and receiving data


#define SD_ChipSelectPin   8     //define chip selected pin of SD card module 

#define pin1  3                   //these are the Arduino pins that we use to activate coils 1-4 of the stepper motor
#define pin2  4
#define pin3  5
#define pin4  6

#define delaytime 12             //delay time in ms to control the stepper motor delaytime.
                                 //Our tests showed that 8 is about the fastest that can yield reliable operation w/o missing steps
#define counts_per_pill  35

#define hours   0                //this define related to location byte of hours in EEPROM
#define minutes 1                //this define related to location byte of minutes in EEPROM
#define AM_PM   2                //this define related to loaction of AM/PM byte in EEPROM
#define AM      0
#define PM      1

#define connecting    'C'
#define disconnecting 'D'
#define cancel        'X'
#define set           'S'
#define Exit          'e'

#define pill_1   'a'
#define P1_state  5
#define P1_hour   6
#define P1_minute 7
#define P1_AM_PM  8
#define pill_2    'b'
#define P2_state  10
#define P2_hour   11
#define P2_minute 12
#define P2_AM_PM  13
#define pill_3    'c'
#define P3_state  15
#define P3_hour   16
#define P3_minute 17
#define P3_AM_PM  18
#define pill_4    'd'
#define P4_state  20
#define P4_hour   21
#define P4_minute 22
#define P4_AM_PM  23
#define pill_5    'e'
#define P5_state  25
#define P5_hour   26
#define P5_minute 27
#define P5_AM_PM  28
#define pill_6    'f'
#define P6_state  30
#define P6_hour   31
#define P6_minute 32
#define P6_AM_PM  33
#define pill_7    'g'
#define P7_state  35
#define P7_hour   36
#define P7_minute 37
#define P7_AM_PM  38
#define pill_8    'h'
#define P8_state  40
#define P8_hour   41
#define P8_minute 42
#define P8_AM_PM  43
#define pill_9    'k'
#define P9_state  45
#define P9_hour   46
#define P9_minute 47
#define P9_AM_PM  48
#define pill_10   'l'
#define P10_state  50
#define P10_hour   51
#define P10_minute 52
#define P10_AM_PM  53
#define pill_11   'm'
#define P11_state  55
#define P11_hour   56
#define P11_minute 57
#define P11_AM_PM  58
#define pill_12   'n'
#define P12_state  60
#define P12_hour   61
#define P12_minute 62
#define P12_AM_PM  63
#define pill_13   'o'
#define P13_state  65
#define P13_hour   66
#define P13_minute 67
#define P13_AM_PM  68
#define pill_14   'p'
#define P14_state  70
#define P14_hour   71
#define P14_minute 72
#define P14_AM_PM  73

#define empty       0
#define setted      1


TMRpcm tmrpcm;                    //create object for speaker device
LiquidCrystal_I2C lcd(0x27,16,2); //create object named by lcd to use it later during working on lcd
                                  //first argment is the address of lcd which is 0x27
                                  //second argument is the number of columns which is 16
                                  //third argument is the number of row which is 2

void welcome_message(void);       //this function to show welcome message
void wait_app_connecting(void);   //this function used to run in void setup only for one time to wait connecting to 
                                  //application to set the time because setting time is very important point and pill
                                  //does not work until user set the time
void wait_setting_time(void);     //this function used to get time from user through app then save it in EEPROM to reuse later
void start_time_counting(void);   //this function used to start counting of time
void device_ready_to_work(void);  //this function to till the user that the device is ready to use 
void time_showing(void);          //this function show current time on LCD
void time_setting(char pill_hour_location);          //this function used to get time of cells from user then set it in specified cell space
int app_command(void);            //this function return last comman received from app
char check_pill_state(char pill); //this function used to check if pill cell is setted or not 
void check_previous_pill(char pill);//this function used to check if previous cell is setted or not
void stepper_moving(void);          //this function used to rotate stepper motor
void Step_A(void);                  //the following four functionts related to motion of stepper motor
void Step_B(void);
void Step_C(void);
void Step_D(void);
void forward(void);
volatile int counter=0;           //this counter related to time calculation process
volatile char Minutes,Hours,carry,ampm; //this globale variables used inside timer interrupt for temporary saving and manipulations
volatile byte memory[100]={0};          //this array of 100 byte to save time and state of each pill 
char app_state='C';                     // this variable used as indicator to the state of android application to know if it connected or disconnected
void setup() {
  // put your setup code here, to run once:
  
  
  lcd.init();                     //initialize the lcd
                                  //Note: This .init() method also starts the I2C bus, i.e. there does not need to be
                                  //a separate "Wire.begin();" statement in the setup.
  lcd.backlight();                //turn on the backlight of LCD
  

  
  Serial.begin(9600);            //set buadrate of UART communication to 9600
 
  pinMode(pin1, OUTPUT);         //the next four lines to define pins which used to control stepper motor
  pinMode(pin2, OUTPUT); 
  pinMode(pin3, OUTPUT); 
  pinMode(pin4, OUTPUT);


  if (!SD.begin(SD_ChipSelectPin)) {      //see if the card is present and can be initialized
    
    return;                               //don't do anything more if not
  }
  
  tmrpcm.speakerPin = 9;         //select output pin of speaker
  tmrpcm.setVolume(6);           //0 to 7. Set volume level
   
  


  welcome_message();            //this to show welcome message
  wait_app_connecting();        //this to wait connecting to application
  wait_setting_time();          //this function called to get time from user
  start_time_counting();        //this function called directly after setting the time to start counting time
  device_ready_to_work();       //run to notify the user that the pill is ready to work
}

void loop() {
  // put your main code here, to run repeatedly:

  int state, command;
  command = app_command();
  state = app_state;
  while(state == connecting){
    if( command == pill_1){
      check_pill_state(P1_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P1_hour);
       // next_outting_pill();
      }
    }else if( command == pill_2){
      if(check_pill_state(P2_state))
        check_previous_pill(P2_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P2_hour);
      }
    }else if( command == pill_3){
      if(check_pill_state(P3_state))
        check_previous_pill(P3_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P3_hour);
      }
    }else if( command == pill_4){
      if(check_pill_state(P4_state))
        check_previous_pill(P4_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P4_hour);
      }
    }else if( command == pill_5){
      if(check_pill_state(P5_state))
        check_previous_pill(P5_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P5_hour);
      }
    }else if( command == pill_6){
      if(check_pill_state(P6_state))
        check_previous_pill(P6_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P6_hour);
      }
    }else if( command == pill_7){
      if(check_pill_state(P7_state))
        check_previous_pill(P7_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P7_hour);
      }
    }else if( command == pill_8){
      if(check_pill_state(P8_state))
        check_previous_pill(P8_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P8_hour);
      }
    }else if( command == pill_9){
      if(check_pill_state(P9_state))
        check_previous_pill(P9_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P9_hour);
      }
    }else if( command == pill_10){
      if(check_pill_state(P10_state))
        check_previous_pill(P10_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P10_hour);
      }
    }else if( command == pill_11){
      if(check_pill_state(P11_state))
        check_previous_pill(P11_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P11_hour);
      }
    }else if( command == pill_12){
      if(check_pill_state(P12_state))
        check_previous_pill(P12_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P12_hour);
      }
    }else if( command == pill_13){
      if(check_pill_state(P13_state))
        check_previous_pill(P13_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P13_hour);
      }
    }else if( command == pill_14){
      if(check_pill_state(P14_state))
        check_previous_pill(P14_state);
      do{
        command = app_command();
        state = app_state;
      }while(!(command == cancel || command==set || state==disconnecting));
      if( command ==  cancel || command == disconnecting){
         time_showing();
      }else if( command == set){
        time_setting(P14_hour);
      }
    }else{
      time_showing();
      delay(100);
    }
    command = app_command();
    state = app_state;
  }
  time_showing();
  delay(100);

}


void start_time_counting(void){

  noInterrupts();                     //this line to disable all interrupts before initializing timer interrupt
  TCCR2A = 0;                         //clear TCCR2A register to use timer in in normal mode
  TCCR2B = 0;                         //clear TCCR2B register to use timer in normal mode
  TCNT2 = 99;                       // preload timer 255-16MHz/1024/100Hz"this value set to get overflow interrupt every 0.01 second"
  TCCR2B |= (1<<CS22)|(1<<CS21)|(1<<CS20);      //set CS20, CS21 and CS22 to set prescaler value to 1024
  TIMSK2 |= (1<<0);               //set TOIE bit to enable interrupt of over flag
  interrupts();                       //this line to enable interrupts again after finish initialization process
}

/* 
 *  implmentation of device ready to work function.
 *  this function used to show messages to users on LCD and play 
 *  sound track to notify them that the pill dispenser is ready to work. 
 */
void device_ready_to_work(void){

  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("your pill is");
  lcd.setCursor(2,1);
  lcd.print("ready to work");
  tmrpcm.play("ready.wav");
  delay(2000);
}

/*
 * implmentation of welcome message function.
 * this function used to show welcome message and play sound track for users. 
 */
void welcome_message(void){
  lcd.clear();
  lcd.print("welcome to pill");
  lcd.setCursor(3,1);
  lcd.print("dispenser");
  tmrpcm.play("welcome.wav");
  delay(2000);
  
}

/*
 * implmentation of wait app connecting function.
 * this function to notify users to connect pill dispenser divce with android app to could set time for device and pills 
 */
void wait_app_connecting(void){
  char reading=0;
  tmrpcm.play("app.wav");
  lcd.clear();
  lcd.print("  connect app");
  lcd.setCursor(3,1);
  lcd.print("to start");
  delay(2000);
}

/*
 * implmentation of wait setting time.
 * this function used to receive currently time from user.
 */
void wait_setting_time(void){
  char reading=0,counter=0;                     //one of these two variables used to save coming data temporary on it and the another used as counter.
  int data_array[7];                            //create this array to save received time data on it.
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("time setting");
  tmrpcm.play("settingTime.wav");
  do{                                         //this do while loop used to receive data from android app through using bluetooth module 
    if(Serial.available()){                   //then save this data on data array to reuse it later.
      reading = Serial.read();
      data_array[counter]=reading-'0';
      counter=(counter+1)%7;
    }
    
  }while(reading != Exit);                     //read data until device receive 'e',indicator for exit command, from user
  memory[hours]= data_array[0]*10 + data_array[1];      //the following three lines to manipulate the received data which saved in data array to save it as time
  memory[minutes]=data_array[3]*10 + data_array[4];     //in the coreponding location in array.
  memory[AM_PM]=(data_array[5] == ('A'-'0'))?AM:PM;
  lcd.setCursor(4,1);
  if(memory[hours]<10)
    lcd.print(0);
  lcd.print(memory[hours]);
  lcd.print(":");
  if(memory[minutes]<10)
    lcd.print(0);
  lcd.print(memory[minutes]);
  lcd.print(" ");
  if(memory[AM_PM] == AM){
    lcd.print("AM");
  }else{
    lcd.print("PM");
  }
  delay(2000);
}

/*
 * implmentation of time showing function.
 * this function have two roles:
 * firstly, it used to show currently time and time of next ready pill to outting if it setted on LCD for user. 
 * secondly, it compare the currently time to time of next raedy outting pill, so if it is the time for outting the stepper moving function 
 *           called to rotate the stepper motor to out the ready pill.
 */
void time_showing(void){
  static char counter=5;    //this counter variable used to know which pill is ready to out.
  lcd.clear();              //the next 16 line related to showing currently time on LCD.
  lcd.setCursor(0,0);
  lcd.print("Time ");
  lcd.setCursor(8,0);
  if(memory[hours]<10)
    lcd.print(0);
  lcd.print(memory[hours]);
  lcd.print(":");
  if(memory[minutes]<10)
    lcd.print(0);
  lcd.print(memory[minutes]);
  lcd.print(" ");
  if(memory[AM_PM] == AM){
    lcd.print("AM");
  }else{
    lcd.print("PM");
  }
  if(memory[counter] == 1){           //this if conditions is used to check if there pill setted before and still on pill dispenser or not

    lcd.setCursor(0,1);               //the next 15 line to show the time of next ready outting pill on LCD. 
    lcd.print("next");
    lcd.setCursor(8,1);
    if(memory[counter+1]<10)
      lcd.print(0);
    lcd.print(memory[counter+1]);
    lcd.print(":");
    if(memory[counter+2]<10)
      lcd.print(0);
    lcd.print(memory[counter+2]);
    lcd.print(" ");
    if(memory[counter+3] == AM){
      lcd.print("AM");
    }else{
    lcd.print("PM");
    }

    if(memory[hours] == memory[counter+1]){         //the following three if conditions to compare currently time 'hours, minutes and AM or PM' to
      if(memory[minutes] == memory[counter+2]){     //the next ready outting pill
        if( memory[AM_PM] == memory[counter+3]){
          if(counter>=10)                           
            memory[counter-5]=0;
          lcd.clear();                             //the following lines to show message for user on LCD to notify about ready pill for outting.
          lcd.print("pill outting now");
          lcd.setCursor(3,1);
          if(memory[counter+1]<10)
          lcd.print(0);
          lcd.print(memory[counter+1]);
          lcd.print(":");
          if(memory[counter+2]<10)
          lcd.print(0);
          lcd.print(memory[counter+2]);
          lcd.print(" ");
          if(memory[counter+3] == AM){
          lcd.print("AM");
          }else{
          lcd.print("PM");
          }
          tmrpcm.play("Dr.wav");
          stepper_moving();                        //calling stepper moving function to rotate the motor.
          counter = (counter+5)%75;                //this line to increament counter value by 5 until 70
          if(counter == 0)                         //this if condition used to reset counter value to 5 
            counter = 5;
        }
      }
    }
    
  }else{                                          //this else statements used to show -- incase of no pill seted on pill dispenser device
    lcd.setCursor(0,1);
    lcd.print("next");
    lcd.setCursor(8,1);
    lcd.print("--:-- --");
  }
  
}

/*
 * implementation of time setting function.
 * this function get hour memory location of pill which required to set it as argument 
 * to save its which users will set in the predetermined locations of time.  
 */

void time_setting(char pill_hour_location){
  char reading=0,counter=0;
  int data_array[7];
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("time setting");
  tmrpcm.play("pillTime.wav");
  do{                                           //this do while loop to receive time from user and save in in data array 
    if(Serial.available()){
      reading = Serial.read();
      data_array[counter]=reading-'0';
      counter=(counter+1)%7;
    }
    
  }while(!(reading == Exit || reading == cancel));
  if(reading == Exit){                                              //this if condition to know if user enter time or not.
    memory[pill_hour_location-1]=1;                                   //this line to set state of this pill by one
    memory[pill_hour_location]= data_array[0]*10 + data_array[1];     //the following three lines to manipulate the received data which saved in data array to save 
    memory[pill_hour_location+1]=data_array[3]*10 + data_array[4];    //it as time in the coresponding locations 
    memory[pill_hour_location+2]=(data_array[5] == ('A'-'0'))?AM:PM;
    lcd.setCursor(4,1);                                               //the following 14 line to show time on LCD
    if(memory[pill_hour_location]<10)
      lcd.print(0);
    lcd.print(memory[pill_hour_location]);
    lcd.print(":");
    if(memory[pill_hour_location+1]<10)
      lcd.print(0);
    lcd.print(memory[pill_hour_location+1]);
    lcd.print(" ");
    if(memory[pill_hour_location+2] == AM){
      lcd.print("AM");
    }else{
      lcd.print("PM");
    }
    delay(2000);
  }
}
/*
 * implmentation of app command function.
 * this function used to get commands from user through android app and determine the state of bluetooth
 * if it connecting or disconnecting
 */

int app_command(void){

  char reading=0;
  if(Serial.available()){
    reading = Serial.read();
    if(reading == disconnecting){
      app_state = disconnecting;
    }else if(app_state == disconnecting){
      app_state = connecting;
    }
  }
  return reading;
}

/*
 * implmentation of check pill state function.
 * this function get pill state location as argument to check if this pill setted before or not.
 * if it setted before LCD will show messages for users to notify them this pill was setted before
 * and if not setted before also will show messages for users to notify them the cell is empty. 
 */

char check_pill_state(char pill){
  char reading = memory[pill];
  char state=0;
  if(reading == empty){
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("this cell is");
    lcd.setCursor(4,1);
    lcd.print("empty");
    tmrpcm.play("empty.wav");
    delay(2000);
    state=1;
  }else{
    lcd.clear();
    lcd.print("setted before at");
    lcd.setCursor(2,1);
    if(memory[pill+1]<10)
      lcd.print(0);
    lcd.print(memory[pill+1]);
    lcd.print(":");
    if(memory[pill+2]<10)
      lcd.print(0);
    lcd.print(memory[pill+2]);
    lcd.print(" ");
    if(memory[pill+3] == AM){
      lcd.print("AM");
    }else{
      lcd.print("PM");
    }
    tmrpcm.play("setted.wav");
    delay(2000);
  }
  return state;
}

/*
 *  implmentation of check previous pill function.
 *  this function used to check pill's cells which is before one that will be set is empity or not.
 */

void check_previous_pill(char pill){
  
  if(memory[pill-5] == 0){
    char available_pill=pill-5;
    lcd.clear();
    lcd.print("you need to set");
    lcd.setCursor(1,1);
    lcd.print("pill ");
    tmrpcm.play("previous.wav");
    while((memory[available_pill] == 0) && (available_pill !=0)){
       available_pill-=5;
    }
    lcd.print((available_pill+5)/5);
    lcd.print(" first");
    delay(4000);
  }
}

/*
 * the following 5 functions related to motion of stepper motor.
 */

void stepper_moving(void){
  for(int counter=0; counter<counts_per_pill; counter++){
    forward();
  }
}
void Step_A(){
  digitalWrite(pin1, HIGH);//turn on coil 1 
  digitalWrite(pin2, LOW); 
  digitalWrite(pin3, LOW); 
  digitalWrite(pin4, LOW); 
}
void Step_B(){
  digitalWrite(pin1, LOW); 
  digitalWrite(pin2, HIGH);//turn on coil 2
  digitalWrite(pin3, LOW); 
  digitalWrite(pin4, LOW); 
}
void Step_C(){
  digitalWrite(pin1, LOW); 
  digitalWrite(pin2, LOW); 
  digitalWrite(pin3, HIGH); //turn on coil 3
  digitalWrite(pin4, LOW); 
}
void Step_D(){
  digitalWrite(pin1, LOW); 
  digitalWrite(pin2, LOW); 
  digitalWrite(pin3, LOW); 
  digitalWrite(pin4, HIGH); //turn on coil 4
}
void step_OFF(){
  digitalWrite(pin1, LOW); //power all coils down
  digitalWrite(pin2, LOW); 
  digitalWrite(pin3, LOW); 
  digitalWrite(pin4, LOW); 
}

void forward(){//one tooth forward
  Step_A();
  delay(delaytime);
  Step_B();
  delay(delaytime);
  Step_C();
  delay(delaytime);
  Step_D();
  delay(delaytime);
}
/*
 * interrupt service rotuine of timer2 over flow interrupt.
 */
ISR(TIMER2_OVF_vect){                 
  
  TCNT2 = 99;                       //loading timer counter by 99 to get 0.01 second for each over flow interrupt.
  counter++;
  if(counter == 6000){             //if counter value reach to 6000 this mean one minute passed so statements of this if condition to update 
    counter =0;                    //time and save the updated time numbers in their specified location in memory array.
    Minutes = memory[minutes];
    Hours = memory[hours];
    ampm = memory[AM_PM];
    carry = (Minutes+1)/60;
    
    Minutes = (Minutes+1)%60;
    if(carry){
      Hours=Hours+1;
      if(Hours==12)
        memory[AM_PM]=ampm ^ 1;
      if(Hours==13)
        Hours=1;
    }
    memory[minutes]=Minutes;
    memory[hours]=Hours;   
  }
}
