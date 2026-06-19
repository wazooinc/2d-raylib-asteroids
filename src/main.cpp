/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "resource_dir.h"	// utility header for SearchAndSetResourceDir
#include <vector>
#include <cmath>
#include <algorithm>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

enum AppScreen {
	MAIN_MENU,
	GAME
};

struct Ship {
	Vector2 pos;
	Vector2 vel;
	float rotation = 0.0f;
};

struct Bullet {
	Vector2 pos;
	Vector2 vel;
	bool active = true;
};

struct Asteroid {
	Vector2 pos;
    Vector2 vel;
	float radius;
	float rotation;
	float rotation_speed;
	bool active = true;
};

struct ExplosionParticle {
	Vector2 pos;
	Vector2 vel;
	float life;
	float max_life;
	float size;
};

AppScreen app_screen;
Texture background_tex;
Texture ship_tex;
Texture asteroid_tex;

Sound hit_sound;
Sound shoot_sound;

Font game_font;

int score = 0;
Ship ship;

std::vector<Bullet> bullets;
std::vector<Asteroid> asteroids;
std::vector<ExplosionParticle> explosions;
std::vector<ExplosionParticle> thrust_particles;

bool game_over = false;
bool should_exit = false;

float DegToRad(float deg) {
	return deg * PI / 180.0f;
}

Vector2 ForwardVector(float rotation_degrees) {
	float r = DegToRad(rotation_degrees - 90.0f);
	return { cosf(r), sinf(r) };
}

void WrapPosition(Vector2& p) {
	if (p.x < 0.0f) p.x = (float)SCREEN_WIDTH;
	if (p.x > (float)SCREEN_WIDTH) p.x = 0.0f;
	if (p.y < 0.0f) p.y = (float)SCREEN_HEIGHT;
	if (p.y > (float)SCREEN_HEIGHT) p.y = 0.0f;

}

Asteroid SpawnAsteroid() {
	Asteroid a;

	a.pos = { (float)GetRandomValue(0, SCREEN_WIDTH),
			  (float)GetRandomValue(0, SCREEN_HEIGHT) };

	a.vel = { (float)GetRandomValue(-80, 80), (float)GetRandomValue(-80,80) };

	a.radius = (float)GetRandomValue(35, 70);
	a.rotation = 0;
	a.rotation_speed = (float)GetRandomValue(-90, 90);

	return a;
}

void PlayHitSound() {
	float pitch = (float)GetRandomValue(50, 90) / 100.0f;
	SetSoundPitch(hit_sound, pitch);
	PlaySound(hit_sound);
}

void PlayShootSound() {
	float pitch = (float)GetRandomValue(90, 150) / 100.0f;
	SetSoundPitch(shoot_sound, pitch);
	PlaySound(shoot_sound);
}

void DrawBackground() {
	Rectangle src = {
		0, 0,
		(float)background_tex.width,
		(float)background_tex.height
	};
	
	Rectangle dst = {0, 0, (float)SCREEN_WIDTH, (float) SCREEN_HEIGHT};

	DrawTexturePro(background_tex, src, dst, {0,0}, 0, WHITE );
}

void DrawShip() {
	Rectangle src = {0, 0, (float)ship_tex.width, (float)ship_tex.height };
	Rectangle dst = { ship.pos.x, ship.pos.y, 32, 32 };
	Vector2 origin = { 16, 16 };

	DrawTexturePro(ship_tex, src, dst, origin, ship.rotation, WHITE);
}

void DrawBullets() {
	for (auto& b: bullets ){
		DrawCircleV(b.pos, 4, WHITE);
	}
}

void DrawAsteroids() {
	for (auto& a: asteroids) {
		Rectangle src = {0,0 , (float)asteroid_tex.width, (float)asteroid_tex.height };
		Rectangle dst = {a.pos.x, a.pos.y, a.radius * 2, a.radius * 2};
		Vector2 origin = {a.radius, a.radius};

		DrawTexturePro(asteroid_tex, src, dst, origin, a.rotation, WHITE);
	}

}

void DrawThrust() {
	for (auto& p : thrust_particles) {
		float alpha = p.life / p.max_life;

		Color c = {
			255,
			180,
			40,
			(unsigned char)(255 * alpha)
		};

		DrawCircleV(p.pos, p.size * alpha, c);
	}

}

void DrawExplosions() {
	for (auto& p : explosions)
	{
		float alpha = p.life / p.max_life;

		Color c = {
			131,
			96,
			73,
			(unsigned char)(255 * alpha)
		};

		DrawCircleV(
			p.pos,
			p.size * alpha,
			c);
	}

}

void DrawCenteredText(Font font, const char* text, Rectangle r, float fontSize, Color color) {
	Vector2 size = MeasureTextEx(font, text, fontSize, 1);

	DrawTextEx(
		font,
		text,
		{
			r.x + r.width / 2.0f - size.x / 2.0f,
			r.y + r.height / 2.0f - size.y / 2.0f - size.y * 0.08f
		},
		fontSize,
		1,
		color
	);
}

bool Button(Font font, const char* text, Rectangle r) {
	Vector2 mouse = GetMousePosition();
	bool hover = CheckCollisionPointRec(mouse, r);

	DrawRectangleRec(r, hover ? Color{ 70, 70, 95, 255 } : Color{ 45, 45, 65, 255 });
	DrawRectangleLinesEx(r, 3, hover ? GOLD : LIGHTGRAY);

	DrawCenteredText(font, text, r, 28, WHITE);

	return hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void DrawMainMenu(Font font) {
	DrawCenteredText(font, "ASTEROID", Rectangle {SCREEN_WIDTH / 2, 120, 10, 10}, 56, YELLOW );

	Rectangle playButton{
		SCREEN_WIDTH / 2.0f - 110,
		280,
		220,
		64
	};

	Rectangle quitButton{
		SCREEN_WIDTH / 2.0f - 110,
		370,
		220,
		64
	};

	if (Button(font, "PLAY", playButton)) {
		score = 0;
		game_over = false;
		app_screen = GAME;
	}

	if (Button(font, "QUIT", quitButton)) {
		should_exit = true;
	}


}

void GameLoad() {

	InitAudioDevice();
	
	app_screen = MAIN_MENU;
	
	background_tex = LoadTexture("background.png");
	ship_tex = LoadTexture("player.png");
	asteroid_tex = LoadTexture("meteor.png");

	hit_sound = LoadSound("hit.wav");
	shoot_sound = LoadSound("shoot.wav");

	game_font = LoadFontEx("04b03.ttf", 32, nullptr, 0);
	SetTextureFilter(game_font.texture, TEXTURE_FILTER_POINT);

	ship.pos = { (float)SCREEN_WIDTH / 2.0f, (float)SCREEN_HEIGHT / 2.0f };
	ship.vel = { 0, 0 };

	bullets.clear();
	asteroids.clear();
	explosions.clear();
	thrust_particles.clear();

	for (int i = 0; i < 8; i++) {
		asteroids.push_back(SpawnAsteroid());
	}

}

void GameUpdate() {
	float dt = GetFrameTime();

	switch(app_screen){
		case MAIN_MENU:
		break;
		case GAME:
			if (!game_over) {
				if (IsKeyDown(KEY_LEFT)) ship.rotation -= 240.0f * dt;
				if (IsKeyDown(KEY_RIGHT)) ship.rotation += 240.0f * dt;

				if (IsKeyDown(KEY_UP)) {
					Vector2 f = ForwardVector(ship.rotation);

					ship.vel.x += f.x * 300.0f * dt;
					ship.vel.y += f.y * 300.0f * dt;
	
					Vector2 back = { -f.x, -f.y };

					for (int i = 0; i < 3; i++) {
						ExplosionParticle p;

						p.pos = {
							ship.pos.x + back.x * 28.0f + (float)GetRandomValue(-4, 4),
							ship.pos.y + back.y * 28.0f + (float)GetRandomValue(-4, 4)
						};

						p.vel = {
							back.x * (float)GetRandomValue(120, 220) + (float)GetRandomValue(-40, 40),
							back.y * (float)GetRandomValue(120, 220) + (float)GetRandomValue(-40, 40)
						};

						p.life = 0.35f;
						p.max_life = p.life;
						p.size = (float)GetRandomValue(2, 5);

						thrust_particles.push_back(p);
					}
					
				}

				ship.vel.x *= 0.99f;
				ship.vel.y *= 0.99f;

				ship.pos.x += ship.vel.x * dt;
				ship.pos.y += ship.vel.y * dt;
				WrapPosition(ship.pos);

				if (IsKeyPressed(KEY_SPACE)) {
					Vector2 f = ForwardVector(ship.rotation);
					Bullet b;
					b.pos = { ship.pos.x + f.x * 28, ship.pos.y + f.y * 28 };
					b.vel = { f.x * 650.0f + ship.vel.x, f.y * 650.0f + ship.vel.y};
					bullets.push_back(b);
			
					PlayShootSound();
				}

				for (auto& b : bullets) {
					b.pos.x += b.vel.x * dt;
					b.pos.y += b.vel.y * dt;
			
					if (b.pos.x < 0 || b.pos.x > SCREEN_WIDTH ||
						b.pos.y < 0 || b.pos.y > SCREEN_HEIGHT) {
					   b.active = false;
					}
				}

				for (auto& a : asteroids ) {
					a.pos.x += a.vel.x * dt;
					a.pos.y += a.vel.y * dt;
					a.rotation += a.rotation_speed * dt;
					WrapPosition(a.pos);
				}

				for (auto& b: bullets) {
					if (!b.active) continue;

					for (auto& a: asteroids) {
						if (!a.active) continue;

						if (CheckCollisionCircles(b.pos, 4, a.pos, a.radius)) {
							b.active = false;
							a.active = false;
							score += 100;
	
							PlayHitSound();

							for (int i = 0; i < 25; i++) {
								float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
								float speed = (float)GetRandomValue(50, 250);
								ExplosionParticle p;
								p.pos = a.pos;
								p.vel = { cosf(angle) * speed, sinf(angle) * speed };
								p.life = 0.6f;
								p.max_life = p.life;
								p.size = (float)GetRandomValue(2, 6);
								explosions.push_back(p);

							}

							break;
						}
					}

				}

				for (auto& p : explosions)
				{
					p.life -= dt;

					p.pos.x += p.vel.x * dt;
					p.pos.y += p.vel.y * dt;
				}

				for (auto& p : thrust_particles) {
					p.life -= dt;

					p.pos.x += p.vel.x * dt;
					p.pos.y += p.vel.y * dt;

					p.vel.x *= 0.96f;
					p.vel.y *= 0.96f;
				}

				for (auto& a : asteroids) {
					if (!a.active) continue;

					if (CheckCollisionCircles(ship.pos, 22, a.pos, a.radius)) {
						game_over = true;
					}
				}


				bullets.erase(
					std::remove_if(bullets.begin(), bullets.end(),
						[](const Bullet& b) { return !b.active; }),
						bullets.end()
				);
			
				asteroids.erase(
					std::remove_if(asteroids.begin(), asteroids.end(),
						[](const Asteroid& a) { return !a.active; }),
					asteroids.end()
				);

				explosions.erase(
					std::remove_if(
						explosions.begin(),
						explosions.end(),
						[](const ExplosionParticle& p)
						{
							return p.life <= 0;
						}),
					explosions.end());

				thrust_particles.erase(
					std::remove_if(
						thrust_particles.begin(),
						thrust_particles.end(),
						[](const ExplosionParticle& p) {
							return p.life <= 0;
						}
					),
					thrust_particles.end()
				);

			}

			if (asteroids.empty()) game_over = true;

			if (game_over && IsKeyPressed(KEY_ENTER)) {
				ship.pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
				ship.vel = { 0, 0};
				ship.rotation = 0;
				bullets.clear();
				asteroids.clear();
				explosions.clear();
				thrust_particles.clear();

				score = 0;
				
				for (int i = 0; i < GetRandomValue(8, 12); i++) {
					asteroids.push_back(SpawnAsteroid());
				} 
				game_over = false;
			}
			
		break;
	}
}

void GameDraw() {

	switch (app_screen) {
	case MAIN_MENU:
			DrawMainMenu(game_font);
		break;
	case GAME:
			DrawBackground();

			DrawShip();
			DrawThrust();
			DrawBullets();
			DrawAsteroids();
			DrawExplosions();

			DrawTextEx(game_font, TextFormat("SCORE: %d", score), {20, 20}, 32, 0, YELLOW);
			
			if (game_over) {
				const char* title = "GAME OVER";
				Vector2 title_size = MeasureTextEx(game_font, title, 72, 0);
				DrawTextEx(game_font, title, 
					{ SCREEN_WIDTH / 2.0f - title_size.x / 2,
					  SCREEN_HEIGHT / 2.0f - 60
					},
					72, 0, RED);

				const char* msg = "PRESS ENTER TO RESTART";
				Vector2 msg_size = MeasureTextEx(game_font, msg, 24, 0);
				DrawTextEx(game_font, msg,
					{ SCREEN_WIDTH / 2.0f - msg_size.x / 2,
					  SCREEN_HEIGHT / 2.0f + 10
					},
					24, 0, YELLOW);
				
			}

		break;
	}
}

void GameUnload() {

	UnloadTexture(background_tex);
	UnloadTexture(ship_tex);
	UnloadTexture(asteroid_tex);

	UnloadFont(game_font);

	UnloadSound(hit_sound);
	UnloadSound(shoot_sound);

	CloseAudioDevice();
}

int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Raylib");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	GameLoad();

	// game loop
	while (!WindowShouldClose() && !should_exit)		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{

		GameUpdate();

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		GameDraw();


		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	GameUnload();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}



/*
int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(800, 600, "Hello Raylib");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	Texture wabbit = LoadTexture("wabbit_alpha.png");

	// game loop
	while (!WindowShouldClose())		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		// draw some text using the default font
		DrawText("Hello Raylib", 200, 200, 20, WHITE);

		// draw our texture to the screen
		DrawTexture(wabbit, 400, 200, WHITE);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(wabbit);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
*/