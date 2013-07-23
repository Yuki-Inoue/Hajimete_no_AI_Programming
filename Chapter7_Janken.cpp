//
//  main.cpp
//  GeneralPurpose
//
//  Created by Yuki Inoue on 2013/06/01.
//  Copyright (c) 2013年 Yuki Inoue. All rights reserved.
//


// !! uses library of c++11.

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>
#include <functional>

std::mt19937 random_engine;


namespace JankenEnvironemnt {
    enum Action { Rock, Paper, Scissors };
    typedef Action State;

    const char *action_to_string(Action a){
        switch (a) {
            case Rock:
                return "Rock";
            case Paper:
                return "Paper";
            case Scissors:
                return "Scissors";
        }
    }

    inline int action_to_int(Action a){
        return (int) a;
    }

    inline int state_to_int(State s){
        return (int) s;
    }

    inline Action ith_action(State, int i){
        return (Action) i;
    }


    Action stronger_hand(Action a){
        switch (a) {
            case Rock:
                return Paper;
            case Paper:
                return Scissors;
            case Scissors:
                return Rock;
        }
    }

    void transition(State s, Action a, double &reward, State &s_) {
	reward =
	    a == (s_ = stronger_hand(s)) ?
	    0.0 : a == s ? -1.0 : 1.0;
    }

}



int softmax_roulette(const double *qs, int size) {

    static std::uniform_real_distribution<> udist;

    std::vector<double> exp_qs;
    transform(qs, qs+size, back_inserter(exp_qs), exp);

    const double r =
	udist(random_engine)
	* accumulate(exp_qs.begin(), exp_qs.end(), 0.0);

    double rsum = 0.0; int i = 0;
    while (true) {
        rsum += exp_qs[i];
        if (rsum >= r) break;
        ++i;
    }
    return i;
}



template <class State, class Action, int NS, int NA>
class TableQAgent {
    double Q[NS][NA];

    double learning_rate;
    double discount_rate;

public:
    typedef std::function<int(const double *, int)> Policy;
private:
    Policy policy;



public:
    TableQAgent(double alpha_, double gamma_, const Policy &policy_ = softmax_roulette)
    : learning_rate(alpha_), discount_rate(gamma_), policy(policy_)
    {
        for (int i=0; i<3; ++i)
            for (int j=0; j<3; ++j)
                Q[i][j] = 0.0;
    }

    void update(State s, Action a, double reward, State s_) {
        int si = state_to_int(s), s_i = state_to_int(s_), ai = action_to_int(a);
        Q[si][ai] =
	    (1 - learning_rate) * Q[si][ai] +
	    learning_rate * (reward + discount_rate * *std::max_element(Q[s_i], Q[s_i]+3));
    }

    Action make_action(State s) {
        return ith_action(s, policy(Q[state_to_int(s)], NA));
    }

    void print_matrix(){
        for (int i=0; i<3; ++i)
            printf("%lf %lf %lf\n", Q[i][0], Q[i][1], Q[i][2]);
    }


};












int main(int argc, const char * argv[])
{
    int N = 1000;

    TableQAgent<JankenEnvironemnt::State, JankenEnvironemnt::Action, 3, 3> agent(0.2, 0.9);

    auto s = JankenEnvironemnt::Scissors;
    auto s_ = s;
    for (int i=0; i<N; ++i) {
        printf("last hand: %s\n", action_to_string(s));

        auto a = agent.make_action(s);
        printf("play hand: %s\n", action_to_string(a));

        double reward;
        transition(s, a, reward, s_);
        printf("oppo hand: %s\n", action_to_string(s_));
        printf("reward   : %lf\n", reward);

        agent.update(s, a, reward, s_);
        puts("");

        std::swap(s, s_);
    }

    agent.print_matrix();

    return 0;
}
