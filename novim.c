/*
 * Totally not a Vim ! Not even close to it.....it's novim
 */


#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

struct termios orig_termios;

void disableRawMode(){ /* turining off echo attribute when quiting the program */
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode(){  /* turning on echo attribute/raw mode. Echo's out the keystrokes back to the terminal  */
	tcgetattr(STDIN_FILENO, &orig_termios);    //  tcgetattr() -> read the current attribute of the terminal
	atexit(disableRawMode);
	
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO);					   //  turning on the ECHO

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);  //  tcsetattr()  -> writes the new attribute back to the terminal
}

int main(){
	enableRawMode();

	char c;
	while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
	return 0;
}
