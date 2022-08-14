#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include "Possibilities.h"
#include <iostream>
#include <string>

using namespace std;

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
  public:
    AwfulPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                                bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
  private:
    Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
 : Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
      // Clustering ships is bad strategy
    for (int k = 0; k < game().nShips(); k++)
        if ( ! b.placeShip(Point(k,0), k, HORIZONTAL))
            return false;
    return true;
}

Point AwfulPlayer::recommendAttack()
{
    if (m_lastCellAttacked.c > 0)
        m_lastCellAttacked.c--;
    else
    {
        m_lastCellAttacked.c = game().cols() - 1;
        if (m_lastCellAttacked.r > 0)
            m_lastCellAttacked.r--;
        else
            m_lastCellAttacked.r = game().rows() - 1;
    }
    return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
      // AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
      // AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
    bool result(cin >> r >> c);
    if (!result)
        cin.clear();  // clear error state so can do more input operations
    cin.ignore(10000, '\n');
    return result;
}


class HumanPlayer : public Player
{
  public:
    HumanPlayer(string nm, const Game& g);
    bool isHuman() const override {return true;};
    bool placeShips(Board& b) override;
    Point recommendAttack() override;
    void recordAttackResult(Point p, bool validShot, bool shotHit,
                            bool shipDestroyed, int shipId) override {return;};
    void recordAttackByOpponent(Point p) override {return;};
  private:
};


HumanPlayer::HumanPlayer(string nm, const Game& g) : Player(nm, g) {}


bool HumanPlayer::placeShips(Board &b) {
    bool badPlacement = 0; //test for bad placements
    unsigned int attempts = 0;
    // like mediocre player, we give a limit on number of tries
    // after 5 bad placements, we assume the board is incompatible with the ships and return false
    
    do {
        badPlacement = 0;
        for (size_t i = 0, N = game().nShips(); i < N; ++i) {
            bool failtest = 0; //storing results of cin.fails
            int r, c = 0;
            string d;
            Direction dir;
            
            // getting the position
            do {
                failtest = 0;
                cout << "Where would you like to place your " << game().shipName(static_cast<int>(i)) << "?" << endl;
                cout << "(Input two integers designating the upper/left row & column)" << endl;
                if (!getLineWithTwoIntegers(r, c)) {
                    cout << "Invalid input, please Try Again." << endl;
                    failtest = 1;
                };
                
            }
            while (failtest); // should only run repeatedly if cin fails
            
            
            // getting the orientation
            do {
                failtest = 0;
                cout << "Would you like its orientation to be VERTICAL or HORIZONTAL?" << endl;
                cin >> d;
                if      (d == "VERTICAL")   {dir = VERTICAL;}
                else if (d == "HORIZONTAL") {dir = HORIZONTAL;}
                else {
                    cout << "Invalid input, please Try Again./nRespond with either VERTICAL or HORIZONTAL." << endl;
                    cin.clear();
                    cin.ignore(10000, '\n');
                    failtest = 1;
                }
            }
            while (failtest);
            
            // placing the ship, attempts counter only counts this section
            if (!(b.placeShip(Point(r, c), static_cast<int>(i), dir))){
                cout << "The ship does not fit in that location. Restarting ship placement" << endl;
                b.clear();
                badPlacement = 1;
                ++ attempts;
                if (attempts < 5) {break;}  // exits the for-loop, triggering the main do-while loop to restart
                else {
                    cout << "You took many attempts." << endl;
                    cout << "Either the board is invalid, or you are just garbage at battleship." << endl;
                    return false;
                }
            }
        }
    }
    while (badPlacement);
    return true;
};



Point HumanPlayer::recommendAttack() {
    int r, c;
    bool failtest = 0;
    
    do {
        failtest = 0;
        cout << "Where would you like to attack?" << endl;
        cout << "(Input two integers designating the row & column)" << endl;
        if (!getLineWithTwoIntegers(r, c)) {
            cout << "Invalid input, please Try Again." << endl;
            failtest = 1;
        };
    }
    // non input errors with attacking (e.x. outside of location or previously attacked)
    // are handled in the Board::attack() and Game::play()
    while (failtest == 1);
    return Point(r, c);
}







//*********************************************************************
//  MediocrePlayer
//*********************************************************************

/*
 -Slight improvement from class instructions-
 
 I am using the mediocre player's attacking to implement a superior Hunt+Parity Algorithm
 This algorithm can be explained here:
 https://towardsdatascience.com/coding-an-intelligent-battleship-agent-bf0064a4b319
 https://youtu.be/LbALFZoRrw8
 
 I apply the parity in the random shooting aspect of the Hunt algorithm, as usual.
 The main difference is some optimizations I make to the targeting in the Hunt algorithm.
 In short, the targetting is more efficient, and we don't stop targeting until we are sure to do so
 In this manner, I have combined the better aspects of both the class' and Hunt algorithms
 More details are available in recomendAttack()

 With regards to placing ships, the only variation is an addition of more randomness.
 Only relying on blocking half the squares resulted in high ship density in the upper left on 10X10 boards.
 */

class MediocrePlayer : public Player
{
  public:
    MediocrePlayer(string nm, const Game& g);
    bool placeShips(Board& b) override;
    Point recommendAttack() override;
    void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) override;
    void recordAttackByOpponent(Point p) override {return;};
  private:
    unsigned int state;
    Point first_hit;
    Point previous_shot;
    unsigned int expected_hits;
    vector<Point> successful_hits;
    vector<Point> hits_of_interest;
    size_t next_shot_index;
    vector<Point> next_shots;
};

MediocrePlayer::MediocrePlayer(string nm, const Game& g) : Player(nm, g), state(1), next_shot_index(0), expected_hits(0) {
    successful_hits = {};
    next_shots = {
        Point(-1, 0), Point(0, 1), Point(1, 0), Point(0, -1),
        Point(-2, 0), Point(0, 2), Point(2, 0), Point(0, -2),
        Point(-3, 0), Point(0, 3), Point(3, 0), Point(0, -3),
        Point(-4, 0), Point(0, 4), Point(4, 0), Point(0, -4),
        Point(-5, 0), Point(0, 5), Point(5, 0), Point(0, -5),
        Point(-6, 0), Point(0, 6), Point(6, 0), Point(0, -6),
        Point(-7, 0), Point(0, 7), Point(7, 0), Point(0, -7),
        Point(-8, 0), Point(0, 8), Point(8, 0), Point(0, -8),
        Point(-9, 0), Point(0, 9), Point(9, 0), Point(0, -9),
    }; //max ship length is 10, so would be a maximum of 36 of these shots to sink it after 1 hit
};


bool placeShipsRecursively(Player &p, Board &b, int shipId) {
    /*
     PlaceShipsRecursively is designed as a helper functions for place ships,
     taking input the player and board by refrence, and the specific ship to be placed
     If placement occurs successfully, placeShipsRecursively will call itself with the next ship
     If this recursion returns true, placeShipsRecursively will return true
     If this recursion returns false, placeShipsRecursively will try a new placement
     If placement is unsuccessful for all possible locations, placeShipsRecursively will return false,
     If the shipId is larger than any ships, then placeShipsRecursively will return true, ending the recursion
     
     This function is explained in more detail in the project requirements
     */
    
    // return condition- shipId too large. i.e. all ships have been placed
    if (shipId >= p.game().nShips()) {return true;}

    // I found that even with blocking half, ships mostly were placed in the top left of the board
    // So I added in another randomness element, the loop starts at a random position on the board
    int board_size = p.game().rows()*p.game().cols();
    int random_shift = randInt(board_size);
    // loop through the board. If a ship is placed, recursively call the function on the next ship
    for (size_t i = random_shift, N = board_size + random_shift; i < N; ++i) {
        Point location;
        if (i < board_size) { //making sure the index is valid on our board
            location = Point(static_cast<int>(i) / p.game().cols(), static_cast<int>(i) % p.game().cols());
        }
        else {
            location = Point(static_cast<int>(i - board_size) / p.game().cols(), static_cast<int>(i - board_size) % p.game().cols());
        }
        
        if (b.placeShip(location, shipId, HORIZONTAL)) {
            if (!placeShipsRecursively(p, b, shipId + 1)) {                 // if the recursion didnt complete with this placement
                b.unplaceShip(location, shipId, HORIZONTAL);                // unplace the ship, and try another placement
            }
            else {return true;}                                             // if it did complete, we are done
        }
        if (b.placeShip(location, shipId, VERTICAL)) {
            if (!placeShipsRecursively(p, b, ++shipId)) {
                b.unplaceShip(location, shipId, VERTICAL);
            }
            else {return true;}
        }
    }
    
    // entire loop without entering recursion-> must backtrack
    return false;
    
}


bool MediocrePlayer::placeShips(Board &b) {
    // real work done by helper function - bool placeShipsRecursively(Player &p, Board &b, int shipId)
    // if that function succeeds at placing the ships within 50 attempts, this function returns true
    unsigned int attempts = 0;
    while (attempts < 50) {
        b.block();
        bool success = placeShipsRecursively(*this, b, 0);
        b.unblock();
        if (success) {return true;}
    }
    return false;
}

// being able to add points is helpful here. Operator overloading is not ideal here due to project instructions not to change .h files
Point add(Point a, Point b) {return Point(a.r + b.r, a.c + b.c);}


bool search (Point p, const vector<Point>& v) {
    // Searching through a vector of points
    for (size_t i = 0, N = v.size(); i < N; ++i) {
        if (v[i].r == p.r && v[i].c == p.c) {return true;}
    }
    return false;
}


Point MediocrePlayer::recommendAttack() {
    /*
     recomendAttack for mediocreplayer has three states, unlike the class and hunt algorithm which have two
     The first state is random shooting, with parity
     The second state is hunting based on a specific hit, but not in the way the class or hunt algorithms do
     The third state is cleanup, if after a sink there are still unaccounted for hits on the board,
     the algorithm recomends shots around them rather than return to random shooting
     
     How the changes between states are addressed is explained in recordAttackResult
     As well as more specificities as to the algorithm employed in state 2
     */
    Point recomendation;
    
    if (state == 1) {
        do { recomendation = game().randomPoint();}
        while ((recomendation.r + recomendation.c) % 2 == 0); // ensuring that my guess is odd on the board
    }
    else if (state == 2) {
        recomendation = add(first_hit, next_shots[next_shot_index]);
    }
    else { //state == 3
        Point interest = hits_of_interest[hits_of_interest.size() - 1]; // using the end makes cleanup easier
        recomendation = add(interest, next_shots[next_shot_index]);
    }
    return recomendation;
}

void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) {
    /*
     recordAttackResult adjusts the member variables of mediocre player after each attack
     so that recommend attack will work properly
     With many possibilities and edge cases, it is easiest to explain this functions actions line by line.
     
     The member variables it adjusts are
     unsigned int state            : which state of attacking we are in, explained and employed by recommendAttack()
     Point previous_shot           : stores the shot before the one we are recording
     vector<Point> successful_hits : stores all successful hits
     unsigned int expected_hits    : stores expected hits, i.e. the length of all the sunk ships
     vector<Point> hits_of_interest: hits where interesting things happened: first hit, changing direction within state 2, etc.
     Point first_hit               : a hit while shooting was random; state 2 bases its recomendations off this location
     size_t next_shots             : a vector with all the possible locatiosn for a ships' cells, in relation to its first hit
     vector<Point> next_shot_index : which element of next shots we are using currently
     */

    if (state == 1) {                                           // while we are in state 1...
        if (!validShot) {                                       // invalid shot, do nothing
            previous_shot = p;                                  // before return, set previous shot
            return;
        }
        
        if (shotHit) {                                          // shot misses, stay in state 1 (do nothing_
            successful_hits.push_back(p);                       // shot hits, store successful hit
            
            if (!shipDestroyed) {                               // shot hits but doesn't destroy
                state = 2;                                      // go to state 2
                first_hit = p;                                  // and store the hit that caused the change
                hits_of_interest.push_back(p);                  // first hit is a hit of interest
            }
            else {                                              // shot hit and destroyed,
                expected_hits += game().shipLength(shipId);     // add the ships length
                if (expected_hits < successful_hits.size()) {   // if we have more hits than expected
                    state = 3;                                  // go to state 3
                }                                               // otherwise stay in state 1
            }                                                   // with the optimizations, this last case should never occur
            next_shot_index = 0;
        }
    }
    
    else if (state == 2 ) {                                     // while we are in state 2...
        if (!validShot) {
            if (search(p, successful_hits)) {                   // invalid shot-> 3 cases
                next_shot_index+=4;                             // 1) already shot and hit... skip and continue in that direction
            }                                                   // 2) already shot and missed... go in next direction
            else {                                              // 3) over edge... go in next direction
                next_shot_index = (next_shot_index + 1) % 4;
                hits_of_interest.push_back(previous_shot);      // change of direction cases stored for optimization
            }
            previous_shot = p;                                  // before return, set previous shot
            return;
        }
        if (!shotHit) {
            next_shot_index = (next_shot_index + 1) % 4;        // shot misses, go to the next index at the lowest clockwise spot
            hits_of_interest.push_back(previous_shot);          // change of direction cases stored for optimization
        }
        else {
            successful_hits.push_back(p);                       // store successful hit and ships length
            if (shipDestroyed) {                                // shot hits and destroys
                expected_hits += game().shipLength(shipId);     // add ships length
                if (expected_hits < successful_hits.size()) {   // we have more hits than expected
                    state = 3;
                }
                else {                                          // sunk ship was expected size
                    state = 1;                                  // return to state 1
                    hits_of_interest = {};                      // and clear hits of interest

                }
                next_shot_index = 0;                            // reset the shot index
                
            }
            else {                                              // shot hits and doesn't destroy
                next_shot_index +=4;                            // and increase shot index by 4,
            }                                                   // coresponding to proceeding linearly from the hit
        }
    }
    else {                                                      // while we are in state 3
        if (!validShot || !shotHit) {                           // if invalid or missed,
            if (next_shot_index < 3) {                          // and still within cell away
                ++next_shot_index;                              // increase the index
                previous_shot = p;                              // before return, set previous shot
                return;
            }
            else {                                              // if no longer within 1 cell
                hits_of_interest.pop_back();                    // that hit was not interesting
                next_shot_index = 0;                            // reset the shot index
                if (hits_of_interest.size() == 0) {             // we have run out of obviously interesting hits
                    hits_of_interest = successful_hits;         // make every hit interesting
                }
            }
        }
        else if (shotHit) {                                     // shot hits
            successful_hits.push_back(p);                       // store successful hit
            if (!shipDestroyed) {                               // shot hits but doesn't destroy
                state = 2;                                      // go to state 2
                first_hit = p;                                  // and store the hit that caused the change
                hits_of_interest.push_back(p);                  // first hit is a hit of interest
            }
            else {                                              // shot hit and destroyed a ship
                expected_hits += game().shipLength(shipId);     // add ships length
                if (expected_hits < successful_hits.size()) {   // we still have more hits than expected, stay in state 3
                    state = 3;
                }
                else {                                          // we have the correct amount of hits
                    state = 1;                                  // return to state 1
                    hits_of_interest = {};
                }
            next_shot_index = 0;                                // reset the shot index
            }
        }
    }
    
    previous_shot = p;
}

//*********************************************************************
//  GoodPlayer
//*********************************************************************

/*
 Good Player is my attempt at the best Battleship NPC
 using the best methods I know of.
 Placement is done using MediocrePlayer's placement,
 which already ensures random placement
 Attacking is similar to the "battleship algorithm" found here:
 https://towardsdatascience.com/coding-an-intelligent-battleship-agent-bf0064a4b319
 https://youtu.be/LbALFZoRrw8
 While I employ the calculation slightly differently (better, in my opinion), the goal is similar.
 Given the current shots, randomly select the cell most likely to contain a ship.
 The placement trials occur within a possibilities board, an original helper class
 
 In total, for a standard game, GoodPlayer's attacking functionality is able to clear a random board in ~43 turns
 which is about as good as I have seen, and it doesn't require any artificial adjustments to the statistical algorithm.
 */

class GoodPlayer : public MediocrePlayer
{
  public:
    GoodPlayer(string nm, const Game& g);
    Point recommendAttack() override;
    void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) override;
    void recordAttackByOpponent(Point p) override {return;};
    
  private:
    vector<int> data;
    Possibilities_Board possibilities;
};


//========================================================================
// Timer t;                 // create a timer and start it
// t.start();               // start the timer
// double d = t.elapsed();  // milliseconds since timer was last started
//========================================================================

#include <chrono>

class Timer
{
  public:
    Timer() { start();}
    void start() { m_time = std::chrono::high_resolution_clock::now();}
    double elapsed() const {
        std::chrono::duration<double,std::milli> diff = std::chrono::high_resolution_clock::now() - m_time;
        return diff.count();
    }
  private:
    std::chrono::high_resolution_clock::time_point m_time;
};


GoodPlayer::GoodPlayer(string nm, const Game& g) : MediocrePlayer(nm, g), data(g.rows()*g.cols(), 0), possibilities(g) {};

Point GoodPlayer::recommendAttack() {

    /*
     Overall recommendAttack() randomly selects 10,000 possibilities of where remaining ships may be placed, if it can
     Then, counting up in those 10,000 possible boards which cell contains a ship most often, it returns that cell
     The 100,000 possible boards are each instances of the Possible_Boards class
     
     Some notes:
     With over 30 billion possibilities for ship placements, and possibly only 1 valid arrangement (say, on the last turn)
     checking a placements validity after creating it is too computationally expensive
     The high level algorithm suggested in the articles I cited for Hunt+Parity avoid this by not selecting entire placements
     Rather, they calculate the likelyhood of a cell containing a ship individually for each ship,
     and then adding them up and artificually biasing the algorithm for squares around hit cells
     I address the issue by making a list of valid placements for each ship to chose from before simulating random placements
     This, combined with other optimizations, makes it far less likely a placement fails the checks at the end
     And makes the algorithm computationally feasible.
     Even with 1,000,000 simulations, the algorithm never took more than the allotted 4 seconds for an attack for any game simulated
     If it did, the timer would force a break after 3.9 seconds, and the algorithm would return the best cell up to that point
     
     Actual calculations for most likely cell are explained and done in the functions of Possibilities_Board
     */
    possibilities.determine_locations();
    
    size_t i = 0;
    Timer timer;
    while (i < 100000) {
        if (i % 20 == 0) { // calling timer.elapsed takes time itself, so only checking every 20 simulations
            if (timer.elapsed() >= 3900) {cout << "TIMER FORCED BREAK" << endl; break;} // break if close to 4 second limit
        }
        if (timer.elapsed() >= 3900) {cout << "TIMER FORCED BREAK" << endl; break;}     // break if close to 4 second limit
        if (!possibilities.place_ships()) {++i; continue;}                              // recursion within possibilities, continue if failed
        if (possibilities.is_valid_board()) {possibilities.read_to(data);}              // update board
        ++i;
        possibilities.unplace_all_ships();
    }
    
    int max = data[0];
    size_t cell = 0;
    for (size_t i = 0, N = data.size(); i < N; ++i) {
        if (data[i] > max) {max = data[i]; cell = i;}
    }
    
    for (size_t i = 0, N = data.size(); i < N; ++i) { data[i] = 0;}
    
    if (max == 0) { cell = static_cast<size_t>(randInt(100));} // failsafe to avoid complete crash
    return Point(static_cast<int>(cell) / game().cols(), static_cast<int>(cell) % game().cols());
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId) {
    /*
     recordAttackResult does far less work than it does for medicore player
     It simply updates the possibilities board with the result of an attack
     Using the ships symbol rather than just a hit if the ship was sunk
     */
    if (!validShot) { return;}
    if (!shotHit) {
        possibilities.update(p, 'o');
    }
    else {
        if (shipDestroyed) {
            possibilities.ship_destroyed(shipId);
            possibilities.update(p, game().shipSymbol(shipId));
        }
        else {possibilities.update(p, 'X');}
    }
    return;
}


//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g) {
    static string types[] = {
        "human", "awful", "mediocre", "good"
    };
    
    int pos;
    for (pos = 0; pos != sizeof(types)/sizeof(types[0])  && type != types[pos]; pos++);
    
    switch (pos)
    {
      case 0:  return new HumanPlayer(nm, g);
      case 1:  return new AwfulPlayer(nm, g);
      case 2:  return new MediocrePlayer(nm, g);
      case 3:  return new GoodPlayer(nm, g);
      default: return nullptr;
    }
}
 
