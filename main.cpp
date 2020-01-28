#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <chrono>
#include <unistd.h>
#include <string>
#include <cmath>

const int xcells = 10 + 2;
const int ycells = 20 + 1;
const int L = 40; // 40 is the norm
const int windowWidth = xcells*L*1.5;
const int windowHeight = ycells*L;
const int LinesPerLevel = 10; // 10 new lines = next level
int score = 0, lines = 0, level = 0;

struct StaticGridCell{
    int c1, c2, c3;
};

struct MovingGridCell{
    int c1, c2, c3;
    int x, y;
};

void sidewaysAct(struct StaticGridCell *StaticBlocks[][ycells], struct MovingGridCell *FallingPiece[], const char side[])
{
    int var;
    if (side == "left")
        var = -1;
    else if (side == "right")
        var = 1;
    else
    {
        printf("ERROR AT FUNCTION 'sidewaysAct': no side to go known as %s.\nOptions are 'left' or 'right'", side);
        var = 0;
    }

    int go = 1; // can it go this way?
    for (int n=0; n<4; n++)
        if (FallingPiece[n]->y>=0)
            if (StaticBlocks[FallingPiece[n]->x+var][FallingPiece[n]->y]->c1 != 0)
                go = 0;
    if (go == 1) // if yes, then go.
        for (int n=0; n<4; n++)
        {
            FallingPiece[n]->x = FallingPiece[n]->x+var;
        }
}

void rotationAct(struct StaticGridCell *StaticBlocks[][ycells], struct MovingGridCell *FallingPiece[])
{
    struct MovingGridCell *RotatedFallingPiece[4];
    for (int n=0; n<4; n++) // make/calculate a test rotation.
    {
        RotatedFallingPiece[n] = (struct MovingGridCell *)malloc(sizeof(struct MovingGridCell));
        RotatedFallingPiece[n]->x = FallingPiece[n]->x;
        RotatedFallingPiece[n]->y = FallingPiece[n]->y;
    
        int temp                  =  RotatedFallingPiece[n]->x - FallingPiece[1]->x + FallingPiece[1]->y;
        RotatedFallingPiece[n]->x = -RotatedFallingPiece[n]->y + FallingPiece[1]->x + FallingPiece[1]->y;
        RotatedFallingPiece[n]->y =  temp;
    }

    int rotate=1; // is the test rotation not allowed? That is,
    for (int n=0; n<4; n++)
    {
        if (RotatedFallingPiece[n]->y>=0) // (negative hight cells are always free)
        {
            if (RotatedFallingPiece[n]->x>0 && RotatedFallingPiece[n]->x<xcells-1 && RotatedFallingPiece[n]->y<ycells-1)
            {
                if (StaticBlocks[RotatedFallingPiece[n]->x][RotatedFallingPiece[n]->y]->c1!=0)
                    rotate = 0; // is it in bounds but still obstructed?
            }
            else
                rotate = 0; // or out of bounds?
        }
    }
    if (rotate == 1) // if it is alowed, then aply the rotation.
        for (int n=0; n<4; n++)
        {
            FallingPiece[n]->x = RotatedFallingPiece[n]->x;
            FallingPiece[n]->y = RotatedFallingPiece[n]->y;
        }
    
    for (int n=0; n<4; n++)
        free(RotatedFallingPiece[n]);
}

int clearlinesSystem(struct StaticGridCell *StaticBlocks[][ycells])
{
    int nfull_lines = 0;
    for (int j=1; j<ycells-1; j++) // for ALL horizontal lines,
    {
        int full_line=1;
        for (int i=1; i<xcells-1; i++) // check if it's full.
        {
            if (StaticBlocks[i][j]->c1 == 0)
            {
                full_line = 0;
                break;
            }
        }
        if (full_line == 1) // if it is,
        {
            nfull_lines++;
            for (int i=1; i<xcells-1; i++) // clear that line
            {
                StaticBlocks[i][j]->c1 = 00;
                StaticBlocks[i][j]->c2 = 30;
                StaticBlocks[i][j]->c3 = 30;
            } 
            for (int j_=j-1; j_>=0; j_--) // and move the above blocks one cell down.
            {
                for (int i=1; i<xcells-1; i++)
                {
                    StaticBlocks[i][j_+1]->c1 = StaticBlocks[i][j_]->c1;
                    StaticBlocks[i][j_+1]->c2 = StaticBlocks[i][j_]->c2;
                    StaticBlocks[i][j_+1]->c3 = StaticBlocks[i][j_]->c3;
                }
            }
        }
    }
    return nfull_lines;
}

int scoringSystem(int score, int level, int nfull_lines)
{
    if (nfull_lines==1)
        return score+40*(level+1);
    else if (nfull_lines==2)
        return score+100*(level+1);
    else if (nfull_lines==3)
        return score+300*(level+1);
    else if (nfull_lines==4)
        return score+1200*(level+1);
}

void levelSystem(int level, double &slow, double &fast)
{
    // OBS: slowness is measured in seconds per grid cell
    if (level < 8)
        slow = (48-5*level)/60.0;
    else
        slow = 6/60.0;
    fast = slow/10;
}

int main()
{
    srand(time(NULL));

    sf::RenderWindow window(sf::VideoMode(windowWidth,windowHeight),"TETRIS");

    sf::Music music;
    music.openFromFile("Audio/music.wav");
    music.setVolume(50);
    double firstpitch = pow(40.0/L,1.0/4);
    music.setPitch(firstpitch);
    music.play();
    music.setLoop(true);

    sf::SoundBuffer LevelUpBuffer;
    sf::Sound LevelUpSound;
    LevelUpBuffer.loadFromFile("Audio/level_up.wav");
    LevelUpSound.setBuffer(LevelUpBuffer);

    sf::SoundBuffer ScoreBuffer;
    sf::Sound ScoreSound;
    ScoreBuffer.loadFromFile("Audio/score.wav");
    ScoreSound.setBuffer(ScoreBuffer);

    sf::Font font1;
    font1.loadFromFile("Fonts/8-Bit Madness.ttf");

    sf::Text SideTexts("", font1);
    SideTexts.setFillColor(sf::Color::Yellow);
    SideTexts.setCharacterSize(1.25*L);
    SideTexts.setPosition(xcells*L*1.125,L);

    sf::Text GameOverText("GAME OVER", font1);
    GameOverText.setFillColor(sf::Color::Red);
    GameOverText.setCharacterSize(3*L);
    GameOverText.setOrigin((GameOverText.getGlobalBounds().width /2),(GameOverText.getGlobalBounds().height));
    GameOverText.setPosition(windowWidth/2,windowHeight/2);

    sf::RectangleShape block(sf::Vector2f(L-5,L-5));
    block.setOutlineThickness(2);

    // The different piece types (x1,x2,x3,x4 , y1,y2,y3,y4):
    const int npieces = 7;
    const int type1[8] = {-1, 0,-1, 0   ,   0, 0,-1,-1};   //O
    const int type2[8] = { 1, 0,-1,-2   ,  -1,-1,-1,-1};   //I
    const int type3[8] = {-1, 0, 0, 0   ,  -1,-1,-2, 0};   //T
    const int type4[8] = { 0,-1,-2,-2   ,   0, 0, 0,-1};   //J
    const int type5[8] = { 1, 0,-1, 1   ,   0, 0, 0,-1};   //L
    const int type6[8] = {-2,-1,-1, 0   ,   0, 0,-1,-1};   //S
    const int type7[8] = {-1, 0, 0, 1   ,  -1,-1, 0, 0};   //Z
    const int *type[npieces] = {type1,type2,type3,type4,type5,type6,type7};

    // The different piece collors (c1, c2, c3): (OBS: a cell is considered ocupied <=> cell->c1 != 0)
    const int ncollors = 7;
    const int yellow[3]  = {150, 150, 0};
    const int cian[3]    = {1, 120, 120};
    const int purple[3]  = {120, 0, 150};
    const int blue[3]    = {1, 0, 150};
    const int orange[3]  = {160, 80, 0};
    const int green[3]   = {1, 120, 0};
    const int red[3]     = {150, 0, 0};
    const int *collor[ncollors] = {yellow, cian, purple, blue, orange, green, red};

    struct MovingGridCell *FallingPiece[4];
    for (int n=0; n<4; n++)
        FallingPiece[n] = (struct MovingGridCell *)malloc(sizeof(struct MovingGridCell));

    struct StaticGridCell *StaticBlocks[xcells][ycells];
    for (int i=0; i<xcells; i++)
    {
        for (int j=0; j<ycells; j++)
        {
            StaticBlocks[i][j] = (struct StaticGridCell *)malloc(sizeof(struct StaticGridCell));
            if (j==ycells-1 || i==0 || i==xcells-1)
            { // walls
                StaticBlocks[i][j]->c1 = 100;
                StaticBlocks[i][j]->c2 = 100;
                StaticBlocks[i][j]->c3 = 100;
            }
            else
            { // background
                StaticBlocks[i][j]->c1 = 00;
                StaticBlocks[i][j]->c2 = 30;
                StaticBlocks[i][j]->c3 = 30;
            }
        }
    }

    while(window.isOpen()) // THE ACTUAL START OF THE GAME
    {
        int piece_type, next_piece_type = rand()%npieces;
        for (int piece_count=0;; piece_count++)
        {
            double slowness, slow, fast; 
            levelSystem(level, slow, fast);  // Set falling slowness constants acording to the level
            piece_type = next_piece_type;
            next_piece_type = rand()%npieces; // Set piece type and collor at random
            for (int n=0; n<4; n++) 
            {
                FallingPiece[n]->x = type[piece_type][n] + xcells/2;
                FallingPiece[n]->y = type[piece_type][n+4];
                FallingPiece[n]->c1 = collor[piece_type][0];
                FallingPiece[n]->c2 = collor[piece_type][1];
                FallingPiece[n]->c3 = collor[piece_type][2];
            }
            int next_piece = 0;
            double dt; // Actual time measured between each grid cell fallen
            auto start = std::chrono::steady_clock::now();
            while (next_piece == 0)
            {
                auto end = std::chrono::steady_clock::now();
                dt = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count()*1e-9;
                if (dt > slowness) // Falling mechanic
                {
                    start = std::chrono::steady_clock::now();
                    for (int n=0; n<4; n++)
                        FallingPiece[n]->y ++;
                }
                        
                sf::Event event;
                while(window.pollEvent(event)) // KEY COMMANDS (left, right, up/rotate, down/fall faster)
                {
                    if (event.type == sf::Event::Closed)
                    {
                        window.close();
                        return 0;
                    }

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                        sidewaysAct(StaticBlocks, FallingPiece, "left");

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                        sidewaysAct(StaticBlocks, FallingPiece, "right");

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                        rotationAct(StaticBlocks, FallingPiece);

                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                        slowness = fast;
                    else
                        slowness = slow; 
                }

                for (int n=0; n<4; n++) // "Piece can't fall anymore" check
                    if (FallingPiece[n]->y>=0)
                        if (StaticBlocks[FallingPiece[n]->x][FallingPiece[n]->y]->c1 != 0)
                            next_piece = 1;

                window.clear(sf::Color::Black); // DISPLAY
                for (int i=0; i<xcells; i++)
                {
                    for (int j=0; j<ycells; j++)
                    {
                        block.setPosition(i*L,j*L);
                        block.setFillColor(sf::Color(StaticBlocks[i][j]->c1,StaticBlocks[i][j]->c2,StaticBlocks[i][j]->c3));
                        block.setOutlineColor(sf::Color(StaticBlocks[i][j]->c1*1.5,StaticBlocks[i][j]->c2*1.5,StaticBlocks[i][j]->c3*1.5));
                        window.draw(block);
                    }
                }
                if (next_piece==0)
                {
                    for (int n=0; n<4; n++)
                    {
                        block.setPosition(FallingPiece[n]->x*L,FallingPiece[n]->y*L);
                        block.setFillColor(sf::Color(FallingPiece[n]->c1,FallingPiece[n]->c2,FallingPiece[n]->c3));
                        block.setOutlineColor(sf::Color(FallingPiece[n]->c1*1.5,FallingPiece[n]->c2*1.5,FallingPiece[n]->c3*1.5));
                        window.draw(block);
                    }
                    for (int n=0; n<4; n++)
                    {
                        block.setPosition(type[next_piece_type][n]*L+windowWidth-3*L,type[next_piece_type][n+4]*L+5*L);
                        block.setFillColor(sf::Color(collor[next_piece_type][0],collor[next_piece_type][1],collor[next_piece_type][2]));
                        block.setOutlineColor(sf::Color(collor[next_piece_type][0]*1.5,collor[next_piece_type][1]*1.5,collor[next_piece_type][2]*1.5));
                        window.draw(block);
                    }
                }
                SideTexts.setString("NEXT:\n\n\n\n\n\n\n\nLEVEL:\n"    + std::to_string(level) +
                                                   "\n\n\nLINES:\n"    + std::to_string(lines) +
                                           "\n\n\n\n\n\n\nSCORE:\n"    + std::to_string(score));
                window.draw(SideTexts);
                window.display();

                //usleep(1500);
            }

            for (int n=0; n<4; n++) // Transfering the moving piece into the static blocks matrix
            { // OBS: the piece is fixed one tile space upwards because the "fall check" waits for the piece to intersect a full block
                StaticBlocks[FallingPiece[n]->x][FallingPiece[n]->y-1]->c1 = FallingPiece[n]->c1;
                StaticBlocks[FallingPiece[n]->x][FallingPiece[n]->y-1]->c2 = FallingPiece[n]->c2;
                StaticBlocks[FallingPiece[n]->x][FallingPiece[n]->y-1]->c3 = FallingPiece[n]->c3;
            }

            // Aplying the scoring system and the level calculation
            int nfull_lines = clearlinesSystem(StaticBlocks);
            int new_score = scoringSystem(score, level, nfull_lines);
            if (new_score > score)
            {
                ScoreSound.play();
            }
            score = new_score;
            lines += nfull_lines;
            if (lines/LinesPerLevel > level)
            {
                LevelUpSound.play();
            }  
            level = lines/LinesPerLevel;

            for (int i=1; i<xcells-1; i++) // Game over mechanic
            {
                if (StaticBlocks[i][0]->c1 != 0)
                {
                    window.draw(GameOverText);
                    window.display();
                    usleep(1000000);
                    return 1;
                }
            }
        }
    }
}
// g++ main.cpp -o main -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio