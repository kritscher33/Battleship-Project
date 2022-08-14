//
//  Possibilities.h
//  Battleship
//
//  Created by Kyle Ritscher on 7/23/22.
//

#ifndef POSSIBILITIES_ORIGINAL
#define POSSIBILITIES_ORIGINAL

#include "globals.h"

/*
 Possibilities_Board and its nested class Possible_Locations are designed to assist GoodPlayer::recomend_attack
 Due to the nature of this projects directions, I am not editing any other h files
 - this is the only h file of my own design
 So, no inheritance from or friendship with other classes
 This restriction is the guiding reason I chose to implement GoodPlayer::recomend_attack in this (roundabout) way
 
 Possibilities Board is designed similarly to Board, but allowing direct access to "play around" with it
 Many public functions can directly change elements in Possibilities_Board.board
 The main goal of the class is to be able to simulate a board placement of ships given current conditions,
 read that board placement to an outside vector,
 and clear the board to allow the process to occur again
 all as efficiently as possible.
 Specific details are present in the individual functions' documentation
 */

class Point;
class Player;

class Possibilities_Board
{
    struct Possible_Location;
  public:
    Possibilities_Board(const Game& g);
    void update(Point p, char c);
    void ship_destroyed(int shipId);
    bool is_ship_destroyed(int shipId) const;
    void read_to(std::vector<int>& data) const;
    void determine_locations();
    bool is_valid_board() const;
    bool place_ships();
    void unplace_all_ships();
  private:
    bool place_ships_recursively(int shipId, bool sunks, size_t which_sunk ); 
    bool place_ship(Possible_Location L);
    bool unplace_ship(Possible_Location L);
    bool is_valid(Possible_Location L) const;
    const Game& m_game;
    std::vector<char> board;
    std::vector<char> refrence_board;
    std::vector<int> destroyed_ships;
    std::vector<std::vector<Possible_Location>> locations_list;
};

struct Possibilities_Board::Possible_Location
{
    Possible_Location(Point _topOrLeft, int _shipId, Direction _dir);
    Point topOrLeft;
    int shipId;
    Direction dir;
};


#endif /* Possibilities_h */
