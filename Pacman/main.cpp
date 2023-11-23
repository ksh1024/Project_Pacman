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

#define BLOCK_SIZE 50 //�� ĭ�� ������ �ִ� �ȼ�


enum GameState {
	Mainmenu,
	Playing,
	GameOver,
	GameClear
};
GameState gameState = Mainmenu; // ���θ޴� ���·� �ʱ�ȭ

//���θ޴��� �׸��� �Լ�
void DrawMainmenu(RenderWindow& window, Text& start_text, Text& exit_text, int selectedItem) {
	start_text.setFillColor(selectedItem == 0 ? Color::Green : Color::White); //0(start)���ý� ���ڻ� �ʷϻ����� ����, �ƴҽ� ���
	exit_text.setFillColor(selectedItem == 1 ? Color::Green : Color::White);  //1(exit)���ý� ���ڻ� �ʷϻ����� ����, �ƴҽ� ���
	window.draw(start_text);
	window.draw(exit_text);
}

class Pacman {
public:
	int dir_; //�̵� ����
	int x_;
	int y_;
	RectangleShape sprite_;
	int hearts_ = 5; //��Ʈ (����)

	Music sound_collide; //���� �浹�� ȿ����
	
	Pacman() {
		if (!sound_collide.openFromFile("Resource/Sound/crash.wav")) {
			cerr << "Failed to load music" << endl;
		}
	}

	// ���� �浹 �� ��Ʈ ����
	void CollideWithEnemy() {
		if (hearts_ > 0) { //��Ʈ�� �Ѱ� �̻� ����������
			hearts_--;     //��Ʈ �ϳ� �پ��
			sound_collide.setVolume(30);
			sound_collide.stop();
			sound_collide.play();
		}
	}

	Clock animationClock;		//�ִϸ��̼� �ӵ� ����
	float animationTime = 0.2f; //�ִϸ��̼� ������ �� �ð� ����
	Time elapsedTime;			//�Լ� ȣ�� �� ��� �ð�
	bool mouthOpen = false;		//���� �����ִ��� �����ִ��� ����

	//���� ���ȴ� �ݴ� �ִϸ��̼�
	void UpdateAnimation(Texture& pac_def, Texture& pac_up, Texture& pac_down, Texture& pac_left, Texture& pac_right) {
		elapsedTime += animationClock.restart();
		if (elapsedTime.asSeconds() >= animationTime) {
			//��� �ð� �ʱ�ȭ
			elapsedTime = Time::Zero;
			//�̵� ���⿡ ���� �Ѹ� �ؽ�ó ����
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
			//���� �������� ���� mouthOpen ���� ��ȯ
			mouthOpen = !mouthOpen;
		}
	}

};

class Enemy {
public:
	int dir_; //�̵� ����
	int x_;
	int y_;
	RectangleShape sprite_;

	//���� ���� �̵�
	void MoveEnemy(Enemy& enemy, bool map_control[18][30]) {
		// ���� ��ġ���� �̵� ������ ������ ������ ����
		vector<int> possibleDirs;

		if (enemy.y_ > 0 && !map_control[enemy.y_ - 1][enemy.x_]) {
			possibleDirs.push_back(1); // 1: ��
		}
		if (enemy.y_ < 17 && !map_control[enemy.y_ + 1][enemy.x_]) {
			possibleDirs.push_back(2); // 2: �Ʒ�
		}
		if (enemy.x_ > 0 && !map_control[enemy.y_][enemy.x_ - 1]) {
			possibleDirs.push_back(3); // 3: ����
		}
		if (enemy.x_ < 29 && !map_control[enemy.y_][enemy.x_ + 1]) {
			possibleDirs.push_back(4); // 4: ������
		}

		if (!possibleDirs.empty()) {
			// �̵� ������ ���� �߿��� �������� ����
			int randomIndex = rand() % possibleDirs.size();
			int randomDir = possibleDirs[randomIndex];

			// ������ �������� �̵�
			if (randomDir == 1) {      // ���� �̵�
				enemy.y_--;
			}
			else if (randomDir == 2) { // �Ʒ��� �̵�
				enemy.y_++;
			}
			else if (randomDir == 3) { // �������� �̵�
				enemy.x_--;
			}
			else if (randomDir == 4) { // ���������� �̵�
				enemy.x_++;
			}
		}
	}

};

class Coin {
public:
	int x;
	int y;
	bool isCollected; //������ ȹ���ߴ��� ����

	Coin(int x, int y) : x(x), y(y), isCollected(false) {} //���� ������
};

//�� ���� �迭
//	0 : �̵��� �� �ִ� ��
//	1 : �̵� �Ұ����� �� (��)
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

	Music sound_select; //�޴� ���� ȿ����
	if (!sound_select.openFromFile("Resource/Sound/select.wav")) {
		cerr << "Failed to load music" << endl;
		return -1;
	}
	Music music_game; //���� ��� ����
	if (!music_game.openFromFile("Resource/Sound/gameplay.wav")) {
		cerr << "Failed to load music" << endl;
		return -1;
	}
	Music sound_coin; //���� ȹ��� ȿ����
	if (!sound_coin.openFromFile("Resource/Sound/coin.wav")) {
		cerr << "Failed to load music" << endl;
		return -1;
	}
	Music music_gameover; //���� ���� ����
	if (!music_gameover.openFromFile("Resource/Sound/gameover.wav")) {
		cerr << "Failed to load music" << endl;
		return -1;
	}
	Music music_gameclear; //���� Ŭ����� ����
	if (!music_gameclear.openFromFile("Resource/Sound/gameclear.wav")) {
		cerr << "Failed to load music" << endl;
		return -1; 
	}

	//�Ѹ� �ؽ��� �⺻, �����¿�
	Texture pac_def, pac_up, pac_down, pac_left, pac_right;
	pac_def.loadFromFile("Resource/Image/pacman_closed.png");
	pac_up.loadFromFile("Resource/Image/pacman_up.png");
	pac_down.loadFromFile("Resource/Image/pacman_down.png");
	pac_left.loadFromFile("Resource/Image/pacman_left.png");
	pac_right.loadFromFile("Resource/Image/pacman_right.png");

	//�� �ؽ���
	Texture enemy_img;
	enemy_img.loadFromFile("Resource/Image/enemy.png");

	//�� �ؽ���
	Texture map;
	map.loadFromFile("Resource/Image/map.png");

	const int WIDTH = 1550;						//�ȼ� �ʺ�
	const int HEIGHT = 900;						//�ȼ� ����
	const int G_WIDTH = WIDTH / BLOCK_SIZE;		//�׸����� �ʺ�
	const int G_HEIGHT = HEIGHT / BLOCK_SIZE;	//�׸����� ����;

	RenderWindow window(VideoMode(WIDTH, HEIGHT), "Pacman Game");

	//��ǻ�Ͱ� 1�� ���� ó���ϴ� Ƚ���� �����Ѵ�.  
	//Frame Per Second�� ����
	window.setFramerateLimit(8);

	Pacman pacman;
	pacman.x_ = 28, pacman.y_ = 15;//�Ѹ��� �׸��� ��ǥ
	pacman.dir_ = DIR_LEFT;			//�Ѹ��� �̵��ϴ� ����
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


	// ������ �׸��� ���� �簢�� ��ü
	RectangleShape coinShape(Vector2f(BLOCK_SIZE / 6, BLOCK_SIZE / 6)); //����� 1/6 ũ��
	//���� ȹ��� ���� ���
	int point = 0;

	Sprite map_sprite;
	map_sprite.setTexture(map);

	vector<Coin> coins; // ���� ��ü�� �����ϴ� ����
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 29; j++) {
			if (map_control[i][j] == 0) {
				coins.push_back(Coin(j, i)); // �� �迭 0�� ���� ��ü �߰�
			}
		}
	}
	Font font;
	if (!font.loadFromFile("Resource/Font/pixel.ttf")) {
		printf("failed to load font file");
		return -1;
	}

	//���� ���õ� �׸� (0 : �����ϱ�, 1 : ������)
	int selectedItem = 0;

	Text title;
	title.setFont(font);
	title.setCharacterSize(200);
	title.setString("PACMAN");
	title.setFillColor(Color::Yellow);
	//�ؽ�Ʈ �߾ӹ�ġ
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
			//�������� x�� ������ �� â�� �ݾ�������
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
						music_game.play(); //���� ���
						music_game.setLoop(true); //�ݺ� ���
						gameState = Playing;
					}
					else if (selectedItem == 1) {
						window.close();
					}
				}
				break;

			case Playing:
				music_game.setVolume(50);  //���� ���� ����
				//����Ű�� ���ÿ� �������� �ʵ��� else ó��
				if (Keyboard::isKeyPressed(Keyboard::Right)) {
					pacman.dir_ = DIR_RIGHT;
					cout << "\n����Ű : right\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Left)) {
					pacman.dir_ = DIR_LEFT;
					cout << "\n����Ű : left\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Up)) {
					pacman.dir_ = DIR_UP;
					cout << "\n����Ű : up\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
				}
				else if (Keyboard::isKeyPressed(Keyboard::Down)) {
					pacman.dir_ = DIR_DOWN;
					cout << "\n����Ű : down\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
				}
				//�Ѹ� �̵�
				if (pacman.dir_ == DIR_UP && pacman.y_ > 1) {
					pacman.y_--;
				}
				else if (pacman.dir_ == DIR_DOWN && pacman.y_ < G_HEIGHT - 2) {
					pacman.y_++; // -2�� �ϴ� ���� : �Ѹ��� '�������𼭸�'�� ��ġ�� y�� ���� ������
								//	-2�� ���� ������ ȭ�鿡�� ��ĭ ������ �����Ե�
				}
				else if (pacman.dir_ == DIR_RIGHT && pacman.x_ < G_WIDTH - 2) {
					pacman.x_++;
				}
				else if (pacman.dir_ == DIR_LEFT && pacman.x_ > 1) {
					pacman.x_--;
				}
				//�� �̵�����
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
				//���� ��Ż
				if (pacman.dir_ == DIR_LEFT && pacman.x_ == 2 && pacman.y_ == 9) {
					pacman.x_ = 28; //���� ��Ż�� ����
				}
				//���� ��Ż
				if (pacman.dir_ == DIR_RIGHT && pacman.x_ == 28 && pacman.y_ == 9) {
					pacman.x_ = 2; //���� ��Ż�� ����
				}
				//������ ������ ���� �ö�
				for (size_t i = 0; i < coins.size(); i++) {
					if (pacman.x_ == coins[i].x && pacman.y_ == coins[i].y && !coins[i].isCollected) {
						coins[i].isCollected = true;
						point += 30;
						// ����Ʈ ����(int)�� ���ڿ��� ��ȯ�� score�� string�� ������Ʈ
						score.setString("score : " + to_string(point));
						sound_coin.stop();
						sound_coin.play(); //���� ������ ȿ���� ���
					}
				}
				
				pacman.UpdateAnimation(pac_def, pac_up, pac_down, pac_left, pac_right);
				pacman.sprite_.setPosition(pacman.x_ * BLOCK_SIZE, pacman.y_ * BLOCK_SIZE);
				enemy_1.MoveEnemy(enemy_1, map_control); //�� �̵� �Լ� ȣ��
				enemy_1.sprite_.setPosition(enemy_1.x_* BLOCK_SIZE, enemy_1.y_* BLOCK_SIZE);
				enemy_2.MoveEnemy(enemy_2, map_control); //�� �̵� �Լ� ȣ��
				enemy_2.sprite_.setPosition(enemy_2.x_* BLOCK_SIZE, enemy_2.y_* BLOCK_SIZE);
			
				if (pacman.x_ == enemy_1.x_ && pacman.y_ == enemy_1.y_) {
					pacman.CollideWithEnemy(); //���� �浹�� �Լ� ȣ��
				}
				if (pacman.x_ == enemy_2.x_ && pacman.y_ == enemy_2.y_) {
					pacman.CollideWithEnemy(); //���� �浹�� �Լ� ȣ��
				}
			
				window.clear(); //��ȭ�� �����

				for (const Coin& coin : coins) {
					if (!coin.isCollected) {
						coinShape.setPosition(coin.x * BLOCK_SIZE + BLOCK_SIZE / 6 + 15, coin.y * BLOCK_SIZE + BLOCK_SIZE / 6 + 15); // ���� ��ġ ����
						coinShape.setFillColor(Color(255, 255, 0)); // ���� ������
						window.draw(coinShape); // ���� �׸���
					}
				}
				for (int i = 0; i < pacman.hearts_; ++i) {
					heartSprite.setPosition(WIDTH - 50 - (i + 1) * 50,0); //��Ʈ ��ġ ����
					window.draw(heartSprite); //��Ʈ �׸���
				}
				if (pacman.hearts_ == 0) { //��Ʈ�� 0�� �Ǹ� ���� ����
					music_game.stop();	   //���� ����
					music_gameover.setVolume(30);  //���� ���� ����
					music_gameover.play(); //���� ���
					gameState = GameOver; 
				}
				if (all_of(coins.begin(), coins.end(), [](const Coin& coin) { return coin.isCollected; })) { //���� ��� ȹ��� ���� Ŭ����
					point += pacman.hearts_ * 500; //���� ��Ʈ ������ ���� �߰� ����
					score.setString("score : " + to_string(point));
					heartSprite.setScale(2.0f, 2.0f);
					music_game.stop(); //���� ����
					music_gameclear.setVolume(30);  //���� ���� ����
					music_gameclear.play(); //���� ���
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
				//���� �ؽ�Ʈ �߾ӹ�ġ
				FloatRect scoreRect = score.getLocalBounds();
				score.setOrigin(scoreRect.width / 2, scoreRect.height / 2);
				score.setPosition(Vector2f(WIDTH / 2.0f, 450));
				score.setCharacterSize(100);
				for (int i = 0; i < pacman.hearts_; ++i) { 
					heartSprite.setPosition(WIDTH / 2 - (pacman.hearts_ * (100 + 30) - 30) / 2 + i * (100 + 30), 600); //��Ʈ ������ �°� ���� �߾� ��ġ
					window.draw(heartSprite); //���� ��Ʈ �׸���
				}
				window.draw(GameClear_text);
				window.draw(score);
				window.display();
				break;
		}
	}
	return 0;
}

