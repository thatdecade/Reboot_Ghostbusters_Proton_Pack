/*
   Reboot Ghostbusters Proton Pack (lights only)
   NeoPixel Driver for Trinket Pro

   Authors: Dustin Westaby, Danielle Gormley
   Date: August 31, 2016

   Use a 24 LED neopixel ring, or loop a strip.
   Animation is to match the Proton Pack from 2016 movie

   Use potentiometer for a color selection knob.

*/

#include <Adafruit_NeoPixel.h>

//neopixel strip config
#define NEO_PIN   6
#define NUM_LEDS  24

//color selection knob config
#define POT_PIN             A0
#define ENABLE_COLOR_KNOB 	1        //Set to 0 if you do not plan to use a potentiometer for color selection
#define USER_COLOR 			0xff0000 //default color, red

//animation options (larger speed = slower)
#define SLOW_SWEEP_SPEED            100
#define SPIN_UP_SPEED               50
#define MIN_SPIN_UP_SPINS            5    //will spin random number of times, at least this many
#define MAX_SPIN_UP_SPINS           15    //will spin random number of times, up to this many
#define FADE_SPEED                  20
#define FADE_DURATION_CHANCE        5     //good chance of escaping loop
#define MOVIE_SPIN_SPEED            50
#define MOVIE_SPIN_PERCENTAGE       0.74  //this is the percentage of the ring that is ON during the chase animation
#define MOVIE_SPIN_DURATION_CHANCE  30    //small chance that startup animation will repeat

//initialization
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);
uint32_t colorselected = USER_COLOR;

void setup()
{

	// This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
	if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
	// End of trinket special code

	strip.begin();
	strip.show(); // Initialize all pixels to 'off'

	randomSeed(analogRead(A1)); //read unused pin

	//get user selected color color on startup
	update_color_and_delay(0);
}


//animation loop
void loop()
{
	//slow sweep
	colorWipe(SLOW_SWEEP_SPEED);

	//spin up, faster and faster
	RunningLights(random(MIN_SPIN_UP_SPINS, MAX_SPIN_UP_SPINS), SPIN_UP_SPEED);

	//fade for a while
	while(random(0, FADE_DURATION_CHANCE))
	{
		FadeOut(FADE_SPEED);
		FadeIn(FADE_SPEED / 4);
	}
	FadeOut(FADE_SPEED);

	//classic movie spin
	while(random(0, MOVIE_SPIN_DURATION_CHANCE))
	{
		three_quadrant_spin(MOVIE_SPIN_SPEED);
	}

	//get ready for starup animations to repeat
	FadeOut(FADE_SPEED / 2);
	setAll(0, 0, 0);
}

void update_color_and_delay(uint16_t wait)
{
	int analog = 0;
	uint32_t colorwheel[] =
	{
		0xff0000, //0 red
		0x800000, //1 maroon
		0xff4000, //2 orange
		0xffff00, //3 yellow
		0x808000, //4 olive
		0x800080, //5 purple
		0xff00ff, //6 fuschia
		0xffffff, //7 white
		0x000080, //8 navy
		0x0000ff, //9 blue
		0x00ffff, //10 aqua
		0x008080, //11 teal
		0x00ff00, //12 lime
		0x008000, //13 green
	};

	if(ENABLE_COLOR_KNOB)
	{
		//read potentiometer and use a map lookup to update global color
		analog = analogRead(POT_PIN);
		colorselected = colorwheel[map(analog, 0, 1024, 0, ( sizeof( colorwheel ) / sizeof( uint32_t ) ) )];
	}

	delay(wait);
}

void three_quadrant_spin(uint8_t wait)
{
	float on_percentage = MOVIE_SPIN_PERCENTAGE;
	float off_percentage = 1 - on_percentage;

	//start with percent on
	for(int i = 0; i < NUM_LEDS; i++ )
	{
		if(i < (NUM_LEDS * on_percentage))
		{
			strip.setPixelColor(i, colorselected);
		}
		else
		{
			strip.setPixelColor(i, 0);
		}
	}
	strip.show();
	delay(1);
	
	//animate 2 pixels at a time (chasers)
	for (int i = 0; i < strip.numPixels(); i++)
	{
		//handle max pixel to pixel 0 transition
		if( ( i + (NUM_LEDS * on_percentage) ) < NUM_LEDS )
		{
			strip.setPixelColor(i, 0);
			strip.setPixelColor(i + (NUM_LEDS * on_percentage), colorselected);
		}
		else
		{
			strip.setPixelColor(i, 0);
			strip.setPixelColor(i - (NUM_LEDS * off_percentage), colorselected);
		}

		strip.show();
		update_color_and_delay(wait);
	}
}


byte getRed(uint32_t c)
{
	return (c >> 16) & 0xFF;
}
byte getGreen(uint32_t c)
{
	return (c >> 8) & 0xFF;
}
byte getBlue(uint32_t c)
{
	return (c) & 0xFF;
}

void FadeIn(uint8_t wait)
{
	float r, g, b;

	for(int k = 0; k < 256; k = k + 1)
	{
		r = (k / 256.0) * getRed(colorselected);
		g = (k / 256.0) * getGreen(colorselected);
		b = (k / 256.0) * getBlue(colorselected);
		setAll(r, g, b);
		strip.show();
		update_color_and_delay(wait);
	}
}

void FadeOut(uint8_t wait)
{
	float r, g, b;

	for(int k = 255; k >= 0; k = k - 2)
	{
		r = (k / 256.0) * getRed(colorselected);
		g = (k / 256.0) * getGreen(colorselected);
		b = (k / 256.0) * getBlue(colorselected);
		setAll(r, g, b);
		strip.show();
		update_color_and_delay(wait);
	}
}

// Fill the dots one after the other with a color
void colorWipe(uint8_t wait)
{
	for(uint16_t i = 0; i < strip.numPixels(); i++)
	{
		strip.setPixelColor(i, colorselected);
		strip.show();
		update_color_and_delay(wait);
	}
}

// Nifty pulsing spinning animation
void RunningLights(uint8_t spins, int WaveDelay)
{
	int Position = 0;

	for(int j = 0; j < NUM_LEDS * spins; j++) //number of spins
	{
		Position--;
		for(int i = 0; i < NUM_LEDS; i++)
		{
			strip.setPixelColor(i,
			                    strip.Color( ((sin(i + Position) * 127 + 128) / 255)*getRed(colorselected),
			                                 ((sin(i + Position) * 127 + 128) / 255)*getGreen(colorselected),
			                                 ((sin(i + Position) * 127 + 128) / 255)*getBlue(colorselected)) );
		}

		strip.show();

		//speed up the animation with each succession
		update_color_and_delay(WaveDelay - map(j, 0, NUM_LEDS * spins, 0, WaveDelay + 1));

		/* example delay math: 24 LEDs * 10 spins, with 50 Delay

			loop	map		delay
			0		 0		50 (slowest)
			40		 8		42
			80		17		33
			120		25		25
			160		34		16
			200		42		 8
			239		50		 0 (fastest)

		*/
	}

}

void setAll(byte red, byte green, byte blue)
{
	for(int i = 0; i < NUM_LEDS; i++ )
	{
		strip.setPixelColor(i, strip.Color(red, green, blue));
	}
	strip.show();
}

