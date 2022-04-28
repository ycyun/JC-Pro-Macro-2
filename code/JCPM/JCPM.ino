//Example for JC Pro Macro board
//set up to work in Mac OS
//Includes support for 1306 display
//Reference: https://github.com/NicoHood/HID/blob/master/src/KeyboardLayouts/ImprovedKeylayouts.h
//Reference: https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press 

// Declare variables etc=========================================

/*
bool SW[1] = 1; //encoder button
bool SW[2] = 1; //lower-left keyswitch
bool SW[3] = 1; //lower-center left 
bool SW[4] = 1; //lower-center right
bool SW[5] = 1; //mid-left
bool SW[6] = 1; //upper-left
bool SW[7] = 1; //upper-right keyswitch
bool SW[8] = 1; //mid-right
bool SW[9] = 1; //lower-right keyswitch
bool SW[10] = 1; //JCPM 2 mode switch
*/
bool SW[11] = {1,1,1,1,1,1,1,1,1,1,1};
bool underLight = 0;

bool increment = 0;
bool decrement = 0;
long oldPosition;
long newPosition;
int inputMode = 0;
int LEDLight = 1;
int LEDCircle[6] = {2, 3, 4, 5, 6, 7};
int counter = 0;
bool nascar = 0;
long newNascarTurnTime = 0;
long oldNascarTurnTime = 0;
long newBeatTime = 0;
long oldBeatTime = 0;
bool beatOn = 0;

int fanSpeed = 0;
bool fanPulse = 0;
long newPulseTime = 0;
long oldPulseTime = 0;
int fanRPM = 0;
char toneNote;

int modeArray[] = {0, 1}; //{0, 1, 3}; //adjust this array to modify sequence of modes - as written, change to {0, 1, 2, 3, 4, 5} to access all modes
int inputModeIndex = 0;
int modeArrayLength = (sizeof(modeArray) / sizeof(modeArray[0]));

// Encoder setup =============================================

#include <Encoder.h>
#include <HID-Project.h>
Encoder myEnc(1,0); //if rotation is backwards, swap 0 and 1

// Screen setup =============================================

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NeoPixel setup =============================================

// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        5 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 15 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

//============================================================

void setup() {
  delay(500); //formerly longer delay to allow programming
  //Serial.begin(9600);
  pinMode(4, INPUT_PULLUP); //SW[1] pushbutton (encoder button)
  pinMode(15, INPUT_PULLUP); //SW[2] pushbutton
  pinMode(A0, INPUT_PULLUP); //SW[3] pushbutton
  pinMode(A1, INPUT_PULLUP); //SW[4] pushbutton
  pinMode(A2, INPUT_PULLUP); //SW[5] pushbutton
  pinMode(A3, INPUT_PULLUP); //SW[6] pushbutton
//===============new pins for JCPM

  pinMode(14, INPUT_PULLUP); //SW[7] pushbutton
  pinMode(16, INPUT_PULLUP); //SW[8] pushbutton
  pinMode(10, INPUT_PULLUP); //SW[9] pushbutton
  pinMode(8, INPUT_PULLUP); //SW[10] pushbutton - acts as mode switch

//================end new pins
  

  randomSeed(analogRead(A9));
     
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setRotation(2); //sets rotation 1 through 4 (2 = 180º rotation vs 4)
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.invertDisplay(0);
  
Mouse.begin();
Keyboard.begin();
//BootKeyboard.begin(); - BootKeyboard use appears to give problems w/ Macintosh


//NeoPixel setup=========================================

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();


  for(int i=0; i<8; i++){
    pixels.setPixelColor(i, pixels.Color(10, 0, 0));
  }
  pixels.show(); // Show results
}


void loop() {

  SW[1] = digitalRead(4);
  SW[2] = digitalRead(15);
  SW[3] = digitalRead(A0);
  SW[4] = digitalRead(A1);
  SW[5] = digitalRead(A2);
  SW[6] = digitalRead(A3);
  SW[7] = digitalRead(14);
  SW[8] = digitalRead(16);
  SW[9] = digitalRead(10);
  SW[10] = digitalRead(8);
  
  newPosition = myEnc.read();
  
  if (newPosition > (oldPosition + 2)) { 
    increment = 1;
    delay(5);
    oldPosition = myEnc.read();
  }
  if (newPosition < (oldPosition - 2)) {
    decrement = 1;
    delay(5);
    oldPosition = myEnc.read();
  }

//========select upload mode==================
    
  // Jump into upload mode for mode select and lower-right button
  if (SW[9] == 0 && SW[10] == 0) {
    screenUpload();
    delay(3600000); // I don't know why, but this delay allows to simply upload code.
  }

//=========change mode=================

   if (SW[10] == 0){
      if (inputModeIndex < modeArrayLength){
        inputModeIndex++;
        inputMode = modeArray[inputModeIndex];
      }
      if (inputModeIndex == modeArrayLength){
        inputModeIndex = 0;
        inputMode = modeArray[inputModeIndex];
      }
      pixels.clear();
      pixels.show();
      SW[10] = 1;
      delay(150);
    }

//======select input mode:=======
/*
if (inputMode == 0) {
  pixels.setPixelColor(LEDCircle[LEDLight], pixels.Color(10, 0, 0));
  pixels.show(); // Show results
  volume();  
}
if (inputMode == 1) jiggler();
if (inputMode == 2) slitherIO();
if (inputMode == 3) {
  pixels.setPixelColor(LEDCircle[LEDLight], pixels.Color(0, 10, 0));
  pixels.show(); // Show results
  FCPX();
}
if (inputMode == 4) fan();
if (inputMode == 5) music();
if (inputMode == 6) textInput();
*/
if (inputMode == 0){
  debug();
}
if (inputMode == 1){
  debug();
}
//Serial.println(inputMode);
}
void debug(){
  screenDebug();
  if (SW[1] == 0){
    Serial.println("SW[1]");
  }if (SW[2] == 0){
    Serial.println("SW[2]");
  }
  char SeOutput[] = "inc: 1, dec: 1, SW1: 1 SW2: 1 SW3: 1 SW4: 1 SW5: 1 SW6: 1 SW7: 1 SW8: 1 SW9: 1 SW10: 1\n";
  sprintf(SeOutput, "inc: %d, dec: %d, SW1: %d SW2: %d SW3: %d SW4: %d SW5: %d SW6: %d SW7: %d SW8: %d SW9: %d SW10: %d\n", increment, decrement, SW[1], SW[2], SW[3], SW[4], SW[5], SW[6], SW[7], SW[8], SW[9], SW[10]);
  //Serial.print(SeOutput);
  if (increment == 1) {
        Consumer.write(MEDIA_VOLUME_UP);
        if (LEDLight == 5) LEDLight = 0;
        else if (LEDLight < 5) LEDLight += 1;
        topPixelsClear();
        pixels.setPixelColor(LEDCircle[LEDLight], pixels.Color(10, 0, 0));
        pixels.show(); // Show results
        increment = 0;
        decrement = 0;
        //delay(10);
      }
      
  if (decrement == 1) {
        Consumer.write(MEDIA_VOLUME_DOWN);
        if (LEDLight == 0) LEDLight = 5;
        else if (LEDLight > 0) LEDLight -= 1;
        topPixelsClear();
        pixels.setPixelColor(LEDCircle[LEDLight], pixels.Color(10, 0, 0));
        pixels.show(); // Show results
        increment = 0;
        decrement = 0;
        //delay(10);
      }
}

/*

void volume(){

  if (increment == 1) {
        Consumer.write(MEDIA_VOLUME_UP);
        if (LEDLight == 5) LEDLight = 0;
        else if (LEDLight < 5) LEDLight += 1;
        topPixelsClear();
        pixels.setPixelColor(LEDCircle[LEDLight], pixels.Color(10, 0, 0));
        pixels.show(); // Show results
        increment = 0;
        decrement = 0;
        //delay(10);
      }
      
  if (decrement == 1) {
        Consumer.write(MEDIA_VOLUME_DOWN);
        if (LEDLight == 0) LEDLight = 5;
        else if (LEDLight > 0) LEDLight -= 1;
        topPixelsClear();
        pixels.setPixelColor(LEDCircle[LEDLight], pixels.Color(10, 0, 0));
        pixels.show(); // Show results
        increment = 0;
        decrement = 0;
        //delay(10);
      }
  if (SW[6] == 0){ //tab to next browser tab Firefox or Chrome
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_TAB);          
        Keyboard.releaseAll();
        delay(50);
      }
  if (SW[5] == 0){ //tab to previous browser tab Firefox or Chrome
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_TAB);
        Keyboard.releaseAll();
        delay(50);
      }
  if (SW[3] == 0) {
        Consumer.write(MEDIA_PLAY_PAUSE); 
        delay(100);
      }
  if (SW[4] == 0) {
        Consumer.write(MEDIA_NEXT);
        delay(50);
      }
  if (SW[2] == 0) {
        Consumer.write(MEDIA_PREVIOUS);
        delay(50);
      }
  if (SW[1] == 0) { //Emulate keyboard enter button when encoder knob is pressed down:
      Keyboard.press(KEY_ENTER);
      Keyboard.release(KEY_ENTER);
      delay(50);
    }
  if ((SW[7] == 0) && (underLight == 0)) {        
    underLight = 1;
    for(int i=8; i<12; i++){
    pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    }
    pixels.show(); // Show results
    delay(100);
  }
  else if ((SW[7] == 0) && (underLight == 1)) {        
    underLight = 0;
    for(int i=8; i<12; i++){
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show(); // Show results
    delay(100);
  }  
  if ((SW[8] == 0)||(SW[9] == 0)){
    fan();
  }

      
screenVolume();
}


void slitherIO(){ //works with new code
//movement tracking
int xMovements[] = {-3, -8, -14, -16, -19, -19, -16, -14, -8, -3, 3, 8, 14, 16, 19, 19, 16, 14, 8, 3, -3};
int yMovements[] = {19, 16, 14, 8, 3, -3, -8, -14, -16, -19, -19, -16, -14, -8, -3, 3, 8, 14, 16, 19, 19};
int multiplier = 2; //changes speed of rotation

//homing routine =====================================================  
  if (SW[6] == 0){
    Mouse.move(40, 10); //was 60, 0 before
    counter = 0;
    delay(250);
  }

//mouse move routine==================================================

  if (increment == 1) {
    //need a routine to loop back to zero etc to loop around the x/y movements for each increment

      for(int i=0; i< multiplier; i++){
        if (counter == 20) counter = 1;
        else if (counter < 20) ++counter;
        Mouse.move(xMovements[counter], yMovements[counter]);
        
        //Serial.print(counter); Serial.print(" "); Serial.print(xMovements[counter]); Serial.print(" "); Serial.println(yMovements[counter]);
        increment = 0;
        decrement = 0;
      }
  }
  

    if (decrement == 1) { //increment and decrement work fine individually, but don't line up when reversed
      for(int i=0; i< multiplier; i++){     
        if (counter == 0) counter = 19; 
        else if (counter > 0) --counter;       
        Mouse.move(-xMovements[counter+1], -yMovements[counter+1]);

        //Serial.print(counter); Serial.print(" "); Serial.print(-xMovements[counter+1]); Serial.print(" "); Serial.println(-yMovements[counter+1]);
        increment = 0;
        decrement = 0;
      }
  }

//worm speed input==============================

if ((SW[2] == 0)||(SW[3] == 0)||(SW[4] ==0)){
        Keyboard.press(HID_KEYBOARD_SPACEBAR);       
        delay(5);
}

if ((SW[2] == 1) && (SW[3] == 1) && (SW[4] == 1)){
        Keyboard.releaseAll();
        delay(5);
}

//worm auto turn input - NASCAR mode: all left turns

if ((SW[5] == 0) && (nascar == 0)){
  nascar = 1;
  delay(20);
}

else if ((SW[5] == 0) && (nascar ==1)){
  nascar = 0;
  delay(20);
}

if (nascar == 1){
  newNascarTurnTime = millis();
  if ((newNascarTurnTime - oldNascarTurnTime) > 200){
    decrement = 1;
    oldNascarTurnTime = newNascarTurnTime;
  }
}


screenVolume();
  
}

void FCPX(){
  
  if (increment == 1) {
        //Keyboard.press(KEY_K);
        //Keyboard.releaseAll();       
        Keyboard.press(HID_KEYBOARD_RIGHTARROW);
        Keyboard.releaseAll();
        if (LEDLight == 5) LEDLight = 0;
        else if (LEDLight < 5) LEDLight += 1;
        topPixelsClear();
        pixels.setPixelColor(LEDCircle[LEDLight], pixels.Color(0, 10, 0));
        pixels.show(); // Show results
        increment = 0;
        decrement = 0;
        //delay(50);
      }
      
  else if (decrement == 1) {
        //Keyboard.press(KEY_K);
        //Keyboard.releaseAll();  
        Keyboard.press(HID_KEYBOARD_LEFTARROW);
        Keyboard.releaseAll();
        if (LEDLight == 0) LEDLight = 5;
        else if (LEDLight > 0) LEDLight -= 1;
        topPixelsClear();
        pixels.setPixelColor(LEDCircle[LEDLight], pixels.Color(0, 10, 0));
        pixels.show(); // Show results
        increment = 0;
        decrement = 0;
        //delay(50);
      }
  else if (SW[7] == 0){ //Zoom in
        Keyboard.press(KEY_LEFT_WINDOWS);
        Keyboard.press(HID_KEYBOARD_EQUALS_AND_PLUS);    
        Keyboard.releaseAll();
        delay(50);
      }
  else if (SW[8] == 0){ //Zoom out
        Keyboard.press(KEY_LEFT_WINDOWS);
        Keyboard.press(HID_KEYBOARD_MINUS_AND_UNDERSCORE);    
        Keyboard.releaseAll();
        delay(50);
      }
  else if (SW[4] == 0) { //fwd
        Keyboard.press(KEY_L);
        Keyboard.releaseAll();
        delay(50);
      }
  else if (SW[3] == 0) { //stop
        Keyboard.press(KEY_K);
        Keyboard.releaseAll();
        delay(50);
      }
  else if (SW[2] == 0) { //BACK
        Keyboard.press(KEY_J);
        Keyboard.releaseAll();
        delay(50);
      }
  else if (SW[1] == 0) { //break all win + shift + b
        Keyboard.press(KEY_LEFT_WINDOWS);        
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_B);        
        Keyboard.releaseAll();
        delay(50);
      }
    else if (SW[6] == 0) { //break current track win + b
        Keyboard.press(KEY_LEFT_WINDOWS);        
        Keyboard.press(KEY_B);        
        Keyboard.releaseAll();
        delay(50);
      }
    else if (SW[5] == 0) { //undo
        Keyboard.press(KEY_LEFT_WINDOWS);        
        Keyboard.press(KEY_Z);        
        Keyboard.releaseAll();
        delay(50);
      }
    else if (SW[9] == 0) { //backspace (delete)  
        Keyboard.press(KEY_BACKSPACE);        
        Keyboard.releaseAll();
        delay(50);
      } 
screenFCPX();
}
void fan(){
  if (SW[8] == 0){
    if (fanSpeed < 5){
    ++fanSpeed;
    }
    delay(20);
  }
  if (SW[9] == 0){
    if (fanSpeed > 0){
    --fanSpeed;
    }
    delay(20);
  }
int fanSpeedScaled = map(fanSpeed, 0, 5, 0, 255);
analogWrite(6, fanSpeedScaled);
//Serial.print(fanSpeed); Serial.print(" "); Serial.println(fanSpeedScaled);

if(fanPulse == 0){
  newPulseTime = millis();
  fanRPM = (newPulseTime - oldPulseTime);
  oldPulseTime = newPulseTime;
}

//screenFan();

}

void music(){

//attach small buzzer to pin 6 on aux jack
//

pinMode(7, OUTPUT);
digitalWrite(7, LOW);

  if (SW[2] == 0){
    tone(6, 220, 100); //A3 
    toneNote = 'A';
  }
  else if (SW[3] == 0){
    tone(6, 247, 100); //B3
    toneNote = 'B';
  }
  else if (SW[4] == 0){
    tone(6, 262, 100); //C4
    toneNote = 'C';
  }  
  else if (SW[9] == 0){
    tone(6, 294, 100); //D4
    toneNote = 'D';
  }
  else if (SW[5] == 0){
    tone(6, 330, 100); //E4
    toneNote = 'E';
  }
  else if (SW[8] == 0){
    tone(6, 349, 100); //F4
    toneNote = 'F';
  }
  else if (SW[6] == 0){
    tone(6, 392, 100); //G4
    toneNote = 'G';
  }
  else if (SW[7] == 0){
    tone(6, 440, 100); //A4
    toneNote = 'A';
  }
  else {
    toneNote = ' ';
  }
  
  if ((SW[1] == 0) && (beatOn == 0)) {
    beatOn = 1;
    delay(100);
  }
  else if ((SW[1] == 0) && (beatOn == 1)){
    beatOn = 0;
    delay(100);
  }
  
  if (beatOn == 1) {
  newBeatTime = millis();
  if ((newBeatTime - oldBeatTime) > 500){
  digitalWrite(7, HIGH);
  delay(35);
  digitalWrite(7, LOW);
    oldBeatTime = newBeatTime;
  }
  }

  if (decrement == 1){
  digitalWrite(7, HIGH);
  delay(35);
  digitalWrite(7, LOW);
  increment = 0;
  decrement = 0;
  }

  if (decrement == 1){
  digitalWrite(7, HIGH);
  delay(35);
  digitalWrite(7, LOW);
  increment = 0;
  decrement = 0;
  }

  if (increment == 1){
  digitalWrite(7, HIGH);
  delay(35);
  digitalWrite(7, LOW);
  increment = 0;
  decrement = 0;
  }
 
screenNote();
}
*/

//======================.96" oled screen=======================
char dpOutput[256]={0,};
void screenDebug(){
  display.setTextSize(1); 
  display.clearDisplay();
  display.setCursor(0,0);
  //display.println(""); 
  display.println(" ");
  sprintf(dpOutput, \
 "          LEDLight: %d \n" \
 "SW1 inc: %d  S6:%d S7:%d\n" \
" %d  dec: %d  S5:%d S8:%d\n" \
"  S2:%d S3:%d S4:%d S9:%d\n" \
, LEDLight, increment, SW[6], SW[7], SW[1], decrement, SW[5], SW[8], SW[2], SW[3], SW[4], SW[9]);
  display.print(dpOutput);
  display.print(" ");
  display.display();
  //Serial.println(SW[1]);
  //delay(10);
}

void screenVolume(){
  display.setTextSize(1); 
  display.clearDisplay();
  display.setCursor(0,0);
  //display.println("");  
  display.println("ENCOD|     |TAB+ |LIT");
  display.println("");
  display.println("VOL- |VOL+ |TAB- |FN+");
  display.println("");
  display.println("BACK |STOP |FORW |FN-");
  display.println("");  
  //display.print(increment);
  //display.print(decrement);
  //display.print(" ");
  display.print(newPosition);
  display.print(" ");
  display.print(LEDLight);
  display.print(" ");
  display.print(SW[1]);
  display.print(SW[2]);
  display.print(SW[3]);
  display.print(SW[4]);
  display.print(SW[5]);
  display.print(SW[6]);
  display.print(SW[7]);
  display.print(SW[8]);
  display.print(SW[9]);
  display.print(SW[10]);
  display.display();
  //Serial.println(SW[1]);
  //delay(10);
}


void screenFCPX(){
  display.setTextSize(1); 
  display.clearDisplay();
  display.setCursor(0,0);
  //display.println("");  
  display.println("FCPX |BRKA |BRK  |ZO+");
  display.println("");
  display.println("FRM- |FRM+ |UNDO |ZO-");
  display.println("");
  display.println("BACK |STOP |FORW |DEL");
  display.println("");  
  //display.println("FCPX");
  //display.print("Sh0rcut");
  display.display();
}

void screenUpload(){
  display.setTextSize(3);
  display.clearDisplay();
  display.setCursor(0,10);
  display.println("Upload Code!");
  display.display();
}

//====================pixel helpers===============

void topPixelsClear(){
    for(int i=0; i<9; i++){
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show(); // Show results
}
