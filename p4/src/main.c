#include "lcd/lcd.h"
#include <string.h>
#include "utils.h"
#include "img.h"

// seed
#define SEED 23333
// general positioning
#define GROUND_HEIGHT 20
#define OBJECT_HEIGHT 15
#define DINO_XPOS 10
// jump related
#define GRAVITY 10
#define JUMP_VELOCITY 200 // v = sqrt(2 * gravity * jump_height)
#define FALLING_MULTIPLIER 1.8
// spawning obstacle
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 80
#define BASE_DISTANCE 160
#define MAX_RANDOM_DISTANCE 80
// speed
#define BASE_SPEED 1.5
#define SPEEDUP 0.2
#define HARD_MULTIPLIER 1.5
// dino hitbox
#define HB_POS_X 5
#define HB_POS_Y 5
#define HB_WIDTH 10
#define HB_HEIGHT 10

typedef struct
{
    int gamestate;
    // 0: start menu
    // 1: setting
    // 2: game
    // 3: gameover
    int score;
    int highscore; // use score for speedup
    int difficulty;
    int frame; // used for animation for sprite
    float speed;
    float ground1;
    float ground2;
    float life;
    int legend; //
} Settings;

typedef struct
{
    float pos;
    float speed; // y speed
    int is_grounded;
    int is_crouched;
    int fly_mode;
} Dino;

typedef struct
{
    int type;
    // 0: tall cactus
    // 1: 1 small cactus
    // 2: 2 small cactus
    // 3: 3 small cactus
    // 4: 4 tall cactus
    // 5: high pterosaur
    // 6: middle pterosaur
    // 7: low pterosaur
    float pos_x;
    int pos_y;
    int height;
    int width;
} Obstacle;

void draw(void);
void Inp_init(void);
void Adc_init(void);
void IO_init(void);
void clear(void);
void init(void);
void init_settings(Settings *settings);
void init_dino(Dino *dino);
void init_obstacles(Obstacle *o1, Obstacle *o2);
void draw_start_menu(void);
void draw_settings_menu(void);
void draw_game_over(void);
int start_menu(Settings *settings, Dino *dino, Obstacle *o1, Obstacle *o2);
int settings_menu(Settings *settings);
int game_over(Settings *settings, Dino *dino, Obstacle *o1, Obstacle *o2);
int game(Settings *settings, Dino *dino, Obstacle *o1, Obstacle *o2);
void draw_score(Settings *settings);
int collision_obstacle(Obstacle *o, Dino *dino);
void clear_obstacle(Obstacle *o, int speed);
void draw_obstacle(Obstacle *o, int frame);
void spawn_obstacle(Obstacle *spawn, Obstacle *other, float difficulty);
void LCD_ShowPic(u16 x1, u16 y1, u16 x2, u16 y2, u8 *img);

int main(void)
{
    IO_init(); // init OLED
    // YOUR CODE HERE

    // init srand
    srand(SEED);
    Settings settings;
    settings.difficulty = 0;
    settings.frame = 0;
    settings.gamestate = 0;
    settings.highscore = 0;
    settings.ground1 = 0;
    settings.ground2 = 160;

    Dino dino;

    // max 2 obstacles on screen
    Obstacle o1;
    Obstacle o2;

    // Init();
    init();

    // screen is updated at the end of each function
    while (!Get_BOOT0())
    {
        switch (settings.gamestate)
        {
        case 0: // start menu;
            settings.gamestate = start_menu(&settings, &dino, &o1, &o2);
            break;

        case 1: // settings menu;
            settings.gamestate = settings_menu(&settings);
            break;

        case 2:
            settings.gamestate = game(&settings, &dino, &o1, &o2);
            break;

        case 3:
            settings.gamestate = game_over(&settings, &dino, &o1, &o2);

            break;
        }
        delay_1ms(1);
    }
}

void Inp_init(void)
{
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
}

void Adc_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);
    RCU_CFG0 |= (0b10 << 14) | (1 << 28);
    rcu_periph_clock_enable(RCU_ADC0);
    ADC_CTL1(ADC0) |= ADC_CTL1_ADCON;
}

void IO_init(void)
{
    Inp_init(); // inport init
    Adc_init(); // A/D init
    Lcd_Init(); // LCD init
}

// clear the screen to black
void clear(void)
{
    LCD_Clear(BLACK);
}

// draw main menu
void init(void)
{
    clear();
    draw_start_menu();
    return;
}

// reset score and speed for start/restart
void init_settings(Settings *settings)
{
    settings->frame = 0;
    settings->score = 0;
    settings->ground1 = 0;
    settings->ground2 = 160;
    settings->speed = BASE_SPEED;
    settings->legend = 0;
    if (settings->difficulty != 1)
        settings->difficulty = 0;
    settings->life = 2 * (1 - settings->difficulty) + 1;
    return;
}

// reset dino state
void init_dino(Dino *dino)
{
    dino->is_crouched = 0;
    dino->is_grounded = 1;
    dino->pos = 0;
    dino->speed = 0;
    dino->fly_mode = 0;
    return;
}

void init_obstacles(Obstacle *o1, Obstacle *o2)
{

    o1->type = 0;
    o1->pos_x = 160;
    o1->pos_y = 0;
    o1->height = 0;
    o1->width = 0;
    o2->type = 0;
    o2->pos_x = 160;
    o2->pos_y = 0;
    o2->height = 0;
    o2->width = 0;

    spawn_obstacle(o1, o2, 1);
    spawn_obstacle(o2, o1, 1);
    return;
}

void draw_start_menu(void)
{
    clear();
    LCD_ShowPic(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20 + 30), DINO_XPOS + 20, SCREEN_HEIGHT - (OBJECT_HEIGHT + 30), trex3);
    LCD_ShowPic(130, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20 + 30), 150, SCREEN_HEIGHT - (OBJECT_HEIGHT + 30), trex6);
    LCD_ShowString(6, 48, (u8 *)"StartRun", WHITE);
    LCD_ShowString(80, 48, (u8 *)"Settings", WHITE);
    draw();
    return;
}

void draw_settings_menu(void)
{
    clear();
    // LCD_Clear(BLACK);
    LCD_ShowString(45, 16, (u8 *)"Settings", WHITE);
    LCD_ShowString(32, 48, (u8 *)"Easy", YELLOW);
    LCD_ShowString(96, 48, (u8 *)"Hard", RED);
    return;
}

void draw_game_over(void)
{
    clear();
    LCD_ShowString(45, 16, (u8 *)"Game over!", RED);
    LCD_ShowString(32, 48, (u8 *)"Retry", GREEN);
    LCD_ShowString(96, 48, (u8 *)"Menu", BLUE);
    return;
}

int start_menu(Settings *settings, Dino *dino, Obstacle *o1, Obstacle *o2)
{
    if (Get_Button(0))
    { // game
        LCD_ShowString(6, 48, (u8 *)"StartRun", GREEN);
        delay_1ms(200);
        init_settings(settings);
        init_dino(dino);
        init_obstacles(o1, o2);
        clear();
        return 2;
    }
    else if (Get_Button(1))
    { // settings
        LCD_ShowString(80, 48, (u8 *)"Settings", GREEN);
        delay_1ms(200);
        draw_settings_menu();
        return 1;
    }
    return 0; // start menu
}

int settings_menu(Settings *settings)
{
    if (Get_Button(0))
    { // easy
        delay_1ms(200);
        settings->difficulty = 0;
        draw_start_menu();
        return 0;
    }
    else if (Get_Button(1))
    { // hard
        delay_1ms(200);
        settings->difficulty = 1;
        draw_start_menu();
        return 0;
    }
    return 1; // settings menu
}

int game_over(Settings *settings, Dino *dino, Obstacle *o1, Obstacle *o2)
{
    if (Get_Button(0))
    { // retry
        clear();
        // LCD_Clear(GREEN);
        delay_1ms(200);
        init_settings(settings);
        init_dino(dino);
        init_obstacles(o2, o1);
        return 2;
    }
    else if (Get_Button(1))
    { // menu
        delay_1ms(200);
        draw_start_menu();
        return 0;
    }
    return 3; // game over
}

int game(Settings *settings, Dino *dino, Obstacle *o1, Obstacle *o2)
{
    if (collision_obstacle(o1, dino) || collision_obstacle(o2, dino))
    {
        if (settings->legend > 0)
            settings->legend--;
        else
        {
            settings->legend = 15;
            LCD_DrawCircle(45, 12, 14 - 2 * (int)(settings->life), BLACK);
            settings->life--;
        }
    }
    if ((int)settings->life <= 0)
    {
        draw_game_over();
        delay_1ms(500);
        return 3; // game over
    }
    if (settings->legend > 0)
        settings->legend--;

    // check button for jump or crouch
    if (dino->is_grounded)
    {
        dino->is_crouched = 0;
        if (Get_Button(0))
        { // jump
            dino->is_grounded = 0;
            dino->speed = JUMP_VELOCITY;
        }
        else if (Get_Button(1))
            dino->is_crouched = 1;
    }
    else
    { // check pos for landing, or apply gravity
        if (dino->pos < 0)
        { // landing, set speed to 0
            dino->pos = 0;
            dino->speed = 0;
            dino->is_crouched = 0;
            dino->is_grounded = 1;
            dino->fly_mode = 0;
            LCD_Fill(30, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20), 36, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos), BLACK);
        }
        else
        {
            if (Get_Button(1))
            {
                dino->fly_mode = 2;
                dino->speed -= GRAVITY * FALLING_MULTIPLIER; // more falling
            }
            else if (Get_Button(0))
            {
                if (dino->fly_mode == 2)
                    LCD_Fill(30, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20), 36, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos), BLACK);
                dino->fly_mode = 1;
                dino->speed -= GRAVITY / FALLING_MULTIPLIER; // less falling
            }
            else
            {
                if (dino->fly_mode == 2)
                    LCD_Fill(30, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20), 36, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos), BLACK);
                dino->fly_mode = 0;
                dino->speed -= GRAVITY; // normal falling`
            }
        }
    }

    // move everything
    settings->frame++;
    if (settings->frame == 5)
    { // score++ every 6 frame
        settings->frame = 0;
        settings->score++;
    }
    if (settings->score > settings->highscore)
        settings->highscore = settings->score; // change highscore
    if (settings->score % 200 ==199 && settings->life < 5)
        settings->life += 0.25;
    //  LCD_ShowNum(0, 10, (int)settings->life, 3,GREEN);
    for (int i = 1; i <= (int)settings->life; i++)
        LCD_DrawCircle(45, 12, 14 - 2 * i, RED);

    settings->speed = (int)(settings->score / 200) * SPEEDUP; // speedup every 100 score
    if (settings->speed >= 0.6)
        settings->speed = 0.6;
    if (settings->difficulty == 1)
        settings->speed *= HARD_MULTIPLIER; // more speedup for hard
    settings->speed += BASE_SPEED;

    // kill then spawn obstacles
    if (o1->pos_x + o1->width < 0)
        spawn_obstacle(o1, o2, settings->speed);
    if (o2->pos_x + o2->width < 0)
        spawn_obstacle(o2, o1, settings->speed);

    dino->pos += dino->speed * 0.01; // move dino
    o1->pos_x -= settings->speed;    // move obstacle
    o2->pos_x -= settings->speed;
    settings->ground1 -= settings->speed; // move ground
    settings->ground2 -= settings->speed;
    if (settings->ground1 < -160)
        settings->ground1 += 320; // reset ground if out of screen
    if (settings->ground2 < -160)
        settings->ground2 += 320;
    // clear space behind obstacles or dino
    clear_obstacle(o1, (int)settings->speed);
    clear_obstacle(o2, (int)settings->speed);
    LCD_Fill(0, 0, 2, 67, BLACK);
    LCD_Fill(8, 65, 30, 67, BLACK);
    if (dino->speed > 0)
    {
        LCD_Fill(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos), DINO_XPOS + 20, SCREEN_HEIGHT - (OBJECT_HEIGHT + (dino->pos - (int)(dino->speed * 0.15) > -1 ? dino->pos - (int)(dino->speed * 0.15) : -1)), BLACK);
    }
    else if (dino->speed < 0)
    {
        LCD_Fill(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20 - (int)(dino->speed * 0.15)), DINO_XPOS + 20, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20), BLACK);
    }
    // ground
    // LCD_ShowPic((int)settings->ground1, SCREEN_HEIGHT - (GROUND_HEIGHT-6), (int)settings->ground1 + 160, SCREEN_HEIGHT-1, g3);
    // LCD_ShowPic((int)settings->ground2, SCREEN_HEIGHT - (GROUND_HEIGHT-6), (int)settings->ground2 + 160, SCREEN_HEIGHT-1, g4);

    // dino
    if (dino->is_grounded == 1)
    {
        if (dino->is_crouched == 1)
        { // crouched dino
            dino->fly_mode = 2;
            if (settings->score % 6 >= 3)
                LCD_ShowPic(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), DINO_XPOS + 27, SCREEN_HEIGHT - OBJECT_HEIGHT, trex4);
            else
                LCD_ShowPic(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), DINO_XPOS + 27, SCREEN_HEIGHT - OBJECT_HEIGHT, trex5);
        }
        else
        { // normal dino
            if (dino->fly_mode == 2)
                LCD_Fill(30, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20), 36, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos), BLACK);
            dino->fly_mode = 0;
            if (settings->score % 6 >= 3)
                LCD_ShowPic(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), DINO_XPOS + 20, SCREEN_HEIGHT - OBJECT_HEIGHT, trex1);
            else
                LCD_ShowPic(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), DINO_XPOS + 20, SCREEN_HEIGHT - OBJECT_HEIGHT, trex2);
        }
    }
    else
    {
        if (dino->fly_mode == 0)
            LCD_ShowPic(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20), DINO_XPOS + 20, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos), trex3);
        else if (dino->fly_mode == 1)
            LCD_ShowPic(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20), DINO_XPOS + 20, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos), trex6);
        else if (dino->fly_mode == 2)
            LCD_ShowPic(DINO_XPOS, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos + 20), DINO_XPOS + 27, SCREEN_HEIGHT - (OBJECT_HEIGHT + dino->pos), trex5);
    }

    // obstacle
    draw_obstacle(o1, settings->score);
    draw_obstacle(o2, settings->score);
    // score
    draw_score(settings);

    return 2; // end of cycle
}

void draw_score(Settings *settings)
{
    LCD_ShowString(60, 2, (u8 *)"HI", WHITE);
    LCD_ShowNum(75, 2, (int)(settings->highscore / 10), 3, WHITE);
    LCD_ShowString(110, 2, (u8 *)"SC", WHITE);
    LCD_ShowNum(125, 2, (int)(settings->score / 10), 3, WHITE);
    return;
}

int collision_obstacle(Obstacle *o, Dino *dino)
{
    if (DINO_XPOS + HB_POS_X + HB_WIDTH > o->pos_x && DINO_XPOS + HB_POS_X < o->pos_x + o->width)
    {
        if (dino->is_crouched == 1)
        {
            if (dino->pos < o->pos_y + o->height && dino->pos + HB_HEIGHT > o->pos_y)
                return 1;
        }
        else
        {
            if (dino->pos + HB_POS_Y < o->pos_y + o->height && dino->pos + HB_POS_Y + HB_HEIGHT > o->pos_y)
                return 1;
        }
    }
    return 0;
}

void clear_obstacle(Obstacle *o, int speed)
{
    switch (o->type)
    {
    // not pterosaur
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        LCD_Fill((int)o->pos_x + o->width, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + o->width + speed, SCREEN_HEIGHT - (GROUND_HEIGHT - 5), BLACK);
        break;

    // all pterosaur
    case 5:
    case 6:
    case 7:
        LCD_Fill((int)o->pos_x + 20, SCREEN_HEIGHT - (OBJECT_HEIGHT + o->pos_y + 20), (int)o->pos_x + 20 + speed, SCREEN_HEIGHT - (OBJECT_HEIGHT + o->pos_y), BLACK);
    }
}

void draw_obstacle(Obstacle *o, int frame)
{
    if (o->pos_x > 159)
        return;
    if (o->pos_x + o->width < 0)
        return;
    switch (o->type)
    {
    case 0:
        LCD_ShowPic((int)o->pos_x, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 12, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus1);
        break;

    case 1:
        LCD_ShowPic((int)o->pos_x, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 12, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus2);
        break;

    case 2:
        LCD_ShowPic((int)o->pos_x, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 12, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus2);
        LCD_ShowPic((int)o->pos_x + 12, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 24, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus2);
        break;

    case 3:
        LCD_ShowPic((int)o->pos_x, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 12, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus2);
        LCD_ShowPic((int)o->pos_x + 12, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 24, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus2);
        LCD_ShowPic((int)o->pos_x + 24, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 36, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus2);
        break;

    case 4:
        LCD_ShowPic((int)o->pos_x, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 12, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus1);
        LCD_ShowPic((int)o->pos_x + 12, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 24, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus1);
        LCD_ShowPic((int)o->pos_x + 24, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 36, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus2);
        LCD_ShowPic((int)o->pos_x + 36, SCREEN_HEIGHT - (OBJECT_HEIGHT + 20), (int)o->pos_x + 48, SCREEN_HEIGHT - OBJECT_HEIGHT, cactus1);
        break;

    // all pterosaur
    case 5:
    case 6:
    case 7:
        if (frame % 6 >= 3)
            LCD_ShowPic((int)o->pos_x, SCREEN_HEIGHT - (OBJECT_HEIGHT + o->pos_y + 20), (int)o->pos_x + 20, SCREEN_HEIGHT - (OBJECT_HEIGHT + o->pos_y), pter1);
        else
            LCD_ShowPic((int)o->pos_x, SCREEN_HEIGHT - (OBJECT_HEIGHT + o->pos_y + 20), (int)o->pos_x + 20, SCREEN_HEIGHT - (OBJECT_HEIGHT + o->pos_y), pter2);
        break;
    }
}

void spawn_obstacle(Obstacle *spawn, Obstacle *other, float difficulty)
{
    // init values
    spawn->type = rand() % 8;
    // change some value according to random type
    switch (spawn->type)
    {
    // 0: tall cactus
    // 1: 1 small cactus
    // 2: 2 small cactus
    // 3: 3 small cactus
    // 4: 4 tall cactus
    // 5: high pterosaur
    // 6: middle pterosaur
    // 7: low pterosaur
    case 0:
        spawn->width = 12;
        spawn->height = 20;
        spawn->pos_y = 0;
        break;

    case 1:
        spawn->width = 12;
        spawn->height = 15;
        spawn->pos_y = 0;
        break;

    case 2:
        spawn->width = 24;
        spawn->height = 15;
        spawn->pos_y = 0;
        break;

    case 3:
        spawn->width = 36;
        spawn->height = 15;
        spawn->pos_y = 0;
        break;

    case 4:
        spawn->width = 48;
        spawn->height = 20;
        spawn->pos_y = 0;
        break;

    case 5:
        spawn->width = 20;
        spawn->height = 20;
        spawn->pos_y = 24;
        break;

    case 6:
        spawn->width = 20;
        spawn->height = 20;
        spawn->pos_y = 12;
        break;

    case 7:
        spawn->width = 20;
        spawn->height = 20;
        spawn->pos_y = 0;
        break;
    }
    // while near the other, respawn
    // difficulty = (difficulty >= 2 ? 2 : difficulty);
    int distance = (BASE_DISTANCE + (rand() % MAX_RANDOM_DISTANCE - MAX_RANDOM_DISTANCE / 2)) / difficulty;
    spawn->pos_x = other->pos_x + other->width + distance;
    return;
}

void LCD_ShowPic(u16 x1, u16 y1, u16 x2, u16 y2, u8 *img)
{
    x2--;
    y2--;
    if (x2 > 159)
        return;
    LCD_Address_Set(x1, y1, x2, y2);
    int w = x2 - x1 + 1, h = y2 - y1 + 1;
    for (int i = 0; i < w * h * 2; i++)
        LCD_WR_DATA8(img[i]);
}
