#include <EEPROM.h>

#define  switch_pin A0 //pin of bd2222
//#define  sensor_pin A3 //pin of the touch sensor
#define  sensor_pin 5 //pin of the touch sensor
#define  readVoltage_pin A5 //pin to check voltage (V/3)

int trans_pin = 10; //pin of n-transistor mosfet
int clock_pin = 3; // pin 3 on TLC5916
int sdi_pin = 2; // pin 2 on TLC5916
int le_pin = 4;  // pin 4 on TLC5916

int num_leds = 5; //number of using leds
int update = 0; //used for monitoring the trasmission of data to the leds only when it's necessary (when there us a change in the number)
int number0 = 1;
int led[5]; //counts the number of touches
unsigned long t0 = 0;

void setup() {
  pinMode(le_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  pinMode(sdi_pin, OUTPUT);
  pinMode(trans_pin, OUTPUT);
  pinMode(switch_pin, OUTPUT);
  pinMode(sensor_pin, INPUT);
  pinMode(readVoltage_pin, INPUT);

  digitalWrite(switch_pin, HIGH); //makes the bd2222 high side switch providing 5V to the TLC5916 chips (put them on-HIGH / off-LOW)
  digitalWrite(trans_pin, HIGH); //makes the transistor enabling the 15V step-up converter (put the leds on-HIGH / off-LOW)
  
  for(int i=0; i<num_leds; i++){
    led[i]=EEPROM.read(i);
  }
}

void loop(){
  //increments the count number because of a new touches
  if(digitalRead(sensor_pin) == LOW){ //increment the number of the touches
    t0 = millis();
    digitalWrite(switch_pin, HIGH); //makes the bd2222 high side switch providing 5V to the TLC5916 chips (put them on-HIGH / off-LOW)
    //digitalWrite(trans_pin, HIGH); //makes the transistor enabling the 15V step-up converter (put the leds on-HIGH / off-LOW)
    if(led[4] != 9){
      led[4]++;
    }else{
      if(led[3] != 9){
        led[3]++;
        led[4]=0;
      }else{
        if(led[2] != 9){
          led[2]++;
          led[3]=0;
          led[4]=0;
        }else{
          if(led[1] != 9){
            led[1]++;
            led[2]=0;
            led[3]=0;
            led[4]=0;
          }else{
            if(led[0] != 9){
              led[0]++;
              led[1]=0;
              led[2]=0;
              led[3]=0;
              led[4]=0;
            }else{
        //final leds view 99999             
            }
          }
        }
      }
    }
    update = 1;
    //eeprom saves the number
    for(int i=0; i<num_leds; i++){
      EEPROM.update(i, led[i]);      
    }
  }

  //check if the number of touches==0 --> starting position (bash) 
  int k=0;
  while(number0 && k<num_leds){
    if(led[k]!=0){
      number0=0;
    }
    k++;
  }

  //starting position of leds (bash)
  if(number0){
    digitalWrite(le_pin, LOW); //hold low until the trasmission ends
    for(int i=0; i<num_leds; i++){
      shiftOut(sdi_pin, clock_pin, LSBFIRST, convertNumberToPattern(10)); //shift a byte of data, a bit per clock's rising edge
    }
    digitalWrite(le_pin, HIGH); //send the data to the outpu
  }  

  //updates the leds by the new touch 
  if(update){
    digitalWrite(le_pin, LOW); //hold low until the trasmission ends
    int zero_before_number = 1; //boolean value
    //cycle to write the whole number on the leds (starting on the left)
    for(int i=0; i<num_leds; i++){
      if(led[i]==0 && zero_before_number){
        shiftOut(sdi_pin, clock_pin, LSBFIRST, convertNumberToPattern(11)); //shift a byte of data, a bit per clock's rising edge
      }else{
        shiftOut(sdi_pin, clock_pin, LSBFIRST, convertNumberToPattern(led[i])); //shift a byte of data, a bit per clock's rising edge
        zero_before_number = 0;
      }
    }
    digitalWrite(le_pin, HIGH); //send the data to the output
    delay(10000); //delay of 10 sec. between touches
    update = 0;
  }

  //put the leds off after 10 minutes from the touch 600000
  if(millis() - t0 > 600000){
     digitalWrite(switch_pin, LOW); //makes the bd2222 high side switch providing 5V to the TLC5916 chips (put them on-HIGH / off-LOW)
     //digitalWrite(trans_pin, LOW); //makes the transistor enabling the 15V step-up converter (put the leds on-HIGH / off-LOW)
  }
  
}

// This function converts a number into the 7-segment pattern.
unsigned char convertNumberToPattern (unsigned char number) {
    // what is the number?
    switch (number) {
        // numbers       edc.bafg
        case 0: return 0b11101110;
        case 1: return 0b00101000;
        case 2: return 0b11001101;
        case 3: return 0b01101101;
        case 4: return 0b00101011;
        case 5: return 0b01100111;
        case 6: return 0b11100111;
        case 7: return 0b00101100;
        case 8: return 0b11101111;
        case 9: return 0b01101111;
        case 10: return 0b00000001; //starting position (bash)
        
        // default symbol is an empty space
        default: return 0b00000000;
    }
}