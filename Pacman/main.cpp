#include <SFML/Graphics.hpp>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace sf;

#define DIR_UP		1
#define DIR_DOWN	2
#define DIR_RIGHT	3
#define DIR_LEFT	4

class Pacman {

public:
	int dir_; //�̵� ����
	int x_;
	int y_;
	RectangleShape sprite_;

};

class Coin {
public:
	int x;
	int y;
	bool isCollected; //������ ȹ���ߴ��� ����

	Coin(int x, int y) : x(x), y(y), isCollected(false) {} //���� ������

};

int main() {
	//�� ���� �迭
	//0 : �̵��� �� �ִ� ��
	//1 : �̵� �Ұ����� �� (��)
	bool map_control[16][28] = 
		{
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
			{1,0,1,1,1,0,1,0,1,1,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
			{1,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
			{1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		};
	
	//�Ѹ� �̹��� �����¿�
	Texture pac_up, pac_down, pac_left, pac_right;
	pac_up.loadFromFile("Resource/Image/pacman_up.png");
	pac_down.loadFromFile("Resource/Image/pacman_down.png");
	pac_left.loadFromFile("Resource/Image/pacman_left.png");
	pac_right.loadFromFile("Resource/Image/pacman_right.png");

	//�� �̹���
	Texture map;
	map.loadFromFile("Resource/Image/map_wall_test.png");

	const int WIDTH = 1400;						//�ȼ� �ʺ�
	const int HEIGHT = 800;						//�ȼ� ����
	const int BLOCK_SIZE = 50;					//�� ĭ�� ������ �ִ� �ȼ�
	const int G_WIDTH = WIDTH / BLOCK_SIZE;		//�׸����� �ʺ�
	const int G_HEIGHT = HEIGHT / BLOCK_SIZE;	//�׸����� ����;

	RenderWindow window(VideoMode(WIDTH, HEIGHT), "Pacman Game");

	//��ǻ�Ͱ� 1�� ���� ó���ϴ� Ƚ���� �����Ѵ�.  
	//Frame Per Second�� ����
	window.setFramerateLimit(8);

	Pacman pacman;
	pacman.x_ = 1, pacman.y_ = 2;//�Ѹ��� �׸��� ��ǥ
	pacman.dir_ = DIR_DOWN;		//�Ѹ��� �̵��ϴ� ����
	pacman.sprite_.setTexture(&pac_down);
	pacman.sprite_.setPosition(pacman.x_ * BLOCK_SIZE, pacman.y_ * BLOCK_SIZE);
	pacman.sprite_.setSize(Vector2f(BLOCK_SIZE, BLOCK_SIZE));

	// ������ �׸��� ���� �簢�� ��ü
	RectangleShape coinShape(Vector2f(BLOCK_SIZE / 4, BLOCK_SIZE / 4)); //������ 1/4 ũ��
	//���� ȹ��� ���� ���
	int point = 0;

	Sprite map_sprite;
	map_sprite.setTexture(map);

	vector<Coin> coins; // ���� ��ü�� �����ϴ� ����

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 28; j++) {
			if (map_control[i][j] == 0) {
				coins.push_back(Coin(j, i)); // �� �迭 0�� ���� ��ü �߰�
			}
		}
	}

	while (window.isOpen()) {
		Event e;
		while (window.pollEvent(e)) {
			//�������� x�� ������ �� â�� �ݾ�������
			if (e.type == Event::Closed)
				window.close();
		}
		//����Ű�� ���ÿ� �������� �ʵ��� else ó��
		if (Keyboard::isKeyPressed(Keyboard::Right)) {
			pacman.dir_ = DIR_RIGHT;
			pacman.sprite_.setTexture(&pac_right);
			cout << "\n����Ű : right\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
		}
		else if (Keyboard::isKeyPressed(Keyboard::Left)) {
			pacman.dir_ = DIR_LEFT;
			pacman.sprite_.setTexture(&pac_left);
			cout << "\n����Ű : left\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
		}
		else if (Keyboard::isKeyPressed(Keyboard::Up)) {
			pacman.dir_ = DIR_UP;
			pacman.sprite_.setTexture(&pac_up);
			cout << "\n����Ű : up\nx : " << pacman.x_ << "\ny : " << pacman.y_ << endl;
		}
		else if (Keyboard::isKeyPressed(Keyboard::Down)) {
			pacman.dir_ = DIR_DOWN;
			pacman.sprite_.setTexture(&pac_down);
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

		for (size_t i = 0; i < coins.size(); i++) {
			if (pacman.x_ == coins[i].x && pacman.y_ == coins[i].y && !coins[i].isCollected) {
				coins[i].isCollected = true;
				point++; // ���� ȹ��� ����Ʈ �ö�
				cout<<"! ���� ȹ�� ! : "<< point << endl;
			}
		}

		pacman.sprite_.setPosition(pacman.x_ * BLOCK_SIZE, pacman.y_ * BLOCK_SIZE);

		window.clear(); //��ȭ�� �����
		window.draw(map_sprite);
		window.draw(pacman.sprite_);

		for (const Coin& coin : coins) {
			if (!coin.isCollected) {
				coinShape.setPosition(coin.x * BLOCK_SIZE + BLOCK_SIZE / 4, coin.y * BLOCK_SIZE + BLOCK_SIZE / 4); // ���� ��ġ ����
				coinShape.setFillColor(Color(255,255,0)); // ���� ������
				// ���� �׸���
				window.draw(coinShape);
			}
		}
		window.display();
	}

	return 0;
}