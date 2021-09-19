#pragma once
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <string.h>

using namespace std;

struct ConsoleEvent
{
	WORD type;
	// KeyEvent
	bool bKeyDown;
	WORD wVirtualKeyCode;
	char asciiChar;
	// MouseEvent
	bool leftClick;
	bool rightClick;
	bool mouseMoved;
	COORD mousePosition;
};

class RenderConsole
{
private:
	size_t screenX;
	size_t screenY;
	HANDLE wHnd; // Write Handle
	HANDLE rHnd; // Read Handle
	size_t consoleBufferSize;
	CHAR_INFO* consoleBuffer;
	COORD bufferSize;
	COORD characterPos;
	SMALL_RECT windowSize;

	INPUT_RECORD* eventBuffer;
	DWORD numEvents;
	DWORD numEventsRead;

	inline void setConsole()
	{
		SetConsoleWindowInfo(wHnd, TRUE, &windowSize);
		SetConsoleScreenBufferSize(wHnd, bufferSize);
	}
	inline void setMode()
	{
		DWORD fdwMode = ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
		SetConsoleMode(rHnd, fdwMode);
	}

public:
	inline RenderConsole(unsigned int width, unsigned int height)
	{
		screenX = width;
		screenY = height;
		wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
		rHnd = GetStdHandle(STD_INPUT_HANDLE);
		consoleBufferSize = width * height;
		consoleBuffer = new CHAR_INFO[consoleBufferSize];
		bufferSize = { (short)screenX,(short)screenY };
		characterPos = { 0,0 };
		windowSize = { 0,0,short((short)screenX - 1),short((short)screenY - 1) };

		numEvents = 0;
		numEventsRead = 0;
		eventBuffer = nullptr;

		setConsole();
		setMode();
	}
	inline ~RenderConsole()
	{
		if (consoleBuffer != nullptr)
			delete consoleBuffer;
	}

#pragma region Events
	// Call this before getNumEvents()
	inline INPUT_RECORD* getEventBuffer()
	{
		clearEventBuffer();
		GetNumberOfConsoleInputEvents(rHnd, &numEvents);
		if (numEvents == 0)
		{
			eventBuffer = nullptr;
			return eventBuffer;
		}

		eventBuffer = new INPUT_RECORD[numEvents];
		ReadConsoleInput(rHnd, eventBuffer, numEvents, &numEventsRead);
		return eventBuffer;
	}

	// Call after getEventBuffer()
	inline DWORD getNumEventsRead()
	{
		return numEventsRead;
	}

	// ConsoleEvent has KEY_EVENT and MOUSE_EVENT type
	// Should not use with the two above function
	inline bool pollEvents(ConsoleEvent& consoleEv)
	{
		clearEventBuffer();
		GetNumberOfConsoleInputEvents(rHnd, &numEvents);
		if (numEvents == 0)
		{
			eventBuffer = nullptr;
			return false;
		}

		eventBuffer = new INPUT_RECORD[numEvents];
		ReadConsoleInputW(rHnd, eventBuffer, 1, &numEventsRead);
		if (eventBuffer[0].EventType == KEY_EVENT)
		{
			consoleEv.type = KEY_EVENT;
			consoleEv.bKeyDown = eventBuffer[0].Event.KeyEvent.bKeyDown;
			consoleEv.wVirtualKeyCode = eventBuffer[0].Event.KeyEvent.wVirtualKeyCode;
			consoleEv.asciiChar = eventBuffer[0].Event.KeyEvent.uChar.AsciiChar;

			consoleEv.leftClick = false;
			consoleEv.rightClick = false;
			consoleEv.mouseMoved = false;
			consoleEv.mousePosition = { 0,0 };
		}
		else if (eventBuffer[0].EventType == MOUSE_EVENT)
		{
			consoleEv.type = MOUSE_EVENT;
			consoleEv.leftClick = eventBuffer[0].Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED;
			consoleEv.rightClick = eventBuffer[0].Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED;
			consoleEv.mouseMoved = eventBuffer[0].Event.MouseEvent.dwEventFlags & MOUSE_MOVED;
			consoleEv.mousePosition = eventBuffer[0].Event.MouseEvent.dwMousePosition;

			consoleEv.bKeyDown = false;
			consoleEv.wVirtualKeyCode = 0;
			consoleEv.asciiChar = 0;
		}

		return true;
	}

	inline bool pollEvents(INPUT_RECORD& evBuffer)
	{
		clearEventBuffer();
		GetNumberOfConsoleInputEvents(rHnd, &numEvents);
		if (numEvents == 0)
		{
			eventBuffer = nullptr;
			return false;
		}

		eventBuffer = new INPUT_RECORD[numEvents];
		ReadConsoleInputW(rHnd, eventBuffer, 1, &numEventsRead);
		evBuffer = eventBuffer[0];
		return true;
	}

	inline void clearEventBuffer()
	{
		if (eventBuffer != nullptr)
		{
			delete eventBuffer;
		}
	}
#pragma endregion

	inline COORD getConsoleSize() { return bufferSize; }

	inline void clear()
	{
		for (int y = 0; y < screenY; ++y)
		{
			for (int x = 0; x < screenX; ++x)
			{
				consoleBuffer[x + screenX * y].Char.AsciiChar = ' ';
				consoleBuffer[x + screenX * y].Attributes = 7;
			}
		}
	}

#pragma region Draw
	inline void draw(char ch, COORD coord)
	{
		consoleBuffer[coord.X + screenX * coord.Y].Char.AsciiChar = ch;
		consoleBuffer[coord.X + screenX * coord.Y].Attributes = 7;
	}

	inline void draw(char ch, COORD coord, unsigned short color)
	{
		consoleBuffer[coord.X + screenX * coord.Y].Char.AsciiChar = ch;
		consoleBuffer[coord.X + screenX * coord.Y].Attributes = color;
	}

	inline void draw(const char* strChar, COORD coord, unsigned short color = 7)
	{
		int x = coord.X;
		int y = coord.Y;
		for (size_t i = 0; i < strlen(strChar); i++)
		{
			if (0 <= x && x < screenX)
			{
				consoleBuffer[x + screenX * y].Char.AsciiChar = strChar[i];
				consoleBuffer[x + screenX * y].Attributes = color;
			}
			x++;
		}
	}
#pragma endregion

	inline void display()
	{
		WriteConsoleOutputA(wHnd, consoleBuffer, bufferSize, characterPos, &windowSize);
	}
};