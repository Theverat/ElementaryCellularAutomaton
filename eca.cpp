#include "eca.h"

ECA::ECA() {
    this->state.reserve(1);
    for(int i = 0; i < 8; i++) {
        this->rules.push_back(false);
    }
}

ECA::ECA(std::vector<bool> init, std::vector<bool> rules) {
    this->state.push_back(init);
    this->rules = rules;
}

std::vector< std::vector<bool> > ECA::getState() {
    return this->state;
}

void ECA::computeNextGeneration(unsigned int numGenerations) {
    std::vector<bool> nextGen;

    for(unsigned int gen = 0; gen < numGenerations; gen++) {
        std::vector<bool> currentGen = state.back();

//#pragma omp parallel for shared(nextGen, currentGen) //OpenMP, doesn't work
        for(unsigned int i = 0; i < currentGen.size(); i++) {
            bool leftCell, cell, rightCell;

            //if cell is at beginning of list, use cell at end of list as left neighbour
            if(i == 0)
                leftCell = currentGen.back();
            else
                leftCell = currentGen.at(i - 1);

            cell = currentGen.at(i);

            //if cell is at end of list, use cell at beginning of list as right neighbour
            if(i == currentGen.size() - 1)
                rightCell = currentGen.at(0);
            else
                rightCell = currentGen.at(i + 1);

            //calculate if cell is alive and write new state to the new generation
            nextGen.push_back(useRule(leftCell, cell, rightCell));
        }

        //add new generation ("line") to state
        state.push_back(nextGen);
        nextGen.clear();
    }
}

//tests which of the 8 rules applies and reads it from the ruleset
bool ECA::useRule(bool leftCell, bool cell, bool rightCell) {

    if(leftCell && cell && rightCell)
        return rules.at(0);
    if(leftCell && cell && !rightCell)
        return rules.at(1);
    if(leftCell && !cell && rightCell)
        return rules.at(2);
    if(leftCell && !cell && !rightCell)
        return rules.at(3);
    if(!leftCell && cell && rightCell)
        return rules.at(4);
    if(!leftCell && cell && !rightCell)
        return rules.at(5);
    if(!leftCell && !cell && rightCell)
        return rules.at(6);
    if(!leftCell && !cell && !rightCell)
        return rules.at(7);

    return false;
}
