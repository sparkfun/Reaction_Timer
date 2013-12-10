/*
 Reaction Timer for the World of Wonder Childrens museum in Lafayette, CO
 By: Nathan Seidle (SparkFun Electronics)
 Date: May 6th, 2013
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Theory of operation: 
 A large button is pulsating. Once pressed the game begins. Once the button illuminates a second time the user 
 must hit the button as fast as they can. The reaction time is displayed on a 7 segment display.
 
 Maybe add a buzzer?
 
 */

#include <SoftwareSerial.h>
SoftwareSerial segmentDisplay(3, 2); // RX, TX

int LED = 9;
int button = 7;

long timeDiff; //Global variable keeps track of your score
int idleLoops = 0;

//These functions allow the LED to be really bright when on, and 
//just barely on when the game is in idle mode
#define LEDON() analogWrite(LED, 255)
#define LEDLOW() analogWrite(LED, 10)
#define LEDOFF() analogWrite(LED, 0)

void setup()
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  LEDOFF(); //Turn off LED

  pinMode(button, INPUT_PULLUP);

  randomSeed(analogRead(A1)); //Get noise to seed the random number generator

  segmentDisplay.begin(9600); //Talk to the Serial7Segment at 9600 bps
  segmentDisplay.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display

  scrollTitle();
}

void loop()
{
  if(digitalRead(button) == LOW)
  {
    playGame();
    idleLoops = 0;
  }

  //If there is no game going on, pulse the LED on/off
  for(int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5)
  {
    if(digitalRead(button) == LOW) break;

    analogWrite(LED, fadeValue);
    delay(30);
  }
  for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5)
  {
    if(digitalRead(button) == LOW) break;

    analogWrite(LED, fadeValue);
    delay(30);
  }

  //Turn LED off for awhile
  for(int x = 0 ; x < 100 ; x++)
  {
    if(digitalRead(button) == LOW) break;

    analogWrite(LED, 0);
    delay(30);
  }
  
  idleLoops++;
  if(idleLoops > 10)
  {
    //Screen saver = display title
    scrollTitle();
    segmentDisplay.write('v'); //Reset the display
    segmentDisplay.print(timeDiff); //Reprint the last score 
  }
}

void playGame()
{
  segmentDisplay.write('v'); //Reset the display

  while(digitalRead(button) == LOW) ; //Wait for user to stop hitting button  

  delay(20);  // delay to debounce the button

  LEDLOW(); //Turn LED on low to indicate the beginning of the game

  //Get random number of milliseconds
  long lightTime = random(2000, 3500);

  long zeroTime = millis();

  while(millis() - zeroTime < lightTime) //Wait for random amount of time to go by
  {
    //If the user hits the button in this time then error out (cheater!)
    if(digitalRead(button) == LOW)
    {
      segmentDisplay.write('v'); //Reset the display
      segmentDisplay.print("-ERR");
      blinkButton();
      return;
    }

  }

  //Begin game
  LEDON();
  long beginTime = millis(); //Record this as the beginning of the test

  while(digitalRead(button) == HIGH)
  {
    //Update the display
    timeDiff = millis() - beginTime;
    //Serial.println(timeDiff);

    //No user interaction for 10 seconds
    if(timeDiff > 9999)
    {
      timeDiff = 9999;
      segmentDisplay.write('v'); //Reset the display
      segmentDisplay.print(timeDiff);
      blinkButton();
      return;
    }
  }

  String disTime = String(timeDiff);

  if(timeDiff < 1000) disTime = " " + disTime;

  segmentDisplay.write('v'); //Reset the display
  segmentDisplay.print(disTime);

  //After the game is complete, display the time for awhile
  blinkButton();  

}

//Quickly blinks to button indicating the end of a game
void blinkButton()
{
  for(int x = 0 ; x < 7 ; x++)
  {
    LEDON();
    delay(60);
    LEDOFF();
    delay(60);
  }
}

//Quickly scrolls the title across the display
void scrollTitle()
{
  String titleStr = String("    reaction speed    ");

  for(int x = 0 ; x < 19 ; x++)
  {
    String tempStr = titleStr.substring(x, x+4);

    segmentDisplay.write('v'); //Reset the display
    segmentDisplay.print(tempStr);

    for(int y = 0 ; y < 25 ; y++)
    {
      if(digitalRead(button) == LOW) return; //If the button is pressed, bail!
      
      delay(10);
    }
  }

}

