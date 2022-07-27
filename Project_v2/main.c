#include <stdio.h>
#include <stdlib.h>
#include "LPC17XX.H"
#include "GLCD.h"
#include "string.h"
#include "serial.h"
#include "timer.h"

#define BUFF_SIZE 1024

/*
INSTRUCTIONS:
HC06 Pins:
P0.10 -> RX
P0.11 -> TX
V -> 3.3V
GND -> GND

COM Port is different each time, last time used COM8

To manually change home/away team scores, send following commands with X being a number:
	homeScore=X
	awayScore=X
This would have been used for live score tracking
	
To change game 1, game 2, game 3 scores:
	firstGame=X_Y
	secondGame=X_Y
	thirdGame=X_Y
*/

/* VARIABLE DECLARATION */
// Timer Variables
volatile uint32_t timer0_counter = 0;
volatile int team_index = 0;
volatile int check = 1;

// Bluetooth Variables
char bt_receive_buff[BUFF_SIZE];
int bt_receive_n = 0;

/* LCD DIRECTORY */
int directory(char team[]){
	if(strcmp("76ers",team) == 0){
		return 0;
	}
	else if(strcmp("Bucks",team) == 0){
		return 1;
	}
	else if(strcmp("Bulls",team) == 0){
		return 2;
	}
	else if(strcmp("Cavaliers",team) == 0){
		return 3;
	}
	else if(strcmp("Celtics",team) == 0){
		return 4;
	}
	else if(strcmp("Clippers",team) == 0){
		return 5;
	}
	else if(strcmp("Grizzlies",team) == 0){
		return 6;
	}
	else if(strcmp("Hawks",team) == 0){
		return 7;
	}
	else if(strcmp("Heat",team) == 0){
		return 8;
	}
	else if(strcmp("Hornets",team) == 0){
		return 9;
	}
	else if(strcmp("Jazz",team) == 0){
		return 10;
	}
	else if(strcmp("Kings",team) == 0){
		return 11;
	}
	else if(strcmp("Knicks",team) == 0){
		return 12;
	}
	else if(strcmp("Lakers",team) == 0){
		return 13;
	}
	else if(strcmp("Magic",team) == 0){
		return 14;
	}
	else if(strcmp("Mavericks",team) == 0){
		return 15;
	}
	else if(strcmp("Nets",team) == 0){
		return 16;
	}
	else if(strcmp("Nuggets",team) == 0){
		return 17;
	}
	else if(strcmp("Pacers",team) == 0){
		return 18;
	}
	else if(strcmp("Pelicans",team) == 0){
		return 19;
	}
	else if(strcmp("Pistons",team) == 0){
		return 20;
	}
	else if(strcmp("Raptors",team) == 0){
		return 21;
	}
	else if(strcmp("Rockets",team) == 0){
		return 22;
	}
	else if(strcmp("Spurs",team) == 0){
		return 23;
	}
	else if(strcmp("Suns",team) == 0){
		return 24;
	}
	else if(strcmp("Thunder",team) == 0){
		return 25;
	}
	else if(strcmp("Timberwolves",team) == 0){
		return 26;
	}
	else if(strcmp("Trail Blazers",team) == 0){
		return 27;
	}
	else if(strcmp("Warriors",team) == 0){
		return 28;
	}
	else if(strcmp("Wizards",team) == 0){
		return 29;
	}
	else{
		//Output ERROR Message
		return -1;
	}
}


/* BLUETOOTH FUNCTIONS */
// NOTE: Perhaps put into it's own header file?
void clear_bt_receive_buff( void)	{
	// clear the buffer by setting all entries to 0 and reset bt_receive_n;
	int i = 0;
	for (i = 0; i < BUFF_SIZE; i++) {
		bt_receive_buff[i] = 0;
	}
	bt_receive_n = 0;
}

void sendCommand( char *command, uint32_t timeout, int debug)	{
	// similar to readFromWiFi(), but first sends an AT command;
	
	// (0) timeout must be in 10's of ms;
	unsigned long num_of_10ms_intervals  = timeout / 10;
	unsigned long millisec_stop_deadline = timer0_counter + num_of_10ms_intervals;
	char c;
	
	// (1) clear buffer first; also, resets counter of received characters;
	clear_bt_receive_buff(); 
	
	// (2) send command to wifi module; command will be send char by char;
	SERIAL_putString(2, command);	// <--- UART2
	
	// (3) check if anything in the receive buffer of the UART2;
	// get received characters one by one and place into bt_receive_buff;
	while ( timer0_counter < millisec_stop_deadline) {
		c = SERIAL_getChar_nb(2);
		if ( (c != 0) && (bt_receive_n < BUFF_SIZE) ) {
			bt_receive_buff[bt_receive_n] = c;
			bt_receive_n++;
		}
	}
	
	// (4) if debug is true, we also print to putty terminal on host PC;
	if (debug) {
		SERIAL_putString(0, "\r\n<-------- START response to sendCommand() -------->\r\n");
		SERIAL_putString(0, bt_receive_buff); // <--- UART0
		SERIAL_putString(0, "\r\n---------> END response to sendCommand() <---------\r\n");
	}
}


void readFromBT( uint32_t timeout, char initialChar)	{
	// Similar implementation to wifi equivalent
	unsigned long num_of_10ms_intervals = timeout / 10;
	unsigned long millisec_stop_deadline = timer0_counter + num_of_10ms_intervals;
	char c;
	
	clear_bt_receive_buff();
	
	bt_receive_buff[0] = initialChar;
	bt_receive_n = 1;
	while ( timer0_counter < millisec_stop_deadline)	{
		c = SERIAL_getChar_nb(2);
		if ( (c != 0) && (bt_receive_n < BUFF_SIZE) )	{
			bt_receive_buff[bt_receive_n] = c;
			bt_receive_n++;
		}
	}
}
/* MAIN FUNCTION */
int main(void)	{
	// LCD Variables
	//Basketball Data Arrays
	uint16_t team_colors[30][2] = {
	{White, Blue}, {DarkGreen, White}, {Red, Black},
	{Red, Yellow}, {Green, White}, {White, Red},
	{Cyan, Navy}, {Red, White}, {Black, Red},
	{Cyan, White}, {Black, Yellow}, {White, Purple},
	{Blue, Orange}, {Yellow, Purple}, {Navy, White},
	{Blue,Black}, {Black,White}, {Navy, Yellow},
	{Yellow,Blue}, {White,Yellow}, {Blue,Red},
	{Black, Grey}, {Red,Black}, {White, Black},
	{Orange, Yellow}, {White, Orange}, {Navy,Blue},
	{White, Red}, {Blue, Yellow}, {White,Navy},
	};
	
	char * teams[30] = {"76ers", "Bucks", "Bulls", "Cavaliers", "Celtics", "Clippers", "Grizzlies", "Hawks", "Heat",
		"Hornets", "Jazz", "Kings", "Knicks", "Lakers", "Magic", "Mavericks", "Nets", "Nuggets", "Pacers", "Pelicans",
		"Pistons", "Raptors", "Rockets", "Spurs", "Suns", "Thunder", "Timberwolves", "Trail Blazers", "Warriors", "Wizards"};
	
	// Initialize local variables
	char c; 						// Used for getChar
	char LCDBuff[64]; 	// LCD String Buffer
	char atoi_buff[10]; // Used to convert string to number
	char btSend[10];
	uint32_t button;		// Used for INT0 Button
	uint8_t teamNameCounter = 0;
	uint8_t teamScoreCounter = 0;
	uint8_t i, j = 0;
	
	// Initial team score variables
	uint8_t team_index = 0;

	
	// Team Scores Array:
	// Game 0 uses 0 and 1
	// Game 1 uses 2 and 3
	// Game 2 uses 4 and 5
	uint32_t team_scores[6];
	uint8_t game_data[3][4];
	char teamNames[6][16];
	
	// Peripheral Initializations
	SystemInit();
	init_timer0(10);
	SERIAL_init(0, 9600);
	SERIAL_init(2, 9600); // Used for Bluetooth
	LCD_Initialization();
	LCD_Clear(Black);
	
	// HC06 BT Initialization
	sendCommand("AT\r\n",1000,1); 
  sendCommand("AT+VERSION\r\n",1000,1); 
  sendCommand("AT+NAMEJUSTIN_HC06\r\n",1000,1);
	
	// PROGRAM INITIALIZATION
	LCD_PutText(120, 120, (unsigned char *)"Initializing...", Yellow, Black); 
	
	// DATA GATHERING LOOP
	while(1)	{
		c = SERIAL_getChar_nb(2);
		if(c != 0)	{
			// Read from BT Buffer
			readFromBT(2000, c);
			// Collect data in stored variables
			if( bt_receive_n > 0)	{
				// Team Name Collection
				if((strstr(bt_receive_buff, "N=") != NULL) && (teamNameCounter < 6))	{
					// SERIAL_putString(2, "Received Team Name \r\n");
					LCD_PutText(120, 140, (unsigned char *)"Receiving Data", White, Black);
					i = 2;
					j = 0;
					while(bt_receive_buff[i] != '\r')	{
						teamNames[teamNameCounter][j] = bt_receive_buff[i];
						i++;
						j++;
					}
					SERIAL_putString(2, teamNames[teamNameCounter]);
					teamNameCounter++;
				}
				// Team Score Collection
				if((strstr(bt_receive_buff, "S=") != NULL) && (teamScoreCounter < 6))	{
					// SERIAL_putString(2, "Received Score \r\n");
					LCD_PutText(120, 160, (unsigned char *)"Keep waiting, beautiful", White, Black);
					i = 2;
					j = 0;
					while(bt_receive_buff[i] != '\r')	{
						atoi_buff[j] = bt_receive_buff[i];
						i++;
						j++;
					}
					team_scores[teamScoreCounter] = atoi(atoi_buff);
					sprintf(btSend, "%i\r\n", team_scores[teamScoreCounter]);
					SERIAL_putString(2, btSend);
					teamScoreCounter++;
					
					memset(atoi_buff, 0, sizeof(atoi_buff));
				}
				// Break Condition
				if((teamNameCounter==6)&&(teamScoreCounter==6))	{
					break;
				}
			}
			sprintf(btSend, "%i %i \r\n", teamNameCounter, teamScoreCounter);
			SERIAL_putString(2, btSend);
		}
	}
	
	// Data gathered
	SERIAL_putString(2, "Data Gathered \r\n");
	LCD_Clear(Black);
	
	// Format Data
	// Placing names into game data
	game_data[0][0] = directory(teamNames[0]);
	game_data[0][1] = directory(teamNames[1]);
	game_data[1][0] = directory(teamNames[2]);
	game_data[1][1] = directory(teamNames[3]);
	game_data[2][0] = directory(teamNames[4]);
	game_data[2][1] = directory(teamNames[5]);
	
	// Placing scores into game data
	game_data[0][2] = team_scores[0];
	game_data[0][3] = team_scores[1];
	game_data[1][2] = team_scores[2];
	game_data[1][3] = team_scores[3];
	game_data[2][2] = team_scores[4];
	game_data[2][3] = team_scores[5];
	
	// Initial Draw
	LCD_draw_home_side(team_colors[game_data[team_index][0]][0], team_colors[game_data[team_index][0]][1]);
  LCD_draw_opponent_side(team_colors[game_data[team_index][1]][0], team_colors[game_data[team_index][1]][1], teams[game_data[team_index][0]],teams[game_data[team_index][1]],game_data[team_index][2],game_data[team_index][3]);
		
	
	// MAIN LOOP
	while(1)	{
		// (1) CHECK FOR BUTTON PRESS TO SWITCH BETWEEN 3 GAMES
		button = LPC_GPIO2->FIOPIN;
		if ((button & (1<<10)) == 0)	{
			if(team_index >= 2)	{
				team_index = 0;
			}
			else	{
				team_index++;
			}
			
			// Redraw board if INT0 Pressed
			LCD_draw_home_side(team_colors[game_data[team_index][0]][0], team_colors[game_data[team_index][0]][1]);
      LCD_draw_opponent_side(team_colors[game_data[team_index][1]][0], team_colors[game_data[team_index][1]][1], teams[game_data[team_index][0]],teams[game_data[team_index][1]],game_data[team_index][2],game_data[team_index][3]);
		}
	}
	// END MAIN LOOP
}
