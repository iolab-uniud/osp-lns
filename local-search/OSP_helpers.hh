#pragma once

#include "OSP_data.hh"
#include <easylocal.hh>

#include <iostream>
#include <string>
#include <vector>
#include <set>

using namespace EasyLocal::Core;


class OSP_SolutionManager : public SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>
{
public:
    OSP_SolutionManager(const OSP_Input & pin) : SolutionManager<OSP_Input, OSP_Output, DefaultCostStructure<int>>(pin, "OSP_SolutionManager")  {}
    void RandomState(OSP_Output& st);
    void GreedyState(OSP_Output& st);
    bool CheckConsistency(const OSP_Output& st) const;
protected:
    // methods for GreedyState
    int GetCurrentShiftOnMachine(int m, int time, int intervals, std::vector<int> availability_start_vector);
    std::map<int,int> GetSetupTimes (int next_attribute, std::set<int> available_machines, std::map<int,Batch>last_batch_assignement_on_machine, std::vector<int> batch_count_per_machine, std::vector<int> initial_status, std::vector<std::vector<int>> setup_times);
    int FindBestMachine(int time, int processing_time, std::map<int,int> setup_time_for_machines, std::set<int> available_machines, std::map<int, std::pair<int, bool>> current_shift_dict, std::vector<std::vector<int>> end_shifts);
    void FillBatch(
                   Batch &current_batch, std::map<int,Batch> &last_assignment_batch, std::set<int> &unscheduled_jobs, std::vector<std::pair<int, int>> &job_to_batch_position, std::set<int> &available_jobs,
                   int current_machine, int current_position, int job_in_batch, int batch_min_time, int batch_max_time, int max_batch_capacity, int min_job_size, int max_time_window, int factor_time_window, int current_shift,
                   std::vector<int> size_per_job, std::vector<std::vector<bool>> eligible_machine_matrix, std::vector<int> earliest_start_per_job, std::vector<int> latest_end_per_job, std::vector<int> min_time_per_job, std::vector<int> max_time_per_job, std::vector<int> attribute_per_job, std::vector<int> start_shift_machine, std::vector<int> end_shift_machine
    );
};


class  OSP_TotalSetUpTime: public CostComponent<OSP_Input,OSP_Output,int>
{
public:
    OSP_TotalSetUpTime(const OSP_Input & in, int w, bool hard) : CostComponent<OSP_Input,OSP_Output,int>(in,w,hard,"OSP_TotalSetUpTime") {}
    int ComputeCost(const OSP_Output& st) const { return st.GetTotalSetUpTime();}
    void PrintViolations(const OSP_Output& st, std::ostream& os = std::cout) const;
};

class  OSP_TotalSetUpCost: public CostComponent<OSP_Input,OSP_Output,int>
{
public:
    OSP_TotalSetUpCost(const OSP_Input & in, int w, bool hard) : CostComponent<OSP_Input,OSP_Output,int>(in,w,hard,"OSP_TotalSetUpCost") {}
    int ComputeCost(const OSP_Output& st) const { return st.GetTotalSetUpCost();}
    void PrintViolations(const OSP_Output& st, std::ostream& os = std::cout) const;
};

class  OSP_NumberOfTardyJobs: public CostComponent<OSP_Input,OSP_Output,int>
{
public:
    OSP_NumberOfTardyJobs(const OSP_Input & in, int w, bool hard) : CostComponent<OSP_Input,OSP_Output,int>(in,w,hard,"OSP_NumberOfTardyJobs") {}
    int ComputeCost(const OSP_Output& st) const { return st.GetNumberOfTardyJobs();}
    void PrintViolations(const OSP_Output& st, std::ostream& os = std::cout) const;
};

class  OSP_CumulativeBatchProcessingTime: public CostComponent<OSP_Input,OSP_Output,int>
{
public:
    OSP_CumulativeBatchProcessingTime(const OSP_Input & in, int w, bool hard) : CostComponent<OSP_Input,OSP_Output,int>(in,w,hard,"OSP_CumulativeBatchProcessingTime") {}
    int ComputeCost(const OSP_Output& st) const { return st.GetCumulativeBatchProcessingTime();}
    void PrintViolations(const OSP_Output& st, std::ostream& os = std::cout) const;
};

class  OSP_NotScheduledBatches: public CostComponent<OSP_Input,OSP_Output,int>
{
public:
    OSP_NotScheduledBatches(const OSP_Input & in, int w, bool hard) : CostComponent<OSP_Input,OSP_Output,int>(in,w,hard,"OSP_NotScheduledBatches") {}
    int ComputeCost(const OSP_Output& st) const { return st.GetNotScheduledBatches();} 
    void PrintViolations(const OSP_Output& st, std::ostream& os = std::cout) const;
};

class OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer : public NeighborhoodExplorer<OSP_Input,OSP_Output,SwapConsecutiveBatchesMove,DefaultCostStructure<int>>
{
public:
    OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer(const OSP_Input & pin, SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>& psm) : NeighborhoodExplorer<OSP_Input,OSP_Output,SwapConsecutiveBatchesMove,DefaultCostStructure<int>>(pin, psm, "OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer") {}
    void RandomMove(const OSP_Output& st, SwapConsecutiveBatchesMove& mv) const override;
    bool FeasibleMove(const OSP_Output& st, const SwapConsecutiveBatchesMove& mv) const override;
    void MakeMove(OSP_Output& st, const SwapConsecutiveBatchesMove& mv) const override;
    void FirstMove(const OSP_Output& st, SwapConsecutiveBatchesMove& mv) const override;
    bool NextMove(const OSP_Output& st, SwapConsecutiveBatchesMove& mv) const override;
protected:
    void AnyRandomMove(const OSP_Output& st, SwapConsecutiveBatchesMove& mv) const;
};
 
class OSP_BatchToNewPositionNeighborhoodExplorer : public NeighborhoodExplorer<OSP_Input,OSP_Output,BatchToNewPositionMove,DefaultCostStructure<int>>
{
public:
    OSP_BatchToNewPositionNeighborhoodExplorer(const OSP_Input & pin, SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>& psm) : NeighborhoodExplorer<OSP_Input,OSP_Output,BatchToNewPositionMove,DefaultCostStructure<int>>(pin, psm, "OSP_BatchToNewPositionNeighborhoodExplorer") {}
    void RandomMove(const OSP_Output& st, BatchToNewPositionMove& mv) const override;
    bool FeasibleMove(const OSP_Output& st, const BatchToNewPositionMove& mv) const override;
    void MakeMove(OSP_Output& st, const BatchToNewPositionMove& mv) const override;
    void FirstMove(const OSP_Output& st, BatchToNewPositionMove& mv) const override;
    bool NextMove(const OSP_Output& st, BatchToNewPositionMove& mv) const override;
protected:
    void AnyRandomMove(const OSP_Output& st, BatchToNewPositionMove& mv) const;
};

class OSP_JobToExistingBatchNeighborhoodExplorer : public NeighborhoodExplorer<OSP_Input,OSP_Output,JobToExistingBatch,DefaultCostStructure<int>>
{
public:
    OSP_JobToExistingBatchNeighborhoodExplorer(const OSP_Input & pin, SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>& psm) : NeighborhoodExplorer<OSP_Input,OSP_Output,JobToExistingBatch,DefaultCostStructure<int>>(pin, psm, "OSP_JobToExistingBatchNeighborhoodExplorer") {}
    void RandomMove(const OSP_Output& st, JobToExistingBatch& mv) const override;
    bool FeasibleMove(const OSP_Output& st, const JobToExistingBatch& mv) const override;
    void MakeMove(OSP_Output& st, const JobToExistingBatch& mv) const override;
    void FirstMove(const OSP_Output& st, JobToExistingBatch& mv) const override;
    bool NextMove(const OSP_Output& st, JobToExistingBatch& mv) const override;
protected:
    void AnyRandomMove(const OSP_Output& st, JobToExistingBatch& mv) const;
};

class OSP_JobToNewBatchNeighborhoodExplorer : public NeighborhoodExplorer<OSP_Input,OSP_Output,JobToNewBatch,DefaultCostStructure<int>>
{
public:
    OSP_JobToNewBatchNeighborhoodExplorer(const OSP_Input & pin, SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>& psm) : NeighborhoodExplorer<OSP_Input,OSP_Output,JobToNewBatch,DefaultCostStructure<int>>(pin, psm, "OSP_JobToNewBatchNeighborhoodExplorer") {}
    void RandomMove(const OSP_Output& st, JobToNewBatch& mv) const override;
    bool FeasibleMove(const OSP_Output& st, const JobToNewBatch& mv) const override;
    void MakeMove(OSP_Output& st, const JobToNewBatch& mv) const override;
    void FirstMove(const OSP_Output& st, JobToNewBatch& mv) const override;
    bool NextMove(const OSP_Output& st, JobToNewBatch& mv) const override;
protected:
    void AnyRandomMove(const OSP_Output& st, JobToNewBatch& mv) const;
};

class OSP_BatchToNewMachineNeighborhoodExplorer : public NeighborhoodExplorer<OSP_Input,OSP_Output,BatchToNewMachine,DefaultCostStructure<int>>
{
public:
    OSP_BatchToNewMachineNeighborhoodExplorer(const OSP_Input & pin, SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>& psm) : NeighborhoodExplorer<OSP_Input,OSP_Output,BatchToNewMachine,DefaultCostStructure<int>>(pin, psm, "OSP_BatchToNewMachineNeighborhoodExplorer") {}
    void RandomMove(const OSP_Output& st, BatchToNewMachine& mv) const override;
    bool FeasibleMove(const OSP_Output& st, const BatchToNewMachine& mv) const override;
    void MakeMove(OSP_Output& st, const BatchToNewMachine& mv) const override;
    void FirstMove(const OSP_Output& st, BatchToNewMachine& mv) const override;
    bool NextMove(const OSP_Output& st, BatchToNewMachine& mv) const override;
protected:
    void AnyRandomMove(const OSP_Output& st, BatchToNewMachine& mv) const;
};

class Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer : public NeighborhoodExplorer<OSP_Input,OSP_Output,BatchToNewMachine,DefaultCostStructure<int>>
{
public:
    Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer(const OSP_Input & pin, SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>& psm) : NeighborhoodExplorer<OSP_Input,OSP_Output,BatchToNewMachine,DefaultCostStructure<int>>(pin, psm, "OSP_BatchToNewMachineNeighborhoodExplorer") {}
    void RandomMove(const OSP_Output& st, BatchToNewMachine& mv) const override;
    bool FeasibleMove(const OSP_Output& st, const BatchToNewMachine& mv) const override;
    void MakeMove(OSP_Output& st, const BatchToNewMachine& mv) const override;
    void FirstMove(const OSP_Output& st, BatchToNewMachine& mv) const override;
    bool NextMove(const OSP_Output& st, BatchToNewMachine& mv) const override;
protected:
    void AnyRandomMove(const OSP_Output& st, BatchToNewMachine& mv) const;
};

class OSP_SwapBatchesNeighborhoodExplorer : public NeighborhoodExplorer<OSP_Input,OSP_Output,SwapBatches,DefaultCostStructure<int>>
{
public:
    OSP_SwapBatchesNeighborhoodExplorer(const OSP_Input & pin, SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>& psm) : NeighborhoodExplorer<OSP_Input,OSP_Output,SwapBatches,DefaultCostStructure<int>>(pin, psm, "OSP_SwapBatchesNeighborhoodExplorer") {}
    void RandomMove(const OSP_Output& st, SwapBatches& mv) const override;
    bool FeasibleMove(const OSP_Output& st, const SwapBatches& mv) const override;
    void MakeMove(OSP_Output& st, const SwapBatches& mv) const override;
    void FirstMove(const OSP_Output& st, SwapBatches& mv) const override;
    bool NextMove(const OSP_Output& st, SwapBatches& mv) const override;
protected:
    void AnyRandomMove(const OSP_Output& st, SwapBatches& mv) const;
};

class OSP_InvertBatchesInMachineNeighborhoodExplorer : public NeighborhoodExplorer<OSP_Input,OSP_Output,InvertBatchesInMachine,DefaultCostStructure<int>>
{
public:
    OSP_InvertBatchesInMachineNeighborhoodExplorer(const OSP_Input & pin, SolutionManager<OSP_Input,OSP_Output,DefaultCostStructure<int>>& psm) : NeighborhoodExplorer<OSP_Input,OSP_Output,InvertBatchesInMachine,DefaultCostStructure<int>>(pin, psm, "OSP_InvertBatchesInMachineNeighborhoodExplorer") {}
    void RandomMove(const OSP_Output& st, InvertBatchesInMachine& mv) const override;
    bool FeasibleMove(const OSP_Output& st, const InvertBatchesInMachine& mv) const override;
    void MakeMove(OSP_Output& st, const InvertBatchesInMachine& mv) const override;
    void FirstMove(const OSP_Output& st, InvertBatchesInMachine& mv) const override;
    bool NextMove(const OSP_Output& st, InvertBatchesInMachine& mv) const override;
protected:
    void AnyRandomMove(const OSP_Output& st, InvertBatchesInMachine& mv) const;
};
