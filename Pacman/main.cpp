#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;
using namespace sf;

#define DIR_UP		1
#define DIR_DOWN	2
#define DIR_RIGHT	3
#define DIR_LEFT	4

#define BLOCK_SIZE 50 //한 칸이 가지고 있는 픽셀


enum GameState {
	Mainmenu,
	Playing,
	GameOver,
	GameClear
};
GameState gameState = Mainmenu; // 메인메뉴 상태로 초기화

//메인메뉴를 그리는 함수
void DrawMainmenu(RenderWindow& window, Text& start_text, Text& exit_text, int selectedItem) {
	start_text.setFillColor(selectedItem == 0 ? Color::Green : Color::White); //0(start)선택시 글자색 초록색으로 지정, 아닐시 흰색
	exit_text.setFillColor(selectedItem == 1 ? Color::Green : Color::White);  //1(exit)선택시 글자색 초록색으로 지정, 아닐시 흰색
	window.draw(start_text);
	window.draw(exit_text);
}

class Pacman {
public:
	int dir_; //이동 방향
	int x_;
	int y_;
	RectangleShape sprite_;
	int hearts_ = 5; //하트 (생명)

	Music sound_collide; //적과 충돌시 효과음
	
	Pacman() {
		if (!sound_collide.openFromFile("Resource/Sound/crash.wav")) {
			cerr << "Failed to load music" << endl;
		}
	}

	// 적과 충돌 시 하트 감소
	void CollideWithEnemy() {
		if (hearts_ > 0) { //하트가 한개 이상 남아있을시
			hearts_--;     //하트 하나 줄어듬
			sound_collide.setVolume(30);
			sound_collide.stop();
			sound_collide.play();
		}
	}

	Clock animationClock;		//애니메이션 속도 제어
	float animationTime = 0.2f; //애니메이션 프레임 간 시간 간격
	Time elapsedTime;			//함수 호출 간 경과 시간
	bool mouthOpen = false;		//입이 열려있는지 닫혀있는지 여부

	//입을 벌렸다 닫는 애니메이션
	void UpdateAnimation(Texture& pac_def, Texture& pac_up, Texture& pac_down, Texture& pac_left, Texture& pac_right) {
		elapsedTime += animationClock.restart();
		if (elapsedTime.asSeconds() >= animationTime) {
			//경과 시간 초기화
			elapsedTime = Time::Zero;
			//이동 방향에 따라 팩맨 텍스처 변경
			if (dir_ == DIR_UP) {
				sprite_.setTexture(mouthOpen ? &pac_up : &pac_def);
			}
			else if (dir_ == DIR_DOWN) {
				sprite_.setTexture(mouthOpen ? &pac_down : &pac_def);
			}
			else if (dir_ == DIR_LEFT) {
				sprite_.setTexture(mouthOpen ? &pac_left : &pac_def);
			}
			else if (dir_ == DIR_RIGHT) {
				sprite_.setTexture(mouthOpen ? &pac_right : &pac_def);
			}
			//다음 프레임을 위해 mouthOpen 상태 전환
			mouthOpen = !mouthOpen;
		}
	}

};

class Enemy {
public:
	int dir_; //이동 방향
	int x_;
	int y_;
	RectangleShape sprite_;

	//적의 랜덤 이동
	void MoveEnemy(Enemy& enemy, bool map_control[18][30]) {
		// 현재 위치에서 이동 가능한 방향을 저장할 벡터
		vector<int> possibleDirs;

		if (enemy.y_ > 0 && !map_control[enemy.y_ - 1][enemy.x_]) {
			possibleDirs.push_back(1); // 1: 위
		}
		if (enemy.y_ < 17 && !map_control[enemy.y_ + 1][enemy.x_]) {
			possibleDirs.push_back(2); // 2: 아래
		}
		if (enemy.x_ > 0 && !map_control[enemy.y_][enemy.x_ - 1]) {
			possibleDirs.push_back(3); // 3: 왼쪽
		}
		if (enemy.x_ < 29 && !map_control[enemy.y_][enemy.x_ + 1]) {
			possibleDirs.push_back(4); // 4: 오른쪽
		}

		if (!possibleDirs.empty()) {
			// 이동 가능한 방향 중에서 랜덤으로 선택
			int randomIndex = rand() % possibleDirs.size();
			int randomDir = possibleDirs[randomIndex];

			// 선택한 방향으로 이동
			if (randomDir == 1) {      // 위로 이동
				enemy.y_--;
			}
			else if (randomDir == 2) { // 아래로 이동
				enemy.y_++;
			}
			else if (randomDir == 3) { // 왼쪽으로 이동
				enemy.x_--;
			}
			else if (randomDir == 4) { // 오른쪽으로 이동
				enemy.x_++;
			}
		}
	}

};

class Coin {
public:
	int x;
	int y;
	bool isCollected; //코인을 획득했는지 여부

	Coin(int x, int y) : x(x), y(y), isCollected(false) {} //코인 생성자
};

//맵 제어 배열
//	0 : 이동할 수 있는 곳
//	1 : 이동 불가능한 곳 (벽)
//bool map_control[18][30] =
//{
//	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
//	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
//	{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//	{1,1,0,1,1,1,0,1,0,1,1,1,1,0,0,1,0,0,1,1,1,1,0,1,0,1,1,1,0,1},
//	{1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1},
//	{1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1},
//	{1,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
//	{1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1},
//	{1,1,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1},
//	{1,1,0,0,0,0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,0,1,1,1,0,0,0,0,0,1},
//	{1,1,1,1,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,0,0,0,1,1,1},
//	{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//	{1,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,0,0,1,0,0,0,0,0,1},
//	{1,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1},
//	{1,1,0,1,1,1,1,1,1,0,0,0,0,0,1,0,1,0,0,0,0,0,1,1,1,1,1,1,0,1},
//	{1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
//	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
//	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
//};
bool map_control[18][30] =
{
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
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
int main() {

	Music sound_select; //메뉴 선택 효과음
	if (!sound_select.openFromFile("Resource/Sound/select.wav")) {
		cerr << "Failed to load music" << endl;
		return -1;
	}
	Music music_game; //게임 배경 음악
	if (!music_game.openFromFile("Resource/Sound/gameplay.wav")) {
		cerr << "Failed to load music" << endl;
		return -1;
	}
	Music sound_coin; //코인 획득시 효과음
	if (!sound_coin.openFromFile("Resource/Sound/coin.wav")) {
		cerr << "Failed to load music" << endl;
		return -1;
	}
	Music music_gameover; //게임 오버 음악
	if (!music_gameover.openFromFile("Resource/Sound/gameover.wav")) {
		cerr << "Failed to load music" << endl;
		return -1;
	}
	Music music_gameclear; //게임 클리어시 음악
	if (!music_gameclear.openFromFile("Resource/Sound/gameclear.wav")) {
		cerr << "Failed to load music" << endl;
		return -1; 
	}

	//팩맨 텍스쳐 기본, 상하좌우
	Texture pac_def, pac_up, pac_down, pac_left, pac_right;
	pac_def.loadFromFile("Resource/Image/pacman_closed.png");
	pac_up.loadFromFile("Resource/Image/pacman_up.png");
	pac_down.loadFromFile("Resource/Image/pacman_down.png");
	pac_left.loadFromFile("Resource/Image/pacman_left.png");
	pac_right.loadFromFile("Resource/Image/pacman_right.png");

	//적 텍스쳐
	Texture enemy_img;
	enemy_img.loadFromFile("Resource/Image/enemy.png");

	//맵 텍스쳐
	Texture map;
	map.loadFromFile("Resource/Image/map.png");

	const int WIDTH = 1550;						//픽셀 너비
	const int HEIGHT = 900;						//픽셀 높이
	const int G_WIDTH = WIDTH / BLOCK_SIZE;		//그리드의 너비
	const int G_HEIGHT = HEIGHT / BLOCK_SIZE;	//그리드의 높이;

	RenderWindow window(VideoMode(WIDTH, HEIGHT), "Pacman Game");

	//컴퓨터가 1초 동안 처리하는 횟수를 제한한다.  
	//Frame Per Second를 조절
	window.setFramerateLimit(8);

	Pacman pacman;
	pacman.x_ = 28, pacman.y_ = 15;//팩맨의 그리드 좌표
	pacman.dir_ = DIR_LEFT;			//팩맨이 이동하는 방향
	pacman.sprite_.setTexture(&pac_left);
	pacman.sprite_.setPosition(pacman.x_ * BLOCK_SIZE, pacman.y_ * BLOCK_SIZE);
	pacman.sprite_.setSize(Vector2f(BLOCK_SIZE, BLOCK_SIZE));

	Enemy enemy_1;
	enemy_1.x_ = 28, enemy_1.y_ = 2;
	enemy_1.dir_ = DIR_LEFT;
	enemy_1.sprite_.setTexture(&enemy_img);
	enemy_1.sprite_.setPosition(enemy_1.x_ * BLOCK_SIZE, enemy_1.y_ * BLOCK_SIZE);
	enemy_1.sprite_.setSize(Vector2f(BLOCK_SIZE, BLOCK_SIZE));

	Enemy enemy_2;
	enemy_2.x_ = 9, enemy_2.y_ = 7;
	enemy_2.dir_ = DIR_RIGHT;
	enemy_2.sprite_.setTexture(&enemy_img);
	enemy_2.sprite_.setPosition(enemy_2.x_ * BLOCK_SIZE, enemy_2.y_ * BLOCK_SIZE);
	enemy_2.sprite_.setSize(Vector2f(BLOCK_SIZE, BLOCK_SIZE));

	Texture heartTexture;
	heartTexture.loadFromFile("Resource/Image/heart.png");
	Sprite heartSprite(heartTexture);


	// 코인을 그리기 위한 사각형 객체
	RectangleShape coinShape(Vector2f(BLOCK_SIZE / 6, BLOCK_SIZE / 6)); //블록의 1/6 크기
	//코인 획득시 점수 상승
	int point = 0;

	Sprite map_sprite;
	map_sprite.setTexture(map);

	vector<Coin> coins; // 코인 객체를 저장하는 벡터
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 29; j++) {
			if (map_control[i][j] == 0) {
				coins.push_back(Coin(j, i)); // 맵 배열 0에 코인 객체 추가
			}
		}
	}
	Font font;
	if (!font.loadFromFile("Resource/Font/pixel.ttf")) {
		printf("failed to load font file");
		return -1;
	}

	//현재 선택된 항목 (0 : 시작하기, 1 : 나가기)
	int selectedItem = 0;

	Text title;
	title.setFont(font);
	title.setCharacterSize(200);
	title.setString("PACMAN");
	title.setFillColor(Color::Yellow);
	//텍스트 중앙배치
	FloatRect titleRect = title.getLocalBounds();
	title.setOrigin(titleRect.width / 2, titleRect.height / 2);
	title.setPosition(Vector2f(WIDTH / 2.0f, 100));

	Text start_text;
	start_text.setFont(font);
	start_text.setCharacterSize(50);
	start_text.setString("START");
	FloatRect start_textRect = start_text.getLocalBounds();
	start_text.setOrigin(start_textRect.width / 2, start_textRect.height / 2);
	start_text.setPosition(Vector2f(WIDTH / 2.0f, HEIGHT / 2.0f));
	
	Text exit_text;
	exit_text.setFont(font);
	exit_text.setCharacterSize(50);
	exit_text.setString("QUIT");
	FloatRect exit_textRect = exit_text.getLocalBounds();
	exit_text.setOrigin(exit_textRect.width / 2, exit_textRect.height / 2);
	exit_text.setPosition(Vector2f(WIDTH / 2.0f, HEIGHT / 2.0f + 100));

	Text score;
	score.setFont(font);
	score.setFillColor(Color::Green);
	score.setCharacterSize(35);
	score.setPosition(50, 0);

	Text GameOver_text;
	GameOver_text.setFillColor(Color::Red);
	GameOver_text.setFont(font);
	GameOver_text.setCharacterSize(200);
	GameOver_text.setString("GAME OVER");
	FloatRect GameOver_textRect = GameOver_text.getLocalBounds();
	GameOver_text.setOrigin(GameOver_textRect.width / 2, GameOver_textRect.height);
	GameOver_text.setPosition(Vector2f(WIDTH / 2.0f, HEIGHT / 2.0f));

	Text GameClear_text;
	GameClear_text.setFillColor(Color::Magenta);
	GameClear_text.setFont(font);
	GameClear_text.setCharacterSize(200);
	GameClear_text.setString("GAME CLEAR");
	FloatRect GameClear_textRect = GameClear_text.getLocalBounds();
	GameClear_text.setOrigin(GameClear_textRect.width / 2, GameClear_textRect.height / 2);
	GameClear_text.setPosition(Vector2f(WIDTH / 2.0f, HEIGHT / 2.0f - 200));
	

	while (window.isOpen()) {
		Event e;
		while (window.pollEvent(e)) {
			//윈도우의 x를 눌렀을 때 창이 닫아지도록
			if (e.type == Event::Closed)
				window.close();
		}
		switch (gameState) {
			case Mainmenu:
				DrawMainmenu(window, start_text, exit_text, selectedItem);
				window.draw(title);
				window.display();

				if (Keyboard::isKeyPressed(Keyboard::Up) && selectedItem > 0) {
					cout << selectedItem << endl;
					sound_select.stop();
					sound_select.play();
					selectedItem--;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Down) && selectedItem < 1) {
					cout << selectedItem << endl;
					sound_select.stop();
					sound_select.play();
					selectedItem++;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Space)|| Keyboard::isKeyPressed(Keyboard::Enter)) {
					if (selectedItem == 0) {
						music_game.play(); //음악 재생
						music_game.setLoop(true); //반복 재생
						gameState = Playing;
					}
					else if (selectedItem == 1) {
						window.close();
					}
				}
				break;

			case Playing:
				music_game.setVolume(50);  //음악 음량 설정
				//방향키가 동시에 눌러지지 않도록 else 처리
				if (Keyboard::isKeyPressed(Keyboard::Right)) {
					pacman.dir_ = DIR_RIGHT;
					cout << "\n누른키 : right\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Left)) {
					pacman.dir_ = DIR_LEFT;
					cout << "\n누른키 : left\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Up)) {
					pacman.dir_ = DIR_UP;
					cout << "\n누른키 : up\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Down)) {
					pacman.dir_ = DIR_DOWN;
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
				//좌측 포탈
				if (pacman.dir_ == DIR_LEFT && pacman.x_ == 2 && pacman.y_ == 9) {
					pacman.x_ = 28; //우측 포탈로 나옴
				}
				//우측 포탈
				if (pacman.dir_ == DIR_RIGHT && pacman.x_ == 28 && pacman.y_ == 9) {
					pacman.x_ = 2; //좌측 포탈로 나옴
				}
				//코인을 먹을시 점수 올라감
				for (size_t i = 0; i < coins.size(); i++) {
					if (pacman.x_ == coins[i].x && pacman.y_ == coins[i].y && !coins[i].isCollected) {
						coins[i].isCollected = true;
						point += 30;
						// 포인트 변수(int)를 문자열로 변환후 score의 string을 업데이트
						score.setString("score : " + to_string(point));
						sound_coin.stop();
						sound_coin.play(); //코인 먹을시 효과음 재생
					}
				}
				
				pacman.UpdateAnimation(pac_def, pac_up, pac_down, pac_left, pac_right);
				pacman.sprite_.setPosition(pacman.x_ * BLOCK_SIZE, pacman.y_ * BLOCK_SIZE);
				enemy_1.MoveEnemy(enemy_1, map_control); //적 이동 함수 호출
				enemy_1.sprite_.setPosition(enemy_1.x_* BLOCK_SIZE, enemy_1.y_* BLOCK_SIZE);
				enemy_2.MoveEnemy(enemy_2, map_control); //적 이동 함수 호출
				enemy_2.sprite_.setPosition(enemy_2.x_* BLOCK_SIZE, enemy_2.y_* BLOCK_SIZE);
			
				if (pacman.x_ == enemy_1.x_ && pacman.y_ == enemy_1.y_) {
					pacman.CollideWithEnemy(); //적과 충돌시 함수 호출
				}
				if (pacman.x_ == enemy_2.x_ && pacman.y_ == enemy_2.y_) {
					pacman.CollideWithEnemy(); //적과 충돌시 함수 호출
				}
			
				window.clear(); //전화면 지우기

				for (const Coin& coin : coins) {
					if (!coin.isCollected) {
						coinShape.setPosition(coin.x * BLOCK_SIZE + BLOCK_SIZE / 6 + 15, coin.y * BLOCK_SIZE + BLOCK_SIZE / 6 + 15); // 코인 위치 지정
						coinShape.setFillColor(Color(255, 255, 0)); // 코인 색지정
						window.draw(coinShape); // 코인 그리기
					}
				}
				for (int i = 0; i < pacman.hearts_; ++i) {
					heartSprite.setPosition(WIDTH - 50 - (i + 1) * 50,0); //하트 위치 지정
					window.draw(heartSprite); //하트 그리기
				}
				if (pacman.hearts_ == 0) { //하트가 0개 되면 게임 오버
					music_game.stop();	   //음악 끄기
					music_gameover.setVolume(30);  //음악 음량 설정
					music_gameover.play(); //음악 재생
					gameState = GameOver; 
				}
				if (all_of(coins.begin(), coins.end(), [](const Coin& coin) { return coin.isCollected; })) { //코인 모두 획득시 게임 클리어
					point += pacman.hearts_ * 500; //남은 하트 개수에 따라 추가 점수
					score.setString("score : " + to_string(point));
					heartSprite.setScale(2.0f, 2.0f);
					music_game.stop(); //음악 끄기
					music_gameclear.setVolume(30);  //음악 음량 설정
					music_gameclear.play(); //음악 재생
					gameState = GameClear;
				}
				window.draw(score);
				window.draw(map_sprite);
				window.draw(pacman.sprite_);
				window.draw(enemy_1.sprite_);
				window.draw(enemy_2.sprite_);
				window.display();
				break;

			case GameOver:
				window.clear();
				window.draw(GameOver_text);
				window.display();
				break;

			case GameClear:
				window.clear();
				//점수 텍스트 중앙배치
				FloatRect scoreRect = score.getLocalBounds();
				score.setOrigin(scoreRect.width / 2, scoreRect.height / 2);
				score.setPosition(Vector2f(WIDTH / 2.0f, 450));
				score.setCharacterSize(100);
				for (int i = 0; i < pacman.hearts_; ++i) { 
					heartSprite.setPosition(WIDTH / 2 - (pacman.hearts_ * (100 + 30) - 30) / 2 + i * (100 + 30), 600); //하트 개수에 맞게 가로 중앙 배치
					window.draw(heartSprite); //남은 하트 그리기
				}
				window.draw(GameClear_text);
				window.draw(score);
				window.display();
				break;
		}
	}
	return 0;
}

