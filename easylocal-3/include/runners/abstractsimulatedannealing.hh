#pragma once

#include <cmath>
#include <stdexcept>
#include <algorithm>

#include "runners/moverunner.hh"
#include "helpers/solutionmanager.hh"
#include "helpers/neighborhoodexplorer.hh"

namespace EasyLocal
{

namespace Core
{

/** The Abstract Simulated annealing runner relies on a probabilistic local
 search technique whose name comes from the fact that it
 simulates the cooling of a collection of hot vibrating atoms.
 
 At each iteration a candidate move is generated at random, and
 it is always accepted if it is an improving move.  Instead, if
 the move is a worsening one, the new solution is accepted with
 time decreasing probability.
 
 The stop condition is delegated to the concrete subclasses
 
 @ingroup Runners
 */
template <class Input, class Solution, class Move, class CostStructure = DefaultCostStructure<int>>
class AbstractSimulatedAnnealing : public MoveRunner<Input, Solution, Move, CostStructure>
{
public:
    double Temperature() const { return temperature; }
    
    AbstractSimulatedAnnealing(const Input &in, SolutionManager<Input, Solution, CostStructure> &e_sm,
                               NeighborhoodExplorer<Input, Solution, Move, CostStructure> &e_ne,
                               std::string name) 
      : MoveRunner<Input, Solution, Move, CostStructure>(in, e_sm, e_ne, name)
    {
        //compute_start_temperature("compute_start_temperature", "Should the runner compute the initial temperature?", this->parameters);
        start_temperature("start_temperature", "Starting temperature", this->parameters);
        cooling_rate("cooling_rate", "Cooling rate", this->parameters);
        max_neighbors_sampled("neighbors_sampled", "Maximum number of neighbors sampled at each temp.", this->parameters);
        max_neighbors_accepted("neighbors_accepted", "Maximum number of neighbor accepted at each temp.", this->parameters);
        // if (!compute_start_temperature.IsSet())
        //    compute_start_temperature = false; // FIXME!!
        start_temperature = 0.0;
    }
protected:
    void InitializeRun() override;
    void UpdateIterationCounter();
    void SelectMove() override;
    void CompleteMove() override;
    void CompleteIteration() override;
    // parameters
    // Parameter<bool> compute_start_temperature;
    Parameter<double> start_temperature;
    Parameter<double> cooling_rate;
    Parameter<unsigned int> max_neighbors_sampled, max_neighbors_accepted;
    // state of SA
    double temperature; /**< The current temperature. */
    size_t neighbors_sampled, neighbors_accepted;
    int number_of_temperatures;
};

/*************************************************************************
 * Implementation
 *************************************************************************/

/**
 Initializes the run by invoking the companion superclass method, and
 setting the temperature to the start value.
 */
// FIXME
template <class Input, class Solution, class Move, class CostStructure>
void AbstractSimulatedAnnealing<Input, Solution, Move, CostStructure>::InitializeRun()
{
    MoveRunner<Input, Solution, Move, CostStructure>::InitializeRun();
    
    if (cooling_rate <= std::numeric_limits<double>::epsilon() || cooling_rate >= 1.0)
        throw IncorrectParameterValue(cooling_rate, "should be a value in the interval ]0, 1[");
    
    if (start_temperature > std::numeric_limits<double>::epsilon())
    {
	// std::cerr << "temperature fixed" << std::endl;
        temperature = start_temperature;
    }
    else
    {
        // Compute a start temperature by sampling the search space and computing the variance
        // according to [van Laarhoven and Aarts, 1987] (allow an acceptance ratio of approximately 80%)
        //State sampled_state(this->in);
        const unsigned int samples = 100;
        std::vector<CostStructure> cost_values(samples);
        double mean = std::numeric_limits<double>::epsilon(), variance = std::numeric_limits<double>::epsilon();
        for (unsigned int i = 0; i < samples; i++)
        {
            //this->sm.RandomState(sampled_state);
            Move mv;
            this->ne.RandomMove(*this->p_current_state, mv);
            cost_values[i] = this->ne.DeltaCostFunctionComponents(*this->p_current_state, mv);
            mean += cost_values[i].total;
        }
        mean /= samples;
        for (unsigned int i = 0; i < samples; i++)
            variance += (cost_values[i].total - mean) * (cost_values[i].total - mean) / samples;
        start_temperature = temperature = variance;
        /*From "An improved annealing scheme for the QAP. Connoly. EJOR 46 (1990) 93-100"
         temperature = min(cost_values.begin(), cost_values.end()) + (max(cost_values.begin(), cost_values.end()) - min(cost_values.begin(), cost_values.end()))/10;*/
    }
    
    // If the number of maximum accepted neighbors for each temperature is not set, default to all of them
    if (!max_neighbors_accepted.IsSet())
        max_neighbors_accepted = max_neighbors_sampled;
    
    neighbors_sampled = 0;
    neighbors_accepted = 0;
    
    number_of_temperatures = 1;
    // std::cerr << "start temp in the abstract runner " << start_temperature << std::endl;
    // std::cerr << "temperature " << temperature << std::endl;
}

/**
 A move is randomly picked.
 */
template <class Input, class Solution, class Move, class CostStructure>
void AbstractSimulatedAnnealing<Input, Solution, Move, CostStructure>::SelectMove()
{
    // TODO: it should become a parameter, the number of neighbors drawn at each iteration (possibly evaluated in parallel)
    size_t sampled;
    double t = this->temperature;
    EvaluatedMove<Move, CostStructure> em = this->ne.RandomFirst(*this->p_current_state, this->max_neighbors_sampled - neighbors_sampled, sampled, [t](const Move &mv, const CostStructure &move_cost) {
        double r = std::max(Random::Uniform<double>(0.0, 1.0), std::numeric_limits<double>::epsilon());
        return move_cost <= 0 || move_cost < (-t * log(r));
    },
                                                                 this->weights);
    this->current_move = em;
    neighbors_sampled += sampled;
    this->evaluations += sampled;
}

/**
 A move is randomly picked.
 */
template <class Input, class Solution, class Move, class CostStructure>
void AbstractSimulatedAnnealing<Input, Solution, Move, CostStructure>::CompleteMove()
{
    neighbors_accepted++;
}

/**
 At regular steps, the temperature is decreased
 multiplying it by a cooling rate.
 */
template <class Input, class Solution, class Move, class CostStructure>
void AbstractSimulatedAnnealing<Input, Solution, Move, CostStructure>::CompleteIteration()
{
    Runner<Input, Solution, CostStructure>::CompleteIteration();
    if (neighbors_sampled >= max_neighbors_sampled || neighbors_accepted >= max_neighbors_accepted)
    {
        temperature *= cooling_rate;
        number_of_temperatures++;
        neighbors_sampled = 0;
        neighbors_accepted = 0;
    }
}
} // namespace Core
} // namespace EasyLocal
