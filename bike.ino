//Code heavily borrowed from https://www.instructables.com/Arduino-Bike-Speedometer/
//This website used coding that was complex at first, but we learned a lot of new things from what it presented, mainly we learned new code involving timing
//A switching function was added however, which switches between the speed being travelled and the distance travelled thus far

//calculations
//tire radius ~ 9.25 inches
//circumference = pi*2*r =~58 inches
//max speed of 56km/h =~ 612inches/second **VOIDABLE
//max rps =~7.25
//NOTE, WE DID NOT HAVE A REED. INSTEAD, WE USED A MAGNETIC SENSOR
//ALL REED VALUES AND VARIABLES REFER TO THE MAGNETIC SENSOR

//storage variables
float radius = 9.25; // tire radius (in inches), Man I wish we had an actual bike instead of Zafir's sister's bike

int reedVal;
long timer = 0; // time between one full rotation (in ms)
float kmh = 0.00;
float circumference;
boolean backlight;
float totalDistance = 0.0; // total distance travelled (in km)

int maxReedCounter = 100; // min time (in ms) of one rotation (for debouncing)
int reedCounter;
unsigned long totalRevolutions = 0; // total number of wheel revolutions
unsigned long lastReedTime = 0; // last time reed switch was activated

void setup(){
  
  reedCounter = maxReedCounter;
  circumference = 2 * 3.14 * radius * 1.6 / 12 / 5280; // convert circumference to KM
  pinMode(1, OUTPUT); // tx
  pinMode(2, OUTPUT); // backlight switch
  pinMode(A0, INPUT);
  
  checkBacklight();
  
  Serial.write(12); // clear
  
  // TIMER SETUP- the timer interrupt allows precise timed measurements of the reed switch
  cli(); // stop interrupts

  // set timer1 interrupt at 1kHz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0;
  // set timer count for 1kHz increments
  OCR1A = 1999; // = (1/1000) / ((1/(16*10^6))*8) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);   
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  sei(); // allow interrupts
  // END TIMER SETUP
  
  Serial.begin(9600);
}

void checkBacklight(){
  backlight = digitalRead(2);
  if (backlight){
    Serial.write(17); // turn backlight on
  }
  else{
    Serial.write(18); // turn backlight off
  }
}

ISR(TIMER1_COMPA_vect) { // Interrupt at freq of 1kHz to measure reed switch
  reedVal = digitalRead(A0); // get val of A0
  if (reedVal){ // if reed switch is closed
    if (reedCounter == 0){ // min time between pulses has passed
      unsigned long currentTime = millis();
      unsigned long timeElapsed = currentTime - lastReedTime;
      lastReedTime = currentTime;
      
      kmh = (circumference / (timeElapsed / 1000.0)) * 3600.0; // calculate KM per hour
      timer = 0; // reset timer
      reedCounter = maxReedCounter; // reset reedCounter
      totalRevolutions++; // increment total revolutions
      totalDistance = totalRevolutions * circumference; // update total distance
    }
    else{
      if (reedCounter > 0){ // don't let reedCounter go negative
        reedCounter -= 1; // decrement reedCounter
      }
    }
  }
  else{ // if reed switch is open
    if (reedCounter > 0){ // don't let reedCounter go negative
      reedCounter -= 1; // decrement reedCounter
    }
  }
  if (timer > 2000){
    kmh = 0; // if no new pulses from reed switch- tire is still, set kmh to 0
  }
  else{
    timer += 1; // increment timer
  } 
}

void calculateKMH() {
  if (millis() - lastReedTime > 2000) {
    kmh = 0; // if no pulses for more than 2 seconds, set speed to 0
  }
}

void displayKMH(){
  Serial.write(12); // clear
  Serial.write("Speed =");
  Serial.write(13); // start a new line
  Serial.print(kmh);
  Serial.write(" KM/H ");
}

void displayDISTANCE(){
  Serial.write(12); // clear
  Serial.write("Distance =");
  Serial.write(13); // start a new line
  Serial.print(totalDistance);
  Serial.write(" Kilometers ");
}

void loop(){
  calculateKMH();
  displayKMH();
  delay(1000); // update speed every 1 seconds
  displayDISTANCE();
  delay(1000); // update distance every 1 seconds
  checkBacklight();
}
