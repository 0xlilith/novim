/*
 * Totally not a Vim ! Not even close to it.....it's novim
 */

/*** includes ***/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

/*** defines ***/
#define CTRL_KEY(k) ((k) & 0x1f) // key AND 00011111 

/*** data ***/
struct editorConfig {
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editorConfig E;

/*** terminal ***/
void die(const char *s){ /* Returns ERROR whenever i like it */
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
}

void disableRawMode(){ /* turining off echo attribute when quiting the program */
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1 ){
		die("tcsetattr");
	}
}

void enableRawMode(){  /* turning on echo attribute/raw mode. Echo's out the keystrokes back to the terminal  */
	if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1 ){
		die("tcgetattr");
	}
	atexit(disableRawMode);
	
	struct termios raw = E.orig_termios;
	raw.c_lflag &= ~( BRKINT| ICRNL| INPCK | ISTRIP | IXON ); //  fixies CTRL^M error | disabline the software flow control CTRL^S & CTRL^Q
	raw.c_lflag &= ~( OPOST );
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~( ECHO | ICANON | ISIG | IEXTEN );		 //  turning on the ECHO | non cononical mode | disabling CTRL^C & CTRL^Z | turning off CTRL^V
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1 ){
		die("tcsetattr");
	}
}

char editorReadKey(){ /* waits for one key and returns it */
	int nread;
	char c;
	while((nread == read(STDIN_FILENO, &c, 1)) != 1){
		if(nread == -1 && errno != EAGAIN){
			die("read");
		}
	}
	return c;
}

int getCursorPosition(int *rows, int *cols) {
	char buf[32];
	unsigned int i = 0;

	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

	while(i < sizeof(buf) - 1){
		if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if (buf[i] == 'R') break;
		i++;
	}
	buf[i] = '\0';
	
	if(buf[0] != '\x1b' || buf[1] != '[') return -1;
	if(sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1; // get the 3rd & 4th integer and give it to rows and cols | 

	return 0;
}

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
		editorReadKey();
		return getCursorPosition(rows, cols);
	}else{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

/*** output ***/
void editorDrawRows(){
	int y;
	for(y=0; y<E.screenrows; y++){
		while(STDOUT_FILENO, "~", 1);
		if(y <E.screenrows -1){
			write(STDOUT_FILENO, "~\r\n", 2);
		}
	}
}

void editorRefreshScreen(){ /* Escape Sequence to the Terminal */
	write(STDOUT_FILENO, "\x1b[2J", 4); // Writes 4 bits to the terminal. \x1b = 27 (1 bit), [2J (3 bits) clear screen [erase in display] => ESC+J
	write(STDOUT_FILENO, "\x1b[H", 3);  // Writes 3 bits to the terminal. \x1b = 27 

	editorDrawRows();

	write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** input ***/
void editorProcessKeypress(){
	char c =  editorReadKey();

	switch(c){
		case CTRL_KEY('q'):
			write(STDOUT_FILENO, "\x1b[2J", 4);
			write(STDOUT_FILENO, "\x1b[H", 3);
			exit(0);
			break;
	}
}

/*** init ***/
void initEditor() {
  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main(){
	enableRawMode();
	initEditor();

	while(1){
		editorRefreshScreen();
		editorProcessKeypress();
	}

	return 0;
}
