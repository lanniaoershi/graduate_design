/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x32|64 size display using SPI or I2C to communicate
4 or 5 pins are required to interface

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution

02/18/2013 	Charles-Henri Hallard (http://hallard.me)
						Modified for compiling and use on Raspberry ArduiPi Board
						LCD size and connection are now passed as arguments on 
						the command line (no more #define on compilation needed)
						ArduiPi project documentation http://hallard.me/arduipi

						
*********************************************************************/

#include "ArduiPi_SSD1306.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include <pthread.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define PRG_NAME        "ssd1306_demo"
#define PRG_VERSION     "1.1"

#define BUFFSIZE 512
// Instantiate the display
Adafruit_SSD1306 display;

// Config Option
struct s_opts
{
	int oled;
	int verbose;
} ;

// default options values
s_opts opts = {
	OLED_ADAFRUIT_SPI_128x32,	// Default oled
  false										// Not verbose
};

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 






static unsigned char DU []= {	
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000011,0b10000000,0b00000000,
0b00000000,0b00000001,0b10000000,0b00000000,
0b00000111,0b11111111,0b11111111,0b11000000,
0b00000111,0b11111111,0b11111111,0b11000000,
0b00000100,0b00000000,0b00000000,0b00000000,
0b00000100,0b00001100,0b00001100,0b00000000,
0b00000100,0b00001100,0b00001100,0b00000000,
0b00000100,0b00001100,0b00001100,0b00000000,
0b00000111,0b11111111,0b11111111,0b11000000,
0b00000111,0b11111111,0b11111111,0b11000000,
0b00000100,0b00001100,0b00001100,0b00000000,
0b00000100,0b00001100,0b00001100,0b00000000,
0b00000100,0b00001111,0b11111100,0b00000000,
0b00000100,0b00001111,0b11111100,0b00000000,
0b00000100,0b00000000,0b00000000,0b00000000,
0b00000100,0b01110000,0b00000000,0b00000000,
0b00001100,0b01111111,0b11111111,0b00000000,
0b00001100,0b00001000,0b00000110,0b00000000,
0b00001100,0b00001100,0b00001100,0b00000000,
0b00001100,0b00000110,0b00011000,0b00000000,
0b00001000,0b00000011,0b00110000,0b00000000,
0b00011000,0b00000001,0b11100000,0b00000000,
0b00011000,0b00000001,0b11110000,0b00000000,
0b00110000,0b00011111,0b11111111,0b10000000,
0b00110011,0b11111100,0b00001111,0b11110000,
0b00000001,0b11000000,0b00000000,0b11100000,
0b00000000,0b00000000,0b00000000,0b00000000
};


static unsigned char SHI []= {	
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00011100,0b00011111,0b11111111,0b11000000,
0b00001110,0b00011111,0b11111111,0b11000000,
0b00000111,0b00011000,0b00000000,0b11000000,
0b00000011,0b10011000,0b00000000,0b11000000,
0b00000000,0b00011111,0b11111111,0b11000000,
0b00000000,0b00011111,0b11111111,0b11000000,
0b00000000,0b00011000,0b00000000,0b11000000,
0b00110000,0b00011000,0b00000000,0b11000000,
0b00111100,0b00011000,0b00000000,0b11000000,
0b00001110,0b00011111,0b11111111,0b11000000,
0b00000110,0b00011111,0b11111111,0b11000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000001,0b10001100,0b00000000,
0b00000000,0b00100001,0b10001100,0b00100000,
0b00000010,0b01100001,0b10001100,0b00110000,
0b00000110,0b00110001,0b10001100,0b01100000,
0b00000110,0b00110001,0b10001100,0b01100000,
0b00000100,0b00011001,0b10001100,0b11000000,
0b00001100,0b00011001,0b10001100,0b11000000,
0b00001100,0b00001111,0b10001111,0b10000000,
0b00011000,0b00000001,0b10001100,0b00000000,
0b00011000,0b00000001,0b10001100,0b00000000,
0b00011000,0b01111111,0b11111111,0b11110000,
0b00110000,0b01111111,0b11111111,0b11110000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000
};


static unsigned char WEN []= {	
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00001000,0b00000000,0b00000000,0b00000000,
0b00001100,0b00011111,0b11111111,0b00000000,
0b00001110,0b00011111,0b11111111,0b00000000,
0b00000011,0b00011000,0b00000001,0b00000000,
0b00000001,0b00011000,0b00000001,0b00000000,
0b00000000,0b00011111,0b11111111,0b00000000,
0b00000000,0b00011111,0b11111111,0b00000000,
0b00010000,0b00011000,0b00000001,0b00000000,
0b00111100,0b00011000,0b00000001,0b00000000,
0b00001110,0b00011000,0b00000001,0b00000000,
0b00000110,0b00011111,0b11111111,0b00000000,
0b00000000,0b00011111,0b11111111,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00111111,0b11111111,0b11000000,
0b00000110,0b00111111,0b11111111,0b11000000,
0b00000110,0b00100001,0b00011000,0b11000000,
0b00000100,0b00100001,0b00011000,0b11000000,
0b00001100,0b00100001,0b00011000,0b11000000,
0b00001100,0b00100001,0b00011000,0b11000000,
0b00001000,0b00100001,0b00011000,0b11000000,
0b00011000,0b00100001,0b00011000,0b11000000,
0b00011000,0b00100001,0b00011000,0b11000000,
0b00111001,0b11111111,0b11111111,0b11110000,
0b00010001,0b11111111,0b11111111,0b11110000,
0b00000000,0b00000000,0b00000000,0b00000000,
0b00000000,0b00000000,0b00000000,0b00000000
};



void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.print("\n");
  }    
  display.display();
}

void testdrawcircle(void) {
  for (int16_t i=0; i<display.height(); i+=2) {
    display.drawCircle(display.width()/2, display.height()/2, i, WHITE);
    display.display();
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.height()/2; i+=3) {
    // alternate colors
    display.fillRect(i, i, display.width()-i*2, display.height()-i*2, color%2);
    display.display();
    color++;
  }
}

void testdrawtriangle(void) {
  for (int16_t i=0; i<min(display.width(),display.height())/2; i+=5) {
    display.drawTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    display.display();
  }
}

void testfilltriangle(void) {
  uint8_t color = WHITE;
  for (int16_t i=min(display.width(),display.height())/2; i>0; i-=5) {
    display.fillTriangle(display.width()/2, display.height()/2-i,
                     display.width()/2-i, display.height()/2+i,
                     display.width()/2+i, display.height()/2+i, WHITE);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}

void testdrawroundrect(void) {
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.drawRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, WHITE);
    display.display();
  }
}

void testfillroundrect(void) {
  uint8_t color = WHITE;
  for (int16_t i=0; i<display.height()/2-2; i+=2) {
    display.fillRoundRect(i, i, display.width()-2*i, display.height()-2*i, display.height()/4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.display();
  }
}
   
void testdrawrect(void) {
  for (int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, WHITE);
    display.display();
  }
}

void testdrawline() {  
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, WHITE);
    display.display();
  }
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, WHITE);
    display.display();
  }
  usleep(250000);
  
  display.clearDisplay();
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(0, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(0, display.height()-1, display.width()-1, i, WHITE);
    display.display();
  }
  usleep(250000);
  
  display.clearDisplay();
  for (int16_t i=display.width()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, i, 0, WHITE);
    display.display();
  }
  for (int16_t i=display.height()-1; i>=0; i-=4) {
    display.drawLine(display.width()-1, display.height()-1, 0, i, WHITE);
    display.display();
  }
  usleep(250000);

  display.clearDisplay();
  for (int16_t i=0; i<display.height(); i+=4) {
    display.drawLine(display.width()-1, 0, 0, i, WHITE);
    display.display();
  }
  for (int16_t i=0; i<display.width(); i+=4) {
    display.drawLine(display.width()-1, 0, i, display.height()-1, WHITE); 
    display.display();
  }
  usleep(250000);
}

void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.print("scroll");
  display.display();
 
  display.startscrollright(0x00, 0x0F);
  sleep(2);
  display.stopscroll();
  sleep(1);
  display.startscrollleft(0x00, 0x0F);
  sleep(2);
  display.stopscroll();
  sleep(1);    
  display.startscrolldiagright(0x00, 0x07);
  sleep(2);
  display.startscrolldiagleft(0x00, 0x07);
  sleep(2);
  display.stopscroll();
}


/* ======================================================================
Function: usage
Purpose : display usage
Input 	: program name
Output	: -
Comments: 
====================================================================== */
void usage( char * name)
{
	printf("%s\n", name );
	printf("Usage is: %s --oled type [options]\n", name);
	printf("  --<o>led type\nOLED type are:\n");
	for (int i=0; i<OLED_LAST_OLED;i++)
		printf("  %1d %s\n", i, oled_type_str[i]);
	
	printf("Options are:\n");
	printf("  --<v>erbose  : speak more to user\n");
	printf("  --<h>elp\n");
	printf("<?> indicates the equivalent short option.\n");
	printf("Short options are prefixed by \"-\" instead of by \"--\".\n");
	printf("Example :\n");
	printf( "%s -o 1 use a %s OLED\n\n", name, oled_type_str[1]);
	printf( "%s -o 4 -v use a %s OLED being verbose\n", name, oled_type_str[4]);
}


/* ======================================================================
Function: parse_args
Purpose : parse argument passed to the program
Input 	: -
Output	: -
Comments: 
====================================================================== */
void parse_args(int argc, char *argv[])
{
	static struct option longOptions[] =
	{
		{"oled"	  , required_argument,0, 'o'},
		{"verbose", no_argument,	  	0, 'v'},
		{"help"		, no_argument, 			0, 'h'},
		{0, 0, 0, 0}
	};

	int optionIndex = 0;
	int c;

	while (1) 
	{
		/* no default error messages printed. */
		opterr = 0;

    c = getopt_long(argc, argv, "vho:", longOptions, &optionIndex);

		if (c < 0)
			break;

		switch (c) 
		{
			case 'v': opts.verbose = true	;	break;

			case 'o':
				opts.oled = (int) atoi(optarg);
				
				if (opts.oled < 0 || opts.oled >= OLED_LAST_OLED )
				{
						fprintf(stderr, "--oled %d ignored must be 0 to %d.\n", opts.oled, OLED_LAST_OLED-1);
						fprintf(stderr, "--oled set to 0 now\n");
						opts.oled = 0;
				}
			break;

			case 'h':
				usage(argv[0]);
				exit(EXIT_SUCCESS);
			break;
			
			case '?':
			default:
				fprintf(stderr, "Unrecognized option.\n");
				fprintf(stderr, "Run with '--help'.\n");
				exit(EXIT_FAILURE);
		}
	} /* while */

	if (opts.verbose)
	{
		printf("%s v%s\n", PRG_NAME, PRG_VERSION);
		printf("-- OLED params -- \n");
		printf("Oled is    : %s\n", oled_type_str[opts.oled]);
		printf("-- Other Stuff -- \n");
		printf("verbose is : %s\n", opts.verbose? "yes" : "no");
		printf("\n");
	}	
}


  // init done
  //display.clearDisplay();   // clears the screen and buffer


  /*// draw a single pixel
  display.drawPixel(10, 10, WHITE);
  display.display();
  sleep(2);
  display.clearDisplay();
*/	
  /*// draw many lines
  testdrawline();
  display.display();
  sleep(20);
  display.clearDisplay();
*/
  /*// draw rectangles
  testdrawrect();
  display.display();
  sleep(2);
  display.clearDisplay();
*/
 /* // draw multiple rectangles
  testfillrect();
  display.display();
  sleep(2);
  display.clearDisplay();

  // draw mulitple circles
  testdrawcircle();
  display.display();
  sleep(2);
  display.clearDisplay();
*/
  /*// draw a white circle, 10 pixel radius
  display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
  display.display();
  sleep(2);
  display.clearDisplay();

  testdrawroundrect();
  sleep(2);
  display.clearDisplay();

  testfillroundrect();
  sleep(2);
  display.clearDisplay();

  testdrawtriangle();
  sleep(2);
  display.clearDisplay();
   
  testfilltriangle();
  sleep(2);
  display.clearDisplay();

  // draw the first ~12 characters in the font
  testdrawchar();
  display.display();
  sleep(2);
  display.clearDisplay();
*/
  /*display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Hello, world! kjhkjkghjhgjhghjg\n");
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.printf("%f\n", 3.141592);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.printf("0x%8X\n", 0xDEADBEEF);
  display.display();
	*/
char  *get_temp_hum(char *temp_hum){

    char row_msg[BUFFSIZE];
    char temperature[8];
    char humidity[8];
    FILE *fp = NULL;
    fp = fopen("./TH.data","r");
    if(fp == NULL){
        printf("can not  open Adafruit_DHT_bak.log\n");
        return "false";
    }
    int row_num = 0;
    char *tmp = NULL;
    memset(temperature,0,sizeof(temperature));
    memset(humidity,0,sizeof(humidity));
    while(!feof(fp)){
        memset(row_msg,0,BUFFSIZE);
        tmp = fgets(row_msg,BUFFSIZE,fp);
        if(tmp == NULL) break;
        ++row_num;
        if(row_num == 3){
            int i = 0;
            int j = 0;
            char *p = strchr(tmp,'=');
            for(;i < strlen(p);i++){
                if(isdigit(p[i])||p[i] == '.'){
                    temperature[j]=p[i];
                    if(temperature[j] == '.'){
                        temperature[++j]=p[++i];
                        break;
                    }
                    ++j;
                }
            }
            for(j=0,i++;i < strlen(p);i++){
                if(isdigit(p[i])||p[i] == '.'){
                    humidity[j]=p[i];
                    if(humidity[j] == '.'){
                        humidity[++j]=p[++i];
                        break;
                    }
                    ++j;
                }
            }
        }
    }
    memset(temp_hum,0,sizeof(temp_hum));
    if(strlen(temperature) > 0 && strlen(humidity) > 0){
        strcat(temp_hum,temperature);
        strcat(temp_hum,"&");
        strcat(temp_hum,humidity);
    }else{
        //printf("Sensor not ready, use old last data\n");
    }

    if(fp) fclose(fp); 


    //printf("%s\n",temp_hum);

    return temp_hum;
}


void* sensor_thread(void * argu);
void* oled_thread(void * argu);
/* ======================================================================
Function: main
Purpose : Main entry Point
Input 	: -
Output	: -
Comments: 
====================================================================== */

int main(int argc, char **argv)
{
	int i = 0;
	int j = 0;
	int count = 0;
	char result[20];
	char temp_hum[20];
	char temp[10] = "0";
	char hum[10] = "0";
	char old_temp[10] = "0";
	char old_hum[10] = "0";
	FILE *webdata;
	// Oled supported display in ArduiPi_SSD1306.h
	// Get OLED type
	parse_args(argc, argv);

	// SPI
	if (display.oled_is_spi_proto(opts.oled))
	{
		// SPI change parameters to fit to your LCD
		if ( !display.init(OLED_SPI_DC,OLED_SPI_RESET,OLED_SPI_CS, opts.oled) )
			exit(EXIT_FAILURE);
	}
	else
	{
		// I2C change parameters to fit to your LCD
		if ( !display.init(OLED_I2C_RESET,opts.oled) )
			exit(EXIT_FAILURE);
	}
		
		/*int ret1,ret2;
		pthread_t id1;
		ret1 = pthread_create(&id1, NULL,sensor_thread, NULL);
		if(ret1 != 0){
		fprintf(stderr,"Create sensor thread error\n");
		exit(0);
		}


		pthread_t id2;
		ret2 = pthread_create(&id2, NULL,oled_thread, NULL);
		if(ret2 != 0){
		fprintf(stderr,"Create sensor thread error\n");
		exit(0);
		}*/

		while(1){
		system("./Adafruit_DHT 22 4 > TH.data");

		strcpy(result,get_temp_hum(temp_hum));
		for(i=0;i<4;i++){
			
			temp[i] = result[i];

		}
		for(i=5,j=0;i<9;i++,j++){
			
			hum[j] = result[i];
		}
		temp[4] = '\0';
		hum[4] = '\0';
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~ID=%d~~~~~~~~~~~~~~~~~~~~~~~~~~\n",count);
		if(strlen(temp) > 1  && strlen(hum) > 1){
		strcpy(old_temp,temp);
		strcpy(old_hum,hum);
		printf("New data from sensor.\n");
		}else if(strlen(temp) < 1  || strlen(hum) < 1){
			strcpy(temp,old_temp);
			strcpy(hum,old_hum);
		//printf("Sensor not ready, use last data.\n");
        printf("Sensor not ready, use old last data\n");
		}
		printf("temp and hum =%s  %s\n",temp,hum);
		webdata = fopen("Data.data","w");
		fprintf(webdata,"@温度 = %s*C   湿度 = %s%%@\n",temp,hum);
		fclose(webdata);
	//	printf("\n\n\n\n%s   %s\n\n\n\n\n",temp,hum);
		display.begin();
	//	display.display();
		display.clearDisplay();
		display.setTextSize(2);
		display.setTextColor(WHITE);
		display.setCursor(65,7);
		display.printf("%s",temp);
		display.setCursor(116,7);
		display.printf("C");
		display.setCursor(65,43);
		display.printf("%s",hum);
		display.setCursor(116,43);
		display.printf("%%");

		display.drawBitmap(0,0, WEN,32,29,1);
		display.drawBitmap(32,0,DU,32,29,1);
		display.drawBitmap(0,36,SHI,32,29,1);
		display.drawBitmap(32,36,DU,32,29,1);
		display.drawLine(0,34,128,34,1);
		display.drawLine(0,33,128,33,1);
		display.drawLine(0,32,128,32,1);
		display.display();
		count++;
		sleep(5);
	
	} 

}
/*void* sensor_thread(void * argu)
{
	for(;;){
	printf("Sensor thread start...\n");
	system("./Adafruit_DHT 22 4 > TH.data");
	sleep(5);
	}
}*/

/*void* oled_thread(void * argu){
	for(;;){
		
		strcpy(result,get_temp_hum(temp_hum));
		for(i=0;i<4;i++){
			
			temp[i] = result[i];

		}
		for(i=5,j=0;i<9;i++,j++){
			
			hum[j] = result[i];
		}
		temp[4] = '\0';
		hum[4] = '\0';
		if(strlen(temp) > 1  && strlen(hum) > 1){
		strcpy(old_temp,temp);
		strcpy(old_hum,hum);
		printf("if!!!!!!\n");
		}else if(strlen(temp) < 1  || strlen(hum) < 1){
			strcpy(temp,old_temp);
			strcpy(hum,old_hum);
		printf("else!!!!!!\n");
		}
		printf("temp and hum =%s\n",result);
		printf("\n\n\n\n%s   %s\n\n\n\n\n",temp,hum);

		display.begin();
	//	display.display();
		display.clearDisplay();
		display.setTextSize(2);
		display.setTextColor(WHITE);
		display.setCursor(65,7);
		display.printf("%s",temp);
		display.setCursor(116,7);
		display.printf("C");
		display.setCursor(65,43);
		display.printf("%s",hum);
		display.setCursor(116,43);
		display.printf("%%");
		display.drawBitmap(0,0, WEN,32,29,1);
		display.drawBitmap(32,0,DU,32,29,1);
		display.drawBitmap(0,36,SHI,32,29,1);
		display.drawBitmap(32,36,DU,32,29,1);
		display.drawLine(0,34,128,34,1);
		display.drawLine(0,33,128,33,1);
		display.drawLine(0,32,128,32,1);
		display.display();
		sleep(6);
	}
}*/



