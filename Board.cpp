#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <vector>

using namespace std;

bool search (char c, const vector<char>& v) {
    for (size_t i = 0, N = v.size(); i < N; ++i) {
        if (v[i] == c) {return true;}
    }
    return false;
}


class BoardImpl
{
  public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const;

    // functions used by possibilities_boards
    void update(Point p, char c);
    void read_to(vector<int>& data);
    
  private:
    const Game& m_game;
    vector<char> board;
};

BoardImpl::BoardImpl(const Game& g) : m_game(g), board(g.rows()*g.cols(), '.') {
}

void BoardImpl::clear() {
    for (size_t i = 0, N = board.size(); i < N; ++i) {
        board[i] = '.';
    }
}

void BoardImpl::block() {
    for (size_t i = 0, N = board.size() / 2; i < N; ++i) {
        Point Position = m_game.randomPoint();
        board[m_game.cols() * Position.r + Position.c] = '-';
    }
}

void BoardImpl::unblock() {
    for (size_t i = 0, N = board.size(); i < N; ++i) {
        if (board[i] == '-') {board[i] = '.';};
    }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir) {
    
    if (shipId >= m_game.nShips())                                          { return false;} // invalid shipId
    if (search(m_game.shipSymbol(shipId), board))                           { return false;} // ship already on the board

    size_t start = m_game.cols() * topOrLeft.r + topOrLeft.c;
    
    // separate checking and changing loops so don't have to erase upon failure
    if (dir == HORIZONTAL) {
        
        // loop to check
        for(size_t i = 0, N = m_game.shipLength(shipId); i < N; ++i) {
            // we are at the end of a row & have more to go
            if ( (((start + i + 1) % m_game.cols()) == 0) && (i+1 < N))     { return false;} // ship goes of end of row
            if ( board[start + i] != '.')                                   { return false;} // ship runs over something
        }
        
        // loop to change
        for(size_t i = 0, N = m_game.shipLength(shipId); i < N; ++i) {
            board[start + i] = m_game.shipSymbol(shipId);
        }
    }
    
    else {
        // differences: indexing is more complicated, but going off end is easy to check
        
        // loop to check
        for(size_t i = 0, N = m_game.shipLength(shipId); i < N; ++i) {
            if (start + m_game.cols()*i > board.size())                     { return false;} // ship goes of end of column
            if ( board[start + m_game.cols()*i] != '.')                     { return false;} // ship runs over something
        }
        
        // loop to change
        for(size_t i = 0, N = m_game.shipLength(shipId); i < N; ++i) {
            board[start + m_game.cols()*i] = m_game.shipSymbol(shipId);
        }
    }
    return true;
}




bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir) {
    
    if (shipId >= m_game.nShips())                                            { return false;} // invalid shipId
    size_t start = m_game.cols() * topOrLeft.r + topOrLeft.c;
    // separate checking and changing loops so don't have to reinput upon failure
    if (dir == HORIZONTAL) {
        
        // loop to check
        for(size_t i = 0, N = m_game.shipLength(shipId); i < N; ++i) {
            if ( board[start + i] != m_game.shipSymbol(shipId))               { return false;} // incomplete ship
        }
        // loop to change
        for(size_t i = 0, N = m_game.shipLength(shipId); i < N; ++i) { board[start + i] = '.';}
    }
    
    else {

        // loop to check
        for(size_t i = 0, N = m_game.shipLength(shipId); i < N; ++i) {
            if ( board[start + m_game.cols()*i] != m_game.shipSymbol(shipId)) { return false;} // incomplete ship
        }
        // loop to change
        for(size_t i = 0, N = m_game.shipLength(shipId); i < N; ++i) { board[start + m_game.cols()*i] = '.';}
    }
    return true;
}


void BoardImpl::display(bool shotsOnly) const {
    // shotsOnly determines whether the board will be displayed in full (false)
    // or only display the results of attacks (true)
    cout << "   ";
    size_t M = m_game.rows();
    size_t N = m_game.cols();
    
    for (size_t i = 0; i < N; ++i ) {cout << i << ' ';}
    cout << endl;
    
    for (size_t j = 0; j < M; ++j ) {
        cout << j << ' ' << ' ';
        for (size_t k = 0; k < N; ++k ) {
            if (shotsOnly && (!(board[m_game.cols() * j + k] == 'o' ||
                                board[m_game.cols() * j + k] == '.' ||
                                board[m_game.cols() * j + k] == 'X' ))) {
                cout << '.' << ' ';
            }
            else {
                cout << board[m_game.cols() * j + k] << ' ';
            }
        }
        cout << endl;
    }
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId) {
    if (p.r >= m_game.rows() || p.c >= m_game.cols() || p.r < 0 || p.c < 0)                 {return false;}
    if (board[m_game.cols() * p.r + p.c] == 'o' || board[m_game.cols() * p.r + p.c] == 'X') {return false;}
                                                      
    
    if (board[m_game.cols() * p.r + p.c] != '.') {
        // hit an undamaged part of a ship
        cout << "The attack hit a ship!" << endl;
        shotHit = true;
        char hit_ship_symbol = board[m_game.cols() * p.r + p.c];
        board[m_game.cols() * p.r + p.c] = 'X';
        
        if (!search(hit_ship_symbol, board)) {
            // means there are no more parts of that ship undamaged (its char is gone)
            shipDestroyed = true;
            
            // finding the correct shipId
            for (size_t i = 0, N = m_game.nShips(); i < N; ++i) {
                if (hit_ship_symbol == m_game.shipSymbol(static_cast<int>(i))) {
                    shipId = static_cast<int>(i);
                }
            }
            cout << "The attack sank the " << m_game.shipName(shipId) << "!" << endl;
        }
        else {
            shipDestroyed = false;
        }
    }

    else {
        board[m_game.cols() * p.r + p.c] = 'o';
        shotHit = false;
    }
    return true;
}

bool BoardImpl::allShipsDestroyed() const {
    for (size_t i = 0, N = board.size(); i < N; ++i) {
        if (!(board[i] == 'o' || board[i] == '.' || board[i] == 'X')) {return false;}
        //if I find a character that isn't one of those 3, its from an undamaged segment of a ship
    }
    return true;
}



//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}







