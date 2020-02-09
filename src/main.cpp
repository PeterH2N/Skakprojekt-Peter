#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <ctime>
#include <chrono>


using namespace std;
//setting up the redering window for the GUI
static sf::ContextSettings settings;
static unsigned int screenWidth = 1000;
static unsigned int screenHeight = 1000;
static sf::Font font;
static sf::Font chessPieceFont;

static sf::RenderWindow chessBoard(sf::VideoMode(screenWidth,screenHeight), "Skak", sf::Style::Default, settings);


static sf::Vector2u size = chessBoard.getSize();

//variables for statistics on the chess engine
static int posCounter = 0;
static int fullPosCounter = 0;
static double fullDuration = 0;


static char abc[8] = {'A','B','C','D','E','F','G','H'};

//in following order: King, Queen, Rook, Bishop, Knight, Pawn
static std::string chessPieces[13] = {"k","q","r","b","n","p","l","w","t","v","m","o"," "};

//used for navigating through menus
static int navigation;
static int gameMode;

//used for opening moves
static unsigned int turns = 0;
//static unsigned int opening;

//returns the minimum of 2 values - used for trying to fix alpha beta pruning
int min(int value1, int value2){
    if (value1 <= value2){
        return value1;
    }
    else{
        return value2;
    }
}
//returns the maximum of 2 values - used for trying to fix alpha beta pruning
int max(int value1, int value2){
    if(value1 >= value2){
        return value1;
    }
    else{
        return value2;
    }
}
//returns the absolute value
int abs(int value){
    if (value < 0){
        return -value;
    }
    else{
        return value;
    }
}

//sizes for the GUI
static unsigned int width = min(screenWidth,screenHeight);
static unsigned int height = min(screenWidth,screenHeight);
static unsigned int screenSize = min(screenWidth,screenHeight);
static unsigned int charSize = screenSize/25;
static int chessPieceSize = screenSize/11;

class Move{
public:
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2= 0;

};

class Piece{
public:
    int x = 0;
    int y = 0;
};

class Board{
public:
    int board[8][8];

    void move(Move M){
        board[M.y2][M.x2] = board[M.y1][M.x1];
        board[M.y1][M.x1] = 0;


    }
    void moveBack(Move M, Board B1){
        board[M.y1][M.x1] = board[M.y2][M.x2];
        board[M.y2][M.x2] = B1.board[M.y2][M.x2];
    }

    //evaluates the board
    int getValue(){
        int boardValue = 0;
        for (int i = 0; i < 8; i++){
            for (int j = 0; j < 8; j++){
                boardValue += board[i][j];
            }
        }
        return boardValue;
    }

    //returns all the pieces of the color given
    vector<Piece> getPieces(int color){
        vector<Piece> pieces;
        Piece valid;
        for (valid.y = 0; valid.y < 8; valid.y++){
            for (valid.x = 0; valid.x < 8; valid.x++){
                if (board[valid.y][valid.x]*color > 0){
                    pieces.push_back(valid);
                }
            }
        }
        return pieces;
    }

};

/*Piece values
int whitePawn = 1;
int whiteKnight = 2;
int whiteBishop = 3;
int whiteRook = 5;
int whiteQueen = 9;
int whiteKing = 999;
int blackPawn = -1;
int blackKnight = -2;
int blackBishop = -3;
int blackRook = -5;
int blackQueen = -9;
int blackKing = -999;
*/

static Board board;                                           //the board, an 8*8 array of ints
static Move lastMove;                                         //used for showing the last move done
static Move empty = {0,0,0,0};                                //used to reset a specific move
static int currentPlayer = 1;                                 //the current player, starting as white


static sf::Vertex arrow[2];                                   //line to show last move


//unused opening moves
/*
static vector<Move> nimzoIndian = {{6,0,5,2},{4,1,4,2},{5,0,1,4}};
static vector<Move> frenchDefense = {{4,1,4,2},{3,1,3,3},{2,1,2,3},{1,0,2,2}};


static vector<vector<Move>> openingMoves = {nimzoIndian,frenchDefense};
*/


//converts a pawn to a queen if at opposite row
void pawnToQueen(Board &B,int color){
    if (color == -1){
        for (int i = 0; i < 8; i++){
            if (B.board[7][i] == -1){
                B.board[7][i] = -9;

            }
        }
    }

    if (color == 1){
        for (int j = 0; j < 8; j++){
            if (B.board[0][j] == 1){
                B.board[0][j] = 9;

            }
        }
    }

}

//returns the middle position of a square on the board, used for the indicators
Piece getMiddlePos(unsigned int size, unsigned int x, unsigned int y){
    Piece P;
    P.x = width/20+width/10-size+width/10*x;
    P.y =width/20+width/10-size+width/10*y;
    return P;
}

//returns the middle position of a square for a chess piece
int getPieceX(int x){
    return width/20+width/10+width/10*x;
}

int getPieceY(int y){
    return width/20+width/10+width/10*y;
}

//returns the correct character to print when a chesspiece value is passed to it
string getChessPiece(int value){
    switch (value){
    case -999:
        return chessPieces[6];
    case -9:
        return chessPieces[7];
    case -5:
        return chessPieces[8];
    case -3:
        return chessPieces[9];
    case -2:
        return chessPieces[10];
    case -1:
        return chessPieces[11];
    case 999:
        return chessPieces[0];
    case 9:
        return chessPieces[1];
    case 5:
        return chessPieces[2];
    case 3:
        return chessPieces[3];
    case 2:
        return chessPieces[4];
    case 1:
        return chessPieces[5];
    case 0:
        return chessPieces[12];
    }
}

//draws chessboard with all pieces
void drawChessBoard(Board B){
    font.loadFromFile("../arial.ttf");
    chessPieceFont.loadFromFile("../CASEFONT.ttf");
    sf::Text boardText;
    sf::Text chessPiece;
    boardText.setCharacterSize(charSize);
    chessPiece.setCharacterSize(chessPieceSize);
    chessPiece.setFont(chessPieceFont);
    boardText.setFont(font);

    chessBoard.clear(sf::Color(50,50,50));
    //draw everything here//

        //the white tile
        sf::RectangleShape whiteTile(sf::Vector2f(width/10, height/10));
        whiteTile.setFillColor(sf::Color(200,200,200));

        //the black tile
        sf::RectangleShape blackTile(sf::Vector2f(width/10, height/10));
        blackTile.setFillColor(sf::Color(100,100,100));

        //drawing the board using those two shapes
        for(unsigned int j = 0; j < 8; j++){
            for (unsigned int i = 0; i < 8; i++){
                if ( j % 2 == 0){
                if ( i % 2 == 0){
                    whiteTile.setPosition(width/10+(width/10)*i,height/10+height/10*j);
                    chessBoard.draw(whiteTile);
                }
                else{
                    blackTile.setPosition(width/10+(width/10)*i,height/10+height/10*j);
                    chessBoard.draw(blackTile);
                }
                }
                else{
                    if ( i % 2 == 0){
                        blackTile.setPosition(width/10+(width/10)*i,height/10+height/10*j);
                        chessBoard.draw(blackTile);
                    }
                    else{
                        whiteTile.setPosition(width/10+(width/10)*i,height/10+height/10*j);
                        chessBoard.draw(whiteTile);
                }
            }
            }
        }

        //draws letters and numbers
        std::string letters[8] = {"A","B","C","D","E","F","G","H"};
        std::string numbers[8] = {"8","7","6","5","4","3","2","1"};
        //puts both rows of letters in place
        for (unsigned int i = 0; i < 8; i++){
            boardText.setString(letters[i]);
            sf::FloatRect textRect = boardText.getLocalBounds();
            boardText.setOrigin(textRect.left + textRect.width/2.0f,
                           textRect.top  + textRect.height/2.0f);
            boardText.setPosition(width/20+width/10+width/10*i,height/20);
            chessBoard.draw(boardText);
            boardText.setPosition(width/20+width/10+width/10*i,(height/20)*19);
            chessBoard.draw(boardText);
        }
        //puts both columns of numbers in place
        for (unsigned int i = 0; i < 8; i++){
            boardText.setString(numbers[i]);
            sf::FloatRect textRect = boardText.getLocalBounds();
            boardText.setOrigin(textRect.left + textRect.width/2.0f,
                           textRect.top  + textRect.height/2.0f);
            boardText.setPosition(width/20,height/20+height/10*(i+1));
            chessBoard.draw(boardText);
            boardText.setPosition((width/20)*19,height/20+height/10*(i+1));
            chessBoard.draw(boardText);
        }

        //draws chess pieces
        chessPiece.setFillColor(sf::Color::Black);
        for (unsigned int i = 0; i < 8; i++){
            for (unsigned int j = 0; j < 8; j++){
                chessPiece.setString(getChessPiece(B.board[i][j]));
                sf::FloatRect pieceRect = chessPiece.getLocalBounds();
                chessPiece.setOrigin(pieceRect.left + pieceRect.width/2.0f,
                               pieceRect.top  + pieceRect.height/2.0f);
                chessPiece.setPosition(getPieceX(j),getPieceY(i));

                chessBoard.draw(chessPiece);

            }

        }
        chessBoard.draw(arrow, 2, sf::Lines);

    //draw everything here//
    //chessBoard.display();
}


//draws the start menu
void drawStartMenu(){
    font.loadFromFile("../arial.ttf");
    string menu[3] = {"Start Game","Options","Quit Game"};
    int buttonWidth = width/2;
    int buttonHeight = height/7;
    sf::Text text;
    sf::Text title;
    text.setFont(font);
    title.setFont(font);
    sf::RectangleShape buttons(sf::Vector2f(buttonWidth, buttonHeight));
    buttons.setFillColor(sf::Color(19,72,133));
    chessBoard.clear(sf::Color(50,50,50));

    text.setString("Chess");
    text.setCharacterSize(100);
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f,
                   textRect.top  + textRect.height/2.0f);

    text.setPosition(width/2,height/9);
    chessBoard.draw(text);

    text.setCharacterSize(50);


    //draw buttons and their text
    for(int i = 0; i < 3; i++){
        buttons.setPosition(width/2-buttonWidth/2, height/7*2*(i+1)-buttonHeight/2);
        chessBoard.draw(buttons);
        text.setString(menu[i]);
        //center text
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width/2.0f,
                       textRect.top  + textRect.height/2.0f);

        text.setPosition(width/2, height/7*2*(i+1));

        chessBoard.draw(text);
    }







}

//draws the gamemode picker
void drawGameModes(){
    drawStartMenu();
    font.loadFromFile("../arial.ttf");
    string menu[4] = {"Player vs Player","Player vs Computer","Multiplayer","Computer vs Computer"};

    int windowWidth = width/3;
    int windowHeight = height/5;
    int buttonWidth = width/4;
    int buttonHeight = windowHeight/6;

    sf::Text text;
    sf::RectangleShape window(sf::Vector2f(windowWidth, windowHeight));
    text.setFont(font);
    text.setCharacterSize(20);


    window.setFillColor(sf::Color(0,0,0,200));
    window.setPosition(width/2-windowWidth/2,height/7*2-windowHeight/2);
    chessBoard.draw(window);
    //draw gamemode buttons
    for (int i = 0; i < 4; i++){
    window.setFillColor(sf::Color(19,72,133));
    window.setSize(sf::Vector2f(width/4, windowHeight/6));
    window.setPosition(width/2-buttonWidth/2,height/7*2-windowHeight/2+buttonHeight/2+buttonHeight*i+buttonHeight/3*i);
    chessBoard.draw(window);
    text.setString(menu[i]);
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f,
                   textRect.top  + textRect.height/2.0f);
    text.setPosition(width/2,height/7*2-windowHeight/2+buttonHeight/2+buttonHeight*i+buttonHeight/3*i+buttonHeight/2);
    chessBoard.draw(text);
    }
    chessBoard.display();

}

//draws the pause menu
void drawPauseMenu(){
    drawChessBoard(board);
    font.loadFromFile("../arial.ttf");
    string menu[4] = {"Resume Game","Options","Main Menu","Quit Game"};

    int windowWidth = width/3;
    int windowHeight = height/5;
    int buttonWidth = width/4;
    int buttonHeight = windowHeight/6;

    sf::Text text;
    sf::RectangleShape window(sf::Vector2f(windowWidth, windowHeight));
    text.setFont(font);
    text.setCharacterSize(20);


    window.setFillColor(sf::Color(0,0,0,200));
    window.setPosition(width/2-windowWidth/2,height/2-windowHeight/2);
    chessBoard.draw(window);
    //draw gamemode buttons
    for (int i = 0; i < 4; i++){
    window.setFillColor(sf::Color(19,72,133));
    window.setSize(sf::Vector2f(width/4, windowHeight/6));
    window.setPosition(width/2-buttonWidth/2,height/2-windowHeight/2+buttonHeight/2+buttonHeight*i+buttonHeight/3*i);
    chessBoard.draw(window);
    text.setString(menu[i]);
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f,
                   textRect.top  + textRect.height/2.0f);
    text.setPosition(width/2,height/2-windowHeight/2+buttonHeight/2+buttonHeight*i+buttonHeight/3*i+buttonHeight/2);
    chessBoard.draw(text);
    }
    chessBoard.display();

}

//makes the pause menu functional
int pauseMenu(){
    startOfPauseMenu:
        drawPauseMenu();
    int windowWidth = width/3;
    int windowHeight = height/5;
    int buttonWidth = width/4;
    int buttonHeight = windowHeight/6;

    int mouseX = 0;
    int mouseY = 0;
    sf::Event mouseClick;
    while(chessBoard.waitEvent(mouseClick)){
        if (mouseClick.type == sf::Event::Closed){
            chessBoard.close();
        }
        if (mouseClick.type == sf::Event::KeyPressed)
        {
        if (mouseClick.key.code == sf::Keyboard::P or mouseClick.key.code == sf::Keyboard::Escape)
        {
            return 2;
        }
        }
        if (mouseClick.type == sf::Event::MouseButtonPressed){
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            sf::Vector2i localPosition = sf::Mouse::getPosition(chessBoard);
            mouseX = localPosition.x;
            mouseY = localPosition.y;
            break;

           }
        }
    }
    navigation = -1;
    if (mouseX > (width/2)-(buttonWidth/2) && mouseX < (width/2)-(buttonWidth/2)+buttonWidth && mouseY > height/2-windowHeight/2+buttonHeight/2 && mouseY < height/2-windowHeight/2+buttonHeight/2+buttonHeight){
        return 2;
    }
    else if (mouseX > (width/2)-(buttonWidth/2) && mouseX < (width/2)-(buttonWidth/2)+buttonWidth && mouseY > height/2-windowHeight/2+buttonHeight/2+buttonHeight*1+buttonHeight/3*1 && mouseY < height/2-windowHeight/2+buttonHeight/2+buttonHeight*1+buttonHeight/3*1+buttonHeight){
        goto startOfPauseMenu;
    }
    else if (mouseX > (width/2)-(buttonWidth/2) && mouseX < (width/2)-(buttonWidth/2)+buttonWidth && mouseY > height/2-windowHeight/2+buttonHeight/2+buttonHeight*2+buttonHeight/3*2 && mouseY < height/2-windowHeight/2+buttonHeight/2+buttonHeight*2+buttonHeight/3*2+buttonHeight){
        lastMove = empty;
        return 1;
    }
    else if (mouseX > (width/2)-(buttonWidth/2) && mouseX < (width/2)-(buttonWidth/2)+buttonWidth && mouseY > height/2-windowHeight/2+buttonHeight/2+buttonHeight*3+buttonHeight/3*3 && mouseY < height/2-windowHeight/2+buttonHeight/2+buttonHeight*3+buttonHeight/3*3+buttonHeight){
        chessBoard.close();
    }
    else {
        return 2;
    }
}

//makes the gamemode picker functional
void gameModes(){
    drawGameModes();

    int windowHeight = height/5;
    int buttonWidth = width/4;
    int buttonHeight = windowHeight/6;

    int mouseX = 0;
    int mouseY = 0;
    sf::Event mouseClick;
    while(chessBoard.waitEvent(mouseClick)){
        if (mouseClick.type == sf::Event::Closed){
            chessBoard.close();
        }
        if (mouseClick.type == sf::Event::MouseButtonPressed){
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            sf::Vector2i localPosition = sf::Mouse::getPosition(chessBoard);
            mouseX = localPosition.x;
            mouseY = localPosition.y;
            break;

           }
        }
    }
    navigation = -1;
    if (mouseX > (width/2)-(buttonWidth/2) && mouseX < (width/2)-(buttonWidth/2)+buttonWidth && mouseY > height/7*2-windowHeight/2+buttonHeight/2 && mouseY < height/7*2-windowHeight/2+buttonHeight/2+buttonHeight){
        gameMode = 1;
    }
    else if (mouseX > (width/2)-(buttonWidth/2) && mouseX < (width/2)-(buttonWidth/2)+buttonWidth && mouseY > height/7*2-windowHeight/2+buttonHeight/2+buttonHeight*1+buttonHeight/3*1 && mouseY < height/7*2-windowHeight/2+buttonHeight/2+buttonHeight*1+buttonHeight/3*1+buttonHeight){
        gameMode = 2;
    }
    else if (mouseX > (width/2)-(buttonWidth/2) && mouseX < (width/2)-(buttonWidth/2)+buttonWidth && mouseY > height/7*2-windowHeight/2+buttonHeight/2+buttonHeight*2+buttonHeight/3*2 && mouseY < height/7*2-windowHeight/2+buttonHeight/2+buttonHeight*2+buttonHeight/3*2+buttonHeight){
        navigation = 1;
    }
    else if (mouseX > (width/2)-(buttonWidth/2) && mouseX < (width/2)-(buttonWidth/2)+buttonWidth && mouseY > height/7*2-windowHeight/2+buttonHeight/2+buttonHeight*3+buttonHeight/3*3 && mouseY < height/7*2-windowHeight/2+buttonHeight/2+buttonHeight*3+buttonHeight/3*3+buttonHeight){
        gameMode = 3;
    }
    else {
        navigation = 0;
    }

}

//makes the startmenu functional
void startMenu(){

    drawStartMenu();
    chessBoard.display();
    int buttonWidth = width/2;
    int buttonHeight = height/7;
    int mouseX = 0;
    int mouseY = 0;
    sf::Event mouseClick;



    while(chessBoard.waitEvent(mouseClick)){
        if (mouseClick.type == sf::Event::Closed)
            chessBoard.close();

        if (mouseClick.type == sf::Event::MouseButtonPressed){
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            sf::Vector2i localPosition = sf::Mouse::getPosition(chessBoard);
            mouseX = localPosition.x;
            mouseY = localPosition.y;
            break;

           }
        }

    }

    if (mouseX > width/2-buttonWidth/2 && mouseX < width/2-buttonWidth/2+buttonWidth && mouseY > height/7-buttonHeight/2+buttonHeight && mouseY < height/7-buttonHeight/2+buttonHeight*2){
        navigation = 1;
    }
    else if (mouseX > width/2-buttonWidth/2 && mouseX < width/2-buttonWidth/2+buttonWidth && mouseY > height/7-buttonHeight/2+buttonHeight*3 && mouseY < height/7-buttonHeight/2+buttonHeight*4){
        navigation = 0;
    }
    else if (mouseX > width/2-buttonWidth/2 && mouseX < width/2-buttonWidth/2+buttonWidth && mouseY > height/7-buttonHeight/2+buttonHeight*5 && mouseY < height/7-buttonHeight/2+buttonHeight*6){
        chessBoard.close();
    }

    else{
        navigation = 0;
    }
}

//draws an arrow that indicates the last move
void showLastMove(){


    arrow[0].position = sf::Vector2f(getMiddlePos(0,lastMove.x1,lastMove.y1).x, getMiddlePos(0,lastMove.x1,lastMove.y1).y);
    arrow[1].position = sf::Vector2f(getMiddlePos(0,lastMove.x2,lastMove.y2).x, getMiddlePos(0,lastMove.x2,lastMove.y2).y);

    arrow[0].color = sf::Color::Red;
    arrow[1].color = sf::Color::Red;

    drawChessBoard(board);
    chessBoard.draw(arrow, 2, sf::Lines);


}

//changes the currentplayer from 1 to -1, or the other way around
void changeCurrentPlayer(){
    currentPlayer = -currentPlayer;
}


vector<Move> validPawnMoves(Board B, Piece P, int color){
    vector<Move> valid;
    Move M;
    M.x1 = P.x;
    M.y1 = P.y;

    if (B.board[P.y-color][P.x] == 0 && P.y-color<= 7 && P.y-color >= 0){
        M.x2 = P.x;
        M.y2 = P.y-color;
        valid.push_back(M);

    }

    if (B.board[P.y-color][P.x-1]*B.board[P.y][P.x] < 0 && P.y-color<= 7 && P.y-color >= 0 && P.x-1 >= 0){
        M.x2 = P.x-1;
        M.y2 = P.y-color;
        valid.push_back(M);

    }

    if (B.board[P.y-color][P.x+1]*B.board[P.y][P.x] < 0 && P.y-color<= 7 && P.y-color >= 0 && P.x+1 <= 7){

        if (P.x < 7){
        M.x2 = P.x+1;
        M.y2 = P.y-color;
        valid.push_back(M);
        }
    }

    if(color == 1){
        if (P.y == 6 && B.board[P.y-1][P.x] == 0 && B.board[P.y-2][P.x] == 0){
            M.x2 = P.x;
            M.y2 = P.y-2;
            valid.push_back(M);
        }
    }

    if(color == -1){
        if (P.y == 1 && B.board[P.y+1][P.x] == 0 && B.board[P.y+2][P.x] == 0){
            M.x2 = P.x;
            M.y2 = P.y+2;
            valid.push_back(M);
        }
    }
return valid;
}

vector<Move> validRookMoves(Board B, Piece P){
    vector<Move> valid;
    Move M;
    M.x1 = P.x;
    M.y1 = P.y;
    for (int i = 1; i < 8; i++){
        if (P.y-i < 0){
            break;
        }
        if (B.board[P.y-i][P.x]*B.board[P.y][P.x] > 0){
            break;
        }
        if (B.board[P.y-i][P.x]*B.board[P.y][P.x] < 0){
            M.x2 = P.x;
            M.y2 = P.y-i;
            valid.push_back(M);
            break;
        }
        else{
            M.x2 = P.x;
            M.y2 = P.y-i;
            valid.push_back(M);
        }
    }

    for (int i = 1; i < 8; i++){
        if (P.y+i > 7){
            break;
        }

        if (B.board[P.y+i][P.x]*B.board[P.y][P.x] > 0){
            break;
        }
        if (B.board[P.y+i][P.x]*B.board[P.y][P.x] < 0){
            M.x2 = P.x;
            M.y2 = P.y+i;
            valid.push_back(M);
            break;
        }

        else{
            M.x2 = P.x;
            M.y2 = P.y+i;
            valid.push_back(M);
        }
    }

    for (int i = 1; i < 8; i++){

        if (P.x-i < 0){
            break;
        }

        if (B.board[P.y][P.x-i]*B.board[P.y][P.x] > 0){
            break;
        }
        if (B.board[P.y][P.x-i]*B.board[P.y][P.x] < 0){
            M.x2 = P.x-i;
            M.y2 = P.y;
            valid.push_back(M);
            break;
        }

        else{
            M.x2 = P.x-i;
            M.y2 = P.y;
            valid.push_back(M);
        }
    }

    for (int i = 1; i < 8; i++){
        if (P.x+i > 7){
            break;
        }

        if (B.board[P.y][P.x+i]*B.board[P.y][P.x] > 0){
            break;
        }
        if (B.board[P.y][P.x+i]*B.board[P.y][P.x] < 0){
            M.x2 = P.x+i;
            M.y2 = P.y;
            valid.push_back(M);
            break;
        }

        else{
            M.x2 = P.x+i;
            M.y2 = P.y;
            valid.push_back(M);
        }
    }
    return valid;

}

vector<Move> validBishopMoves(Board B,Piece P){
    vector<Move> valid;
    Move M;
    M.x1 = P.x;
    M.y1 = P.y;
    for (int i = 1; i < 8; i++){
        if (P.y-i < 0 or P.x-i < 0){
            break;
        }

        if (B.board[P.y-i][P.x-i]*B.board[P.y][P.x] > 0){
            break;
        }
        if (B.board[P.y-i][P.x-i]*B.board[P.y][P.x] < 0){
            M.x2 = P.x-i;
            M.y2 = P.y-i;
            valid.push_back(M);
            break;
        }

        else{
            M.x2 = P.x-i;
            M.y2 = P.y-i;
            valid.push_back(M);
        }
    }

    for (int i = 1; i < 8; i++){
        if (P.y-i < 0 or P.x+i > 7){
            break;
        }

        if (B.board[P.y-i][P.x+i]*B.board[P.y][P.x] > 0){
            break;
        }
        if (B.board[P.y-i][P.x+i]*B.board[P.y][P.x] < 0){
            M.x2 = P.x+i;
            M.y2 = P.y-i;
            valid.push_back(M);
            break;
        }

        else{
            M.x2 = P.x+i;
            M.y2 = P.y-i;
            valid.push_back(M);
        }
    }

    for (int i = 1; i < 8; i++){
        if (P.y+i > 7 or P.x-i < 0){
            break;
        }

        if (B.board[P.y+i][P.x-i]*B.board[P.y][P.x] > 0){
            break;
        }
        if (B.board[P.y+i][P.x-i]*B.board[P.y][P.x] < 0){
            M.x2 = P.x-i;
            M.y2 = P.y+i;
            valid.push_back(M);
            break;
        }

        else{
            M.x2 = P.x-i;
            M.y2 = P.y+i;
            valid.push_back(M);
        }
    }

    for (int i = 1; i < 8; i++){
        if (P.y+i > 7 or P.x+i > 7){
            break;
        }

        if (B.board[P.y+i][P.x+i]*B.board[P.y][P.x] > 0){
            break;
        }
        if (B.board[P.y+i][P.x+i]*B.board[P.y][P.x] < 0){
            M.x2 = P.x+i;
            M.y2 = P.y+i;
            valid.push_back(M);
            break;
        }

        else{
            M.x2 = P.x+i;
            M.y2 = P.y+i;
            valid.push_back(M);
        }
    }
    return valid;
}

vector<Move> validKnightMoves(Board B, Piece P){
    vector<Move> valid;
    Move M;
    M.x1 = P.x;
    M.y1 = P.y;
    if (B.board[P.y-2][P.x-1]*B.board[P.y][P.x] <= 0 && P.y-2 >= 0 && P.x-1  >= 0){
        M.x2 = P.x-1;
        M.y2 = P.y-2;
        valid.push_back(M);
    }

    if (B.board[P.y-2][P.x+1]*B.board[P.y][P.x] <= 0 && P.y-2 >= 0 && P.x+1  <= 7){
        M.x2 = P.x+1;
        M.y2 = P.y-2;
        valid.push_back(M);
    }

    if (B.board[P.y+2][P.x-1]*B.board[P.y][P.x] <= 0 && P.y+2 <= 7 && P.x-1  >= 0){
        M.x2 = P.x-1;
        M.y2 = P.y+2;
        valid.push_back(M);
    }

    if (B.board[P.y+2][P.x+1]*B.board[P.y][P.x] <= 0 && P.y+2 <= 7 && P.x+1  <= 7){
        M.x2 = P.x+1;
        M.y2 = P.y+2;
        valid.push_back(M);
    }

    if (B.board[P.y-1][P.x-2]*B.board[P.y][P.x] <= 0 && P.y-1 >= 0 && P.x-2  >= 0){
        M.x2 = P.x-2;
        M.y2 = P.y-1;
        valid.push_back(M);
    }

    if (B.board[P.y+1][P.x-2]*B.board[P.y][P.x] <= 0 && P.y+1 <= 7 && P.x-2  >= 0){
        M.x2 = P.x-2;
        M.y2 = P.y+1;
        valid.push_back(M);
    }

    if (B.board[P.y-1][P.x+2]*B.board[P.y][P.x] <= 0 && P.y-1 >= 0 && P.x+2  <= 7){
        M.x2 = P.x+2;
        M.y2 = P.y-1;
        valid.push_back(M);
    }

    if (B.board[P.y+1][P.x+2]*B.board[P.y][P.x] <= 0 && P.y+1 <= 7 && P.x+2  <= 7){
        M.x2 = P.x+2;
        M.y2 = P.y+1;
        valid.push_back(M);
    }


return valid;

}

vector<Move> validQueenMoves(Board B, Piece P){
    vector<Move> valid = validBishopMoves(B,P);

    for (Move moves : validRookMoves(B,P)){
        valid.push_back(moves);
    }
    return valid;
}

vector<Move> validKingMoves(Board B, Piece P){
    vector<Move> valid;
    Move M;
    M.x1 = P.x;
    M.y1 = P.y;
    if (B.board[P.y][P.x+1]*B.board[P.y][P.x] <= 0 && P.x+1 <= 7){
        M.x2 = P.x+1;
        M.y2 = P.y;
        valid.push_back(M);
    }
    if (B.board[P.y][P.x-1]*B.board[P.y][P.x] <= 0 && P.x-1 >= 0){
        M.x2 = P.x-1;
        M.y2 = P.y;
        valid.push_back(M);
    }
    if (B.board[P.y+1][P.x]*B.board[P.y][P.x] <= 0 && P.y+1 <= 7){
        M.x2 = P.x;
        M.y2 = P.y+1;
        valid.push_back(M);
    }
    if (B.board[P.y-1][P.x]*B.board[P.y][P.x] <= 0 && P.y-1 >= 0){
        M.x2 = P.x;
        M.y2 = P.y-1;
        valid.push_back(M);
    }
    if (B.board[P.y+1][P.x+1]*B.board[P.y][P.x] <= 0 && P.y+1 <= 7 && P.x+1 <= 7){
        M.x2 = P.x+1;
        M.y2 = P.y+1;
        valid.push_back(M);
    }
    if (B.board[P.y-1][P.x-1]*B.board[P.y][P.x] <= 0 && P.y-1 >= 0 && P.x-1 >= 0){
        M.x2 = P.x-1;
        M.y2 = P.y-1;
        valid.push_back(M);
    }
    if (B.board[P.y+1][P.x-1]*B.board[P.y][P.x] <= 0 && P.y+1 <= 7 && P.x-1 >= 0){
        M.x2 = P.x-1;
        M.y2 = P.y+1;
        valid.push_back(M);
    }
    if (B.board[P.y-1][P.x+1]*B.board[P.y][P.x] <= 0 && P.y-1 >= 0 && P.x+1 <= 7){
        M.x2 = P.x+1;
        M.y2 = P.y-1;
        valid.push_back(M);
    }
    return valid;
}

//returns valid moves for a position, before check is calculated
vector<Move> validMovesBefore(Board B,Piece P,int color){
    switch (B.board[P.y][P.x]*color) {

        case 1:
           return validPawnMoves(B,P,color);

        case 2:
            return validKnightMoves(B,P);

        case 3:
           return validBishopMoves(B,P);

        case 5:
        return validRookMoves(B,P);

        case 9:
            return validQueenMoves(B,P);

        case 999:
            return validKingMoves(B,P);
    }
}

//returns all possible moves for a color, before check
vector<Move> allPossibleMovesBefore(Board B,int color){
    vector<Move> allpossiblemoves;
    for (Piece P : B.getPieces(color)){
        for (Move M : validMovesBefore(B,P,color)){
            allpossiblemoves.push_back(M);
        }
    }
    return allpossiblemoves;
}

//returns whether the your king is threatened or not
bool kingIsAttakced(Board B,int color){
    Piece King;
    //locate opponents King
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            if (B.board[i][j] == 999*color){
                King.x = j;
                King.y = i;
            }
        }
    }
    //cout<<"\n"<<King.x1<<King.y1;
    for (Move moves : allPossibleMovesBefore(B,-color)){
        //cout<<"\n"<<allMoves[i].x2<<allMoves[i].y2;
        if (King.x == moves.x2 && King.y == moves.y2){
            return true;
        }
    }
    return false;
}

//returns whether a specific move puts you in check or not
bool check(Board B, Move M, int color){
Board copyBoard = B;
    copyBoard.move(M);
       return kingIsAttakced(copyBoard,color);

}

//returns valid moves for a position, after all moves that put you in check are excluded
vector<Move> validMoves(Board B, Piece P, int color){
    vector<Move> valid;
for (Move v : validMovesBefore(B,P,color)){
    if (!check(B,v,color)){
        valid.push_back(v);
    }
}
return valid;

}

//returns all possible moves, after check
vector<Move> allPossibleMoves(Board B,int color){
    vector<Move> allpossiblemoves;
    for (Piece P : B.getPieces(color)){
        for (Move M : validMoves(B,P,color)){
            allpossiblemoves.push_back(M);
        }
    }
    return allpossiblemoves;
}

//checks whether you are in checkmate or not
bool checkMate(Board B, int color){
    if (allPossibleMoves(B,color).size() == 0 && kingIsAttakced(B,-color)){
        return true;
    }
    return false;
}

//checks whether you are in stalemate or not
bool staleMate(Board B, int color){
    if (allPossibleMoves(B,color).size() == 0 && !kingIsAttakced(B,-color)){
        return true;
    }
    return false;
}

//messy function that handles the player move, as well as the grapichs for it
int playerMove(int color) {

    Piece P;
    startOfPlayerMove:
    int indicatorSize = width/100;
    sf::CircleShape indicator(indicatorSize);
    indicator.setPointCount(10);
    indicator.setFillColor(sf::Color::Red);
    unsigned int mouseX = 0;
    unsigned int mouseY = 0;
sf::Event mouseClick;
drawChessBoard(board);
showLastMove();
chessBoard.display();

    while(chessBoard.waitEvent(mouseClick)){
        if (mouseClick.type == sf::Event::MouseButtonPressed){
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            sf::Vector2i localPosition = sf::Mouse::getPosition(chessBoard);
            mouseX = localPosition.x;
            mouseY = localPosition.y;
            break;
        }

        }

        if (mouseClick.type == sf::Event::KeyPressed)
        {
            if (mouseClick.key.code == sf::Keyboard::P or mouseClick.key.code == sf::Keyboard::Escape)
            {
                return pauseMenu();
            }
        }
        if (mouseClick.type == sf::Event::Closed)
            chessBoard.close();
    }
    for (unsigned int i = 0; i < 8; i++){
        for (unsigned int j = 0; j < 8; j++){
            if(mouseX > width/10*(j+1) && mouseX < width/10*(j+2) && mouseY > height/10*(i+1) && mouseY < height/10*(i+2)){
                if(board.board[i][j]*color <= 0){
                    goto startOfPlayerMove;
                }
                P.x = j;
                P.y = i;
                if(validMoves(board,P,color).size() == 0){
                    goto startOfPlayerMove;
                }
                drawChessBoard(board);
                for (Move M :validMoves(board,P,color)){
                    indicator.setPosition(getMiddlePos(indicatorSize,M.x2,M.y2).x,getMiddlePos(indicatorSize,M.x2,M.y2).y);
                    chessBoard.draw(indicator);

                }
            }
        }
    }
    chessBoard.display();

    while(chessBoard.waitEvent(mouseClick)){
        if (mouseClick.type == sf::Event::MouseButtonPressed){
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)){
            sf::Vector2i localPosition = sf::Mouse::getPosition(chessBoard);
            mouseX = localPosition.x;
            mouseY = localPosition.y;
            break;
        }


        }
        if (mouseClick.type == sf::Event::KeyPressed)
        {
            if (mouseClick.key.code == sf::Keyboard::P or mouseClick.key.code == sf::Keyboard::Escape)
            {
                return pauseMenu();
            }
        }
        if (mouseClick.type == sf::Event::Closed)
            chessBoard.close();
    }
    bool validClick = false;
    Move move;
for (Move M :validMoves(board,P,color)){
    if(mouseX > width/10*(M.x2+1) && mouseX < width/10*(M.x2+2) && mouseY > height/10*(M.y2+1) && mouseY < height/10*(M.y2+2)){
        validClick = true;
        move = M;
    }
}
if (validClick){
    board.move(move);
    lastMove = empty;
    showLastMove();
    pawnToQueen(board,color);
    drawChessBoard(board);
    chessBoard.display();


}
else{
    drawChessBoard(board);
    chessBoard.display();
    goto startOfPlayerMove;
}

return -1;
}

//returns the largest of the two, used for sorting
bool sortinrev(const pair<Move,int> &a,
               const pair<Move,int> &b)
{
       return (a.second > b.second);
}

//sorts a vector of moves, by the value of the board afther making the move
vector<Move> sortedMoves(Board &B,int color){
    vector<Move> moves;
    vector< pair <Move,int>> sorted;
    Board copyBoard = B;
    vector<Move> allMoves = allPossibleMoves(B,color);
    for (unsigned int i = 0; i < allMoves.size(); i++){

        copyBoard.move(allMoves[i]);
        pawnToQueen(copyBoard,color);
        sorted.push_back({allMoves[i], B.getValue()});
        copyBoard.moveBack(allMoves[i],B);

    }

    sort(sorted.begin(), sorted.end(), sortinrev);

        /*for(unsigned int i = 0; i < sorted.size(); i++){
            cout<<sorted[i].second<<" ";
        }

        cout<<endl;*/
    for (unsigned int i = 0; i < sorted.size(); i++){
        moves.push_back(sorted[i].first);
    }
    /*for (unsigned int i = 0; i < moves.size(); i++){
        cout<<abc[moves[i].x2]<<7-moves[i].y2+1<<" ";
    }
    cout<<endl;*/

    return moves;
}

//finds the best move value
int negaMax(Board &B, int depth, int color, int alpha, int beta){

    if(depth == 0){
        return color*B.getValue();
    }
    if (checkMate(B,color)){
        return 10000000;
    }
    int value = -9999999;
    Board copyBoard = B;
    for (Move M : sortedMoves(B,color)){
        posCounter++;
        fullPosCounter++;
        copyBoard.move(M);
        pawnToQueen(copyBoard,color);
        value = max(value,-negaMax(copyBoard, depth-1, -color, -beta, -alpha));
        copyBoard.moveBack(M,B);

        alpha = max(alpha, value);
        if (alpha >= beta){
            break;
        }
    }
    return value;
}

//jumps in 1 layer in the negamax to figure out which move is the best
int comMove(int color){

    clock_t start = clock();
    double duration;

int bestMoveScore = 100000;
int alpha = -1000000;
int beta = 1000000;
int depth = 4;


// opening moves
/*if (color == -1 && turns < openingMoves[opening].size()){

    if (board.board[openingMoves[opening][turns].y2][openingMoves[opening][turns].x2] >= 0){
        board.move(openingMoves[opening][turns]);
        lastMove = openingMoves[opening][turns];
        return 0;
    }



}

else{*/
Move bestMove;
    Board copyBoard = board;
for (Move M : allPossibleMoves(board,color)){
    posCounter++;
    fullPosCounter++;
    copyBoard.move(M);
    pawnToQueen(copyBoard,color);
    int value = negaMax(copyBoard,depth-1,-color,-beta,-alpha);
    copyBoard.moveBack(M,board);

    cout<<value<<" "<<abc[M.x1]<<7-M.y1+1<<" to "<<abc[M.x2]<<7-M.y2+1<<endl;


    if (value <= bestMoveScore){
        bestMoveScore = value;
        bestMove = M;
        lastMove = M;
    }
    //alpha-beta pruning doesn't work properly
    /*alpha = max(alpha, value);
    if (alpha >= beta){
        break;
    }*/

}
duration = (clock() - start) / (double)CLOCKS_PER_SEC;
fullDuration += duration;
cout<<"The best move was: "<<abc[bestMove.x1]<<7-bestMove.y1+1<<" to "<<abc[bestMove.x2]<<7-bestMove.y2+1<<endl;
cout<<"Evaluated postitions: "<<posCounter<<endl;
cout<<"Duration in seconds: "<<duration<<endl;
cout<<"Positions per second: "<<posCounter / duration<<endl;
cout<<"Average positions per second: "<<fullPosCounter / fullDuration<<endl;
posCounter = 0;
board.move(bestMove);
pawnToQueen(board,color);
drawChessBoard(board);
showLastMove();
chessBoard.display();
//}
}

//initializes the board with the starting position, and resets some variables
void startGame(){
    fullDuration = 0;
    fullPosCounter = 0;
    currentPlayer = 1;
    int startingPos[8][8]={{-5,-2,-3,-9,-999,-3,-2,-5},                 //array med alle brættets felter og brikkernes startposition
                           {-1,-1,-1,-1,-1,-1,-1,-1},
                           {0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0,0},
                           {0,0,0,0,0,0,0},
                           {1,1,1,1,1,1,1,1},
                           {5,2,3,9,999,3,2,5}};
    for (int i = 0; i < 8; i++){                                        //for loop der indsætter startpositionen på boardet
        for (int j = 0; j < 8; j++){
            board.board[i][j] = startingPos[i][j];
        }
        }
    drawChessBoard(board);
    chessBoard.display();
    }

//the player vs computer gamemode
int playervscom(){
startOfPlayervsCom:

    switch (playerMove(currentPlayer)){
    case 2:
        goto startOfPlayervsCom;

    case 1:
        return 1;
    }




    if (checkMate(board,-currentPlayer) or staleMate(board, -currentPlayer)){
        return pauseMenu();
    }
    changeCurrentPlayer();



    comMove(currentPlayer);
    turns++;



    if (checkMate(board,-currentPlayer) or staleMate(board, -currentPlayer)){
        chessBoard.close();
    }
    changeCurrentPlayer();
    return 0;
}

//the computer vs computer gamemode
void comvscom(){


    comMove(currentPlayer);

    if (checkMate(board,-currentPlayer) or staleMate(board, -currentPlayer)){
        chessBoard.close();
    }

    changeCurrentPlayer();

}

//the player vs player gamemode
int playervsplayer(){
    startOfPlayervsPlayer:

    switch (playerMove(currentPlayer)){
    case 2:
        goto startOfPlayervsPlayer;

    case 1:
        return 1;
    }


    if (checkMate(board,-currentPlayer) or staleMate(board, -currentPlayer)){
        chessBoard.close();
    }
    changeCurrentPlayer();
return 0;
}

//messy navigation for the startmenu, redo pls
void startMenuNavigation(){
    startOfNavigation:
    startMenu();
    switch (navigation){
    case 0:
    goto startOfNavigation;

    case 1:
    startOfGameModes:
    gameModes();
    if (navigation == 0)
        goto startOfNavigation;
    if (navigation == 1)
        goto startOfGameModes;


    }

}



int main(){
    startOfMain:
    startGame();
    startMenuNavigation();
    settings.antialiasingLevel = 0;
    chessBoard.setFramerateLimit(60);

        switch (gameMode){
        case 1:
            while (chessBoard.isOpen()){


                sf::Event event;
                while (chessBoard.pollEvent(event)){

                    if (event.type == sf::Event::Closed)
                        chessBoard.close();


                }

        startOfPP:
        if (playervsplayer() == 1) //means that "go to menu" is picked in the pause menu
            goto startOfMain;
        else
            goto startOfPP;
        }
        case 2:
            //picking opening moves
            /*srand(time(0));
            opening = (rand() % openingMoves.size());*/

            while (chessBoard.isOpen()){


                sf::Event event;
                while (chessBoard.pollEvent(event)){

                    if (event.type == sf::Event::Closed)
                        chessBoard.close();


                }
        startOfPC:


        if (playervscom() == 1)
            goto startOfMain;
        else
            goto startOfPC;
            }

        case 3:
            turns = 100;

            while (chessBoard.isOpen()){


                sf::Event event;
                while (chessBoard.pollEvent(event)){

                    if (event.type == sf::Event::Closed)
                        chessBoard.close();

                }
        comvscom();
}
        }






    return 0;
}
