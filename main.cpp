// main.cpp:								MSe, 2021-03-26 
// v1.0
// FH Oberösterreich / Campus Hagenberg
// ---- 
// Time effort: approx. 10 h
// ----
// Exercise 1 in computer graphics:
// A simple 3D maze with collectibles and other objects 
// ========================================================================

#include "GL\glew.h"
#include "GL\freeglut.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// GLUT Window ID
int windowid;

// ------ constant variables --------------------------------------
constexpr bool		fly_mode_active		= false;						// if TRUE: allows you to fly up or down with "q" and "e" 

constexpr int		labyrinth_size_x	= 10;							// array position: width of the labyrinth 
constexpr int		labyrinth_size_z	= labyrinth_size_x + 5;					// array position: length of the labyrinth + starting room
constexpr float		wall_size		= 5.0f;							// length and width of a wall in the labyrinth
constexpr float		start_room_z		= (labyrinth_size_z - labyrinth_size_x) * wall_size;	// actual length of the starting room
constexpr float		game_board_x		= labyrinth_size_x * wall_size;				// actual width of the labyrinth 
constexpr float		game_board_z		= game_board_x + start_room_z;				// actual length of the labyrinth + starting room

constexpr float		move_speed		= 0.5f;							// speed of movement of the camera
constexpr float		rotate_speed		= 0.1f;							// speed of rotation of the camera 
constexpr float		jump_limit		= 5.0f;							// maximum jump height of the camera 
constexpr float		coins_animation_max	= 1.0f;							// maximum height for the animation when picking up a coin 

constexpr float		default_cam_x		= 0.0f;							// default camera settings for the x-coordinate
constexpr float		default_cam_y		= 0.5f;							// default camera settings for the y-coordinate
constexpr float		default_cam_z		= game_board_z / 2 - 15.0f;				// default camera settings for the z-coordinate

// ------ prototypes --------------------------------------
void draw_text_collectables				(void);
bool collision_back					(int x, int z);
bool collision_front					(int x, int z);
bool collision_right					(int x, int z);
bool collision_left					(int x, int z);
bool collision_detected					(bool (*function) (int x, int y), int object);
void keyboard						(unsigned char key, int x, int y);
void reshape_func					(int x, int y);
void draw_part_of_wall					(void);
void draw_floor						(void);
void draw_object_1					(void);
void draw_object_2					(void);
void draw_object_3_primitives				(void);
void draw_collectables					(void);
void draw_labyrinth					(void);
void draw_game_board					(void);
void mouse_move						(int mx, int my);
void on_mouse_click					(int button, int state, int x, int y);
void collectables_collected_animation			(int value);
void rotate_collectables				(int value);
void rotate_cam_with_mouse				(int value);
void cam_jump						(int value);
void create_collectables				(void);


// ------ labyrinth --------------------------------------
// 0 ... Nothing
// 1 ... Wall
// 2 ... Collectables (Coins)
// 3 ... Object 1 (basic forms)
// 4 ... Object 2 (basic forms)
// 5 ... Object 3 (primitives)
int labyrinth[labyrinth_size_z][labyrinth_size_x] = {
	{ 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 },
	{ 1, 0, 0, 2, 0, 1, 1, 0, 1, 1 },
	{ 1, 0, 1, 1, 1, 0, 1, 0, 1, 1 },
	{ 1, 0, 1, 0, 0, 0, 1, 0, 0, 1 },
	{ 1, 0, 1, 0, 1, 0, 1, 1, 0, 1 },
	{ 1, 0, 1, 0, 1, 0, 1, 1, 0, 1 },
	{ 1, 0, 2, 0, 1, 0, 0, 0, 0, 1 },
	{ 1, 0, 1, 1, 1, 2, 1, 2, 1, 1 },
	{ 1, 1, 1, 1, 1, 0, 1, 1, 1, 1 },
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 0, 3, 0, 0, 0, 0, 0, 1 },
	{ 1, 0, 0, 0, 0, 0, 0, 5, 0, 1 },
	{ 1, 0, 0, 4, 0, 0, 0, 0, 0, 1 },
	{ 1, 2, 0, 0, 0, 0, 0, 0, 0, 1 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};

// ------ camera --------------------------------------
class camera {
public:
	camera() = default;

	void rotate_left() {
		angle -= rotate_speed;
		lx = sin(angle);
		lz = -cos(angle);
	}

	void rotate_right() {
		angle += rotate_speed;
		lx = sin(angle);
		lz = -cos(angle);
	}

	void move_left() {
		nav_x += lz * move_speed;
		nav_z += -lx * move_speed;
	}

	void move_right() {
		nav_x -= lz * move_speed;
		nav_z -= -lx * move_speed;
	}

	void move_forward() {
		nav_x += lx * move_speed;
		nav_z += lz * move_speed;
	}

	void move_back() {
		nav_x -= lx * move_speed;
		nav_z -= lz * move_speed;
	}

	void fly_up() {
		nav_y += move_speed;
	}

	void fly_down() {
		nav_y -= move_speed;
	}

	void jump() {
		jumping = true;
	}

	void handle_camera() {
		gluLookAt(
			nav_x, nav_y, nav_z,
			nav_x + lx, nav_y, nav_z + lz,
			0.0f, 1.0f, 0.0f
		);
	}

	GLfloat get_lx			() const { return lx; }
	GLfloat get_lz			() const { return lz; }
	GLfloat get_nav_x		() const { return nav_x; }
	GLfloat get_nav_y		() const { return nav_y; }
	GLfloat get_nav_z		() const { return nav_z; }
	bool	get_jumping		() const { return jumping; }
	bool	get_jump_limit_reached	() const { return jump_limit_reached; }
	bool	get_mouse_is_right	() const { return mouse_is_right; }
	bool	get_mouse_is_left	() const { return mouse_is_left; }

	void	set_nav_y		(const GLfloat y = 0.5) { nav_y = y; }
	void	set_nav_x		(const GLfloat x)	{ nav_x = x; }
	void	set_jumping		(const bool is_jumping) { jumping = is_jumping; }
	void	set_jump_limit_reached	(const bool reached)	{ jump_limit_reached = reached; }
	void	set_mouse_is_right	(const bool right)	{ mouse_is_right = right; }
	void	set_mouse_is_left	(const bool left)	{ mouse_is_left = left; }

private:
	GLfloat angle			{ 0.0f };		// angle of rotation for the camera direction
	GLfloat lx			{ 0.0f };		// actual vector representing the camera's direction x
	GLfloat lz			{ -1.0f };		// actual vector representing the camera's direction z
	GLfloat nav_x			{ default_cam_x };	// x position of the camera
	GLfloat nav_y			{ default_cam_y };	// y position of the camera
	GLfloat nav_z			{ default_cam_z };	// z position of the camera
	bool	jumping			{ false };
	bool	jump_limit_reached	{ false };
	bool	mouse_is_right		{ false };
	bool	mouse_is_left		{ false };
};

// ------ collectable --------------------------------------
class collectable {
public:
	collectable(int z, int x) : pos_z_labyrinth{ z }, pos_x_labyrinth{ x } {
	}

	int	get_pos_x		() const { return pos_x_labyrinth; }
	int	get_pos_z		() const { return pos_z_labyrinth; }
	GLfloat get_pos_y		() const { return pos_y; }
	bool	get_near_coin		() const { return near_coin; }
	bool	get_is_collected	() const { return is_collected; }
	bool	get_animation_max_point	() const { return animation_max_point; }

	void	set_pos_y		(const GLfloat y)	{ pos_y = y; }
	void	set_near_coin		(const bool n)		{ if (!is_collected) near_coin = n; }
	void	set_is_collected	(const bool col)	{ is_collected = col; near_coin = false; }
	void	set_animation_max_point	(const bool max)	{ animation_max_point = max; }
	
private:
	int	pos_x_labyrinth		{ 0 };
	int	pos_z_labyrinth		{ 0 };
	GLfloat	pos_y			{ -1.0f };
	bool	near_coin		{ false };
	bool	is_collected		{ false };
	bool	animation_max_point	{ false };
};

// ------ collectables --------------------------------------
class collectables {
public:
	collectables() = default;

	// inserts coins into the vector 
	void insert_collectables(int z, int x) {
		coins.push_back(make_shared<collectable>(z, x));
	}

	// returns whether any coin is near the camera 
	bool near_any_coins() {
		for (auto& obj : coins) {
			if (obj->get_near_coin()) {
				return true;
			}
		}
		return false;
	}

	// resets the flags indicating whether a coin is near the camera
	void reset_near_coins() {
		for (auto& obj : coins) {
			obj->set_near_coin(false);
		}
	}

	// finds a specific coin in the labyrinth 
	std::shared_ptr<collectable> get_coin(int z, int x) {
		for (auto& obj : coins) {
			if (obj->get_pos_x() == x && obj->get_pos_z() == z) {
				return obj;
			}
		}

		return nullptr;
	}

	// finds the coin that is currently close to the camera
	std::shared_ptr<collectable> get_near_coin() {
		for (auto& obj : coins) {
			if (obj->get_near_coin()) {
				return obj;
			}
		}

		return nullptr;
	}

	// performs the animation while collecting a coin 
	void animation_collected_coins() {
		for (auto& obj : coins) {
			if (obj->get_is_collected()) {
				if (!obj->get_animation_max_point()) {
					obj->set_pos_y(obj->get_pos_y() + 0.1);
					if (obj->get_pos_y() > coins_animation_max) {
						obj->set_animation_max_point(true);
					}
				}
				else {
					obj->set_pos_y(obj->get_pos_y() - 0.1);
					if (obj->get_pos_y() < -7.0f) {
						labyrinth[obj->get_pos_z()][obj->get_pos_x()] = 0;
						obj->set_is_collected(false);
					}
				}
			}
		}
	}

	int	get_max_collectables	()	const { return max_collectables; }
	int	get_collected		()	const { return collected; }
	GLfloat get_angle		()	const { return angle; }

	void	set_max_collectables	(const int max)		{ max_collectables = max; }
	void	set_collected		(const int increase)	{ collected = increase; }
	void	set_angle		(const GLfloat ang)	{ 
									angle = ang; 
									if (angle >= 360.0f) {
										angle -= 360.0f;
									}
								}

private:
	vector<shared_ptr<collectable>>	coins;					// vector to save the coins
	int				max_collectables	{ 0 };		// maximum number of coins that can be collected 
	int				collected		{ 0 };		// number of coins already collected 
	GLfloat				angle			{ 0.0f };	// current angle of the coins 
};

// ------ global variables --------------------------------------
camera			cam = camera();
collectables	coins = collectables();

// ------ draw_text_collectables --------------------------------------
// text output: how many coins have already been collected 
// at the top left of the screen 
void draw_text_collectables(void) {
	glDisable(GL_TEXTURE_2D); 
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0, glutGet(GLUT_WINDOW_WIDTH), 0.0, glutGet(GLUT_WINDOW_HEIGHT));
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 30);
	string highscore;
	if (coins.near_any_coins()) {
		highscore = "Coins: " + to_string(coins.get_collected()) + " / " + to_string(coins.get_max_collectables()) + "\nPress left mouse button to pick up";
	}
	else {
		highscore = "Coins: " + to_string(coins.get_collected()) + " / " + to_string(coins.get_max_collectables());
	}
	glutBitmapString(GLUT_BITMAP_HELVETICA_18, (unsigned char*) highscore.c_str());
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW); 
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

// ------ collision_back --------------------------------------
// auxiliary function for collision handling 
// collision detection when walking backwards 
bool collision_back(int x, int z) {
	float collision_area_wall = wall_size / 2 + 1.0f;
	return	x + collision_area_wall >= cam.get_nav_x() - cam.get_lx() * move_speed &&
			z + collision_area_wall >= cam.get_nav_z() - cam.get_lz() * move_speed &&
			x - collision_area_wall <= cam.get_nav_x() - cam.get_lx() * move_speed &&
			z - collision_area_wall <= cam.get_nav_z() - cam.get_lz() * move_speed;
}

// ------ collision_front --------------------------------------
// auxiliary function for collision handling 
// collision detection when walking forward 
bool collision_front(int x, int z) {
	float collision_area_wall = wall_size / 2 + 1.0f;
	return	x + collision_area_wall >= cam.get_nav_x() + cam.get_lx() * move_speed &&
			z + collision_area_wall >= cam.get_nav_z() + cam.get_lz() * move_speed &&
			x - collision_area_wall <= cam.get_nav_x() + cam.get_lx() * move_speed &&
			z - collision_area_wall <= cam.get_nav_z() + cam.get_lz() * move_speed;
}

// ------ collision_right --------------------------------------
// auxiliary function for collision handling 
// collision detection when going to the right 
bool collision_right(int x, int z) {
	float collision_area_wall = wall_size / 2 + 1.0f;
	return	x + collision_area_wall >= cam.get_nav_x() - cam.get_lz() * move_speed &&
			z + collision_area_wall >= cam.get_nav_z() + cam.get_lx() * move_speed &&
			x - collision_area_wall <= cam.get_nav_x() - cam.get_lz() * move_speed &&
			z - collision_area_wall <= cam.get_nav_z() + cam.get_lx() * move_speed;
}

// ------ collision_right --------------------------------------
// auxiliary function for collision handling 
// collision detection when going to the left 
bool collision_left(int x, int z) {
	float collision_area_wall = wall_size / 2 + 1.0f;
	return	x + collision_area_wall >= cam.get_nav_x() + cam.get_lz() * move_speed &&
			z + collision_area_wall >= cam.get_nav_z() - cam.get_lx() * move_speed &&
			x - collision_area_wall <= cam.get_nav_x() + cam.get_lz() * move_speed &&
			z - collision_area_wall <= cam.get_nav_z() - cam.get_lx() * move_speed;
}

// ------ collision_detected --------------------------------------
// collision treatment against walls and collectables
bool collision_detected(bool (*function) (int x, int y), int object) {
	float z = -game_board_z / 2 + wall_size / 2;
	float x = -game_board_x / 2 + wall_size / 2;

	for (int i = 0; i < labyrinth_size_z; i++) {
		for (int j = 0; j < labyrinth_size_x; j++) {
			if (labyrinth[i][j] == object &&
				function(x, z)) {
				if (object == 2) {
					coins.get_coin(i, j)->set_near_coin(true);
				}
				return true;
			}
			else if (object == 2) {
				coins.reset_near_coins();
			}
			x += wall_size;
		}
		x = -game_board_x / 2 + wall_size / 2;
		z += wall_size;
	}

	return false;
}

// ------ keyboard --------------------------------------
// Control the character with the keyboard 
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'a': // lowercase character 'a'
		if (!collision_detected(&collision_left, 1)) {
			cam.move_left();
		}
		collision_detected(&collision_left, 2);
		break;
	case 'd': // lowercase character 'd'
		if (!collision_detected(&collision_right, 1)) {
			cam.move_right();
		}
		collision_detected(&collision_right, 2);
		break;
	case 'w': // lowercase character 'w'
		if (!collision_detected(&collision_front, 1)) {
			cam.move_forward();
		}
		collision_detected(&collision_front, 2);
		break;
	case 's': // lowercase character 's'
		if (!collision_detected(&collision_back, 1)) {
			cam.move_back();
		}
		collision_detected(&collision_back, 2);
		break;
	case ' ': // space
		cam.jump();
		break;
	case 'q': // lowercase character 'q' (only available if fly mode is activated)
		if (fly_mode_active) {
			cam.fly_up();
		}
		break;
	case 'e': // lowercase character 'e' (only available if fly mode is activated)
		if (fly_mode_active) {
			cam.fly_down();
		}
		break;
	case 27: // Escape key
		glutDestroyWindow(windowid);
		exit(0);
		break;
	}
	glutPostRedisplay();
}

// ------ reshape_func --------------------------------------
// specify the new window 
// is triggered immediately before a window's first display and when the window is reshaped
void reshape_func(int x, int y) {
	if (y == 0 || x == 0) return;  //Nothing is visible then, so return

	glMatrixMode(GL_PROJECTION); //Set a new projection matrix
	glLoadIdentity();
	//Angle of view: 40 degrees
	//Near clipping plane distance: 0.5
	//Far clipping plane distance: 20.0

	gluPerspective(40.0, (GLdouble) x / (GLdouble) y, 0.5, 100.0);
	glViewport(0, 0, x, y);  //Use the whole window for rendering
}

// ------ draw_part_of_wall --------------------------------------
// draws a cube as a wall 
void draw_part_of_wall(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();
	glColor3d(0.0, 0.0, 0.0);
	glScaled(wall_size - 0.01f, wall_size - 0.01f, wall_size - 0.01f);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPushMatrix();
	glColor3d(0.0, 1.0, 0.0);
	glScaled(wall_size, wall_size, wall_size);
	glutSolidCube(1.0f);
	glPopMatrix();
}

// ------ draw_part_of_wall --------------------------------------
// draws a floor for the game board that is as big as the labyrinth + starting room
void draw_floor(void) {
	glPushMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glTranslatef(0.0f, -2.5f, 0.0f);
	glColor3d(0.1, 0.1, 0.1);
	glScaled(game_board_x, 0.0f, game_board_z);
	glutSolidCube(1.0f);
	glPopMatrix();
}

// ------ draw_object_1 --------------------------------------
// for exercise 1.1: draws a teapot placed on a cylinder 
void draw_object_1(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();						// cylinder
	glColor3d(0.0, 0.0, 0.0);
	glScaled(1.99f, 1.99f, 1.99f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCylinder(0.5f, 1.0f, 20.0f, 20.0f);
	glPopMatrix();

	glPushMatrix();						// teapot
	glColor3d(0.0, 0.0, 0.0);
	glScaled(1.99f, 1.99f, 1.99f);
	glTranslatef(0.0f, 0.76f, 0.0f);
	glutSolidTeapot(1.0f);
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPushMatrix();						// cylinder
	glColor3d(0.0, 1.0, 0.0);
	glScaled(2.0f, 2.0f, 2.0f);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCylinder(0.5f, 1.0f, 20.0f, 20.0f);
	glPopMatrix();

	glPushMatrix();						// teapot
	glColor3d(0.0, 1.0, 0.0);
	glScaled(2.0f, 2.0f, 2.0f);
	glTranslatef(0.0f, 0.75f, 0.0f);
	glutSolidTeapot(1.0f);
	glPopMatrix();
}

// ------ draw_object_2 --------------------------------------
// for exercise 1.1: draws a cone which is continued by a cylinder 
//					 and a sphere is placed on the cylinder  
void draw_object_2(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();						// cone
	glColor3d(0.0, 0.0, 0.0);
	glScaled(1.99f, 1.99f, 1.99f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCone(0.5f, 1.0f, 20.0f, 20.0f);
	glPopMatrix();

	glPushMatrix();						// cylinder
	glColor3d(0.0, 0.0, 0.0);
	glScaled(0.99f, 0.99f, 0.99f);
	glTranslatef(0.0f, 1.5f, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCylinder(0.25f, 1.0f, 20.0f, 20.0f);
	glPopMatrix();

	glPushMatrix();						// sphere
	glColor3d(0.0, 0.0, 0.0);
	glScaled(1.99f, 1.99f, 1.99f);
	glTranslatef(0.0f, 1.76f, 0.0f);
	glutSolidSphere(0.5f, 50.0f, 50.0f);
	glPopMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPushMatrix();						// cone
	glColor3d(0.0, 1.0, 0.0);
	glScaled(2.0f, 2.0f, 2.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCone(0.5f, 1.0f, 20.0f, 20.0f);
	glPopMatrix();

	glPushMatrix();						// cylinder
	glColor3d(0.0, 1.0, 0.0);
	glScaled(1.0f, 1.0f, 1.0f);
	glTranslatef(0.0f, 1.5f, 0.0f);
	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glutSolidCylinder(0.25f, 1.0f, 20.0f, 20.0f);
	glPopMatrix();

	glPushMatrix();						// sphere
	glColor3d(0.0, 1.0, 0.0);
	glScaled(2.0f, 2.0f, 2.0f);
	glTranslatef(0.0f, 1.75f, 0.0f);
	glutSolidSphere(0.5f, 50.0f, 50.0f);
	glPopMatrix();
}

// ------ draw_object_3_primitives --------------------------------------
// for exercise 1.2: draws a polygon that looks like a robot   
void draw_object_3_primitives(void) {
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_POLYGON);
	glVertex3f(0.0f, 0.0f, -2.0f);	// feet
	glVertex3f(0.0f, 0.0f, -1.0f);
	glVertex3f(0.0f, 2.0f, -1.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 3.5f, 1.0f);		
	glVertex3f(0.0f, 3.5f, 1.5f);
	glVertex3f(0.0f, 2.0f, 1.5f);	// arm right
	glVertex3f(0.0f, 2.0f, 2.0f);
	glVertex3f(0.0f, 4.0f, 2.0f);
	glVertex3f(0.0f, 4.0f, 0.0f);	// head
	glVertex3f(0.0f, 4.5f, 0.0f);	
	glVertex3f(0.0f, 4.5f, 0.5f);
	glVertex3f(0.0f, 5.5f, 0.5f);
	glVertex3f(0.0f, 5.5f, -1.5f);
	glVertex3f(0.0f, 4.5f, -1.5f);
	glVertex3f(0.0f, 4.5f, -1.0f);
	glVertex3f(0.0f, 4.0f, -1.0f);
	glVertex3f(0.0f, 4.0f, -3.0f);	// arm left
	glVertex3f(0.0f, 2.0f, -3.0f);
	glVertex3f(0.0f, 2.0f, -2.5f);
	glVertex3f(0.0f, 3.5f, -2.5f);
	glVertex3f(0.0f, 3.5f, -2.0f);
	glEnd();
}

// ------ draw_collectables --------------------------------------
// draws the collectibles that look like coins  
void draw_collectables(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPushMatrix();
	glColor3d(0.0, 0.0, 0.0);
	glScaled(1.99f, 1.99f, 1.99f);
	glRotatef(coins.get_angle(), 0.0f, 1.0f, 0.0f);
	glutSolidCylinder(0.25f, 0.1f, 10.0f, 10.0f);
	glPopMatrix();


	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPushMatrix();
	glColor3d(1.0, 1.0, 0.0);
	glScaled(2.0f, 2.0f, 2.0f);
	glRotatef(coins.get_angle(), 0.0f, 1.0f, 0.0f);
	glutSolidCylinder(0.25f, 0.1f, 10.0f, 10.0f);
	glPopMatrix();
}

// ------ draw_labyrinth --------------------------------------
// draws the labyrinth made up of walls, coins and other objects 
void draw_labyrinth(void) {
	float z = -game_board_z / 2 + wall_size / 2;
	float x = -game_board_x / 2 + wall_size / 2;

	for (int i = 0; i < labyrinth_size_z; i++) {
		for (int j = 0; j < labyrinth_size_x; j++) {
			if (labyrinth[i][j] == 1) {			// 1 = wall
				glPushMatrix();
				glTranslatef(x, 0.0f, z);
				draw_part_of_wall();
				glPopMatrix();
			}
			else if (labyrinth[i][j] == 2) {	// 2 = collectables (coins)
				glPushMatrix();
				glTranslatef(x, coins.get_coin(i, j)->get_pos_y(), z);
				draw_collectables();
				glPopMatrix();
			}
			else if (labyrinth[i][j] == 3) {	// 3 = object 1 (basic forms)
				glPushMatrix();
				glTranslatef(x, -0.5f, z);
				draw_object_1();
				glPopMatrix();
			}
			else if (labyrinth[i][j] == 4) {	// 4 = object 2 (basic forms)
				glPushMatrix();
				glTranslatef(x, -2.5f, z);
				draw_object_2();
				glPopMatrix();
			}
			else if (labyrinth[i][j] == 5) {	// 5 = object 3 (primitives)
				glPushMatrix();
				glTranslatef(x, -2.5f, z);
				draw_object_3_primitives();
				glPopMatrix();
			}
			x += wall_size;
		}
		x = -game_board_x / 2 + wall_size / 2;
		z += wall_size;
	}
}

// ------ draw_game_board --------------------------------------
// takes care of the output of the maze, the text for the coins and controls the camera 
void draw_game_board(void) {
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	cam.handle_camera();

	draw_floor();
	draw_labyrinth(); 
	draw_text_collectables();

	glutSwapBuffers();
}

// ------ mouse_move --------------------------------------
// is called up when the mouse is moving
// to rotate the camera when the mouse is near the edges of the window 
void mouse_move(int mx, int my) {
	if (mx >= glutGet(GLUT_WINDOW_WIDTH) / 2 + glutGet(GLUT_WINDOW_WIDTH) / 4) {
		cam.set_mouse_is_right(true);
	}
	else if (mx <= glutGet(GLUT_WINDOW_WIDTH) / 2 - glutGet(GLUT_WINDOW_WIDTH) / 4) {
		cam.set_mouse_is_left(true);
	}
	else {
		cam.set_mouse_is_right(false);
		cam.set_mouse_is_left(false);
	}
}

// ------ on_mouse_click --------------------------------------
// is called up with a mouse click
// if the camera is near a coin and a left click is made, the coin will be collected (with animation)
void on_mouse_click(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && coins.near_any_coins()) {
		coins.get_near_coin()->set_is_collected(true);
		coins.set_collected(coins.get_collected() + 1);
	}
}

// ------ collectables_collected_animation --------------------------------------
// timer: controls the animation of the coin when it is picked up 
void collectables_collected_animation(int value) {
	coins.animation_collected_coins();
	glutTimerFunc(20, collectables_collected_animation, 0);
	glutPostRedisplay();
}

// ------ rotate_collectables --------------------------------------
// timer: controls the rotation of the coins 
void rotate_collectables(int value) {
	coins.set_angle(coins.get_angle() + 5.0f);
	glutTimerFunc(20, rotate_collectables, 0);
	glutPostRedisplay();
}

// ------ rotate_collectables --------------------------------------
// timer: controls the rotation of the camera
void rotate_cam_with_mouse(int value) {
	if (cam.get_mouse_is_right()) {
		cam.rotate_right();
	}
	else if (cam.get_mouse_is_left()) {
		cam.rotate_left();
	}

	glutTimerFunc(80, rotate_cam_with_mouse, 0);
	glutPostRedisplay();
}

// ------ cam_jump --------------------------------------
// timer: controls the camera jumping 
void cam_jump(int value) {
	if (cam.get_jumping()) {
		if (cam.get_jump_limit_reached()) {
			cam.set_nav_y(cam.get_nav_y() - 0.1f);
		}
		else {
			cam.set_nav_y(cam.get_nav_y() + 0.1f);
		}

		if (cam.get_nav_y() >= jump_limit) {
			cam.set_jump_limit_reached(true);
		}
		else if (cam.get_jump_limit_reached() && cam.get_nav_y() <= default_cam_y) {
			cam.set_nav_y();
			cam.set_jump_limit_reached(false);
			cam.set_jumping(false);
		}
	}

	glutTimerFunc(10, cam_jump, 0);

	glutPostRedisplay();
}

// ------ create_collectables --------------------------------------
// inserts all coins into the used vector 
void create_collectables(void) {
	for (int i = 0; i < labyrinth_size_z; i++) {
		for (int j = 0; j < labyrinth_size_x; j++) {
			if (labyrinth[i][j] == 2) {
				coins.set_max_collectables(coins.get_max_collectables() + 1);
				coins.insert_collectables(i, j);
			}
		}
	}
}

// ------ main --------------------------------------
int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(50, 50); //determines the initial position of the window
	glutInitWindowSize(800, 600); //determines the size of the window
	windowid = glutCreateWindow("Our Second OpenGL Window"); // create and name window

	create_collectables();

	// register callbacks
	glutKeyboardFunc(keyboard);
	glutMouseFunc(on_mouse_click);
	glutPassiveMotionFunc(mouse_move);

	glutReshapeFunc(reshape_func);

	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(draw_game_board);

	// timer functions
	rotate_collectables(0);
	rotate_cam_with_mouse(0);
	cam_jump(0);
	collectables_collected_animation(0);

	glutMainLoop(); // start the main loop of GLUT

	return 0;
}
