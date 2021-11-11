#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <ctime>
#include <conio.h>
#include <Windows.h>

#include "FindPathBFS.h"
using namespace std;

int dir[4][2] = { { 0,1 },{ 0,-1 },{ 1,0 },{ -1,0 } };

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define HEAD 0

int speed = 0;
//move the cursor position to (x,y)
void gotoxy(int x, int y)
{
	COORD c;
	c.X = x; c.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
//Setting colors by using Windows API
void setColor(unsigned short ForeColor = 7, unsigned short BackGroundColor = 0)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);//Get the current window
	SetConsoleTextAttribute(handle, ForeColor + BackGroundColor * 0x10);//Setting color
}
//Game setting, put all the functions to one class and define it to static
class GameSetting
{
public:
	//Initial game window's height and width
	static const int window_height = 40;
	static const int window_width = 80;
public:
	static void GameInit()
	{
		//Initial game window's size
		char buffer[32];
		sprintf_s(buffer, "mode con cols=%d lines=%d",window_width, window_height);
		system(buffer);

		//Hide cursor
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO CursorInfo;
		GetConsoleCursorInfo(handle, &CursorInfo);//Get the controller cursor info
		CursorInfo.bVisible = false; //Hide the controller cursor
		SetConsoleCursorInfo(handle, &CursorInfo);//Setting the controller cursor status
		//Randomly seed
		srand((unsigned int)time(0));
	}
};
//Print out info, like welcome to the game, scores, instructions, and game over, etc.
class PrintInfo
{
public:
	//Choose mode: manual or AI?
	static void DrawChoiceInfo()
	{
		gotoxy(GameSetting::window_width / 2 - 10, GameSetting::window_height / 2 - 5);
		cout << "Please choose the game mode：" << endl;
		gotoxy(GameSetting::window_width / 2 - 10, GameSetting::window_height / 2 - 3);
		cout << "1. Manual mode" << endl;
		gotoxy(GameSetting::window_width / 2 - 10, GameSetting::window_height / 2 - 1);
		cout << "2. AI mode" << endl;
		gotoxy(GameSetting::window_width / 2 - 10, GameSetting::window_height / 2 + 1);
		cout << "Please enter your choice-> ";
	}
	//Draw the map 
	static void DrawMap()
	{
		system("cls");
		int i, j;
		for (i = 0; i < GameSetting::window_width; i++)
			cout << "#";
		cout << endl;
		for (i = 0; i < GameSetting::window_height-2; i++)
		{
			for (j = 0; j < GameSetting::window_width; j++)
			{
				if (i == 13 && j >= GameSetting::window_width - 29)
				{
					cout << "#";
					continue;
				}

				if (j == 0 || j == GameSetting::window_width - 29 || j == GameSetting::window_width-1)
				{
					cout << "#";
				}
				else
					cout << " ";
				
			}
			cout << endl;
		}
		for (i = 0; i < GameSetting::window_width; i++)
			cout << "#";
		
	}
	//Game Over
	static void GameOver(int score)
	{
		setColor(12, 0);
		gotoxy(GameSetting::window_width / 2 - 20, GameSetting::window_height / 2 - 5);
		cout << "Game is over, yor are lost" << endl;;
		gotoxy(GameSetting::window_width / 2 - 20, GameSetting::window_height / 2 - 3);
		cout << "Your score is: " << score << endl;
	}
	//Scores
	static void DrawScore(int score)
	{
		gotoxy(GameSetting::window_width - 22+14, 6);
		cout << "  ";
		gotoxy(GameSetting::window_width - 22+14, 4);
		cout << "  ";

		gotoxy(GameSetting::window_width - 22, 6);
		cout << "The current player's score is: " << score << endl;
		gotoxy(GameSetting::window_width - 22, 4);
		cout << "The current play speed is: " << 10 - speed / 25 << endl;

	}
	//Game instruction
	static void DrawGameInfo(bool model)
	{
		gotoxy(GameSetting::window_width - 22, 8);
		if (model)
		{
			cout << "The current game mode is: " << "Manual" << endl;
		}
		else
		{
			cout << "The current game mode is: " << "AI" << endl;
		}
		gotoxy(GameSetting::window_width - 22, 10);
		cout << "The highest score is: " << 100 << endl;

		gotoxy(GameSetting::window_width - 22, 18);
		cout << "Instruction: " << endl;
		gotoxy(GameSetting::window_width - 22, 20);
		cout << "W: up    S: down" << endl;
		gotoxy(GameSetting::window_width - 22, 22);
		cout << "A: left    D: right" << endl;

		gotoxy(GameSetting::window_width - 22, 24);
		cout << "Adjust the game speed: " << endl;
		gotoxy(GameSetting::window_width - 22, 26);
		cout << "Keypad + : faster" << endl;
		gotoxy(GameSetting::window_width - 22, 28);
		cout << "Keypad - : slower" << endl;

		gotoxy(GameSetting::window_width - 22, 32);
		cout << "Anthor: Mia" << endl;
		gotoxy(GameSetting::window_width - 22, 34);
		cout << "Edition: 1.0" << endl;
	}

};
//Class food, defind the creation of food and its related instructions
class Food
{
private:
	//Food cursor
	COORDINATE m_coordinate;
public:
	//Coordinate range：
	//x: 1 to GameSetting::window_width - 30 Closed interval
	//y: 1 to GameSetting::window_height - 2 Closed interval
	void RandomXY(vector<COORDINATE> & coord)
	{
		m_coordinate.x = rand() % (GameSetting::window_width - 30) + 1;
		m_coordinate.y = rand() % (GameSetting::window_height - 2) + 1;
		unsigned int i;
		//In principle, food is not allowed to appear in the position of the snake. If so, it will be regenerated
		for (i = 0; i < coord.size(); i++)
		{
			//Food appears in the position of the snake. Regenerate
			if (coord[i].x == m_coordinate.x && coord[i].y == m_coordinate.y)
			{
				m_coordinate.x = rand() % (GameSetting::window_width - 30) + 1;
				m_coordinate.y = rand() % (GameSetting::window_height - 2) + 1;
				i = 0;
			}
		}
	}
	//Constructor
	Food() {}
	//Constructor, the passed in parameter is snake body coordinates
	Food(vector<COORDINATE> & coord)
	{
		RandomXY(coord);
	}
	//Draw the food position
	void DrawFood()
	{
		setColor(12, 0);
		gotoxy(m_coordinate.x, m_coordinate.y);
		cout << "@";
		setColor(7, 0);
	}
	//Get the food position
	COORDINATE GetFoodCoordinate()
	{
		return m_coordinate;
	}

};
//Greedy snakes, which define the movement, printing, eating food and so on
//Map range: width:2 to width-2  height: 2 to height-2
class Snake
{
private:
	bool m_model; //true: Manual  false: AI
	int m_direction;
	bool m_is_alive;
private: //the function of AI
	bool m_chess[GameSetting::window_width - 29 + 1][GameSetting::window_height]; //function of AI
	FindPathBFS m_AISnake;
	COORDINATE map_size;
public://Snake cursor
	vector<COORDINATE> m_coordinate;

public://Constructor
	Snake(bool model = false) : m_model(model) //Manual mode
	{
		map_size.x = GameSetting::window_width - 29 + 1;
		map_size.y = GameSetting::window_height;
		//Move the direction up
		m_direction = 1;
		m_is_alive = true;
		COORDINATE snake_head;
		snake_head.x = GameSetting::window_width / 2 - 15;
		snake_head.y = GameSetting::window_height / 2;

		this->m_coordinate.push_back(snake_head);
		snake_head.y++;
		this->m_coordinate.push_back(snake_head);
		snake_head.y++;
		this->m_coordinate.push_back(snake_head); //The initial snake length is three knots

		//Walls are obstacles
		for (int i = 0; i < GameSetting::window_width - 29 + 1; i++)
		{
			m_chess[i][0] = true;
			m_chess[i][GameSetting::window_height - 1] = true;
		}

		for (int j = 0; j < GameSetting::window_height - 1; j++)
		{
			m_chess[0][j] = true;
			m_chess[GameSetting::window_width - 29][j] = true;
		}

	}
	//Game setting mode
	void set_model(bool m) { m_model = m; }
	//Listen keyborad
	void listen_key_borad()
	{
		char ch;

		if (_kbhit())					//kbhit Non blocking function
		{
			ch = _getch();	//using getch to get the input of keyboard 
			switch (ch)
			{
			case 'w':
			case 'W':
				if (this->m_direction == DOWN)
					break;
				this->m_direction = UP;
				break;
			case 's':
			case 'S':
				if (this->m_direction == UP)
					break;
				this->m_direction = DOWN;
				break;
			case 'a':
			case 'A':
				if (this->m_direction == RIGHT)
					break;
				this->m_direction = LEFT;
				break;
			case 'd':
			case 'D':
				if (this->m_direction == LEFT)
					break;
				this->m_direction = RIGHT;
				break;
			case '+':
				if (speed >= 25)
				{
					speed -= 25;
				}
				break;
			case '-':
				if (speed < 250)
				{
					speed += 25;
				}
				break;
			}
		}
	}
	//AI function
	void AI_speed()
	{
		char ch;

		if (_kbhit())					//Non blocking function
		{
			ch = _getch();	//using getch to get the input of keyboard 
			switch (ch)
			{
			case '+':
				if (speed >= 25)
				{
					speed -= 25;
				}
				break;
			case '-':
				if (speed < 250)
				{
					speed += 25;
				}
				break;
			}
		}
	}
	//AI function
	void AI_find_path(Food &f)
	{
		static int not_found = 1;
		COORDINATE fpoint = f.GetFoodCoordinate();
		
		for (unsigned int i = 0; i < m_coordinate.size(); i++)
		{
			m_chess[m_coordinate[i].x][m_coordinate[i].y] = true; //Snake body are obstacles
		}
		COORDINATE begin_point, end_point;
		
		begin_point = m_coordinate[HEAD];
		end_point = fpoint;

		m_AISnake.InitMap((bool**)m_chess);
		m_AISnake.GetPath(begin_point, end_point);

	}
	//Test if snake touched itself
	bool self_collision(COORDINATE head)
	{
		for (unsigned int i = 1; i < m_coordinate.size(); i++)
		{
			if (head.x == m_coordinate[i].x && head.y == m_coordinate[i].y)
			{
				return true;
			}
		}
		return false;
	}
	//AI function
	void AI_move_snake()
	{
		static int cot = 0;
		AI_speed();
		COORDINATE head, temp;
		if (!m_AISnake.m_paths_queue.empty())
		{
			head = m_AISnake.m_paths_queue.front();
			m_AISnake.m_paths_queue.pop();
		}
		else
		{
			//                          Down     Up      Left    Right
			//int direction[4][2] = { { 0,1 },{ 0,-1 },{ 1,0 },{ -1,0 } };
			//Take a random step, but don't touch the wall or itself
			for (int i = 0; i < 4; i++)
			{
				int break_num = rand() % 4;
				temp = m_coordinate[HEAD];
				temp.x = temp.x + dir[i][0];
				temp.y = temp.y + dir[i][1];

				if (temp.x <= 0 ||
					(temp.x >= (map_size.x - 1)) ||
					temp.y <= 0 ||
					(temp.y >= (map_size.y - 1)) ||
					self_collision(temp)
					)  //Road is blocked, can't pass
				{
					continue;
				}
				head = temp;
				if(break_num == i)
					break;
			}
		}

		m_coordinate.insert(m_coordinate.begin(), head);
	}
	//Move snake
	void move_snake()
	{
		//Listen the keyboard
		listen_key_borad();
		//Head of snake
		COORDINATE head = m_coordinate[0];
		//direction方向:1 up  2 down  3 left  4 right
		switch (this->m_direction)
		{
		case UP:
			head.y--;
			break;
		case DOWN:
			head.y++;
			break;
		case LEFT:
			head.x--;
			break;
		case RIGHT:
			head.x++;
			break;
		}
		//Insert new snake head after moveable.
		m_coordinate.insert(m_coordinate.begin(), head);
	}
	//Test if snake gets food
	bool is_eat_food(Food & f)
	{
		//Get the food cursor
		COORDINATE food_coordinate = f.GetFoodCoordinate();
		//If eats food, food re-generate and don't delete the snake back.
		if (m_coordinate[HEAD].x == food_coordinate.x && m_coordinate[HEAD].y == food_coordinate.y)
		{
			f.RandomXY(m_coordinate);
			return true;
		}
		else
		{
			//If doesn't eat food, delete the snake back.
			m_coordinate.erase(m_coordinate.end() - 1);
			return false;
		}
	}
	//Test if snake is dead
	bool snake_is_alive()
	{
		if (m_coordinate[HEAD].x <= 0 ||
			m_coordinate[HEAD].x >= GameSetting::window_width - 29 ||
			m_coordinate[HEAD].y <= 0 ||
			m_coordinate[HEAD].y >= GameSetting::window_height - 1)
		{
			//Out of the map boarder
			m_is_alive = false;
			return m_is_alive;
		}
		//Touched with itself
		for (unsigned int i = 1; i < m_coordinate.size(); i++)
		{
			if (m_coordinate[i].x == m_coordinate[HEAD].x && m_coordinate[i].y == m_coordinate[HEAD].y)
			{
				m_is_alive = false;
				return m_is_alive;
			}
		}
		m_is_alive = true;

		return m_is_alive;
	}
	//Draw snake
	void draw_snake()
	{
		//Initial the color is green
		setColor(10, 0);
		for (unsigned int i = 0; i < this->m_coordinate.size(); i++)
		{
			gotoxy(m_coordinate[i].x, m_coordinate[i].y);
			cout << "*";
		}
		//Set color back
		setColor(7, 0);
	}
	//Clean up
	void ClearSnake()
	{
		for (unsigned int i = 0; i < m_coordinate.size(); i++)
		{
			m_chess[m_coordinate[i].x][m_coordinate[i].y] = false;
		}
		gotoxy(m_coordinate[this->m_coordinate.size()-1].x, m_coordinate[this->m_coordinate.size() - 1].y);
		cout << " ";

	}
	//Get the length of snake
	int GetSnakeSize()
	{
		return m_coordinate.size();
	}
	//Get the current game mode
	bool GetModel()
	{
		return m_model;
	}
};

//main function, to combine every class and start the game.
int main()
{
	GameSetting setting;
	PrintInfo print_info;
	Snake  snake;
	//Initial the game
	setting.GameInit();
	//Game mode choosing
	print_info.DrawChoiceInfo();

	char ch = _getch();
	switch (ch)
	{
	case '1':
		snake.set_model(true);
		speed = 50;
		break;
	case '2':
		snake.set_model(false);
		break;
	default:
		gotoxy(GameSetting::window_width / 2 - 10, GameSetting::window_height / 2 + 3);
		cout << "You've input a wrong response，Bye！" << endl;
		cin.get();
		cin.get();
		return 0;
	}
	gotoxy(GameSetting::window_width / 2 - 10, GameSetting::window_height / 2 + 3);
	system("pause");
	//Draw map
	print_info.DrawMap();
	print_info.DrawGameInfo(snake.GetModel());
	//Generate food
	Food food(snake.m_coordinate);
	//a dead while loop to continue the game
	while (true)
	{
		//Print score
		print_info.DrawScore(snake.GetSnakeSize());
		//Draw the food
		food.DrawFood();
		//Before draw snake, clear the snake back
		snake.ClearSnake();
		//Test if snake got food
		snake.is_eat_food(food);
		//According to player's choosen mode to run different mode
		if (snake.GetModel() == true)
		{
			snake.move_snake();
		}
		else
		{
			snake.AI_find_path(food);
			snake.AI_move_snake();
		}
		snake.draw_snake();
		//Test if snake is still alive
		if (!snake.snake_is_alive())
		{
			print_info.GameOver(snake.GetSnakeSize());
			break;
		}
		//Control the game speed
		Sleep(speed);
	}

	cin.get();
	cin.get();

	return 0;
}