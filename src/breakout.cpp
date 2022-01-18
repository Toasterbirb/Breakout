#include <iostream>
#include <birb2d/Logger.hpp>
#include <birb2d/Renderwindow.hpp>
#include <birb2d/Timestep.hpp>
#include <birb2d/Values.hpp>
#include <birb2d/Entity.hpp>
#include <birb2d/Physics.hpp>

enum Side
{
	Left, Right, Top, Bottom, None
};

void GenerateTiles();
void DrawPlayArea();
void MirrorBallVector(Birb::Vector2f* ballVector, Side side);

struct TileCollider
{
	Birb::Rect collisionRect;
	Side side;
};

SDL_Texture* tileSprite;
struct Tile
{
	Birb::Entity entity = Birb::Entity("Tile", Birb::Rect(0, 0, 40, 18), tileSprite);
	TileCollider colliders[4];
	SDL_Color collisionColors[4] = { Birb::Colors::Blue, Birb::Colors::Red, Birb::Colors::Yellow, Birb::Colors::White };
	bool alive = true;

	void GenerateColliders()
	{
		/* Order */
		// 0: Top
		// 1: Bottom
		// 2: Left
		// 3: Right
		
		colliders[0].collisionRect = Birb::Rect(entity.rect.x, entity.rect.y, entity.rect.w, 4);
		colliders[0].side = Side::Top;

		colliders[1].collisionRect = Birb::Rect(entity.rect.x, entity.rect.y + entity.rect.h - 4, entity.rect.w, 4);
		colliders[1].side = Side::Bottom;

		colliders[2].collisionRect = Birb::Rect(entity.rect.x, entity.rect.y + 4, 4, entity.rect.h - 8);
		colliders[2].side = Side::Left;

		colliders[3].collisionRect = Birb::Rect(entity.rect.x + entity.rect.w - 4, entity.rect.y + 4, 4, entity.rect.h - 8);
		colliders[3].side = Side::Right;

	}

	void VisualizeColliders()
	{
		for (int i = 0; i < 4; i++)
			Birb::Render::DrawRect(collisionColors[i], colliders[i].collisionRect);
	}

	Side CheckCollision(Birb::Rect ballCollider, Birb::Vector2f ballMovementVector)
	{
		GenerateColliders();

		/* Go trough all of the colliders */
		for (int i = 0; i < 4; i++)
		{
			if (Birb::Physics::RectCollision(ballCollider, colliders[i].collisionRect))
			{
				/* Check for illegal collisions */
				if (colliders[i].side == Top && ballMovementVector.y < 0) /* Colliding to top, but ball going up */
					return Side::Bottom;

				if (colliders[i].side == Bottom && ballMovementVector.y > 0) /* Colliding to bottom, but ball going down */
					return Side::Top;

				if (colliders[i].side == Left && ballMovementVector.x < 0) /* Colliding to left side, but ball going to left */
					return Side::Right;

				if (colliders[i].side == Right && ballMovementVector.x > 0) /* Colliding to right side, but ball going to right */
					return Side::Left;

				return colliders[i].side;
			}
		}
		return Side::None;
	}
};

Birb::Window window("Breakout", Birb::Vector2int(1280, 720), 240, false);
SDL_Texture* playerSprite = Birb::Resources::LoadTexture("./res/textures/player.png");
Birb::Entity player = Birb::Entity("Player", Birb::Rect((float)window.window_dimensions.x / 2 - 64, 680, 128, 12), playerSprite);
bool GameRunning = true;
int playAreaSize = 480;
int playAreaThickness = 12;
Tile tiles[10][6];

int tileCountX = 10;
int tileCountY = 6;

int main(int argc, char **argv)
{
	Birb::TimeStep timeStep;
	timeStep.Init();

	/* Load textures */
	tileSprite = Birb::Resources::LoadTexture("./res/textures/tile.png");

	/* Gameloop variables */
	Side playerMovementDirection;
	float playerSpeed = 10;

	bool ballMoving = false;
	Birb::Vector2int ballPos = { (int)player.rect.x + (int)(player.rect.w / 2), 660 };
	int ballSize = 8;
	Birb::Vector2f ballMovementVector = { 4, -4 };
	Birb::Rect ballCollider = { ballPos.x - (float)ballSize, ballPos.y - (float)ballSize, (float)ballSize * 2, (float)ballSize * 2 };
	Side lastBallCollisionSide = Side::None;

	GenerateTiles();

	while (GameRunning)
	{
		timeStep.Start();
		while (timeStep.Running())
		{
			/* Handle input */
			while (window.PollEvents())
			{
				window.EventTick(window.event, &GameRunning);

				if (window.event.type == SDL_KEYDOWN)
				{
					switch (window.event.key.keysym.scancode)
					{
						/* Left */
						case (SDL_SCANCODE_LEFT):
							playerMovementDirection = Side::Left;
							break;

						/* Right */
						case (SDL_SCANCODE_RIGHT):
							playerMovementDirection = Side::Right;
							break;

						/* Release the ball */
						case (SDL_SCANCODE_SPACE):
							ballMoving = true;
							break;

						default:
							break;
					}
				}
				else
				{
					playerMovementDirection = Side::None;
				}
			}

			timeStep.Step();
		}

		/* Player movement */
		switch (playerMovementDirection)
		{
			case (Side::Left):
				if (player.rect.x - playerSpeed > window.window_dimensions.x / 2.0 - (playAreaSize / 2.0) + playerSpeed)
					player.rect.x -= playerSpeed;
				break;

			case (Side::Right):
				if (player.rect.x + playerSpeed < window.window_dimensions.x / 2.0 + (playAreaSize / 2.0) - 118 - playerSpeed)
					player.rect.x += playerSpeed;
				break;

			default:
				break;
		}

		/* Move the ball with the player if the ball isn't moving yet */
		if (!ballMoving)
			ballPos.x = (int)player.rect.x + (int)(player.rect.w / 2);
		else
		{
			/* Handle ball collisions to the playing arena and player */
			{
				/* Left side */
				if (ballPos.x < window.window_dimensions.x / 2 - (playAreaSize / 2) + ballSize + playAreaThickness)
				{
					lastBallCollisionSide = Side::None;
					MirrorBallVector(&ballMovementVector, Side::Right);
				}

				/* Right side */
				if (ballPos.x > window.window_dimensions.x / 2 + (playAreaSize / 2) - ballSize)
				{
					lastBallCollisionSide = Side::None;
					MirrorBallVector(&ballMovementVector, Side::Right);
				}

				/* Top */
				if (ballPos.y < playAreaThickness + ballSize)
				{
					MirrorBallVector(&ballMovementVector, Side::Top);
				}

				/* Bottom */
				//if (ballPos.y > 720) // lose health

				/* Paddle */
				ballCollider = { ballPos.x - (float)ballSize, ballPos.y - (float)ballSize, (float)ballSize * 2, (float)ballSize * 2 }; /* Update ball collider */
				if (Birb::Physics::RectCollision(ballCollider, player.rect))
				{
					lastBallCollisionSide = Side::None;
					MirrorBallVector(&ballMovementVector, Side::Bottom);
				}
			}

			/* Tile collision */
			Side collisionSide = Side::None;
			{
				/* Go trough all of the tiles and check for their colliders */
				for (int i = 0; i < tileCountX; i++)
				{
					for (int j = 0; j < tileCountY; j++)
					{
						/* Check if the tile is even alive */
						if (!tiles[i][j].alive) continue;

						collisionSide = tiles[i][j].CheckCollision(ballCollider, ballMovementVector);
						if (collisionSide != Side::None && collisionSide != lastBallCollisionSide)
						{
							MirrorBallVector(&ballMovementVector, collisionSide);
							tiles[i][j].alive = false;
							lastBallCollisionSide = collisionSide;
							goto collisionCheckFinished;
						}
					}
				}
			}
			collisionCheckFinished:


			/* Move the ball */
			ballPos.x += ballMovementVector.x;
			ballPos.y += ballMovementVector.y;
		}

		/* Rendering */
		window.Clear();

		DrawPlayArea();
		Birb::Render::DrawEntity(player);
		Birb::Render::DrawCircle(Birb::Colors::White, ballPos, ballSize);

		/* Render tiles */
		for (int i = 0; i < tileCountX; i++)
		{
			for (int j = 0; j < tileCountY; j++)
			{
				if (tiles[i][j].alive)
				{
					Birb::Render::DrawEntity(tiles[i][j].entity);
					//Birb::Render::DrawRect(Birb::Colors::Green, tiles[i][j].entity.rect);
					//tiles[i][j].VisualizeColliders();
				}
			}
		}

		window.Display();
		timeStep.End();
	}

	return 0;
}

void GenerateTiles()
{
	for (int i = 0; i < tileCountX; i++)
	{
		for (int j = 0; j < tileCountY; j++)
		{
			Tile tile;
			tile.entity.rect.x = i + (tile.entity.rect.w * i) + playAreaSize - 38;
			tile.entity.rect.y = j + (tile.entity.rect.h * j) + 64;
			//tile.GenerateColliders();
			tiles[i][j] = tile;
		}
	}
}

void DrawPlayArea()
{
	Birb::Render::DrawRect(Birb::Colors::White, Birb::Rect(window.window_dimensions.x / 2.0 - (playAreaSize / 2.0), 0, playAreaThickness, 708));
	Birb::Render::DrawRect(Birb::Colors::White, Birb::Rect(window.window_dimensions.x / 2.0 + (playAreaSize / 2.0), 0, playAreaThickness, 708));
	Birb::Render::DrawRect(Birb::Colors::White, Birb::Rect(window.window_dimensions.x / 2.0 - (playAreaSize / 2.0), 720 - playAreaThickness, playAreaSize + playAreaThickness, 12));
	Birb::Render::DrawRect(Birb::Colors::White, Birb::Rect(window.window_dimensions.x / 2.0 - (playAreaSize / 2.0), 0, playAreaSize + playAreaThickness, 12));
}

void MirrorBallVector(Birb::Vector2f* ballVector, Side side)
{
	switch (side)
	{
		case (Top):
			ballVector->y *= -1;
			break;

		case (Bottom):
			ballVector->y *= -1;

		case (Left):
			ballVector->x *= -1;

		case (Right):
			ballVector->x *= -1;

		default:
			break;
	}
}
