#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;
#ifdef _WIN32
#include <direct.h>  // Windows-only for getting working directory
#define getcwd _getcwd
#else
#include <unistd.h>  // Unix-based systems
#endif

const int SCREEN_WIDTH = 1800;
const int SCREEN_HEIGHT = 1600;
const int GRID_SIZE = 100;
const int GRID_WIDTH = SCREEN_WIDTH / GRID_SIZE;
const int GRID_HEIGHT = SCREEN_HEIGHT / GRID_SIZE;

enum Direction { UP, DOWN, LEFT, RIGHT };

struct Snake {
    std::vector<std::pair<int, int>> body;
    Direction direction;
};

struct Food {
    int x, y;
};

Snake snake;
Food food;
bool gameOver = false;
ALLEGRO_FONT* font = nullptr;
ALLEGRO_DISPLAY* display = nullptr;
ALLEGRO_EVENT_QUEUE* event_queue = nullptr;
ALLEGRO_TIMER* timer = nullptr;

void spawnFood();
void initialize();
void handleInput(ALLEGRO_EVENT event);
void update();
void render();
void cleanup();

void initialize() {
    if (!al_init()) {
        std::cerr << "Failed to initialize Allegro!" << std::endl;
        exit(1);
    }

    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();  // Ensure TTF addon is initialized
    al_install_keyboard();

    display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!display) {
        std::cerr << "Failed to create display!" << std::endl;
        exit(1);
    }

    // Print working directory to debug font location
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << "Current Working Directory: " << cwd << std::endl;
    }
    else {
        std::cerr << "Error getting current directory!" << std::endl;
    }

    // Try using an absolute path or checking multiple locations
    font = al_load_ttf_font("arial.ttf", 24, 0);

    if (!font) {
        std::cerr << "Failed to load 'arial.ttf'! Ensure the file is in the correct directory." << std::endl;
        std::cerr << "Trying alternative font..." << std::endl;

        font = al_load_ttf_font("D:/dnake/snake/x64/Debug/arial.ttf", 24, 0);
        if (!font) {
            std::cerr << "Still failed to load font! Please place 'arial.ttf' in the same directory as the .exe" << std::endl;
            exit(1);
        }
    }

    std::srand(std::time(0));

    snake.body.push_back({ GRID_WIDTH / 8, GRID_HEIGHT / 8 });
    snake.direction = RIGHT;

    spawnFood();

    event_queue = al_create_event_queue();
    if (!event_queue) {
        std::cerr << "Failed to create event queue!" << std::endl;
        exit(1);
    }

    timer = al_create_timer(1.0 / 10);
    if (!timer) {
        std::cerr << "Failed to create timer!" << std::endl;
        exit(1);
    }

    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_start_timer(timer);
}

void spawnFood() {
    food.x = rand() % GRID_WIDTH;
    food.y = rand() % GRID_HEIGHT;

   /* food.x = 15;
    food.y = 10;*/
}

void handleInput(ALLEGRO_EVENT event) {
    if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event.keyboard.keycode) {
        case ALLEGRO_KEY_UP:
            if (snake.direction != DOWN) snake.direction = UP;
            break;
        case ALLEGRO_KEY_DOWN:
            if (snake.direction != UP) snake.direction = DOWN;
            break;
        case ALLEGRO_KEY_LEFT:
            if (snake.direction != RIGHT) snake.direction = LEFT;
            break;
        case ALLEGRO_KEY_RIGHT:
            if (snake.direction != LEFT) snake.direction = RIGHT;
            break;
        }
    }
}

void update() {
    if (gameOver) return;

    auto head = snake.body[0];
    switch (snake.direction) {
    case UP:    head.second--; break;
    case DOWN:  head.second++; break;
    case LEFT:  head.first--;  break;
    case RIGHT: head.first++;  break;

    /*case UP:    head.second-=2; break;
    case DOWN:  head.second+=2; break;
    case LEFT:  head.first-=2;  break;
    case RIGHT: head.first+=2;  break;*/
    }

    if (head.first < 0 || head.first >= GRID_WIDTH || head.second < 0 || head.second >= GRID_HEIGHT) {
        gameOver = true;
        return;
    }

    for (size_t i = 1; i < snake.body.size(); i++) {
        if (head == snake.body[i]) {
            gameOver = true;
            return;
        }
    }

    if (head.first == food.x && head.second == food.y) {
        snake.body.push_back(snake.body.back());
        spawnFood();
    }
    else {
        for (size_t i = snake.body.size() - 1; i > 0; i--) {
            snake.body[i] = snake.body[i - 1];
        }
        snake.body[0] = head;
    }
}

void render() {
    al_clear_to_color(al_map_rgb(0, 0, 0));

    for (auto segment : snake.body) {
        al_draw_filled_rectangle(
            segment.first * GRID_SIZE, segment.second * GRID_SIZE,
            (segment.first + 1) * GRID_SIZE, (segment.second + 1) * GRID_SIZE,
            al_map_rgb(0, 255, 0)
        );
    }

    al_draw_filled_rectangle(
        food.x * GRID_SIZE, food.y * GRID_SIZE,
        (food.x + 1) * GRID_SIZE, (food.y + 1) * GRID_SIZE,
        al_map_rgb(255, 0, 0)
    );

    if (gameOver) {
        al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "GAME OVER!");
    }

    al_flip_display();
}

void cleanup() {
    al_destroy_event_queue(event_queue);
    al_destroy_timer(timer);
    al_destroy_font(font);
    al_destroy_display(display);
}

int main() {
    initialize();

    while (true) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        }

        handleInput(event);

        if (event.type == ALLEGRO_EVENT_TIMER) {
            update();
            render();
        }
    }

    cleanup();
    return 0;
}
