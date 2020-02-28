#include <ncurses.h>

int main(void)
{
	int value = 0;

	initscr();

	addstr("Press any key to begin:\n");
	refresh();
	getch();

	nodelay(stdscr,TRUE);		/* turn off getch() wait */
	addstr("Press any key to stop the insane loop!\n");
	int ch;
	while(1)
	{
		if((ch=getch()) !=ERR){
			break;
		}
		printw("%d\r",value++);
		refresh();
	}
	
	endwin();
	return 0;
}
