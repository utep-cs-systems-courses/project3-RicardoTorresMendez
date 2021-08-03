#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

u_char interrupts = 31;

static char 
switch_update_interrupt_sense(){
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

void draw_hourglass_frame();
void draw_hourglass_sand();
void update_shape();
void reset_screen();
void state_advance( u_char state );

int switches = 0;
u_char state;
static int secCount = 0;

void
switch_interrupt_handler(){
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
  if( switches & SW1 ){ state = 0; }
  if( switches & SW2 ){ state = 1; }
  if( switches & SW3 ){ state = 2; }
  if( switches & SW4 ){ state = 3; }
  if( switches ){
    state_advance( state );
  }
}

void state_advance( u_char state ){
  switch( state ){
    case 0:
      interrupts = 31;//5s
      break;
    case 1:
      interrupts = 62;//10s
      break;
    case 2:
      interrupts = 185;//30s
      break;
    case 3:
      secCount = 0;
      break;
  }
  buzzer_set_period( 0 );
}


// axis zero for col, axis 1 for row
short drawPos[2] = {10,10}, controlPos[2] = {10,10};
short velocity[2] = {3,8}, limits[2] = {screenWidth-36, screenHeight-8};

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;

void wdt_c_handler(){
  secCount ++;
  if ( secCount >= interrupts ) {		/* 10/sec */
    secCount = 0;
    redrawScreen = 1;
  }     
}
  

void main(){
  
  P1DIR |= LED;		/**< Red led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  
  enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  reset_screen();

  while(1){			/* forever */
    if (redrawScreen && state < 3) {
      redrawScreen = 0;
      draw_hourglass_sand();
    }
    P1OUT &= ~LED;	/* led off */
    or_sr(0x10);	/**< CPU OFF */
    P1OUT |= LED;	/* led on */
  }
}

void reset_screen(){
  clearScreen(COLOR_GREEN_YELLOW);
  draw_hourglass_frame();
}

void draw_hourglass_sand(){
  static int layer = 0;
  static int col = 23, row = 19, width = 80;
  static int col_2 = 13, row_2 = 111, width_2 = 100, j = 0;
  int total = 40;
  if( layer == total ){
    layer = 0;
    state = 0;
    buzzer_set_period( 300 );
    return;
  }



  fillRectangle( col + layer, row + layer, width - layer - layer, 1, COLOR_GREEN_YELLOW);

  if( layer % 2 == 0 ){
    j = layer / 2;
    fillRectangle( col_2 + j, row_2 - j, width_2 - j - j, 1, COLOR_GRAY);
  }

  layer++;
}

void draw_hourglass_frame(){
  //glass
  fillRectangle(10, 7, 107, 2, COLOR_WHITE);
  fillRectangle(10,112, 107, 2, COLOR_WHITE);
  drawDiagonal( 10, 7, 1, 107, COLOR_WHITE );
  drawDiagonal( 10, 112, -1, 107, COLOR_WHITE );

  //sand
  int col = 62, row = 58, width = 2;
  for( int i=0; i<40; i++ ){
    fillRectangle( col - i, row - i, width + i + i, 1, COLOR_GRAY);
  }

}

void update_shape(){
  static unsigned char row = screenHeight / 2, col = screenWidth / 2;
  static char blue = 31, green = 0, red = 31;
  static unsigned char step = 0;
  if(step <= 60){
    int startCol = col - step;
    int endCol = col + step;
    int width = 1 + endCol - startCol;

    fillRectangle(startCol, row+step, width, 1, COLOR_WHITE);
    fillRectangle(startCol, row-step, width, 1, COLOR_WHITE);

    step++;
  }
  else{
     clearScreen(COLOR_BLUE);
     step = 0;
  }
}

int color( char blue, char green, char red ){
  return (blue << 11) | (green << 5) | red;
}

/* Switch on S2 */
void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
