#include "ConsoleHandler.hpp"
#include <fstream>
#include <iostream>
#include <windows.h>
#include <vector>
#include <thread>
#include <map>
#include <sstream>

#define DIRECTION_NULL 0
#define DIRECTION_UP 1
#define DIRECTION_DOWN 2
#define DIRECTION_LEFT 3
#define DIRECTION_RIGHT 4

struct Point {
	unsigned int x = 0;
	unsigned int y = 0;
};

struct Entity {
	Point position;
	unsigned short direction = DIRECTION_NULL;
};

void handlerKey(int key);
void drawMap();
void drawScore();
void handlerLogic();
void playerTick();
bool checkCollision(Point position, int relativeX, int relativeY);
void playerMove(int relativeX, int relativeY);

Point relativeMap;
unsigned int score = 0;
unsigned int tickDuration = 200;

Entity player;
ConsoleHandler handler;

std::vector<std::wstring> map;
std::map<wchar_t, Style::Theme> textures;

int main() {
	std::wfstream fin(L"map");

	while (!fin.eof()) {
		map.push_back(L"");
		getline(fin, map.back());
	}

	relativeMap.y = 1;

	handler.init(map.size() + relativeMap.y, map[0].size());
	handler.setTitle(L"Pacman");

	handler.registerHandlerCallback(handlerKey);

	handler.registerKeyCallback(ConsoleHandler::KEY_CODE_W);
	handler.registerKeyCallback(ConsoleHandler::KEY_CODE_A);
	handler.registerKeyCallback(ConsoleHandler::KEY_CODE_S);
	handler.registerKeyCallback(ConsoleHandler::KEY_CODE_D);

	textures.insert(std::pair<wchar_t, Style::Theme> ('=', Style::create(Color::LIGHTBLUE, Color::BLACK)));
	textures.insert(std::pair<wchar_t, Style::Theme> ('.', Style::create(Color::BROWN, Color::BLACK)));

	drawMap();
	drawScore();

	player.position.x = 1;
	player.position.y = 1;

	std::thread threadHandlerLogic(handlerLogic);
	threadHandlerLogic.detach();

	handler.mainLoop();
}

void handlerKey(int key) {
	switch (key) {
		case ConsoleHandler::KEY_CODE_W: {
			player.direction = DIRECTION_UP;
			break;
		}
		case ConsoleHandler::KEY_CODE_A: {
			player.direction = DIRECTION_LEFT;
			break;
		}
		case ConsoleHandler::KEY_CODE_S: {
			player.direction = DIRECTION_DOWN;
			break;
		}
		case ConsoleHandler::KEY_CODE_D: {
			player.direction = DIRECTION_RIGHT;
			break;
		}
	}
}

void drawMap() {
	for (unsigned int y = 0; y < map.size(); ++y) {
		for (unsigned int x = 0; x < map[y].size(); ++x) {
			handler.changePixel(x, y + relativeMap.y, map[y][x], textures[map[y][x]]);
		}
	}
}

void drawScore() {
	std::wstringstream scoreStream;
	scoreStream << "Score: " << score;

	handler.writeText(0, 0, scoreStream.str(), map[0].size(), Style::create(Color::WHITE, Color::BLACK));
}

void handlerLogic() {
	while(true) {
		playerTick();

		Sleep(tickDuration);
	}
}

void playerTick() {
	int relativeX = 0;
	int relativeY = 0;

	switch (player.direction) {
		case DIRECTION_UP: {
			relativeY = -1;
			break;
		}
		case DIRECTION_LEFT: {
			relativeX = -1;
			break;
		}
		case DIRECTION_DOWN : {
			relativeY = 1;
			break;
		}
		case DIRECTION_RIGHT : {
			relativeX = 1;
			break;
		}
	}

	if (!checkCollision(player.position, relativeX, relativeY)) {
		playerMove(relativeX, relativeY);
	}
}

bool checkCollision(Point position, int relativeX, int relativeY) {
	return map[position.y + relativeY][position.x + relativeX] == '=';
}

void playerMove(int relativeX, int relativeY) {
	handler.changePixel(player.position.x, player.position.y + relativeMap.y, ' ');

	player.position.x += relativeX;
	player.position.y += relativeY;

	wchar_t symbol = 'O';
	switch (player.direction) {
		case DIRECTION_UP: {
			symbol = 'A';
			break;
		}
		case DIRECTION_LEFT: {
			symbol = '<';
			break;
		}
		case DIRECTION_DOWN : {
			symbol = 'V';
			break;
		}
		case DIRECTION_RIGHT : {
			symbol = '>';
			break;
		}
	}

	handler.changePixel(player.position.x, player.position.y + relativeMap.y, symbol, Style::create(Color::YELLOW, Color::BLACK));

	if (map[player.position.y][player.position.x] == '.') {
		score++;
		drawScore();
		map[player.position.y][player.position.x] = ' ';
	}
}
