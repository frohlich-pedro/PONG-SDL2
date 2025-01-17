#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define PADDLE_WIDTH 45
#define PADDLE_HEIGHT 300
#define BALL_SIZE 45
#define PADDLE_SPEED 1
#define BALL_SPEED 1

using namespace std;

int main(void) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_GameController* controller1 = nullptr;
    SDL_GameController* controller2 = nullptr;
    TTF_Font* font = nullptr;

    auto init = [&window, &renderer, &controller1, &controller2, &font]() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
            throw runtime_error("SDL could not initialize! SDL_Error: "s + SDL_GetError());
        }

        if (TTF_Init() == -1) {
            throw runtime_error("SDL_ttf could not initialize! SDL_ttf Error: "s + TTF_GetError());
        }

        font = TTF_OpenFont("Cantarell-Bold.ttf", 72);
        if (font == nullptr) {
            throw runtime_error("Failed to load font! SDL_ttf Error: "s + TTF_GetError());
        }

        if (SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt") < 0) {
            throw runtime_error("Controller mappings could not be loaded! SDL_Error: "s + SDL_GetError());
        }

        window = SDL_CreateWindow("PONG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        if (window == nullptr) {
            throw runtime_error("Window could not be created! SDL_Error: "s + SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr) {
            throw runtime_error("Renderer could not be created! SDL_Error: "s + SDL_GetError());
        }

        controller1 = SDL_GameControllerOpen(0);
        if (controller1 == nullptr){
            throw runtime_error("Controller 1 could not be opened! SDL_Error: "s + SDL_GetError());
        } else {
            cout << "Controller 1 connected: " << SDL_GameControllerName(controller1) << endl;
        }

        controller2 = SDL_GameControllerOpen(1);
        if (controller2 == nullptr) {
            throw runtime_error("Controller 2 could not be opened! SDL_Error: "s + SDL_GetError());
        } else {
            cout << "Controller 2 connected: " << SDL_GameControllerName(controller2) << endl;
        }
    };

    auto handlePaddleMovement = [](SDL_Rect& paddle, const Uint8* keystate, SDL_Scancode up, SDL_Scancode down) {
        if (keystate[up] && paddle.y > 0) {
            paddle.y -= PADDLE_SPEED;
        }

        if (keystate[down] && paddle.y + PADDLE_HEIGHT < SCREEN_HEIGHT) {
            paddle.y += PADDLE_SPEED;
        }
    };

    auto handleControllerMovement = [](SDL_Rect& paddle, SDL_GameController* controller) {
        int axis = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        cout << "Axis value: " << axis << endl;
    
        if (axis < -5000 && paddle.y > 0) {
            paddle.y -= PADDLE_SPEED;
        }

	else if (axis > 5000 && paddle.y + PADDLE_HEIGHT < SCREEN_HEIGHT) {
            paddle.y += PADDLE_SPEED;
        }
    };

    auto handleBallMovement = [](SDL_Rect& ball, SDL_Rect& paddle1, SDL_Rect& paddle2, int& ballVelX, int& ballVelY, int& score1, int& score2) {
        ball.x += ballVelX;
        ball.y += ballVelY;
    
        if (ball.y <= 0 || ball.y + BALL_SIZE >= SCREEN_HEIGHT) {
            ballVelY = -ballVelY;
	    cout << '\a';
        }

        if (SDL_HasIntersection(&ball, &paddle1) || SDL_HasIntersection(&ball, &paddle2)) {
            ballVelX = -ballVelX;
	    cout << '\a';
        }

        if (ball.x <= 0) {
            score2++;
            ball.x = (SCREEN_WIDTH - BALL_SIZE) / 2;
            ball.y = (SCREEN_HEIGHT - BALL_SIZE) / 2;
	    cout << '\a';
	    SDL_Delay(1000);
        }

        if (ball.x + BALL_SIZE >= SCREEN_WIDTH) {
            score1++;
            ball.x = (SCREEN_WIDTH - BALL_SIZE) / 2;
            ball.y = (SCREEN_HEIGHT - BALL_SIZE) / 2;
	    cout << '\a';
	    SDL_Delay(1000);
        }
    };

    auto renderScore = [&renderer, &font](int score1, int score2) {
	SDL_Color color = {255, 255, 255, 255};
	
	string scoreText1 = to_string(score1);
	SDL_Surface* scoreSurface1 = TTF_RenderText_Solid(font, scoreText1.c_str(), color);
	SDL_Texture* scoreTexture1 = SDL_CreateTextureFromSurface(renderer, scoreSurface1);
	
	SDL_Rect scoreRect1 = {SCREEN_WIDTH / 4 - scoreSurface1->w / 2, 50, scoreSurface1->w, scoreSurface1->h};
	SDL_FreeSurface(scoreSurface1);
	SDL_RenderCopy(renderer, scoreTexture1, nullptr, &scoreRect1);
	SDL_DestroyTexture(scoreTexture1);
	
	string scoreText2 = to_string(score2);
	SDL_Surface* scoreSurface2 = TTF_RenderText_Solid(font, scoreText2.c_str(), color);
	SDL_Texture* scoreTexture2 = SDL_CreateTextureFromSurface(renderer, scoreSurface2);
	
	SDL_Rect scoreRect2 = {3 * SCREEN_WIDTH / 4 - scoreSurface2->w / 2, 50, scoreSurface2->w, scoreSurface2->h};
	SDL_FreeSurface(scoreSurface2);
	SDL_RenderCopy(renderer, scoreTexture2, nullptr, &scoreRect2);
	SDL_DestroyTexture(scoreTexture2);
    };
    
    auto gameLoop = [&renderer, &controller1, &controller2, &handlePaddleMovement, &handleControllerMovement, &handleBallMovement, &renderScore]() {
        bool quit = false;
        SDL_Event e;
	SDL_Rect paddle1 = {20, (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2, PADDLE_WIDTH, PADDLE_HEIGHT};
	SDL_Rect paddle2 = {SCREEN_WIDTH - PADDLE_WIDTH - 20, (SCREEN_HEIGHT - PADDLE_HEIGHT) / 2, PADDLE_WIDTH, PADDLE_HEIGHT};
	
        SDL_Rect ball = {(SCREEN_WIDTH - BALL_SIZE) / 2, (SCREEN_HEIGHT - BALL_SIZE) / 2, BALL_SIZE, BALL_SIZE};
        int ballVelX = BALL_SPEED;
	int ballVelY = BALL_SPEED;
	
        int score1 = 0;
	int score2 = 0;
    
        while (!quit) {
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                }
            }

            const Uint8* keystate = SDL_GetKeyboardState(nullptr);
            handlePaddleMovement(paddle1, keystate, SDL_SCANCODE_W, SDL_SCANCODE_S);
            handlePaddleMovement(paddle2, keystate, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN);
	    
            handleControllerMovement(paddle1, controller1);
            handleControllerMovement(paddle2, controller2);

	    handleBallMovement(ball, paddle1, paddle2, ballVelX, ballVelY, score1, score2);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &paddle1);
            SDL_RenderFillRect(renderer, &paddle2);
            SDL_RenderFillRect(renderer, &ball);

            renderScore(score1, score2);

            SDL_RenderPresent(renderer);
            SDL_Delay(1);
        }
    };

    try {
        init();
        gameLoop();
        TTF_CloseFont(font);
        SDL_GameControllerClose(controller1);
        SDL_GameControllerClose(controller2);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
    } catch (runtime_error& e) {
        cerr << e.what() << endl;

        if (font != nullptr) {
            TTF_CloseFont(font);
        }

        if (renderer != nullptr) {
            SDL_DestroyRenderer(renderer);
        }

	if (window != nullptr) {
            SDL_DestroyWindow(window);
        }

        TTF_Quit();
        SDL_Quit();

        return -1;
    }

    return 0;
}
