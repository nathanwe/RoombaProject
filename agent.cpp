#include <ctime> //for random seed
#include <cmath> 
#include "agent.h"
#include <queue>
#include <vector>
#include <tuple>
#include <list>
#include <unordered_map>
#include <map>
#include <set>
Agent::Agent(int random_seed) { 
  //supplying your own seed may help debugging, same seed will cause 
  //same random number sequence 
  if (random_seed==0) std::srand( static_cast<unsigned>(std::time(0))); // random seed from time
  else                std::srand( random_seed ); // random seed from user
} 
// if there is not a plan in moveCache, make one. then execute the plan
Action Agent::GetAction(Percept p) {
    updatePosMap(p);
    if (moveCache.empty()) {
        updateCache();
    }
    if (p.dirt) {
        lastAct = SUCK;
        return SUCK;
    }
    Action ret = moveCache.front();
    lastAct = ret;
    moveCache.pop();
    return ret;
    
}
//build your internal map of the world
//0 = unexplored, 1 = wall, 2= floor
void Agent::updatePosMap(Percept p) {
    if (lastAct == SUCK) {
        map[myx][myy] = 2;
    }
    if (lastAct == LEFT) {
        myHeading = (Heading)(((int)myHeading + 3)% 4);
        map[myx][myy] = 2;
    }
    if (lastAct == RIGHT) {
        myHeading = (Heading)(((int)myHeading + 1) % 4);
        map[myx][myy] = 2;
    }
    if (lastAct == FORWARD) {
        int forwardx = myx;
        int forwardy = myy;
        if (myHeading == NORTH) {
            forwardy++;
        }
        if (myHeading == EAST) {
            forwardx++;
        }
        if (myHeading == SOUTH) {
            forwardy--;
        }
        if (myHeading == WEST) {
            forwardx--;
        }
        if (p.bump) {
            map[forwardx][forwardy] = 1;
        }
        if (!p.bump) {
            myx = forwardx;
            myy = forwardy;
            map[myx][myy] = 2;
        }
    }

}
//make a plan
void Agent::updateCache() {
    
    //list of plans to try
    std::queue<std::vector<std::tuple<Action, int, int, Heading>>>planQueue;

    //if I have planed to do an action in a square facing a direction, record it.
    std::set< std::tuple<Action, int, int, Heading>> closed_list;
    closed_list = std::set< std::tuple<Action, int, int, Heading>>();

    //try to reach an unexplored square
    planQueue.push(std::vector<std::tuple<Action, int, int, Heading>>());
    while (!planQueue.empty()) {
        bool done = evaluatePlan(planQueue.front(), false, &planQueue, &closed_list);
        if (done) {
            for (std::tuple<Action, int, int, Heading> a : planQueue.front()) {
                moveCache.push(std::get<0>(a));
            }
            return;
        }
        planQueue.pop();
    }

    //if you cant reach an unexplored square, go home
    planQueue.push(std::vector<std::tuple<Action, int, int, Heading>>());
    closed_list.clear();
    while (!planQueue.empty()) {
        bool done = evaluatePlan(planQueue.front(), true, &planQueue, &closed_list);
        if (done) {
            for (std::tuple<Action, int, int, Heading> a : planQueue.front()) {
                moveCache.push(std::get<0>(a));
            }
            moveCache.push(SHUTOFF);
            return;
        }
        planQueue.pop();
    }
    
}

//pretend to make a move. the v stands for virtual
//if you hit a wall return true
bool Agent::makevMove(Action a) {

    if (a == FORWARD) {
        int forwardx = myvx;
        int forwardy = myvy;
        if (myvHeading == NORTH) {
            forwardy++;
        }
        if (myvHeading == EAST) {
            forwardx++;
        }
        if (myvHeading == SOUTH) {
            forwardy--;
        }
        if (myvHeading == WEST) {
            forwardx--;
        }
        myvx = forwardx;
        myvy = forwardy;
        if (map[myvx][myvy] == 1) {
            return true;
        }
    }
    if (a == LEFT) {
        myvHeading = (Heading)(((int)myvHeading + 3) % 4);
    }
    if (a == RIGHT) {
        myvHeading = (Heading)(((int)myvHeading + 1) % 4);
    }
    return false;
}

// deal with a plan from the queue
//findHome depends of if you are trying to find an enexplored square or home
//returns true if the plan finds what its looking for
//side effects the planQueue and the closed list
bool Agent::evaluatePlan(std::vector<std::tuple<Action, int, int, Heading>> plan, bool findHome,
    std::queue<std::vector<std::tuple<Action, int, int, Heading>>>* pQueue, std::set<std::tuple<Action, int, int, Heading>>* pClosed) {
    
    //try the plan
    myvx = myx;
    myvy = myy;
    myvHeading = myHeading;
    bool hitWall = false;
    for (std::tuple<Action, int, int, Heading> a : plan) {
        hitWall = makevMove(std::get<0>(a)) || hitWall;
    }
    //if the plan hits a wall, it sucks
    if (!hitWall) {

        //plan sucess checks
        if (!findHome && map[myvx][myvy] == 0) {
            return true;
        }
        if (findHome && myvx == homex && myvy == homey) {
            return true;
        }

        //add new plans to the queue
        std::vector<std::tuple<Action, int, int, Heading>> nextPlan = plan;
        std::tuple<Action, int, int, Heading> nextNode;

        //add this plan, but with forward at the end
        nextNode = std::tuple<Action, int, int, Heading>(FORWARD, myvx, myvy, myvHeading);
        if (pClosed->count(nextNode) == 0) { //if any other plan has added this move at this place facing this way to the end, this pland does not
            pClosed->insert(nextNode);
            nextPlan.push_back(nextNode);
            pQueue->push(nextPlan);
            nextPlan.pop_back();//save on copy constuction, reuse nextPlan
        }
        //add this plan, but with left at the end
        nextNode = std::tuple<Action, int, int, Heading>(LEFT, myvx, myvy, myvHeading);
        if (pClosed->count(nextNode) == 0) {
            pClosed->insert(nextNode);
            nextPlan.push_back(nextNode);
            pQueue->push(nextPlan);
            nextPlan.pop_back();
        }
        //add this plan, but with right at the end
        nextNode = std::tuple<Action, int, int, Heading>(RIGHT, myvx, myvy, myvHeading);
        if (pClosed->count(nextNode) == 0) {
            pClosed->insert(nextNode);
            nextPlan.push_back(nextNode);
            pQueue->push(nextPlan);
            nextPlan.pop_back();
        }
        
    }
    return false;
}

//dijkstra ish
//not used now that i have the closed list
bool Agent::checkExplored() {
    std::queue<std::tuple<int, int>> open_list;
    std::set<std::tuple<int, int>> closed_list;
    open_list.push(std::tuple<int, int>(myx, myy));
    std::tuple<int, int> next;
    while (!open_list.empty()) {
        closed_list.insert(open_list.front());
        
        
        //north
        next = open_list.front();
        std::get<1>(next)++;
        if (closed_list.count(next) == 0) {
            if (map[std::get<0>(next)][std::get<1>(next)] == 0) {
                return true;
            }
            if (map[std::get<0>(next)][std::get<1>(next)] == 2) {
                open_list.push(next);
            }
            
        }
        //east
        next = open_list.front();
        std::get<0>(next)++;
        if (closed_list.count(next) == 0) {
            if (map[std::get<0>(next)][std::get<1>(next)] == 0) {
                return true;
            }
            if (map[std::get<0>(next)][std::get<1>(next)] == 2) {
                open_list.push(next);
            }
        }
        //south
        next = open_list.front();
        std::get<1>(next)--;
        if (closed_list.count(next) == 0) {
            if (map[std::get<0>(next)][std::get<1>(next)] == 0) {
                return true;
            }
            if (map[std::get<0>(next)][std::get<1>(next)] == 2) {
                open_list.push(next);
            }
        }
        //west
        next = open_list.front();
        std::get<0>(next)--;
        if (closed_list.count(next) == 0) {
            if (map[std::get<0>(next)][std::get<1>(next)] == 0) {
                return true;
            }
            if (map[std::get<0>(next)][std::get<1>(next)] == 2) {
                open_list.push(next);
            }
        }
        open_list.pop();
    }
    return false;
}
