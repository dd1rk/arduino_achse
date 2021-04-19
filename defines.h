/* pinning */
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin
#define TS_MINX 160
#define TS_MAXX 880
#define TS_MINY 100
#define TS_MAXY 880
#define STEPDIRPIN  10
#define STEPSTEPPIN 11
#define MOTORENABLE 12
#define HOMESWITCH  13
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4


/* colors */
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


/* Buttons */
#define BUTTONSIZE_X	100
#define BUTTONSIZE_Y	40
#define FIRSTBUTTON_X	40
#define FIRSTBUTTON_Y	20
#define BUTTONDIST_X	120
#define BUTTONDIST_Y	50

/* Touch */
#define MINPRESSURE	10
#define MAXPRESSURE	1000

/* speed */
#define PULSEDELAY	400
#define MYTIMEOUT	10

#define HUBSTEPS	500
