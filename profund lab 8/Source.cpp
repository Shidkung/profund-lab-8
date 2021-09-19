#include "Header.h"
#define scount 80
COORD star[scount];
int HP = 10;
const char* playerShip = "(-O-)";
COORD playerPos = { 0,0 };
short color = 7;
int randrange(int start, int stop) { return (rand() % (start - stop + 1)) + start; }
void init_star(RenderConsole* console)
{
	for (size_t i = 0; i < 80; i++)
	{
		COORD position = { randrange(0, console->getConsoleSize().X - 1),randrange(0, console->getConsoleSize().Y - 1) };
		star[i] = position;
	}
}
void star_fall(RenderConsole* console)
{
	for (int i = 0; i < scount; i++)
	{
		if (star[i].Y >= console->getConsoleSize().Y - 1)
		{
			star[i] = { short(rand() % console->getConsoleSize().X),1 };
		}
		else
		{
			star[i].Y++;
			if (star[i].Y == playerPos.Y && playerPos.X <= star[i].X && star[i].X < playerPos.X + 5)
			{
				HP--;
				star[i] = { short(rand() % console->getConsoleSize().X),1 };
			}
		}
	}
}
int main()
{
	srand(time(0));
	RenderConsole console(80, 25);
	init_star(&console);
	bool running = true;
	while (running && HP > 0)
	{
		ConsoleEvent ev;
		while (console.pollEvents(ev))
		{
			if (ev.type == KEY_EVENT)
			{
				if (ev.asciiChar == 'c')
				{
					color = randrange(1, 15);
				}
				if (ev.wVirtualKeyCode == VK_ESCAPE)
				{
					running = false;
				}
			}
			else if (ev.type == MOUSE_EVENT)
			{
				if (ev.leftClick)
				{
					color = randrange(1, 15);
				}
				if (ev.mouseMoved)
				{
					playerPos = ev.mousePosition;
					playerPos.X -= 2;
				}
			}
		}
		star_fall(&console);
		console.clear();
		for (int i = 0; i < scount; i++)
		{
			console.draw('*', star[i]);
		}
		console.draw(playerShip, playerPos, color);
		console.display();
		Sleep(100);
	}
	return 0;
}
