#include <SSD1306.h>
#include <Preferences.h>

#include "asset.h"

#define BUTTON_PIN      23
#define BOOT_BUTTON_PIN 0
#define BUZZER_PIN      18

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64

#define TUBE_DISTANCE   32
#define TUBE_WIDTH      6
#define PATH_WIDTH      30

// IIC default address is 0x3c
SSD1306  display(0x3c, 21, 22);

// Preferences to read/write flash memory
Preferences preferences;

float tubeX[4];
int bottomTubeHeight[4];

unsigned int score = 0;
unsigned int highScore;
unsigned int gameState = 0;  // 0 - start, 1 - play, 2 - score

bool isFlyingUp = false;
bool isBuzzerOn = false;
bool increaseSpeed = false;
bool hasScored[4];

float birdX = 20.00;
float birdY = 28.00;
float speed = 0.01;

unsigned long keyPressTime = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  pinMode(BUZZER_PIN, OUTPUT);

  preferences.begin("Flappy", false);
  // Get the high score, if the key does not exist, return a default value of 0
  highScore = preferences.getUInt("highScore", 0);
  // Close the Preferences
  preferences.end();

  display.init();

  // Initialize tubes on the right outside of the screen
  for(int i = 0; i < 4; i++) {
    tubeX[i] = 128 + i * TUBE_DISTANCE;
    bottomTubeHeight[i] = random (8, 32);
  }

  // Make all asset in the game is right side up
  display.flipScreenVertically();
}

void loop() {
  display.clear();

  // Display start screen
  if(gameState == 0) {  
    // Reinitialize in-game status
    birdY = 28.00;
    score = 0;
    speed = 0.01;

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 4, "Flappy ");

    display.drawXbm(64, 0, Building_width, Building_height, Building);
    display.drawXbm(birdX, birdY, Flappy_width, Flappy_height, Flappy);
    display.setColor(WHITE);
    display.fillRect(0, SCREEN_HEIGHT - 5, SCREEN_WIDTH, 5);

    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 44, "Press to start");

    // Reinitialize all tubes
    for(int i = 0; i < 4; i++) {
      tubeX[i] = 128 + ((i+1) * TUBE_DISTANCE);
      bottomTubeHeight[i] = random(8, 32);
      hasScored[i] = false;
    }

    if(digitalRead(BUTTON_PIN) == LOW) {
      gameState = 1;
      delay(50);
    }
  }

  //Display in-game screen
  else if (gameState == 1) {  
    //Display score
    display.setFont(ArialMT_Plain_10);
    display.drawString(3, 0, String(score));
   
    // Setup variables and flags if button is pressed
    if(digitalRead(BUTTON_PIN) == LOW) {
    keyPressTime = millis();
      isFlyingUp = true;
      isBuzzerOn = true;
    } 

    // Display bird and tubes
    display.setColor(WHITE);
    display.drawXbm(birdX, birdY, Flappy_width, Flappy_height, Flappy);

    for(int i = 0; i < 4; i++) {
      display.fillRect(tubeX[i], 0, TUBE_WIDTH, bottomTubeHeight[i]);
      display.fillRect(tubeX[i], bottomTubeHeight[i] + PATH_WIDTH, TUBE_WIDTH, SCREEN_HEIGHT - bottomTubeHeight[i] - PATH_WIDTH);
    }
  
    for(int i = 0; i < 4; i++) {
      //Move all tube to the left
      tubeX[i] -= speed;

      // If a tube pass the bird, add a point
      if(tubeX[i] < birdX && !hasScored[i]){
        score++;
        hasScored[i] = true;

        // Increase speed every 10 tubes
        if(score % 10 == 0){
          speed += 0.01;
        }
      }

      // If a tube pass the screen, reinitialize that tube on the right of the screen
      if(tubeX[i] + TUBE_WIDTH < 0){
        bottomTubeHeight[i] = random(8, 32);
        tubeX[i] = 128;
        hasScored[i] = false;
      }
    }

    // The bird will fly up for 80 milliseconds
    if((keyPressTime + 80) < millis()) {
      isFlyingUp = false;
    }

    // The buzzer will make sound for 10 milliseconds
    if((keyPressTime + 10) < millis()) {
      isBuzzerOn = false;
    }
  
    // Bird fly up (y-axis is reverted)
    if(isFlyingUp) {
      birdY -= 0.025;
    } 
    else {
      birdY += 0.015;
    }

    // Sound when click
    if(isBuzzerOn){
      digitalWrite(BUZZER_PIN,1);
    }
    else{
      digitalWrite(BUZZER_PIN,0);
    }

    if(birdY > 63 || birdY < 0) { // Check if out of bound on vertical axis
      // Ending sound
      digitalWrite(BUZZER_PIN,1);
      delay(200);
      digitalWrite(BUZZER_PIN,0); 
      delay(50);
      digitalWrite(BUZZER_PIN,1);
      delay(50);
      digitalWrite(BUZZER_PIN,0); 
      delay(50);
      digitalWrite(BUZZER_PIN,1);
      delay(50);
      digitalWrite(BUZZER_PIN,0); 

      if(score > highScore){
        highScore = score;

        // Write new high score to flash memory
        preferences.begin("Flappy", false);
        preferences.putUInt("highScore", highScore);
        preferences.end();
      }

      gameState = 2;
    }

    //Check for collision with tube
    for(int i = 0; i < 4; i++){
      if(tubeX[i] <= birdX + 7 && birdX + 7 <= tubeX[i] + 6) {
        if(birdY < bottomTubeHeight[i] || birdY + 8 > bottomTubeHeight[i] + PATH_WIDTH){
          // Ending sound
          digitalWrite(BUZZER_PIN,1);
          delay(200);
          digitalWrite(BUZZER_PIN,0); 
          delay(50);
          digitalWrite(BUZZER_PIN,1);
          delay(50);
          digitalWrite(BUZZER_PIN,0); 
          delay(50);
          digitalWrite(BUZZER_PIN,1);
          delay(50);
          digitalWrite(BUZZER_PIN,0); 

          if(score > highScore){
            highScore = score;

            // Write new high score to flash memory
            preferences.begin("Flappy", false);
            preferences.putUInt("highScore", highScore);
            preferences.end();
          }

          gameState = 2;

          delay(50);
        }
      }
    }

    // Display boundary
    display.drawRect(0, 0, 128, 64);
  }

  // Display ending (score) screen
  else{ 
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Your score: " + String(score));
    display.drawString(0, 20, "High score: " + String(highScore));

    display.setFont(ArialMT_Plain_10);
    display.drawString(32, 44, "Click to restart");

    display.setFont(ArialMT_Plain_10);
    display.drawString(5, 54, "Click BOOT to reset score");

    if(digitalRead(BUTTON_PIN) == LOW){
      gameState = 0;
      delay(200);
    }

    // If BOOT button is pressed, reset high score in game and in the flash memory
    if(digitalRead(BOOT_BUTTON_PIN) == LOW){
      score = 0;
      highScore = 0;

      // Write high score (= 0) to flash memory
      preferences.begin("Flappy", false);
      preferences.putUInt("highScore", highScore);
      preferences.end();
    }
  }
  
   display.display();
}