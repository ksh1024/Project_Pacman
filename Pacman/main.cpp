//사운드, 클래스 추가, 맵을 vector클래스로..어떻게 하지 - jwp 조언
#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;
using namespace sf;

#define DIR_UP		1
#define DIR_DOWN	2
#define DIR_RIGHT	3
#define DIR_LEFT	4

class Pacman {

public:
	int dir_; //이동 방향
	int x_;
	int y_;
	RectangleShape sprite_;

};

class Coin {
public:
	int x;
	int y;
	bool isCollected; //코인을 획득했는지 여부

	Coin(int x, int y) : x(x), y(y), isCollected(false) {} //코인 생성자

};

int main() {
	//맵 제어 배열
	//0 : 이동할 수 있는 곳
	//1 : 이동 불가능한 곳 (벽)
	bool map_control[18][30] = 
		{
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
			{1,1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,0,1,1,1,0,1,0,1,1,1,0,1,1},
			{1,1,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,1,0,1,1},
			{1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		};
	
	//팩맨 이미지 상하좌우
	Texture pac_up, pac_down, pac_left, pac_right;
	pac_up.loadFromFile("Resource/Image/pacman_up.png");
	pac_down.loadFromFile("Resource/Image/pacman_down.png");
	pac_left.loadFromFile("Resource/Image/pacman_left.png");
	pac_right.loadFromFile("Resource/Image/pacman_right.png");

	//맵 이미지
	Texture map;
	map.loadFromFile("Resource/Image/map.png");

	const int WIDTH = 1500;						//픽셀 너비
	const int HEIGHT = 900;						//픽셀 높이
	const int BLOCK_SIZE = 50;					//한 칸이 가지고 있는 픽셀
	const int G_WIDTH = WIDTH / BLOCK_SIZE;		//그리드의 너비
	const int G_HEIGHT = HEIGHT / BLOCK_SIZE;	//그리드의 높이;

	RenderWindow window(VideoMode(WIDTH, HEIGHT), "Pacman Game");

	//컴퓨터가 1초 동안 처리하는 횟수를 제한한다.  
	//Frame Per Second를 조절
	window.setFramerateLimit(8);

	Pacman pacman;
	pacman.x_ = 2, pacman.y_ = 3;//팩맨의 그리드 좌표
	pacman.dir_ = DIR_LEFT;		//팩맨이 이동하는 방향
	pacman.sprite_.setTexture(&pac_down);
	pacman.sprite_.setPosition(pacman.x_ * BLOCK_SIZE, pacman.y_ * BLOCK_SIZE);
	pacman.sprite_.setSize(Vector2f(BLOCK_SIZE, BLOCK_SIZE));

	// 코인을 그리기 위한 사각형 객체
	RectangleShape coinShape(Vector2f(BLOCK_SIZE / 4, BLOCK_SIZE / 4)); //블록의 1/4 크기
	//코인 획득시 점수 상승
	int point = 0;

	Sprite map_sprite;
	map_sprite.setTexture(map);

	vector<Coin> coins; // 코인 객체를 저장하는 벡터

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 28; j++) {
			if (map_control[i][j] == 0) {
				coins.push_back(Coin(j, i)); // 맵 배열 0에 코인 객체 추가
			}
		}
	}
	Font font;
	//한글이 지원되는 폰트로 변경
	if (!font.loadFromFile("C:\\Users\\ksh25\\AppData\\Local\\Microsoft\\Windows\\Fonts\\pixel.ttf"))
	{
		printf("폰트 불러오기 실패");
		return -1;
	}


	Text score;
	score.setFont(font);
	score.setFillColor(Color::Red);
	score.setCharacterSize(35);
	score.setPosition(50, 0);


	while (window.isOpen()) {
		Event e;
		while (window.pollEvent(e)) {
			//윈도우의 x를 눌렀을 때 창이 닫아지도록
			if (e.type == Event::Closed)
				window.close();
		}
		//방향키가 동시에 눌러지지 않도록 else 처리
		if (Keyboard::isKeyPressed(Keyboard::Right)) {
			pacman.dir_ = DIR_RIGHT;
			pacman.sprite_.setTexture(&pac_right);
			cout << "\n누른키 : right\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
		}
		else if (Keyboard::isKeyPressed(Keyboard::Left)) {
			pacman.dir_ = DIR_LEFT;
			pacman.sprite_.setTexture(&pac_left);
			cout << "\n누른키 : left\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
		}
		else if (Keyboard::isKeyPressed(Keyboard::Up)) {
			pacman.dir_ = DIR_UP;
			pacman.sprite_.setTexture(&pac_up);
			cout << "\n누른키 : up\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
		}
		else if (Keyboard::isKeyPressed(Keyboard::Down)) {
			pacman.dir_ = DIR_DOWN;
			pacman.sprite_.setTexture(&pac_down);
			cout << "\n누른키 : down\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
		}

		//팩맨 이동
		if (pacman.dir_ == DIR_UP && pacman.y_ > 1) {
			pacman.y_--;
		}
		else if (pacman.dir_ == DIR_DOWN && pacman.y_ < G_HEIGHT - 2) { 
			pacman.y_++; // -2을 하는 이유 : 팩맨의 '왼쪽위모서리'의 위치가 y에 들어가기 때문에
						//	-2을 하지 않으면 화면에서 한칸 밖으로 나가게됨
		}				 
		else if (pacman.dir_ == DIR_RIGHT && pacman.x_ < G_WIDTH - 2) {
			pacman.x_++;
		}
		else if (pacman.dir_ == DIR_LEFT && pacman.x_ > 1) {
			pacman.x_--;
		}
		//벽 이동제한
		if (pacman.dir_ == DIR_UP && map_control[pacman.y_][pacman.x_]) {
			pacman.y_++;
		}
		if (pacman.dir_ == DIR_DOWN && map_control[pacman.y_][pacman.x_]) {
			pacman.y_--;
		}
		if (pacman.dir_ == DIR_LEFT && map_control[pacman.y_][pacman.x_]) {
			pacman.x_++;
		}
		if (pacman.dir_ == DIR_RIGHT && map_control[pacman.y_][pacman.x_]) {
			pacman.x_--;
		}

		for (size_t i = 0; i < coins.size(); i++) {
			if (pacman.x_ == coins[i].x && pacman.y_ == coins[i].y && !coins[i].isCollected) {
				coins[i].isCollected = true;
				point+=10;
				// 포인트 변수를 문자열로 변환후 score의 string 업데이트
				score.setString("score: " + std::to_string(point)); 
			}
		}

		pacman.sprite_.setPosition(pacman.x_ * BLOCK_SIZE, pacman.y_ * BLOCK_SIZE);

		window.clear(); //전화면 지우기
		window.draw(score); // 점수 텍스트 그리기
		window.draw(map_sprite);
		window.draw(pacman.sprite_);

		for (const Coin& coin : coins) {
			if (!coin.isCollected) {
				coinShape.setPosition(coin.x * BLOCK_SIZE + BLOCK_SIZE / 4, coin.y * BLOCK_SIZE + BLOCK_SIZE / 4); // 코인 위치 설정
				coinShape.setFillColor(Color(255,255,0)); // 코인 색지정
				// 코인 그리기
				window.draw(coinShape);
			}
		}
		window.display();
	}

	return 0;
}