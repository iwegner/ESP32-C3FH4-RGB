//! This is a programm used for a carnival setup consisting of an infinite mirrow and a mask.
//! Both parts are connected to each other with one sequentially chain of WS2812b LEDs.
//! The following program controls the WS2812b strip which gets connected to an ESP32C3 board.
//! Requirements: 
//! * Program shall consist of several led color and animation modes.
//! * Board internal LEDs should not light up as board is hidden behind infinite mirror. 
//!   (Other possibility would have been to show the currently selected mode on the internal leds)
//! * User can press a push button to toggle through different lighning modes. Board continuously loops though modes.
//!
//! Hardware setup:
//! Look for guidance in web on how to attach a WS2812 strip to an arduino boad. 
//! Connect LED strip DIN wire to LED communication pin 10
//! Connect push button one side on pin4 and other side on ground of board.
//! 63 LEDs were using ~0.4A so using a powerbar connected to boards USB-C connection and using 5V provided by board for the LEDs was working fine.
//! Animatiion code copied from RGBWstrandtest and extended to the requirements mentioned above.


// For leds
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Pin configurations of ESP32C3 see https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf
// Name     No. Type    Power Domain    Function
// GPIO3    8   I/O/T   VDD3P3_RTC      GPIO3, ADC1_CH3                 <- used by internal LED matrix
// MTDI     10  I/O/T   VDD3P3_RTC      GPIO5, ADC2_CH0, FSPIWP, MTDI   <- can use for external LED strip

// Pin connected to the ESP32C3 internal LEDs
#define INTERNAL_LED_PIN 8
// Pin connected to the external LED strip
#define EXTERNAL_LED_PIN 10

// Pin for mode changing button
#define BUTTON_PIN 4

// Number of LEDs attached to board
#define INTERNAL_LED_COUNT  25
#define EXTERNAL_INFINITY_MIRROR_LED_COUNT 53 //infinity mirror
#define EXTERNAL_EYE_LED_COUNT 5              //each eye
#define EXTERNAL_LED_COUNT  63                //mirror + two eyes

// NeoPixel brightness, 0 (min) to 255 (max)
//#define INTERNAL_BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)
#define EXTERNAL_BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)


// Declare our NeoPixel strip objects:
// internl leds won't be visible because of mounting, so disable here
//Adafruit_NeoPixel internal_strip(INTERNAL_LED_COUNT, INTERNAL_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel external_strip(EXTERNAL_LED_COUNT, EXTERNAL_LED_PIN, NEO_GRB + NEO_KHZ800);


int mode_index = 0;              //!< variable to loop through different modes
const int NUMBER_OF_MODES = 9;   //!< number of modes


//! Fill strip pixels one after another with a color. Strip is NOT cleared first.
//! \param strip strip to be modified
//! \param color single 'packed' 32-bit value, which you can get by calling strip->Color(red, green, blue)
//! \param wait_ms delay time (in milliseconds) between pixels.
void ColorWipe(Adafruit_NeoPixel* strip, uint32_t color, int wait_ms = 0) 
{
    if (nullptr == strip) {
        return;
    }
    
    for(int i=0; i<strip->numPixels(); i++) {    // For each pixel in strip...
        strip->setPixelColor(i, color);         //  Set pixel's color (in RAM)
        strip->show();                          //  Update strip to match
        delay(wait_ms);                            //  Pause for a moment
    }
}


//! Displays a rainbow over pixels and a moving white strip
//! \param strip strip to be modified
//! \param white_speed speed of the white strip
//! \param white_length amount of pixels of the white strip
void WhiteOverRainbow(Adafruit_NeoPixel* strip, unsigned int white_speed, unsigned int white_length) 
{
    if (nullptr == strip) {
        return;
    }
    if( white_length >= strip->numPixels() ) {
        white_length = strip->numPixels() - 1;
    }

    int      head            = white_length - 1;
    int      tail            = 0;
    int      loops           = 1;
    int      loop_num        = 0;
    uint32_t last_time       = millis();
    uint32_t first_pixel_hue = 0;

    for(;;) { // Repeat forever (or until a 'break' or 'return')
        for(int i=0; i<strip->numPixels(); i++) {  // For each pixel in strip...
            if( ((i >= tail) && (i <= head)) ||      //  If between head & tail...
                ((tail > head) && ((i >= tail) ||
                 (i <= head))) ) 
            {
                strip->setPixelColor(i, strip->Color(255, 255, 255)); // Set white
            } else {                                             // else set rainbow
                int pixel_hue = first_pixel_hue + (i * 65536L / strip->numPixels());
                strip->setPixelColor(i, strip->gamma32(strip->ColorHSV(pixel_hue)));
            }
        }
        strip->show(); // Update strip with new contents
        // There's no delay here, it just runs full-tilt until the timer and
        // counter combination below runs out.

        first_pixel_hue += 40; // Advance just a little along the color wheel

        if((millis() - last_time) > white_speed) { // Time to update head/tail?
            if(++head >= strip->numPixels()) {      // Advance head, wrap around
                head = 0;
                if(++loop_num >= loops) {
                    return;
                }
            }
            if(++tail >= strip->numPixels()) {      // Advance tail, wrap around
                tail = 0;
            }
            last_time = millis();                   // Save time of last movement
        }
    }
}

//! Pulsing up and down with white color
//! \param strip strip to be modified
//! \param wait_ms speed for pulse, the higher the slower
void PulseWhite(Adafruit_NeoPixel* strip, uint8_t wait_ms) 
{
    if (nullptr == strip) {
        return;
    }
    for(int j=0; j<256; j++) { // Ramp up from 0 to 255
        // Fill entire strip with white at gamma-corrected brightness level 'j':
        strip->fill(strip->Color(strip->gamma8(j), strip->gamma8(j), strip->gamma8(j)));
        strip->show();
        delay(wait_ms);
    }

    for(int j=255; j>=0; j--) { // Ramp down from 255 to 0
        strip->fill(strip->Color(strip->gamma8(j), strip->gamma8(j), strip->gamma8(j)));
        strip->show();
        delay(wait_ms);
    }
}

//! Showing rainbow fade and a white fade
//! \param strip strip to be modified
//! \param wait_ms speed for animation
//! \param rainbow_loops amount of rainbow loops
//! \param white_loops amount of white loops
void RainbowFade2White(Adafruit_NeoPixel* strip, int wait_ms, int rainbow_loops, int white_loops) 
{
    if (nullptr == strip) {
        return;
    }

    int fade_val=0, fade_max=100;

    // Hue of first pixel runs 'rainbow_loops' complete loops through the color
    // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to rainbow_loops*65536, using steps of 256 so we
    // advance around the wheel at a decent clip.
    for(uint32_t first_pixel_hue = 0; first_pixel_hue < rainbow_loops*65536; first_pixel_hue += 256) {
        for(int i = 0; i < strip->numPixels(); i++) { // For each pixel in strip...

            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            uint32_t pixel_hue = first_pixel_hue + (i * 65536L / strip->numPixels());

            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the three-argument variant, though the
            // second value (saturation) is a constant 255.
            strip->setPixelColor(i, strip->gamma32(strip->ColorHSV(pixel_hue, 255, 255 * fade_val / fade_max)));
        }

        strip->show();
        delay(wait_ms);

        if(first_pixel_hue < 65536) { // First loop,
            if(fade_val < fade_max) {
                // fade in
                fade_val++;
            }
        } else if (first_pixel_hue >= ((rainbow_loops-1) * 65536)) { // Last loop,
            if(fade_val > 0) {
                // fade out
                fade_val--;
            }
        } else {
            fade_val = fade_max; // Interim loop, make sure fade is at max
        }
    }

    // White fade
    for(int k=0; k<white_loops; k++) {
        for(int j=0; j<256; j++) { // Ramp up 0 to 255
            // Fill entire strip with white at gamma-corrected brightness level 'j':
            strip->fill(strip->Color(strip->gamma8(j), strip->gamma8(j), strip->gamma8(j)));
            strip->show();
        }
        delay(1000); // Pause 1 second
        for(int j=255; j>=0; j--) { // Ramp down 255 to 0
            strip->fill(strip->Color(strip->gamma8(j), strip->gamma8(j), strip->gamma8(j)));
            strip->show();
        }
    }

    //delay(500); // Pause 1/2 second
}

//! Showing rainbow colors over time but all leds the same color
//! \param strip strip to be modified
//! \param wait_ms speed for animation
void RainbowFull(Adafruit_NeoPixel* strip, int wait_ms) 
{
    if (nullptr == strip) {
        return;
    }

    // Hue of first pixel runs 'rainbow_loops' complete loops through the color
    // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to rainbow_loops*65536, using steps of 256 so we
    // advance around the wheel at a decent clip.
    for(uint32_t pixel_hue_over_time = 0; pixel_hue_over_time < 65536; pixel_hue_over_time += 256) {
        for(int i = 0; i < strip->numPixels(); i++) { // For each pixel in strip...
            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the three-argument variant, though the
            // second value (saturation) is a constant 255.
            strip->setPixelColor(i, strip->gamma32(strip->ColorHSV(pixel_hue_over_time, 255, 255)));
        }
        strip->show();
        delay(wait_ms);
    }
}

//! Showing rainbow colors devided up over the led strip
//! \param strip strip to be modified
//! \param wait_ms speed for animation
void Rainbow(Adafruit_NeoPixel* strip, int wait_ms) 
{
    if (nullptr == strip) {
        return;
    }

    // Hue of first pixel runs 'rainbow_loops' complete loops through the color
    // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to rainbow_loops*65536, using steps of 256 so we
    // advance around the wheel at a decent clip.
    for(uint32_t first_pixel_hue = 0; first_pixel_hue < 65536; first_pixel_hue += 256) {
        for(int i = 0; i < strip->numPixels(); i++) { // For each pixel in strip...

            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            uint32_t pixel_hue = first_pixel_hue + (i * 65536L / strip->numPixels());

            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the three-argument variant, though the
            // second value (saturation) is a constant 255.
            strip->setPixelColor(i, strip->gamma32(strip->ColorHSV(pixel_hue, 255, 255)));
        }

        strip->show();
        delay(wait_ms);
    }
}

//! Show moving stripes so that it looks like a gear rotating
//! \param strip strip to be modified
//! \param color color to be shown during animation
//! \param wait_ms speed for animation
void Gears(Adafruit_NeoPixel* strip, uint32_t color, int wait_ms)
{
    if (nullptr == strip) {
        return;
    }
    // Set eyes to the given color
    for (int i = EXTERNAL_INFINITY_MIRROR_LED_COUNT; 
        i < EXTERNAL_INFINITY_MIRROR_LED_COUNT + (EXTERNAL_EYE_LED_COUNT*2); 
        i++) 
    {
        strip->setPixelColor(i, color);
    }
    strip->show();
    
    const int skip_width = 5; //!< how many leds shall be skipped
    // Only do in infinity mirror
    for ( unsigned int gear_animaton_counter = 0; 
          gear_animaton_counter < (skip_width*2); 
          gear_animaton_counter++){
        bool on = gear_animaton_counter<skip_width?true:false;
        //Serial.println(gear_animaton_counter);
        for (int i = 0; i < EXTERNAL_INFINITY_MIRROR_LED_COUNT; i++) {
           if ( ((skip_width*2)+ i - gear_animaton_counter) % skip_width == 0){
                on = !on;
            }
            if (on) {
                strip->setPixelColor(i, color);
            } else {
                strip->setPixelColor(i, external_strip.Color(0, 0, 0));
            }
        }
        strip->show();
        delay(wait_ms);
    }
}


//! Show leds pulsing like a haert beat
//! \param strip strip to be modified
//! \param r, g, b color aspect between [0,1] to be shown during animation
//! \param wait_ms speed for animation
void HeartBeat(Adafruit_NeoPixel* strip, float r, float g, float b, int wait_ms)
{
    if (nullptr == strip) {
        return;
    }

    for(int j=100; j<256; j+=2) { // Ramp up from 0 to 255
        // Fill entire strip with white at gamma-corrected brightness level 'j':
        strip->fill(strip->Color(r * strip->gamma8(j), g * strip->gamma8(j), b * strip->gamma8(j)));
        strip->show();
        delay(wait_ms);
    }

    for(int j=255; j>=180; j-=2) { // Ramp down from 255 to 127
        strip->fill(strip->Color(r * strip->gamma8(j), g * strip->gamma8(j), b * strip->gamma8(j)));
        strip->show();
        delay(wait_ms);
    }
    for(int j=181; j<256; j+=2) { // Ramp up from 128 to 255
        // Fill entire strip with white at gamma-corrected brightness level 'j':
        strip->fill(strip->Color(r * strip->gamma8(j), g * strip->gamma8(j), b * strip->gamma8(j)));
        strip->show();
        delay(wait_ms);
    }
    for(int j=255; j>=100; j-=2) { // Ramp down from 255 to 0
        strip->fill(strip->Color(r * strip->gamma8(j), g * strip->gamma8(j), b * strip->gamma8(j)));
        strip->show();
        delay(wait_ms);
    }
}

//! Show leds flashing in a circle
//! \param strip strip to be modified
//! \param r, g, b color aspect between [0,1] to be shown during animation
//! \param background_color color of all leds while the circle shows
//! \param wait_ms speed for animation
void CircleFlash(Adafruit_NeoPixel* strip, uint32_t color, uint32_t background_color, int wait_ms)
{
    if (nullptr == strip) {
        return;
    }

    // Set eyes to constant color
    for (int i = EXTERNAL_INFINITY_MIRROR_LED_COUNT; 
        i < EXTERNAL_INFINITY_MIRROR_LED_COUNT + (EXTERNAL_EYE_LED_COUNT*2); 
        i++) {
        strip->setPixelColor(i, color);
    }
    
    // Set all infinity mirror leds to off
    for (int i = 0; i < EXTERNAL_INFINITY_MIRROR_LED_COUNT; i++) {
        strip->setPixelColor(i, background_color);
    }
    strip->show();
    
    // Now shoot a flash along the infinite mirror
    strip->setPixelColor(0, color);
    strip->show();
    for (int i = 0; i < EXTERNAL_INFINITY_MIRROR_LED_COUNT-1; i++) {
        strip->setPixelColor(i, background_color);
        strip->setPixelColor(i+1, color);
        strip->show();
        delay(wait_ms);
    }
}

void setup()
{
    // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
    // Any other board, you can remove this part (but no harm leaving it):
    #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
        clock_prescale_set(clock_div_1);
    #endif

    // Setup button
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Initialize the internal strip
    // internal_strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    // internal_strip.show();            // Turn OFF all pixels ASAP
    // internal_strip.setBrightness(INTERNAL_BRIGHTNESS);

    // Initialize the external strip
    external_strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    external_strip.show();            // Turn OFF all pixels ASAP
    external_strip.setBrightness(EXTERNAL_BRIGHTNESS); 
}

void loop()
{
    // Read button state to change mode
    byte buttonState = digitalRead(BUTTON_PIN);
    if (buttonState == LOW) {
        Serial.println("Changing mode to " + mode_index);
        // slow down to take care of double inputs
        delay(250);
        mode_index++;
        
        // Loop through modes
        if ( mode_index >= NUMBER_OF_MODES ) {
            mode_index = 0;
        }
    }

    // Individual LED animations should be as short as possible to allow changing modes
    switch (mode_index) {
        case 0:
            RainbowFull(&external_strip, 35);
            break;
        case 1:
            Rainbow(&external_strip, 5);
            break;
        case 2:
            ColorWipe(&external_strip, external_strip.Color(255,   0,   0), 10); // Red            
            break;
        case 3:
            ColorWipe(&external_strip, external_strip.Color(0, 255,   0), 10); // Green
            break;
        case 4:
            ColorWipe(&external_strip, external_strip.Color(0,   0, 255), 10); // Blue
            break;
        case 5:
            ColorWipe(&external_strip, external_strip.Color(  255,   255,   255), 10); // (RGB white)
            break;
        case 6:
            Gears(&external_strip, external_strip.Color(  255,   0,   0), 50);
            break;
        case 7:
            Gears(&external_strip, external_strip.Color(  0,   0,   255), 50);
            break;
        case 8:
            HeartBeat(&external_strip, 1.0, 0.0, 0.0, 4);
            break;
        // case 9:
        //     CircleFlash(&external_strip, external_strip.Color(255, 0, 0), external_strip.Color(20, 20, 20), 15);
        //     break;
        default: 
            Serial.println("Exceeded mode! Check implementation.");
    }
}
