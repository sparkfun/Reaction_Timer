/*
 Reaction Timer for the World of Wonder Childrens museum in Lafayette, CO
 By: Nathan Seidle (SparkFun Electronics)
 Date: May 6th, 2013
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 How to play:
 The button will be pulsing with light. Pressing the button will cause the light to turn off and the game will begin.
 After a few seconds of darkness, the light will light up and the user must press the button as fast as they can. 
 Their reaction time is displayed on a 7 segment display.
 
 If the user presses the button before it is illuminated the display will show '-Err' to indicate error.
 
 This example incorporates recording a game count to EEPROM. Each time the unit is powered the number of games is displayed.
 
 */

#include <SoftwareSerial.h>
SoftwareSerial segmentDisplay(3, 2); //RX, TX to the OpenSegment display

#include <EEPROM.h> //Used to record the number of games played

int LED = 9;
int button = 7;

long timeDiff; //Global variable keeps track of your score
int idleLoops = 0;

String gameTime; //Contains the last game time
int gamesPlayed; //Contains the total number of games played for the life of the device

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

  displayGames(); //Display the number of games played
}

void loop()
{
  if(digitalRead(button) == LOW)
  {
    Serial.println("Playing");
    playGame();
    idleLoops = 0;
  }

  pulseTheButton(); //If no one is playing, pulse LED to intice them. Function takes 6 seconds to complete.

  idleLoops++;
  if(idleLoops > 9) //Play a screen saver every 60 seconds.
  {
    scrollTitle(); //Screen saver = display title
    idleLoops = 0;
  }
}

void playGame()
{
  segmentDisplay.write('v'); //Reset the display
  
  delay(25); //Debounce the button a bit

  while(digitalRead(button) == LOW) ; //Wait for user to stop hitting button  

  LEDLOW(); //Turn LED on low to indicate the beginning of the game

  //Get random number of milliseconds
  long lightTime = random(2000, 3500); //From 2 to 3.5 seconds

  long zeroTime = millis();

  while(millis() - zeroTime < lightTime) //Wait for random amount of time to go by
  {
    //If the user hits the button in this time then error out (cheater!)
    if(digitalRead(button) == LOW)
    {
      segmentDisplay.write('v'); //Reset the display
      segmentDisplay.print("-Err");
      Serial.println("Err!");
      blinkButton();
      return;
    }
  }

  //Begin game
  Serial.println("Go!");
  LEDON();
  long beginTime = millis(); //Record this as the beginning of the test

  //Wait for user to hit the button
  while(digitalRead(button) == HIGH)
  {
    //Check to see if the user fails to respond in 10 seconds
    timeDiff = millis() - beginTime;
    if(timeDiff > 9999)
    {
      timeDiff = 9999;
      segmentDisplay.write('v'); //Reset the display
      segmentDisplay.print(timeDiff);
      blinkButton();
      return;
    }
  }

  gameTime = String(timeDiff);

  //Right adjust the time
  if(timeDiff < 10) 
    gameTime = "   " + gameTime;
  else if(timeDiff < 100) 
    gameTime = "  " + gameTime;
  else if(timeDiff < 1000) 
    gameTime = " " + gameTime;

  segmentDisplay.write('v'); //Reset the display
  segmentDisplay.print(gameTime); //Display the game time

  Serial.print("Reaction time:");
  Serial.println(gameTime);

  blinkButton(); //Blink the LED to indicate the end of the game 
  
  //Record that we have played this game
  gamesPlayed++;
  Serial.print("This time played:");
  Serial.println(gamesPlayed);
  
  EEPROM.write(0x00, gamesPlayed >> 8); //MSB
  EEPROM.write(0x01, gamesPlayed & 0x00FF); //LSB

  //After the game is complete, the display will show the gameTime for awhile
}

//If there is no game going on, pulse the LED on/off
//If the user ever presses the button then return immediately
//This function takes approximately 6 seconds to complete
void pulseTheButton(void)
{
  //Fade LED on
  for(int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5)
  {
    if(digitalRead(button) == LOW) return;

    analogWrite(LED, fadeValue);
    delay(30);
  }

  //Fade LED off
  for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5)
  {
    if(digitalRead(button) == LOW) return;

    analogWrite(LED, fadeValue);
    delay(30);
  }

  //Turn LED off for awhile
  for(int x = 0 ; x < 100 ; x++)
  {
    if(digitalRead(button) == LOW) return;

    analogWrite(LED, 0);
    delay(30);
  }
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

  for(int x = 0 ; x < titleStr.length() - 4 ; x++)
  {
    String tempStr = titleStr.substring(x, x+4); //Chop out four letters from the string

    segmentDisplay.write('v'); //Reset the display
    segmentDisplay.print(tempStr); //Display this substring

    for(int y = 0 ; y < 25 ; y++)
    {
      if(digitalRead(button) == LOW) return; //If the button is pressed, bail!
      delay(10);
    }
  }

  segmentDisplay.write('v'); //Reset the display
  segmentDisplay.print(gameTime); //Display the last game time
}

//Display the number of games played
void displayGames(void)
{
  gamesPlayed = ((int)EEPROM.read(0x00) << 8) | EEPROM.read(0x01); //Convert two 8-bit numbers into 16-bit int
  if(gamesPlayed == 0xFFFF) gamesPlayed = 0; //Ignore this initial value
  
  Serial.print("Played:");
  Serial.println(gamesPlayed);
  
  segmentDisplay.write('v'); //Reset the display
  
  String numGames = (String) gamesPlayed; //Turn this variable into a string
  numGames = "    played-" + numGames;
  
  for(int j = 0 ; j < 2 ; j++)
  {
    for(int x = 0 ; x < numGames.length() - 4 ; x++)
    {
      String tempStr = numGames.substring(x, x+4); //Chop out four letters from the string
  
      segmentDisplay.write('v'); //Reset the display
      segmentDisplay.print(tempStr);
      
      for(int y = 0 ; y < 25 ; y++)
      {
        if(digitalRead(button) == LOW) return; //If the button is pressed, bail!
        delay(10);
      }
    }
  }

  segmentDisplay.write('v'); //Reset the display - this forces the cursor to return to the beginning of the display
  segmentDisplay.write("0000"); //Leave the display in a clean state
}
