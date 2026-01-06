#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>


struct NFA
{
    std::map<int, std::map<char, std::set<int>>> transitions;
    std::map<int, std::set<int>> epsilonTransitions;
    std::set<int> acceptingStates;
    int startState;

    NFA(int start) : startState(start) {}

    void addTransition(int from, char symbol, int to)
    {
        transitions[from][symbol].insert(to);
    }

    void addEpsilonTransition(int from, int to)
    {
        epsilonTransitions[from].insert(to);
    }

    void addAcceptingState(int state)
    {
        acceptingStates.insert(state);
    }

    // epsilon closure
    std::set<int> epsilonClosure(const std::set<int>& states)
    {
        std::set<int> closure = states;
        std::vector<int> stack(states.begin(), states.end());

        while (!stack.empty())
        {
            int s = stack.back();
            stack.pop_back();
            if (epsilonTransitions.count(s))
            {
                for (int nxt : epsilonTransitions[s])
                {
                    if (!closure.count(nxt))
                    {
                        closure.insert(nxt);
                        stack.push_back(nxt);
                    }
                }
            }
        }
        return closure;
    }
};


struct DFA
{
    std::map<std::set<int>, std::map<char, std::set<int>>> transitions;
    std::set<std::set<int>> acceptingStates;
    std::set<int> startState;

    void addTransition(const std::set<int>& from, char symbol, const std::set<int>& to)
    {
        transitions[from][symbol] = to;
    }

    void addAcceptingState(const std::set<int>& state)
    {
        acceptingStates.insert(state);
    }

    bool checkAcceptance(const std::string& input)
    {
        std::set<int> currentState = startState;
        for (char symbol : input)
        {
            if (transitions[currentState].count(symbol))
            {
                currentState = transitions[currentState][symbol];
            }
            else
            {
                return false;
            }
        }
        return acceptingStates.count(currentState);
    }
};



DFA convertNFAtoDFA(NFA& nfa)
{
    DFA dfa;
    std::set<int> startSet = nfa.epsilonClosure({nfa.startState});
    dfa.startState = startSet;

    std::vector<std::set<int>> dfaStates = {startSet};
    std::map<std::set<int>, bool> visited;

    for (size_t i = 0; i < dfaStates.size(); ++i)
    {
        std::set<int> currentSet = dfaStates[i];
        visited[currentSet] = true;

        for (char symbol :
                {'a','b'
                })
        {
            std::set<int> moveSet;
            for (int s : currentSet)
            {
                if (nfa.transitions[s].count(symbol))
                {
                    moveSet.insert(nfa.transitions[s][symbol].begin(),
                                   nfa.transitions[s][symbol].end());
                }
            }
            if (!moveSet.empty())
            {
                std::set<int> closure = nfa.epsilonClosure(moveSet);
                if (!visited.count(closure))
                {
                    dfaStates.push_back(closure);
                }
                dfa.addTransition(currentSet, symbol, closure);

                // FIX: check all states, not just *begin()
                for (int s : closure)
                {
                    if (nfa.acceptingStates.count(s))
                    {
                        dfa.addAcceptingState(closure);
                        break;
                    }
                }
            }
        }
    }
    return dfa;
}

//main
int main()
{
    // Build NFA for regex (ba)* b* a [a + ba(a+b)]*
    NFA nfa(0);

    // (ba)* loop
    nfa.addTransition(0, 'b', 1);
    nfa.addTransition(1, 'a', 0);

    // epsilon to b* fragment
    nfa.addEpsilonTransition(0, 2);
    nfa.addTransition(2, 'b', 2);

    // epsilon to mandatory 'a'
    nfa.addEpsilonTransition(2, 3);
    nfa.addTransition(3, 'a', 4);

    // epsilon to [a + ba(a+b)]* fragment
    nfa.addEpsilonTransition(4, 5);

    // union: branch 1 = 'a'
    nfa.addTransition(5, 'a', 6);
    nfa.addEpsilonTransition(6, 5); // loop back

    // branch 2 = ba(a+b)
    nfa.addTransition(5, 'b', 7);
    nfa.addTransition(7, 'a', 9);
    nfa.addTransition(9, 'a', 5); // baa → loop back
    nfa.addTransition(9, 'b', 5); // bab → loop back

    // accepting states
    nfa.addAcceptingState(4); // after mandatory 'a'
    nfa.addAcceptingState(5); // inside star
    nfa.addAcceptingState(6);
    nfa.addAcceptingState(9);

    // Convert to DFA
    DFA dfa = convertNFAtoDFA(nfa);

    // Test
    std::string input;
    std::cout << "Enter a string to check acceptance: ";
    std::cin >> input;

    if (dfa.checkAcceptance(input))
    {
        std::cout << "The string is ACCEPTED by the DFA.\n";
    }
    else
    {
        std::cout << "The string is REJECTED by the DFA.\n";
    }
    return 0;
}
