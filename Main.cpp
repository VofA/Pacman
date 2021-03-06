#include <map>
#include <cmath>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <windows.h>
#include "ConsoleHandler.hpp"

#define BOT_RED 0
#define BOT_PINK 1

#define DIRECTION_UP 0
#define DIRECTION_DOWN 1
#define DIRECTION_LEFT 2
#define DIRECTION_RIGHT 3
#define DIRECTION_NULL 4

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
int getPathBot(Entity first, Entity second);
void redBot();
void pinkBot();
bool checkCollision(Point position, int relativeX, int relativeY);
void playerMove(int relativeX, int relativeY);
unsigned int getLengthVector(Point first, Point second);
Point getPoint(unsigned int x, unsigned int y);
void checkLose();
void drawLives();
Point getRelativePoint(int direction, int speed);

unsigned int lives = 3;
unsigned int score = 0;
unsigned int tickDuration = 200;

Entity bots[4];
Entity player;
Point relativeMap;
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

	handler.init((unsigned int)map.size() + 2, (unsigned int)map[0].size());
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
	drawLives();

	player.position.x = 1;
	player.position.y = 1;

	for (int i = 0; i < 4; ++i) {
		bots[i].position.x = 20;
		bots[i].position.y = 5;
	}

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
	scoreStream << L"Score: " << score;

	handler.writeText(0, 0, scoreStream.str(), (unsigned int)map[0].size(), Style::create(Color::WHITE, Color::BLACK));
}

void handlerLogic() {
	while(true) {
		playerTick();
		redBot();
		pinkBot();

		Sleep(tickDuration);
	}
}

void playerTick() {
	
	Point relativePoint = getRelativePoint(player.direction, 1);

	if (!checkCollision(player.position, relativePoint.x, relativePoint.y)) {
		playerMove(relativePoint.x, relativePoint.y);
		checkLose();
	}
}

int getPathBot(Entity first, Entity second) {
	struct botDirection {
		unsigned int length = 0;
		int relativeX = 0;
		int relativeY = 0;
	};

	botDirection lengthVectors[4];

	lengthVectors[DIRECTION_UP].length = getLengthVector(first.position, getPoint(second.position.x, second.position.y - 1));
	lengthVectors[DIRECTION_UP].relativeY = -1;

	lengthVectors[DIRECTION_LEFT].length = getLengthVector(first.position, getPoint(second.position.x - 1, second.position.y));
	lengthVectors[DIRECTION_LEFT].relativeX = -1;

	lengthVectors[DIRECTION_DOWN].length = getLengthVector(first.position, getPoint(second.position.x, second.position.y + 1));
	lengthVectors[DIRECTION_DOWN].relativeY = 1;

	lengthVectors[DIRECTION_RIGHT].length = getLengthVector(first.position, getPoint(second.position.x + 1, second.position.y));
	lengthVectors[DIRECTION_RIGHT].relativeX = 1;

	unsigned int maxLengthVector = getLengthVector(getPoint(0, 0), getPoint((unsigned int)map[0].size(), (unsigned int)map.size()));

	switch (second.direction) {
		case DIRECTION_UP: {
			lengthVectors[DIRECTION_DOWN].length = maxLengthVector;
			break;
		}
		case DIRECTION_LEFT: {
			lengthVectors[DIRECTION_RIGHT].length = maxLengthVector;
			break;
		}
		case DIRECTION_DOWN: {
			lengthVectors[DIRECTION_UP].length = maxLengthVector;
			break;
		}
		case DIRECTION_RIGHT: {
			lengthVectors[DIRECTION_LEFT].length = maxLengthVector;
			break;
		}
	}

	unsigned int minLengthVector = maxLengthVector;
	unsigned int minLengthDirection = 0;

	for (unsigned int i = 0; i < 4; ++i) {
		if (lengthVectors[i].length <= minLengthVector && !checkCollision(second.position, lengthVectors[i].relativeX, lengthVectors[i].relativeY)) {
			minLengthDirection = i;
			minLengthVector = lengthVectors[i].length;
		}
	}

	return minLengthDirection;
}

void redBot() {
	handler.changePixel(bots[BOT_RED].position.x, bots[BOT_RED].position.y + relativeMap.y, map[bots[BOT_RED].position.y][bots[BOT_RED].position.x], textures[map[bots[BOT_RED].position.y][bots[BOT_RED].position.x]]);

	int direction = getPathBot(player, bots[BOT_RED]);

	Point relativePoint = getRelativePoint(direction, 1);

	bots[BOT_RED].direction = direction;

	bots[BOT_RED].position.x += relativePoint.x;
	bots[BOT_RED].position.y += relativePoint.y;

	checkLose();

	handler.changePixel(bots[BOT_RED].position.x, bots[BOT_RED].position.y + relativeMap.y, '@', Style::create(Color::RED, Color::BLACK));
}

void pinkBot() {
	handler.changePixel(bots[BOT_PINK].position.x, bots[BOT_PINK].position.y + relativeMap.y, map[bots[BOT_PINK].position.y][bots[BOT_PINK].position.x], textures[map[bots[BOT_PINK].position.y][bots[BOT_PINK].position.x]]);

	Entity copyPlayer = player;

	Point relativePointPlayer = getRelativePoint(copyPlayer.direction, 4);

	copyPlayer.position.x += relativePointPlayer.x;
	copyPlayer.position.y += relativePointPlayer.y;

	int direction = getPathBot(copyPlayer, bots[BOT_PINK]);

	Point relativePointBot = getRelativePoint(direction, 1);

	bots[BOT_PINK].direction = direction;

	bots[BOT_PINK].position.x += relativePointBot.x;
	bots[BOT_PINK].position.y += relativePointBot.y;

	checkLose();

	handler.changePixel(bots[BOT_PINK].position.x, bots[BOT_PINK].position.y + relativeMap.y, '@', Style::create(Color::LIGHTMAGENTA, Color::BLACK));
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

unsigned int getLengthVector(Point first, Point second) {
	return (first.x - second.x) * (first.x - second.x) + (first.y - second.y) * (first.y - second.y);
}

Point getPoint(unsigned int x, unsigned int y) {
	Point point;

	point.x = x;
	point.y = y;

	return point;
}

void checkLose() {
	for (int i = 0; i < 2; ++i) {
		if (player.position.x == bots[i].position.x && player.position.y == bots[i].position.y) {
			lives--;
			drawLives();
		}
	}

	if (lives == 0) {

	}
}

void drawLives() {
	std::wstringstream scoreStream;
	scoreStream << L"Lives: " << lives;

	handler.writeText(0, (unsigned int)map.size() + relativeMap.y, scoreStream.str(), (unsigned int)map[0].size(), Style::create(Color::WHITE, Color::BLACK));
}

Point getRelativePoint(int direction, int speed) {
	Point relativePoint;

	switch (direction) {
		case DIRECTION_UP : {
			relativePoint.y = -speed;
			break;
		}
		case DIRECTION_LEFT : {
			relativePoint.x = -speed;
			break;
		}
		case DIRECTION_DOWN : {
			relativePoint.y = speed;
			break;
		}
		case DIRECTION_RIGHT : {
			relativePoint.x = speed;
			break;
		}
	}
	
	return relativePoint;
}