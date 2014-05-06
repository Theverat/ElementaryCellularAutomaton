#ifndef ECA_H
#define ECA_H

#include <vector>

//ECA = Elementary Cellular Automaton
class ECA
{
public:
    ECA();
    ECA(std::vector<bool> init, std::vector<bool> rules);
    std::vector< std::vector<bool> > getState();
    void computeNextGeneration(unsigned int numGenerations);

private:
    bool useRule(bool leftCell, bool cell, bool rightCell);

    std::vector< std::vector<bool> > state;
    std::vector<bool> rules;
};

#endif // ECA_H
