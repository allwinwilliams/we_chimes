#define PIR 26  //yellow

#define VIBRATION 27  //orange

#define ACCEL_X 33  //brown
#define ACCEL_Y 32  //grey
#define ACCEL_Z 35  //white

#define LED_R 18
#define LED_G 19
#define LED_B 21

#define MAXLED 255

#define RANGE 100


uint8_t ledArray[3] = {1, 2, 3}; // three led channels

const boolean invert = true; // set true if common anode, false if common cathode

uint8_t color = 0, hue = 200, save_hue = 200;          // a value from 0 to 255 representing the hue
uint32_t R, G, B ;           // the Red Green and Blue color components
uint8_t brightness = 255;  // 255 is maximum brightness, but can be changed.  Might need 256 for common anode to fully turn off.

uint8_t pir_reading = 0, prev_pir_reading = 0, vibration_reading = 0, prev_vibration_reading = 0;
float x = 0, y = 0, z = 0, acceleration = 0, prev_x =0, prev_y =0, prev_z =0, prev_acceleration = 0;

void initSensors(){
  pinMode(PIR, INPUT);
  pinMode(VIBRATION, INPUT);
  
  pinMode(ACCEL_X, INPUT);
  pinMode(ACCEL_Y, INPUT);
  pinMode(ACCEL_Z, INPUT); 
}

void initLED(){
  ledcAttachPin(LED_R, 1); // assign RGB led pins to channels
  ledcAttachPin(LED_G, 2);
  ledcAttachPin(LED_B, 3);
  
  // Initialize channels 
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(2, 12000, 8);
  ledcSetup(3, 12000, 8);

  brightness = 100;
}


void readPIR(){
  prev_pir_reading = pir_reading;
  pir_reading = analogRead(PIR);
}

void readVIBRATION(){
  prev_vibration_reading = vibration_reading;
  vibration_reading = analogRead(VIBRATION);
}

void readAccelerometer(){
  prev_x = x;
  prev_y = y;
  prev_z = z;
  prev_acceleration = acceleration;
  
  x = analogRead(ACCEL_X);
  y = analogRead(ACCEL_Y);
  z = analogRead(ACCEL_Z);
  acceleration = sqrt(x * x + y * y + z * z);
}

void readAll(){
  readPIR();
  readVIBRATION();
  readAccelerometer();
}


uint32_t max(uint32_t r, uint32_t g, uint32_t b){
  if(r > g){
    if(r > b)
      return r;
    return b;
  }else{
    if(g > b)
      return g;
    return b;
  }
}

uint32_t min(uint32_t r, uint32_t g, uint32_t b){
  if(r < g){
    if(r < b)
      return r;
    return b;
  }else{
    if(g < b)
      return g;
    return b;
  }
}

uint32_t max(uint32_t a, uint32_t b){
  if(a > b)
    return a;
  return b;
}

void setColor(uint32_t r, uint32_t g, uint32_t b){
  R = r;
  G = g;
  B = b;
  
  if(R > 255)
    R = 255;
  if(G > 255)
    G = 255;
  if(B > 255)
    B = 255;

  if(R < 0)
    R = 0;
  if(G < 0)
    G = 0;
  if(B < 0)
    B = 0;
}

void setBrightness(uint32_t bright){
  brightness = bright;
}


void changeBrightness(uint32_t value){
  brightness = brightness + value;
}

void writeRGB(){
  ledcWrite(1, 256 - R);
  ledcWrite(2, 256 - G);
  ledcWrite(3, 256 - B);
}

void writeRGB(uint32_t r, uint32_t g, uint32_t b){
  ledcWrite(1, 256 - r);
  ledcWrite(2, 256 - g);
  ledcWrite(3, 256 - b);
}

void saveHue(){
  save_hue = hue;
 }
uint8_t newHue(uint8_t h){
  return (h + 100) % 255;
}
void collision(){
  Serial.println("------ COLLISION -----------");
  Serial.print("Vibration: ");
  Serial.print(vibration_reading);
  Serial.print("diff: ");
  Serial.print(vibration_reading - prev_vibration_reading);
  Serial.println();
  writeRGB(0, 0, 0);
  delay(200);

  hue = newHue(hue);
  saveHue();
  hueToRGB(hue, brightness);
  writeRGB();
}

bool threshold(uint32_t v1, uint32_t v2){
  if(abs(v1 - v2) < 50)
    return false;
    
   return true;
}


bool accelerate_enough(){
//  if(threshold(x, prev_x) && threshold(y, prev_y) && threshold(z, prev_z)){
  if(abs(acceleration - prev_acceleration) > 100){
    Serial.println("*** ENOUGH ACCELERATION  ***");
    Serial.println("ACCELERATION:: ");
      
    Serial.println(acceleration);


    return true;
  }
  return false;
  
}

int32_t LEDChange(uint32_t value){
  uint32_t mag = hue + sqrt(sqrt(value));
  if(random(2) == 0){
    return -mag;  
  }
  return mag;
}

int32_t combine(int32_t r, uint32_t g, uint32_t b){
    return (r + g + b) /3;
}

int32_t change_brightness(int32_t value){
  int32_t avg = combine(R, G, B);
  int inc = value > 0 ? 1 : -1;
  while(abs(avg - value) > 10)
  {
    setColor(R + inc, G + inc, B + inc);  
    writeRGB();  
  }
  
}

void controlLED(){
   if(vibration_reading - prev_vibration_reading > 100) 
      collision();

  if(accelerate_enough()){
//    writeRGB(R + LEDChange(x), G + LEDChange(y), B + LEDChange(z));
    uint32_t hue_diff = int(sqrt(abs(acceleration - prev_acceleration)));
    uint32_t inc = 1;
    Serial.println("diff:: ");
    Serial.println(hue_diff);
    if(acceleration - prev_acceleration < 0){
        inc = -1;
    }
    for(int i = hue; hue_diff != 0; hue_diff = hue_diff - 1){
      if(hue + inc > 255)
        break;
      hueToRGB(hue + inc, brightness);
      writeRGB();
      delay(10);
    }
  } 

  if(pir_reading - prev_pir_reading > 30){
    Serial.println(">>>>>> motion detected <<<<<< ");
    Serial.print("PIR: ");  
    Serial.print(pir_reading);
    Serial.println();
    uint32_t bright_diff = int(sqrt(abs(pir_reading - prev_pir_reading)) * 8);
    Serial.print("bright_diff: ");  
    Serial.print(bright_diff);
    uint32_t max_color = max(R, G, B);
    
    Serial.println();
    
    for(int i = brightness; bright_diff != 0; bright_diff = bright_diff - 1){
//      hueToRGB(hue, brightness - 1);
      setColor(R - 1, B - 1, G - 1);
      writeRGB();
      delay(10);
    }
  }
}

void setup() {
  Serial.begin(115200);
  initSensors();   
  initLED();
  hueToRGB(hue, brightness);
  writeRGB();
}

void loop() {
  readAll();
    
//    Serial.print("PIR: ");
//    Serial.println(pir_reading);
//    
//    Serial.print("VIBRAION: ");
//    Serial.println(vibration_reading);
//    
//    Serial.println("ACCELERATION:: ");
//    
//    Serial.print("x: ");
//    Serial.println(x);
//    Serial.print("y: ");
//    Serial.println(y);
//    Serial.print("z: ");
//    Serial.println(z);    

//    Serial.println("----------------------------------");
  controlLED();
  delay(10);
}


void hueToRGB(uint8_t hue, uint8_t brightness)
{
    uint16_t scaledHue = (hue * 6);
    uint8_t segment = scaledHue / 256; // segment 0 to 5 around the
                                            // color wheel
    uint16_t segmentOffset =
      scaledHue - (segment * 256); // position within the segment

    uint8_t complement = 0;
    uint16_t prev = (brightness * ( 255 -  segmentOffset)) / 256;
    uint16_t next = (brightness *  segmentOffset) / 256;

    if(invert)
    {
      brightness = 255 - brightness;
      complement = 255;
      prev = 255 - prev;
      next = 255 - next;
    }

    switch(segment) {
    case 0:      // red
        setColor(brightness, next, complement);
    break;
    case 1:     // yellow
        setColor(prev, brightness, complement);
    break;
    case 2:     // green
        setColor(complement, brightness, next);
    break;
    case 3:    // cyan
        setColor(complement, prev, brightness);
    break;
    case 4:    // blue
        setColor(next, complement, brightness);
    break;
   case 5:      // magenta
    default:
        setColor(brightness, complement, prev);
    break;
    }
}
