//
//  Possibilities.cpp
//  Battleship
//
//  Created by Kyle Ritscher on 7/23/22.
//

#include "Board.h"
#include "Game.h"
#include "Possibilities.h"
#include <vector>
#include <iostream>

using namespace std;

Possibilities_Board::Possible_Location::Possible_Location(Point _topOrLeft, int _shipId, Direction _dir) :
     topOrLeft(_topOrLeft), shipId(_shipId), dir(_dir) {};


//*********************************************************************
//  Possibilities Board Public and Helper Functions
//*********************************************************************


Possibilities_Board::Possibilities_Board(const Game& g) : m_game(g), locations_list(g.nShips(), vector<Possible_Location>()), board(g.rows()*g.cols(), '.') {
    refrence_board = board;
};

void Possibilities_Board::update(Point p, char c) {
    board[m_game.cols() * p.r + p.c] = c;
    refrence_board[m_game.cols() * p.r + p.c] = c;
}

void Possibilities_Board::ship_destroyed(int shipId) {
    destroyed_ships.push_back(shipId);
}

bool Possibilities_Board::is_ship_destroyed(int shipId) const{
    for (size_t i = 0, N = destroyed_ships.size(); i < N; ++i) {
        if (shipId == destroyed_ships[i]) {return true;}
    }
    return false;
}


void Possibilities_Board::read_to(vector<int> &data) const{
    if (board.size() != data.size()) {return;}
    for (size_t i = 0, N = data.size(); i < N; ++i) {
        if (!(board[i] == 'o' || board[i] == '.') && (refrence_board[i] == '.')) {++data[i];}
    }
}

void Possibilities_Board::determine_locations() {
    /*
     determine locations determines and saves all the possible places ships could be located
     the locations are saved in possibilities_board.locations_list, a vector of vectors, of lists
     an optimization is also included in the case where only one possible location exists.
     That location is saved to avoid recalculating possibilities,
     and the ship is permenantly placed on the board
     to provide more accuracy in calculating other ships' locations
     */
    for (size_t i = 0, N = m_game.nShips(); i < N; ++i) {
        if (locations_list[i].size() == 1) {            // if there was only one option from the last one,
            if (is_valid(locations_list[i][0])) {       // ensure that it is a valid option
                continue;                               // and skip to the next ship if it is
            }
        }
        locations_list[i].clear();                      // ensure that the vector is empty at the start
        int shipId = static_cast<int>(i);
        for (size_t i = 0, N = board.size(); i < N; ++i) {
            Point p(static_cast<int>(i) / m_game.cols(), static_cast<int>(i) % m_game.cols());
            Possible_Location LH(p, shipId, HORIZONTAL);
            if (is_valid(LH)) { locations_list[shipId].push_back(LH);}
            Possible_Location LV(p, shipId, VERTICAL);
            if (is_valid(LV)) { locations_list[shipId].push_back(LV);}
        }
        if (locations_list[i].size() == 1) {            // in the case of only one option, force it into the refrence board
            place_ship(locations_list[i][0]);           // this will save time in later calls to determine_location()
            refrence_board = board;
        }
    }
}

bool Possibilities_Board::is_valid_board() const {
    /*
     bool isValid checks if the possibilities board is overall valid
     I.E., no 'X' character
     */
    for (size_t i = 0, N = board.size(); i < N; ++i) {
        if (board[i] == 'X') {return false;}
    }
    return true;
}

bool Possibilities_Board::place_ships() {
    bool success;
    if (destroyed_ships.size() != 0) { success = place_ships_recursively(destroyed_ships[0], 1, 0);}
    else { success = place_ships_recursively(0, 0, destroyed_ships.size() + 1);}
    return success;
}

void Possibilities_Board::unplace_all_ships() { board = refrence_board;}


//*********************************************************************
//  Possibilities Board Private Functions
//*********************************************************************


bool Possibilities_Board::is_valid(Possible_Location L) const{
    /*
     bool isValid(L) checks if a certain ship location (L) is valid on a the board
     first, determines if the ship has been sunk
     if not sunk, ensures the ship only crosses '.', 'X', or its own symbol
     if sunk, ensures the ship only crosses 'X' or its own symbol
     and ensures the ship did indeed cross its own symbol
     */
    if (L.shipId >= m_game.nShips()) { return false;} // invalid shipId
    
    size_t start = m_game.cols() * L.topOrLeft.r + L.topOrLeft.c;
    
    bool destroyed = is_ship_destroyed(L.shipId); // determine if the ship has been sunk (extra validity checks)
    bool occupies_sunk_square = false;
    
    if (!destroyed) { // checks for ships that haven't been destroyed
        if (L.dir == HORIZONTAL) {
            for(size_t i = 0, N = m_game.shipLength(L.shipId); i < N; ++i) {
                // we are at the end of a row & have more to go
                if ( (((start + i + 1) % m_game.cols()) == 0) && (i+1 < N))                           { return false;} // runs off edge
                if (!(board[start + i] == '.' || board[start + i] == 'X'
                      || board[start + i] == m_game.shipSymbol(L.shipId)))                            { return false;} // runs over something
            }
        }
        else {
            for(size_t i = 0, N = m_game.shipLength(L.shipId); i < N; ++i) {
                if (start + m_game.cols()*i > board.size())                                           { return false;} // runs of edge
                if (!(board[start + m_game.cols()*i] == '.' || board[start + m_game.cols()*i] == 'X'
                      || board[start + m_game.cols()*i] == m_game.shipSymbol(L.shipId)))              { return false;} // runs over something
            }
        }
    }
    
    else { // checks for ships that have been destroyed
        if (L.dir == HORIZONTAL) {
            for(size_t i = 0, N = m_game.shipLength(L.shipId); i < N; ++i) {
                // we are at the end of a row & have more to go
                if ( (((start + i + 1) % m_game.cols()) == 0) && (i+1 < N))                           { return false;} // runs off edge
                if (board[start + i] != 'X') {                                                                         // runs over a non-hit
                    if (board[start + i] == m_game.shipSymbol(L.shipId)) {                                             // if own symbol, good
                        occupies_sunk_square = true;
                    }
                    else { return false;}
                }
            }
        }
        else {
            for(size_t i = 0, N = m_game.shipLength(L.shipId); i < N; ++i) {
                if (start + m_game.cols()*i > board.size())                                           { return false;} // runs of edge
                if (board[start + m_game.cols()*i] != 'X') {                                                           // runs over a non-hit
                    if (board[start + m_game.cols()*i] == m_game.shipSymbol(L.shipId)) {                               // if own symbol, good
                        occupies_sunk_square = true;
                    }
                    else { return false;}
                }
            }
        }
        if (!occupies_sunk_square) { return false;} // we never ran over sunk square
    }
    return true;
}

bool Possibilities_Board::place_ship(Possible_Location L) {
    
    if (!is_valid(L)) {return false;}
    
    size_t start = m_game.cols() * L.topOrLeft.r + L.topOrLeft.c;
    if (L.dir == HORIZONTAL) {
        for(size_t i = 0, N = m_game.shipLength(L.shipId); i < N; ++i) {
            board[start + i] = m_game.shipSymbol(L.shipId);
        }
    }
    
    else {
        for(size_t i = 0, N = m_game.shipLength(L.shipId); i < N; ++i) {
            board[start + m_game.cols()*i] = m_game.shipSymbol(L.shipId);
        }
    }
    return true;
}

bool Possibilities_Board::unplace_ship(Possible_Location L) {
    if (L.shipId >= m_game.nShips()) { return false;}
    size_t start = m_game.cols() * L.topOrLeft.r + L.topOrLeft.c;
    
    // skipping some of the checks done in boards unplace ships,
    // I find them unneccessary with unplace_ship being a private function for this class
    if (L.dir == HORIZONTAL) {
        for(size_t i = 0, N = m_game.shipLength(L.shipId); i < N; ++i) {
            board[start + i] = refrence_board[start + i];
        }
    }
    
    else {
        for(size_t i = 0, N = m_game.shipLength(L.shipId); i < N; ++i) {
            board[start + m_game.cols()*i] = refrence_board[start + m_game.cols()*i];
        }
    }
    return true;
}


bool Possibilities_Board::place_ships_recursively(int shipId, bool sunks_now, size_t which_sunk) {
    /*
     Place ships recursively has two main phases, one where we are placing the ships that have been sunk,
     and the next where we are just placing ships one by one in order
     Some control flow is necessary to manage interactions between these two phases
     
     Overall, every placement prompts a recursive call,
     if the call returns false, then a different placement is tried for the current ship
     if there are no more possible placements, then the current function returns false
     if the call had returned true, then the current function returns true.
     The condition for the first return true would be if we are in phase two (normal, in order recursion)
     and our shipId is equal to our number of ships
     
     @param int shipId: the ship being placed
     @param bool sunks_now: which phase we are in, true being placing sunk ships only, false being normal recursion
     @param size_t which_sunk: the index of destroyed_ships we are currently on
     */
    
    // return condition: not sunks_now and shipId >= our number of ships
    if (!sunks_now && shipId >= m_game.nShips() )          { return true;}
    // shift from sunks_now to normal recursion once our index is larger than destroyed ships
    if (sunks_now && which_sunk >= destroyed_ships.size()) { return place_ships_recursively(0, 0, which_sunk);}
    // skip a step in normal recursion if the ship is in destroyed ships
    if (!sunks_now && is_ship_destroyed(shipId))           { return place_ships_recursively(shipId+1, 0, which_sunk);}
    
    vector<int> already_guessed = {};
    size_t number_of_options = locations_list[shipId].size();
    
    bool valid_guess;
    int choice;
    while (already_guessed.size() < number_of_options) {
        // the do-while is to ensure that we don't repeat guesses
        // this could take awhile if there are limited possibilities and we are unlucky
        // we try to avoid this being catastrophic by placing the sunk ships first
        do {
            valid_guess = true;
            choice = randInt(static_cast<int>(number_of_options));
            for (size_t i = 0, N = already_guessed.size(); i < N; ++i) {
                if (choice == already_guessed[i]) {valid_guess = false;}
            }
        }
        while (!valid_guess);
        
        already_guessed.push_back(choice);
        Possible_Location random_location = locations_list[shipId][choice];
        
        // if the placement was successful, try to place the next ship
        // while managing which phase of the recursion we are in
        if (place_ship(random_location)) {
            if (sunks_now) {
                if (!place_ships_recursively(destroyed_ships[which_sunk+1], 1, which_sunk+1)) {
                    unplace_ship(random_location);
                }
                else {return true;}
            }
            else {
                if (!place_ships_recursively(shipId + 1, 0, which_sunk)) {
                    unplace_ship(random_location);
                }
                else {return true;}
            }
        }
    }
    return false; // we've run out of possible locations
}


