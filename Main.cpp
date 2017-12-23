#include <cmath>
#include <ctime>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include "ConsoleHandler.hpp"

struct TPOINT {
	int x = 0;
	int y = 0;
};

ConsoleHandler handler;

void handlerKey(int key);
void handlerLogic();
bool playerCheckCollision(int x, int y);
void playerMove(int x, int y);
void lose();
void drawMap();
void drawScore();
void spawnFood();
void eaten(int x, int y);

TPOINT getPoint(int x, int y);

std::vector<TPOINT> playerPosition;
std::vector<std::string> map;

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
int playerDirection = RIGHT;

int tickTime = 200;
int score = 0;
int maxScore = 0;

int main() {
	srand(time(NULL));

	std::fstream fin("map");

	while (!fin.eof()) {
		map.push_back("");
		getline(fin, map.back());
	}

	handler.init(map.size() + 1, map[0].size());
	handler.setTitle(L"Snake");

	drawMap();
	drawScore();
	spawnFood();

	handler.registerHandlerCallback(handlerKey);
	handler.registerKeyCallback(ConsoleHandler::KEY_CODE_W);
	handler.registerKeyCallback(ConsoleHandler::KEY_CODE_A);
	handler.registerKeyCallback(ConsoleHandler::KEY_CODE_S);
	handler.registerKeyCallback(ConsoleHandler::KEY_CODE_D);

	playerPosition.push_back(getPoint(1, 1));

	std::thread threadLogic(handlerLogic);
	threadLogic.detach();

	handler.mainLoop();
}

void handlerKey(int key) {
	switch (key) {
	case ConsoleHandler::KEY_CODE_W:
		playerDirection = UP;
		break;
	case ConsoleHandler::KEY_CODE_S:
		playerDirection = DOWN;
		break;
	case ConsoleHandler::KEY_CODE_A:
		playerDirection = LEFT;
		break;
	case ConsoleHandler::KEY_CODE_D:
		playerDirection = RIGHT;
		break;
	}
}

void handlerLogic() {
	while (true) {
		int relativeX = 0;
		int relativeY = 0;

		switch (playerDirection) {
		case UP:
			relativeY = -1;
			break;
		case DOWN:
			relativeY = 1;
			break;
		case LEFT:
			relativeX = -1;
			break;
		case RIGHT:
			relativeX = 1;
			break;
		}

		if (!playerCheckCollision(relativeX, relativeY)) {
			playerMove(relativeX, relativeY);
		}
		else {
			relativeX = 0;
			relativeY = 0;
		}

		eaten(relativeX, relativeY);

		Sleep(tickTime);
	}
}

bool playerCheckCollision(int x, int y) {
	for (int i = 0; i < playerPosition.size(); i++) {
		char symbol = map[y + playerPosition[i].y][x + playerPosition[i].x];

		if (symbol == '=') {
			return true;
		}
	}
	return false;
}

void playerMove(int x, int y) {
	playerPosition.insert(
		playerPosition.begin(),
		getPoint(playerPosition[0].x + x, playerPosition[0].y + y));
	char symbol;
	switch (playerDirection) {
	case UP:
		symbol = 'A';
		break;
	case DOWN:
		symbol = 'V';
		break;
	case LEFT:
		symbol = '<';
		break;
	case RIGHT:
		symbol = '>';
		break;
	}

	handler.changePixel(playerPosition.front().x, playerPosition.front().y, symbol, Style::create(Color::YELLOW, Color::BLACK));


		handler.changePixel(playerPosition.back().x, playerPosition.back().y, ' ', Style::create(Color::BLACK, Color::BLACK));
		playerPosition.pop_back();

}

void lose() {
	if (maxScore < score) {
		maxScore = score;
	}

	playerPosition.clear();
	playerPosition.push_back(getPoint(1, 1));
	playerDirection = RIGHT;

	score = 0;

	drawMap();
	drawScore();
}

TPOINT getPoint(int x, int y) {
	TPOINT result;

	result.x = x;
	result.y = y;

	return result;
}

void drawMap() {
	for (int y = 0; y < map.size(); y++) {
		for (int x = 0; x < map[y].size(); x++) {
			if (map[y][x] == '=') {
				handler.changePixel(x, y, '=', Style::create(Color::BLUE, Color::BLACK));
			} else {
				handler.changePixel(x, y, map[y][x], Style::create(Color::WHITE, Color::WHITE));
			}
		}
	}
}

void drawScore() {
	std::wstringstream scoreStream;
	scoreStream << "Score: " << score;
	handler.writeText(0, 24, scoreStream.str(), 32, Style::create(Color::WHITE, Color::BLACK));
}

void spawnFood() {
    for (int y = 0; y < map.size(); y++) {
		for (int x = 0; x < map[y].size(); x++) {
            if (map[y][x] != '=') {
                handler.changePixel(x, y, '.', Style::create(Color::YELLOW, Color::BLACK));
            }
		}
    }
}

void eaten(int x, int y) {
    for (int i = 0; i < playerPosition.size(); i++) {
        char symbol = map[y + playerPosition[i].y][x + playerPosition[i].x];
        if (symbol = '.') {
            score++;
        }
    }
}
