#pragma once

#include <stdexcept>

#include "helpers/solutionmanager.hh"
#include "helpers/neighborhoodexplorer.hh"
#include "runners/moverunner.hh"

namespace EasyLocal
{
  
  namespace Core
  {
    
    /** The Hill Climbing runner considers random move selection. A move
     is then performed only if it does improve or it leaves unchanged
     the value of the cost function.
     @ingroup Runners
     */
    template <class Input, class Solution, class Move, class CostStructure = DefaultCostStructure<int>>
    class HillClimbing : public MoveRunner<Input, Solution, Move, CostStructure>
    {
    public:
      HillClimbing(const Input &in, SolutionManager<Input, Solution, CostStructure> &sm,
                   NeighborhoodExplorer<Input, Solution, Move, CostStructure> &ne,
                   std::string name) : MoveRunner<Input, Solution, Move, CostStructure>(in, sm, ne, name)
        {
            max_idle_iterations("max_idle_iterations", "Total number of allowed idle iterations", this->parameters);
        }
      
    protected:
      Parameter<unsigned long int> max_idle_iterations;
      bool MaxIdleIterationExpired() const;
      bool StopCriterion();
      void SelectMove();
      // parameters
    };
    
    /*************************************************************************
     * Implementation
     *************************************************************************/
    
    /**
     The select move strategy for the hill climbing simply looks for a
     random move that improves or leaves the cost unchanged.
     */
    template <class Input, class Solution, class Move, class CostStructure>
    void HillClimbing<Input, Solution, Move, CostStructure>::SelectMove()
    {
      // TODO: it should become a parameter, the number of neighbors drawn at each iteration (possibly evaluated in parallel)
      const size_t samples = 10;
      size_t sampled;
      EvaluatedMove<Move, CostStructure> em = this->ne.RandomFirst(*this->p_current_state, samples, sampled, [](const Move &mv, const CostStructure &move_cost) {
        return move_cost <= 0;
      },
                                                                   this->weights);
      this->current_move = em;
      this->evaluations += static_cast<unsigned long int>(sampled);
    }
    
    template <class Input, class Solution, class Move, class CostStructure>
    bool HillClimbing<Input, Solution, Move, CostStructure>::MaxIdleIterationExpired() const
    {
      return this->max_idle_iterations.IsSet() && this->iteration - this->iteration_of_best >= this->max_idle_iterations;
    }
    
    /**
     The stop criterion is based on the number of iterations elapsed from
     the last strict improvement of the best state cost.
     */
    template <class Input, class Solution, class Move, class CostStructure>
    bool HillClimbing<Input, Solution, Move, CostStructure>::StopCriterion()
    {
      return MaxIdleIterationExpired() || this->MaxEvaluationsExpired();
    }
    
  } // namespace Core
} // namespace EasyLocal
