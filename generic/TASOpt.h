#include <set>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include "Map.h"
#include <cmath>
#include <algorithm>
#include "MR1PermutationPDB.h"
#include <math.h>



template<class State>
struct OpenClosedData
{
public:

    double g = 0;
    State parent;
    int loc = -1;
};

template <class Env, class State>
class TASOptFrontier
{
public:
    Env *env;
	bool isIdle;
	unsigned int seed;
	std::vector<State> candidates;
	std::vector<State> otherCandidates;
	std::vector<State> children;
	std::vector<State> open;
	std::set<uint64_t> closed;
	//std::unordered_map<uint64_t, double> gValues;
	//std::unordered_map<uint64_t, State> parents;
	//std::unordered_map<uint64_t, int> loc;
    std::unordered_map<uint64_t, OpenClosedData<State>> openClosed;
	std::vector<int> occurrences;
	std::vector<int> _occurrences;
	int numOfExp = 0;
	State rendezvous;
	TASOptFrontier *other;
    int sampleCount;
	State anchor;
	State start;
    Heuristic<State> *h;
	TASOptFrontier(Env *_env, State _start, Heuristic<State> *h, int _sampleCount);
	~TASOptFrontier(){}
	bool DoSingleSearchStep();
	void GetPath(std::vector<State> &path);
	void SetSeed(unsigned int seed);
	std::vector<State> GetPath(State node, bool forward);
	void ExtractPath(std::vector<State> &path);
	bool validSolution;
    double anchorH, anchorG;

	int internalSeed = 10;

	//double comps = 0;



	double AdditiveDH(Heuristic<State> *heu, State s1, State s2)
	{
		double res = 0;
	    for (int i = 0; i < heu->heuristics.size(); i++)
	    {
	    	res += fabs(heu->heuristics[i]->HCost(s1, s1) - heu->heuristics[i]->HCost(s2, s2));
	    }
		return res;
	}

	double MaxDH(State s1, State s2)
	{
		return max(AdditiveDH(h, s1, s2), AdditiveDH(other->h, s1, s2));
	}

	double HCost(State s1, State s2)
	{
		//return abs(h->HCost(s1, s1) - h->HCost(s2, s2)) + abs(other->h->HCost(s1, s1) - other->h->HCost(s2, s2));

		return h->HCost(s1, s2); 
		//return MaxDH(s1, s2);
	}

};


template <class Env,  class State>
TASOptFrontier<Env, State>::TASOptFrontier(Env *_env, State _start, Heuristic<State> *_h, int _sampleCount)
{
    sampleCount = _sampleCount;
	env = _env;
	open.resize(0);
	//open.reserve(env->GetMaxHash());
	h = _h;
	start = _start;
    openClosed.clear();
    //openClosed.reserve(1000000);
    //openClosed.max_load_factor(3.0);
    //openClosed.max_load_factor(0.5);
    //gValues.reserve(1000000);
    //loc.reserve(1000000);
    //parents.reserve(1000000);
    //gValues.max_load_factor(3.0);
    //loc.max_load_factor(3.0);
    //parents.max_load_factor(3.0);
	open.push_back(start);
	//gValues[env->GetStateHash(start)] = 0;
	//loc.clear();
	//loc[env->GetStateHash(start)] = 0;
	//loc.insert({env->GetStateHash(start), 0});
    OpenClosedData<State> data;
    data.g = 0;
    data.loc = 0;
    data.parent = start;
    //openClosed[env->GetStateHash(start)] = data;
    openClosed.insert({env->GetStateHash(start), data});
	anchor = start;
	numOfExp = 0;
	validSolution = true;
	internalSeed = 10;
}


template <class Env,  class State>
void TASOptFrontier<Env, State>::ExtractPath(std::vector<State> &path)
{
	auto current = rendezvous;
	while (true)
	{
		path.push_back(current);
		if (openClosed.find(env->GetStateHash(current)) != openClosed.end())
			current = openClosed[env->GetStateHash(current)].parent;
		else
			break;
	}
	path.push_back(start);
}




template <class Env,  class State>
bool TASOptFrontier<Env, State>::DoSingleSearchStep()
{
	if (open.size() == 0)
	{
		//std::cout << "******* No Solution Found! ********" << std::endl;
		validSolution = false;
		return true;
	}
		
	double minDist = 99999999;
	State bestCandidate = open.back();
	uint64_t bestCandidateHash = env->GetStateHash(bestCandidate);
	int _samples = sampleCount;
	int index = open.size() - 1;
    while (index >= 0 && _samples > 0)
    {
        State c = open[index];
        auto hash = env->GetStateHash(c);
		auto dist = HCost(c, other->anchor);
		if (dist < minDist || dist == minDist && openClosed[hash].g > openClosed[bestCandidateHash].g)
		{
			minDist = dist;
			bestCandidate = c;
			bestCandidateHash = env->GetStateHash(bestCandidate);
		}
        index--;
		_samples--;
    }
	open[openClosed[bestCandidateHash].loc] = open.back();
	openClosed[env->GetStateHash(open.back())].loc = openClosed[bestCandidateHash].loc;
	open.pop_back();
	openClosed[bestCandidateHash].loc = -1;
	//closed.insert(bestCandidateHash);
	//_occurrences[int(HCost(bestCandidate, other->anchor))]++;

	numOfExp++;
	children.resize(0);
	env->GetSuccessors(bestCandidate, children);
	for (State neighbor : children)
	{
		//occurrences[int(HCost(neighbor, other->anchor))]++;
		auto nhash = env->GetStateHash(neighbor);
		double g = openClosed[bestCandidateHash].g + env->GCost(bestCandidate, neighbor);
        auto ent = openClosed.find(nhash);
		if (ent != openClosed.end())
		{
			if (g < ent->second.g)
			{
				ent->second.g = g;
				ent->second.parent = bestCandidate;
			}
			else if (ent->second.loc != -1)
			{
				open[ent->second.loc] = open.back();
			    openClosed[env->GetStateHash(open.back())].loc = ent->second.loc;
			    open[open.size() - 1] = neighbor;
			    ent->second.loc = open.size() - 1;
			}
		}
		else
		{
			//gValues[nhash] = g;
			open.push_back(neighbor);
            OpenClosedData<State> data;
            data.g = g;
            data.loc = open.size() - 1;
            data.parent = bestCandidate;
            openClosed[nhash] = data;
			//loc.insert({nhash, open.size() - 1});
			//parents[nhash] = bestCandidate;
		}
	}
    auto hh = HCost(bestCandidate, other->start);
    if (hh < anchorH)
    {
        anchor = bestCandidate;
        anchorH = hh;
        anchorG = openClosed[bestCandidateHash].g;
    }
    else if (hh == anchorH)
    {
        if (openClosed[bestCandidateHash].g < anchorG)
        {
            anchor = bestCandidate;
            anchorG = openClosed[bestCandidateHash].g;
        }
    }
	//anchor = bestCandidate;
    
	if (other->openClosed.find(bestCandidateHash) != other->openClosed.end())// || bestCandidate == other->start)
	{
		rendezvous = bestCandidate;
		other->rendezvous = rendezvous;
		return true;
	}
	return false;
}


template <class Env,  class State>
void TASOptFrontier<Env, State>::SetSeed(unsigned int seed)
{
	this->seed = seed;
}

template <class Env, class State>
class TASOpt
{
private:
	int turn = 0;
public:
	TASOptFrontier<Env, State>* ff;
	TASOptFrontier<Env, State>* bf;
	TASOpt();
	TASOpt(Env *_env, State _start, State _goal, Heuristic<State> *hf, Heuristic<State> *hb, int _sampleCount);
	~TASOpt(){}
	void Init(Env *_env, State _start, State _goal, Heuristic<State> *hf, Heuristic<State> *hb, int _sampleCount)
	{
		ff = new TASOptFrontier<Env, State>(_env, _start, hf, _sampleCount);
		bf = new TASOptFrontier<Env, State>(_env, _goal, hb, _sampleCount);
		ff->other = bf;
		bf->other = ff;
		ff.anchorH = ff.HCost(_start, _goal);
		bf.anchorH = bf.HCost(_goal, _start);
	}
	int GetPathLength()
	{

	}
	int GetNodesExpanded()
	{
		return ff->numOfExp + bf->numOfExp;
	}
	bool DoSingleSearchStep()
	{
		if (turn == 0)
		{
			turn = 1;
			return ff->DoSingleSearchStep();
		}
		else
		{
			turn = 0;
			return bf->DoSingleSearchStep();
		}
	}
	void ExtractPath(std::vector<State> &path)
	{
		auto current = bf->rendezvous;
		std::vector<State> front, back;
		while (true)
		{
			back.push_back(current);
			if (bf->openClosed.find(bf->env->GetStateHash(current))->second.parent != current)
				current = bf->openClosed[bf->env->GetStateHash(current)].parent;
			else
				break;
		}
		back.push_back(bf->start);
		current = ff->rendezvous;
		while (true)
		{
			front.push_back(current);
			if (ff->openClosed.find(ff->env->GetStateHash(current))->second.parent != current)
				current = ff->openClosed[ff->env->GetStateHash(current)].parent;
			else
				break;
		}
		front.push_back(ff->start);
		path.resize(0);
		for (int i = front.size() - 1; i >= 1; i--)
			path.push_back(front[i]);
		for (int i = 0; i < back.size(); i++)
			path.push_back(back[i]);
	}
	void GetPath(std::vector<State> &path)
	{
		path.resize(0);
		while (true)
		{
			if (DoSingleSearchStep())
			{
				break;
			}
		}
		if (!ff->validSolution || !bf->validSolution)
			return;
        //std::cout << "Found" << std::endl;
		ExtractPath(path);
		//std::cout << "-------------" << std::endl;
		//for (int i = 0; i < 100; i++)
		//{
		//	std::cout << i << ": " << ff->occurrences[i] + bf->occurrences[i] << std::endl;
		//}
		//std::cout << "-------------" << std::endl;
		//for (int i = 0; i < 100; i++)
		//{
		//	std::cout << i << ": " << ff->_occurrences[i] + bf->_occurrences[i] << std::endl;
		//}
		//std::cout << "-------------" << std::endl;
	}
	void SetSeed(unsigned int seed)
	{
		this.seed = seed;
	}
};


template <class Env, class State>
TASOpt<Env, State>::TASOpt(Env *_env, State _start, State _goal, Heuristic<State> *hf, Heuristic<State> *hb, int _sampleCount)
{
	ff = new TASOptFrontier<Env, State>(_env, _start, hf, _sampleCount);
	bf = new TASOptFrontier<Env, State>(_env, _goal, hb, _sampleCount);
	ff->other = bf;
	bf->other = ff;
}
