#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>

using namespace std;

class GameImpl
{
  public:
    GameImpl(int _nRows, int _nCols);
    ~GameImpl(); // needed since my implementation of ships go on the heap
    int rows() const;
    int cols() const;
    bool isValid(Point p) const;
    Point randomPoint() const;
    bool addShip(int length, char symbol, string name);
    int nShips() const;
    int shipLength(int shipId) const;
    char shipSymbol(int shipId) const;
    string shipName(int shipId) const;
    void display() const;
    Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause);
  private:
    struct Ship {
        Ship(int _length, char _symbol, string _name);
        int length;
        char symbol;
        string name;
    };
    int nRows;
    int nCols;
    vector<Ship*> Ships;
};

GameImpl::Ship::Ship(int _length, char _symbol, string _name) : length(_length), symbol(_symbol), name(_name) {};

void waitForEnter()
{
    cout << "Press enter to continue: ";
    cin.ignore(10000, '\n');
}

GameImpl::GameImpl(int _nRows, int _nCols) : nRows(_nRows), nCols(_nCols) {}
GameImpl::~GameImpl() {
    // destructor loops through Ships to delete the ship at each pointer
    for (size_t i = 0, N = Ships.size(); i < N; ++i) {
        delete Ships[i];
    }
}

int GameImpl::rows() const { return nRows;}
int GameImpl::cols() const { return nCols;}

bool GameImpl::isValid(Point p) const {
    return p.r >= 0  &&  p.r < rows()  &&  p.c >= 0  &&  p.c < cols();
}

Point GameImpl::randomPoint() const {
    return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name) {
    // creates the Ship on heap and adds a pointer to it to ships
    Ship* p = new Ship(length, symbol, name);
    Ships.push_back(p);
    return true;
}

int GameImpl::nShips() const { return static_cast<int>(Ships.size());}

int    GameImpl::shipLength(int shipId) const { return Ships[shipId]->length;}
char   GameImpl::shipSymbol(int shipId) const { return Ships[shipId]->symbol;}
string GameImpl::shipName  (int shipId) const { return Ships[shipId]->name;  }


Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause) {
    //placing ships and ensuring completion
    cout << "Players may place their ships" << endl;
    bool ships_placed_one = p1->placeShips(b1);
    bool ships_placed_two = p2->placeShips(b2);
    if (!(ships_placed_one && ships_placed_two)) {return nullptr;}
    
    cout << "All ships have been placed, let the game begin!" << endl << endl;

    //variables to change while attacking
    
    bool shotHit = 0;
    bool shipDestroyed = 0;
    int  shipId = -1;
    bool whoseTurn = 1;
    unsigned int turn_counter = 0;
    
    // end criteria, one board has all ships destroyed
    while (!(b1.allShipsDestroyed() || b2.allShipsDestroyed())) {
        if (whoseTurn) {
            cout << p1->name() << "'s turn to attack" << endl;
            cout << p2->name() << "'s board before the attack: " << endl;
            b2.display(p1->isHuman());                                                  // display p2 board
            
            Point recomended = p1->recommendAttack();                                   // p1 recomends an attack
            bool failtest = b2.attack(recomended, shotHit, shipDestroyed, shipId);      // p1 attacks
            while (!failtest) {
                p1->recordAttackResult(recomended, 0, shotHit, shipDestroyed, shipId);  // p1 records his failed attack
                recomended = p1->recommendAttack();                                     // re-recomending and attacking
                failtest = b2.attack(recomended, shotHit, shipDestroyed, shipId);       // if previous attack was invalid
            }
            cout << p2->name() << "'s board after the attack: " << endl;
            b2.display(p1->isHuman());                                                  // display p2 board
            p1->recordAttackResult(recomended, 1, shotHit, shipDestroyed, shipId);      // p1 records his attack
            p2->recordAttackByOpponent(recomended);                                     // p2 records the attack
            
            whoseTurn = 0;                                                              // changing turns
            ++turn_counter;
            if (turn_counter > nRows * nCols) { return nullptr;}                        // break condition
            cout << endl;
        }
        else {
            cout << p2->name() << "'s turn to attack" << endl;
            cout << p1->name() << "'s board before the attack: " << endl;
            b1.display(p2->isHuman());                                                  // display p1 board
            
            Point recomended = p2->recommendAttack();                                   // p2 recomends attack
            bool failtest = b1.attack(recomended, shotHit, shipDestroyed, shipId);      // p2 attacks
            while (!failtest) {
                p2->recordAttackResult(recomended, 0, shotHit, shipDestroyed, shipId);  // p2 records his failed attack
                recomended = p2->recommendAttack();                                     // re-recomending and attacking
                failtest = b1.attack(recomended, shotHit, shipDestroyed, shipId);       // if previous attack was invalid
            }
            
            cout << p1->name() << "'s board after the attack: " << endl;
            b1.display(p2->isHuman());                                                  // display p1 board
            p2->recordAttackResult(recomended, 1, shotHit, shipDestroyed, shipId);      // p2 records his attack
            p1->recordAttackByOpponent(recomended);                                     // p1 records the attack
            whoseTurn = 1;                                                              // changing turns
            cout << endl;
        }
    }
    
    
    
    // Game Conclusion: announce winner, display winner's board if loser is human
    Player* winner = nullptr;
    if (b1.allShipsDestroyed()) {
        winner = p2;
        cout << p1->name() << " has no remaining ships. " << p2->name() <<" Wins in " << turn_counter << " turns."<< endl;
        if (p1->isHuman()) {
            b2.display(0);
        }
    }
    else {
        winner = p1;
        cout << p2->name() << " has no remaining ships. " << p1->name() <<" Wins in " << turn_counter << " turns."<< endl;
        if (p2->isHuman()) {
            b1.display(0);
        }
    }
    cout << endl;
    cout << "Game Over." << endl;
    cout << "Thanks for playing :)" << endl;
    cout << "               - Kyle " << endl << endl;
    cout << "P.S. Remember to delete your players. "<< endl << endl << endl << endl;
    return winner;
}









//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
    if (nRows < 1  ||  nRows > MAXROWS)
    {
        cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
        exit(1);
    }
    if (nCols < 1  ||  nCols > MAXCOLS)
    {
        cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
        exit(1);
    }
    m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
    delete m_impl;
}

int Game::rows() const
{
    return m_impl->rows();
}

int Game::cols() const
{
    return m_impl->cols();
}

bool Game::isValid(Point p) const
{
    return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
    return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
    if (length < 1)
    {
        cout << "Bad ship length " << length << "; it must be >= 1" << endl;
        return false;
    }
    if (length > rows()  &&  length > cols())
    {
        cout << "Bad ship length " << length << "; it won't fit on the board"
             << endl;
        return false;
    }
    if (!isascii(symbol)  ||  !isprint(symbol))
    {
        cout << "Unprintable character with decimal value " << symbol
             << " must not be used as a ship symbol" << endl;
        return false;
    }
    if (symbol == 'X'  ||  symbol == '.'  ||  symbol == 'o' || symbol == '-')
    {
        cout << "Character " << symbol << " must not be used as a ship symbol"
             << endl;
        return false;
    }
    int totalOfLengths = 0;
    for (int s = 0; s < nShips(); s++)
    {
        totalOfLengths += shipLength(s);
        if (shipSymbol(s) == symbol)
        {
            cout << "Ship symbol " << symbol
                 << " must not be used for more than one ship" << endl;
            return false;
        }
    }
    if (totalOfLengths + length > rows() * cols())
    {
        cout << "Board is too small to fit all ships" << endl;
        return false;
    }
    return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
    return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
    if (p1 == nullptr  ||  p2 == nullptr  ||  nShips() == 0)
        return nullptr;
    Board b1(*this);
    Board b2(*this);
    return m_impl->play(p1, p2, b1, b2, shouldPause);
}

