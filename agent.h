#ifndef AGENT_H
#define AGENT_H
#include "definitions.h"
#include <vector>
#include <queue>
#include <set>


class Agent {
  public:
    Agent(int random_seed);
    Action GetAction(Percept p);
    // 0 = unexplored, 1 = wall, 2= space,
    int map[40][40] = { 0 }; //internal map
    int myx = 20; //where you are
    int myy = 20; //you start in the middle of your map
    int myvx; //where you plan to be
    int myvy;
    const int homex = 20;// you always start at hame
    const int homey = 20;
    Heading myHeading = NORTH; //which way you are facing
    Heading myvHeading; //wich way you plan to be facing
    Action lastAct = SUCK; //the last thing you did.
    bool checkExplored(); //unused
    void updatePosMap(Percept p); //updates you position and your internal map
    void updateCache();
    bool makevMove(Action a); //think about taking an action. only affects myvx, myvy, and myvHeading
    bool evaluatePlan(std::vector<std::tuple<Action, int, int, Heading>> plan, bool findHome,
        std::queue<std::vector<std::tuple<Action, int, int, Heading>>>* pQueue, std::set<std::tuple<Action, int, int, Heading>>* pClosed);
    std::queue<Action> moveCache; //the final plan
};

#endif

