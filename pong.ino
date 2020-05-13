#include <Servo.h>//imports the library to communicate with the servo motor
Servo myservo;//creates servo variable


//---------------------------------------------------
//------------------- PIN VARS ----------------------
//---------------------------------------------------

int ledPin[] = {4,5,6,7,8,9};//assigns pin places to leds
int playerOneButton = 3;//first button pin (2 and 3 are for interrupts)
int playerTwoButton = 2;//second button pin (2 and 3 are for interrupts)
int scoreLed1 = 12;//pin for p1 led
int scoreLed2 = 11;//pin for p2 led
int speakerPin = 10;//pin for piezo
int servoPin = 13;//pin for servo motor
int potentiometerPin = 0;//pin for the potentiometer

//---------------------------------------------------
//------------------- PIN VARS ----------------------
//---------------------------------------------------



//---------------------------------------------------
//------------------- GAME VARS ---------------------
//---------------------------------------------------

int posServo = 0;//sets the initial value to zero (middle)
int sensorValue = 0;
bool forward=true;//variable that knows the "direction of the ball"
int currentPos=0;//position of the ball (0,1,2,3,4,5)
int delayTime = 100;//default game speed (will be alterated by potentiometer's state)
int p1pts=0;//player 1 points
int p2pts=0;//player 2 points
int bounces=0;//counts the number of "ball bounces"
int maxBounces=60;//sets the time limit for the end of the game
int iterations=0;//loop counter
unsigned long lastInterrupt;//variable that avoids the button to trigger multiple interrupts in a single press

//---------------------------------------------------
//------------------- GAME VARS ---------------------
//---------------------------------------------------


//---------------------------------------------------
//------------------- SOUND VARS --------------------
//---------------------------------------------------

int length = 3;//lenth of start, point and error melodies
int lengthMario = 25;//lenth of victory melody
char pointMelody[] = "ceg";//just short melody
char errorMelody[] = "add";//just short melody
char startMelody[] = "cCc";//just short melody
char marioMelody[] = "pcegCEGEocyxCYXYidfzdFZZ6";//victory melody
int beatsMario[] = {1,1,1,1,1,1,4,4,1,1,1,1,1,1,4,4,1,1,1,1,1,1,4,4,8};//lengths of victory melody notes
int beats[] = {1,1,1};//legths of short melodies
int tempo = 20;//speed of short melodies

//---------------------------------------------------
//------------------- SOUND VARS --------------------
//---------------------------------------------------



void setup()//function to initialize pins and components
{
  Serial.begin(9600);//sets up piezo element
  attachInterrupt(1, playerOneInput, CHANGE);//button connected to interrupt pin
  attachInterrupt(0, playerTwoInput, CHANGE);//button connected to interrupt pin

  myservo.attach(servoPin);//init servo
  pinMode(speakerPin, OUTPUT);//set piezo mode

  
  for(int i = 0; i < 5; i++) {
    pinMode(ledPin[i], OUTPUT);//sets the game leds mode
  }
  pinMode(playerOneButton, INPUT);//sets button as input elements
  pinMode(playerTwoButton, INPUT);//sets button as input elements
  pinMode(scoreLed1, OUTPUT);//sets score leds mode
  pinMode(scoreLed2, OUTPUT);//sets score leds mode
}

void loop()//main loop
{
  if(bounces<maxBounces)//if 60 bounces haven't been made yet: then continue the game; else end it
  {
    gamePhase();//game stuff
  }
  else
  {
    winPhase();//end of game stuff
  }
}






//---------------------------------------------------
//------------------- GAME STUFF --------------------
//---------------------------------------------------

void gamePhase()
{
  sensorValue = analogRead(potentiometerPin);//checks the state of the potentiometer every time...
  delayTime=50+((1023-sensorValue)/10);//...and changes the game speed accordingly (the range of the sensor is from 0 to 1023, in the equation I inverted the orentation so that it's more natural for the user to increase and reduce speed: clockwise --> increase ; counterclockwise --> decrease)
    if(iterations<1)
    {
      startSound();//just a quick sound that notifies that the game has started
    }
    iterations++;
    
//---------------------------------------------------
//----------- MAIN GAME MANAGER CONDITION -----------
//---------------------------------------------------

/* Basically what we do below is keep adding 1 to the current position, if the
 * counter reaches the top limit the boolean variable that stores the direction
 * changes and we start to subtract 1 until the current position counter reaches
 * the bottom limit; the result is the light moving from left to right and from
 * right to left constantly
 */

    if(forward)
    {
      currentPos++;
      if(currentPos>4)
      {
        forward=false;
        bounces++;
      }
      refresh(currentPos);
    }
    else
    {
      currentPos--;
      if(currentPos<1)
      {
        forward=true;
        bounces++;
      }
      refresh(currentPos);
    }
    
//---------------------------------------------------
//----------- MAIN GAME MANAGER CONDITION -----------
//---------------------------------------------------
    
    if (digitalRead(scoreLed1) == HIGH)//this statement activates if the score led is on, and turns it of after a short amount of time
    {
      delay(delayTime);
      digitalWrite(scoreLed1, LOW);
    }
    
    if (digitalRead(scoreLed2) == HIGH)//this statement activates if the score led is on, and turns it of after a short amount of time
    {
      delay(delayTime);
      digitalWrite(scoreLed2, LOW);
    }
}


void refresh(int i)
{
  digitalWrite(ledPin[i], HIGH);//turns on the light that indicates the position of the ball
  delay(delayTime);
  digitalWrite(ledPin[i], LOW);//turns it of after a while
}


void updateServo()
{
  if(((90+(p1pts-p2pts)*15)>0)&&((90+(p1pts-p2pts)*15)<180))//makes sure that the servo does not go under 0 degrees and over 180 (check 2 lines below for formula explaination)
  {
    myservo.write(90+(p1pts-p2pts)*15);//ok, so we start from the servo in the middle (90 degrees) and add the product between the difference of the 2 scores (the advantage value) and 15 degrees; so each point of advantage generates a 15 degrees shift in a certain direction
  }
}


void playerOneInput()
{
  if(millis()-lastInterrupt>200)//we avoid another interrupt being created within 200 milliseconds
  {
    if(digitalRead(ledPin[0]) == HIGH)//if the final green pin is on when the button has been pressed it gives a point to player 1
    {
      digitalWrite(scoreLed1, HIGH);
      p1pts=p1pts+1;
      updateServo();
      scoreSound();
    }
    else//else it gives a point to player 2
    {
      digitalWrite(scoreLed2, HIGH);
      p2pts=p2pts+1;
      updateServo();
      errorSound();
    }
    lastInterrupt = millis();//updates the state of the last time the button has been pressed
  }
  
}


void playerTwoInput()
{
  if(millis() - lastInterrupt > 200)//same as before
  {
    if(digitalRead(ledPin[5]) == HIGH)//if the final green pin is on when the button has been pressed it gives a point to player 2
    {
      digitalWrite(scoreLed2, HIGH);
      p2pts=p2pts+1;
      updateServo();
      scoreSound();
    }
    else//else it gives a point to player 1
    {
      digitalWrite(scoreLed1, HIGH);
      p1pts=p1pts+1;
      updateServo();
      errorSound();
    }
    lastInterrupt = millis();//same as before
  }
}

//---------------------------------------------------
//------------------- GAME STUFF --------------------
//---------------------------------------------------








//---------------------------------------------------
//--------------- WINNING CEREMONIES ----------------
//---------------------------------------------------

void winPhase()
{
  int i=0;
    if(bounces<maxBounces+3)//animation that signals the end of the game
    {
      winAnimation(i);
      endSound();
    }
    if((p1pts-p2pts)>0)//p1 wins!
    {
      win1(i);
    }
    if((p1pts-p2pts)<0)//p2 wins!
    {
      win2(i);
    }
    if((p1pts-p2pts)==0)//stalemate!
    {
      stalemate(i);
    }
}


void winAnimation(int i)//creates a lovely animation turning on all the lights from one side to the other
{
  digitalWrite(scoreLed1, LOW);
  digitalWrite(scoreLed2, LOW);
  for(i=0;i<6;i++)
  {
    digitalWrite(ledPin[i], HIGH);
    delay(delayTime/2);
    bounces++;
  }
  delay(200);
  for(i=0;i<6;i++)
  {
    digitalWrite(ledPin[i], LOW);
    delay(delayTime/2);
  }
  delay(200);
}


void win1(int i)//keeps turning on the lights on the side of player 1
{
  digitalWrite(scoreLed1, HIGH);
  for(i=0;i<3;i++)
  {
    delay(delayTime/2);
    digitalWrite(ledPin[i], HIGH);
  }
  
  delay(200);
  digitalWrite(scoreLed1, LOW);
  for(i=0;i<3;i++)
  {
    delay(delayTime/2);
    digitalWrite(ledPin[i], LOW);
  }
  delay(200);
}


void win2(int i)//keeps turning on the lights on the side of player 2
{
  for(i=3;i<6;i++)
  {
    digitalWrite(ledPin[i], HIGH);
    delay(delayTime/2);
  }
  digitalWrite(scoreLed2, HIGH);
  delay(200);
  for(i=3;i<6;i++)
  {
    digitalWrite(ledPin[i], LOW);
    delay(delayTime/2);
  }
  digitalWrite(scoreLed2, LOW);
  delay(200);
}


void stalemate(int i)//keeps turning on all the lights since both players won (or nobody won, it depens if you are more likely to see the glass half empty)
{
  for(i=0;i<6;i++)
  {
    digitalWrite(ledPin[i], HIGH);
    delay(delayTime/2);
  }
  delay(200);
  for(i=0;i<6;i++)
  {
    digitalWrite(ledPin[i], LOW);
    delay(delayTime/2);
  }
  delay(200);
}

//---------------------------------------------------
//--------------- WINNING CEREMONIES ----------------
//---------------------------------------------------








//---------------------------------------------------
//------------------- MUSIC STUFF -------------------
//---------------------------------------------------

void scoreSound()//calls 3 times the function playNote, passing the right values
{
  for (int i = 0; i < length; i++)
  {
    playNote(pointMelody[i], beats[i] * tempo);
  }
}

void errorSound()//calls 3 times the function playNote, passing the right values
{
  for (int i = 0; i < length; i++)
  {
    playNote(errorMelody[i], beats[i] * tempo);
  }
}

void startSound()//calls 3 times the function playNote, passing the right values
{
  for (int i = 0; i < length; i++)
  {
    playNote(startMelody[i], beats[i] * tempo);
  }
}


void endSound()//calls 25 times the function playNote, passing the right values
{
  for (int k = 0; k < 25; k++)
  {
    playNote(marioMelody[k], beatsMario[k] * 100);
  }
}



void playTone(int tone, int duration)//plays the single note given the tone and duration parameters
{
  for (long i=0; i<duration*1000L; i+=tone*2)
  {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);//turns the piezo on and off to generate a certain frequency
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration)
{
  char names[] = {'c','d','e','f','g','a','b','C','D','E','F','G','X','Y','Z','x','y','z','p','o','i','6'};//array of all of the notes used
  /*
   * x = g#
   * y = d#
   * z = a#
   * 
   * p = -g
   * o = -g#
   * i = -g#
   * 
   * These are just a few sharp notes from different tonalities tha I needed to replicate the Super Mario song
   * 
   */
  int tones[] = {1915,1700,1519,1432,1275,1136,1014,956,852,759,716,638,602,804,536,1205,1608,1073,2551,2404,2146,478};//given a tone frequency, this is an array that translates the tone with the formula 1/(2*toneFrequency)*1000000 into a value that the Arduino can understand 
  for (int i = 0; i < 22; i++)
  {
    if (names[i] == note)//checks all of the notes and compares it to the input value of the melody
    {
      playTone(tones[i], duration);//if it is founs, it calls the function playTone
    }
  }
}

//---------------------------------------------------
//------------------- MUSIC STUFF -------------------
//---------------------------------------------------




/*      -----> REFERENCES <-----
 * 
 * To build this game I owe my experience to the Arduino kit examples
 * 
 * Furthermore I used a function that I found in StackOverflow.com to 
 * solve the multiple interrupts problem
 * (http://stackoverflow.com/questions/29756145/arduino-attachinterrupt-seems-to-run-twice)
 * 
 */
