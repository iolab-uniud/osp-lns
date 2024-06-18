#pragma once
#include <string>
#include <vector>
#include <set>
#include <iostream>

enum class FileFormat { DZN, DAT, JSON };

class OSP_Input
{
    friend std::ostream& operator<<(std::ostream& os, const OSP_Input& bs);
public:
    OSP_Input(std::string file_name);
    
    // getters for plain numbers and counters
    int Machines() const { return machines; }
    int Jobs() const { return jobs; }
    int Attributes() const { return attributes; }
    int Intervals() const { return intervals; }
    int Horizon() const { return horizon; }
    
    // getters for machine related characteristics
    int MinCapacityMachine(int m) const { return min_cap[m]; }
    int MaxCapacityMachine(int m) const { return max_cap[m]; }
    int InitialStateMachine(int m) const { return initial_state[m]; }
    std::vector<int> InitialStatuses() const {return initial_state; }
    int AvailabilityStart(int m, int s) const { return m_a_s[m][s]; }
    int AvailabilityEnd(int m, int s) const { return m_a_e[m][s]; }
    std::vector<int> AvailabilityStartVector(int m) const { return m_a_s[m]; }
    std::vector<int> AvailabilityEndVector(int m) const { return m_a_e[m]; }
    std::vector<std::vector<int>> AvailabilityStartMatrix() const { return m_a_s; }
    std::vector<std::vector<int>> AvailabilityEndMatrix() const {return m_a_e; }
    
    
    // getters for set up costs and times
    int SetUpCost(int prev, int next) const { return setup_costs[prev][next]; }
    int SetUpTime(int prev, int next) const { return setup_times[prev][next]; }
    std::vector<std::vector<int>> SetupTimes() const { return setup_times; }
    
    // getters for jobs related characteristics
    std::set<int> EligibleMachineSet(int j) const { return eligible_machine_set[j]; }
    bool IsMachineEligible(int m, int j) const { return eligible_machine_matrix[m][j]; }
    std::vector<std::vector<bool>> EligibleMachineMatrix() const { return eligible_machine_matrix; }
    int EarliestStartJob(int j) const { return earliest_start[j]; }
    int LatestEndJob(int j) const { return latest_end[j]; }
    int MinTimeJob(int j) const { return min_time[j]; }
    int MaxTimeJob(int j) const { return max_time[j]; }
    int SizeJob(int j) const { return size[j]; }
    int AttributeJob(int j) const { return attribute[j]; }
    std::vector<int> SizeJobs() const { return size;}
    std::vector<int> AttributeJobs() const { return attribute;}
    std::vector<int> MinTimeJobs() const { return min_time; }
    std::vector<int> MaxTimeJobs() const { return max_time; }
    std::vector<int> EarliestStartJobs() const { return earliest_start; }
    std::vector<int> LatestEndJobs() const { return latest_end; }
    
    // getters for the multiplication factors (weights)
    int MultFactorTotalRunTime() const { return mult_factor_total_runtime; }
    int MultFactorFinishedTooLate() const { return mult_factor_finished_toolate; }
    int MultFactorTotalSetUpTimes() const { return mult_factor_total_setuptimes; }
    int MultFactorTotalSetUpCosts() const { return mult_factor_total_setupcosts; }
    
private:
    FileFormat FindFileFormat(std::string file_name) const;
    void ReadDznFormat(std::string file_name);
    
    int machines, jobs, attributes, intervals, horizon;
    std::vector<std::vector<int>> setup_costs, setup_times;
    std::vector<int> min_cap, max_cap; // min and max capacity for machines
    std::vector<int> initial_state; // initial state of the machine
    std::vector<std::vector<int>> m_a_s, m_a_e; // begin and end intervals, m_a_s[machine][interval]
    std::vector<std::set<int>> eligible_machine_set; // eligible_machine[j] will provide you the set of elible mahcine for job j
    std::vector<std::vector<bool>> eligible_machine_matrix; // eligible_machine_matrix[m][j] = true if machine m is eligible for job j
    std::vector<int> earliest_start, latest_end;
    std::vector<int> min_time, max_time;
    std::vector<int> size;
    std::vector<int> attribute;
    int upper_bound_integer_objective, mult_factor_total_runtime, mult_factor_finished_toolate, mult_factor_total_setuptimes, mult_factor_total_setupcosts, running_time_bound;
};

class Batch
{
public:
    int size;
    int attribute;
    int batch_processing_time;
    int start_time; // horizon + 1 = not scheduled;
    int end_time; // horizon + 1 + preocessing_time = not scheduled
    int setup_cost;
    int setup_time;
};

class OSP_Output
{
    friend std::ostream& operator<<(std::ostream& os, const OSP_Output& out);
    friend std::istream& operator>>(std::istream& is, OSP_Output& out);
    friend bool operator==(const OSP_Output& out1, const OSP_Output& out2);
public:
    OSP_Output(const OSP_Input& in);
    OSP_Output& operator=(const OSP_Output& out);
    
    // some getters for the inputs
    int Intervals() const { return in.Intervals(); }
    int Horizon() const { return in.Horizon(); }
    int Jobs() const { return in.Jobs(); }
    int Machines() const { return in.Machines(); }
    int Attributes() const { return in.Attributes(); }
    int MaxCapacityMachine(int m) const { return in.MaxCapacityMachine(m); }
    bool IsMachineEligible(int m, int j) const { return in.IsMachineEligible(m, j); }
    std::vector<std::vector<bool>> EligibleMachineMatrix() const { return in.EligibleMachineMatrix(); }
    std::set<int> EligibleMachineSet(int j) const { return in.EligibleMachineSet(j); }
    int EarliestStartJob(int j) const { return in.EarliestStartJob(j); }
    int LatestEndJob(int j) const { return in.LatestEndJob(j); }
    int SizeJob(int j) const { return in.SizeJob(j); }
    int AttributeJob(int j) const { return in.AttributeJob(j); }
    int MinTimeJob(int j) const { return in.MinTimeJob(j); }
    int MaxTimeJob(int j) const { return in.MaxTimeJob(j); }
    int AvailabilityStart(int m, int s) const { return in.AvailabilityStart(m, s); }
    int AvailabilityEnd(int m, int s) const { return in.AvailabilityEnd(m, s); }
    std::vector<int> InitialStatuses() const { return in.InitialStatuses(); }
    std::vector<std::vector<int>> SetupTimes() const { return in.SetupTimes(); }
    std::vector<int> AvailabilityStartVector(int m) const { return in.AvailabilityStartVector(m); }
    std::vector<std::vector<int>> AvailabilityStartMatrix() const { return in.AvailabilityStartMatrix(); }
    std::vector<int>  AvailabilityEndVector(int m) const {return in.AvailabilityEndVector(m); }
    std::vector<std::vector<int>> AvailabilityEndMatrix() const {return in.AvailabilityEndMatrix(); }
    std::vector<int> SizeJobs() const { return in.SizeJobs();}
    std::vector<int> MinTimeJobs() const { return in.MinTimeJobs(); }
    std::vector<int> MaxTimeJobs() const { return in.MaxTimeJobs(); }
    std::vector<int> EarliestStartJobs() const { return in.EarliestStartJobs(); }
    std::vector<int> LatestEndJobs() const { return in.LatestEndJobs(); }
    std::vector<int> AttributeJobs() const { return in.AttributeJobs();}
    int MultFactorTotalRunTime() const { return in.MultFactorTotalRunTime(); }
    int MultFactorFinishedTooLate() const { return in.MultFactorFinishedTooLate(); }
    int MultFactorTotalSetUpTimes() const { return in.MultFactorTotalSetUpTimes(); }
    int MultFactorTotalSetUpCosts() const { return in.MultFactorTotalSetUpCosts(); }

    // given the job_to_batch_position, create all other data structures from scratch
    void PopulateAllFromScratch();
    void PopulateBatchesPerMachine();
    void PopulateJobsAtBatchPosition();
    void PopulateBatchCharacteristics();
    void PopulateBatchesPerAttribute();
    
    // calculate from scratch the costs
    void CalculateAllCostsFromScratch();
    void CalculateTotalSetUpTime();
    void CalculateTotalSetUpCost();
    void CalculateNumberOfTardyJobs();
    void CalculateCumulativeBatchProcessingTime();
    void CalculateNotScheduledBatches();
    
    // getters for costs
    int GetTotalSetUpTime() const { return total_set_up_time; }
    int GetTotalSetUpCost() const { return total_set_up_cost; }
    int GetNumberOfTardyJobs() const { return number_tardy_jobs; }
    int GetCumulativeBatchProcessingTime() const { return cumulative_batch_processing_time; }
    int GetNotScheduledBatches() const { return not_scheduled_batches; }
    
    Batch CalculateBatchProperties(int m, int p);
    int CalculateBatchStartTime(int machine, int earliest_start, int setup_time, int processing_time, int previous_end);
    int CalculateEarliestSuitableMachineIntervalStart(int machine, int earliest_start, int setup_time, int processing_time);
    
    // modifiers
    void ModifyJobToBatchPosition(int job, int machine, int position) { job_to_batch_position[job] = std::make_pair(machine, position); }
    // this is used by the OSP_SwapBatchesMoveNeighborhoodExplorer
    void SwapBatchesSameMachine (int m, int p1, int p2); // this only modifies the solution data structure, then you need to do something for the costs...
    void InsertBatchToNewPosition (int m, int o_p, int n_p); // this only modifies the solution data structure, then you need to do something for the costs...
    void InsertJobInExistingBatch(int job, std::pair<int,int> old_machine_position, std::pair<int,int> new_machine_position);
    void InsertJobToNewBatch (int job, std::pair<int,int> old_position, std::pair<int,int> new_position, bool is_alone);
    void InsertBatchToNewMachine (std::set<int> jobs_to_move, std::pair<int,int> old_position, std::pair<int,int> new_position);
    void InverseBatchesInMachine(int m, int p_1, int p_2);
    
    // checkers for moves
    bool IsJobCompatibleForBatch(int job, int machine, int position) const;
    
    // getters for solution components
    int GetBatchesPerMachine(int m) const { return batches_per_machine[m]; }
    std::set<int> GetJobsAtBatchPosition (int m, int p) const { return jobs_at_batch_position[m][p]; }
    std::pair<int,int> GetJobToBatchPosition(int j) const { return job_to_batch_position[j]; }
    Batch GetBatchCharacteristics (int m, int p) const { return batch_characteristics[m][p]; }
    std::set<std::pair<int,int>> GetBatchesPerAttribute(int a) const { return batches_per_attribute[a]; }
    
private:
    // TODO: REMEMBER TO ADD TO THE POPULATION/UPDATION/= WHATEVER YOU PUT HERE
    const OSP_Input& in;
    std::vector<std::pair<int,int>> job_to_batch_position; // for each job, the machine and the position (batch), thereof job_to_batch_position[j] = <mach,pos> will be that job j is inserted in patch at position pos of machine mach
    
    std::vector<int> batches_per_machine; // position_per_machine counts how many batch are there for a given machine
    std::vector<std::vector<std::set<int>>> jobs_at_batch_position; // for each machine and position, which jobs are there.
    std::vector<std::vector<Batch>> batch_characteristics; // batch_characteristics[mach][pos] provides you with the information of the batch located in position pos in machine mach
    std::vector<std::set<std::pair<int,int>>> batches_per_attribute; // for every attribute, which are the batch with that attribute
    
    // costs
    int number_tardy_jobs, total_set_up_time, total_set_up_cost, cumulative_batch_processing_time;
    int not_scheduled_batches;
    
    // checker for updates on batch characteristics, to use only in debug mode
    void CheckerForBatchCharacteristicsUpdate();
    void CheckerForBatchesPerAttributeUpdate();
    void CheckerForJobsAtBatchPositionUpdate();
    void CheckForNumberofBatchesUpdate();
};

class SwapConsecutiveBatchesMove
{
    // select two consectuive batches on the same oven and swap them
    friend bool operator==(const SwapConsecutiveBatchesMove& m1, const SwapConsecutiveBatchesMove& m2);
    friend bool operator!=(const SwapConsecutiveBatchesMove& m1, const SwapConsecutiveBatchesMove& m2);
    friend bool operator<(const SwapConsecutiveBatchesMove& m1, const SwapConsecutiveBatchesMove& m2);
    friend std::ostream& operator<<(std::ostream& os, const SwapConsecutiveBatchesMove& m);
    friend std::istream& operator>>(std::istream& is, SwapConsecutiveBatchesMove& m);
public:
    SwapConsecutiveBatchesMove(int m = -1, int p1 = -1, int p2 = -1) {machine = m; position_1 = p1; position_2 = p2;}
    int machine;
    int position_1;
    int position_2;
};

class BatchToNewPositionMove
{
    // select two consectuive batches on the same oven and swap them
    friend bool operator==(const BatchToNewPositionMove& m1, const BatchToNewPositionMove& m2);
    friend bool operator!=(const BatchToNewPositionMove& m1, const BatchToNewPositionMove& m2);
    friend bool operator<(const BatchToNewPositionMove& m1, const BatchToNewPositionMove& m2);
    friend std::ostream& operator<<(std::ostream& os, const BatchToNewPositionMove& m);
    friend std::istream& operator>>(std::istream& is, BatchToNewPositionMove& m);
public:
    BatchToNewPositionMove(int m = -1, int p_o = -1, int p_n = -1) {machine = m; old_position = p_o; new_position = p_n;}
    int machine;
    int old_position;
    int new_position;
};

class JobToExistingBatch
{
    // select a job and move it to an existing compatible batch
    friend bool operator==(const JobToExistingBatch& m1, const JobToExistingBatch& m2);
    friend bool operator!=(const JobToExistingBatch& m1, const JobToExistingBatch& m2);
    friend bool operator<(const JobToExistingBatch& m1, const JobToExistingBatch& m2);
    friend std::ostream& operator<<(std::ostream& os, const JobToExistingBatch& m);
    friend std::istream& operator>>(std::istream& is, JobToExistingBatch& m);
public:
    JobToExistingBatch(int j = -1, int o_m = -1, int o_p = -1, int n_m = -1, int n_p = -1, std::set<std::pair<int,int>> o_p_b = {}) 
        {job = j; old_machine = o_m; old_position = o_p; new_machine = n_m, new_position = n_p; other_possible_batches = o_p_b;}
    int job;
    int old_machine;
    int old_position;
    int new_machine;
    int new_position;
    std::set<std::pair<int,int>> other_possible_batches;
};

class JobToNewBatch
{
    // select a job and move it to a new batch
    friend bool operator==(const JobToNewBatch& m1, const JobToNewBatch& m2);
    friend bool operator!=(const JobToNewBatch& m1, const JobToNewBatch& m2);
    friend bool operator<(const JobToNewBatch& m1, const JobToNewBatch& m2);
    friend std::ostream& operator<<(std::ostream& os, const JobToNewBatch& m);
    friend std::istream& operator>>(std::istream& is, JobToNewBatch& m);
public:
    JobToNewBatch(int j = -1, std::pair<int,int> o_p = std::make_pair(-1,-1), std::pair<int,int> n_p = std::make_pair(-1,-1), std::set<int> m_t_n = {}, bool i_a = false) {job = j; old_position = o_p; new_position = n_p; machine_to_try_next = m_t_n; is_alone = i_a;}
    int job;
    std::pair<int,int> old_position;
    std::pair<int,int> new_position;
    std::set<int> machine_to_try_next;
    bool is_alone;
};

class BatchToNewMachine
{
    friend bool operator==(const BatchToNewMachine& m1, const BatchToNewMachine& m2);
    friend bool operator!=(const BatchToNewMachine& m1, const BatchToNewMachine& m2);
    friend bool operator<(const BatchToNewMachine& m1, const BatchToNewMachine& m2);
    friend std::ostream& operator<<(std::ostream& os, const BatchToNewMachine& m);
    friend std::istream& operator>>(std::istream& is, BatchToNewMachine& m);
public:
    BatchToNewMachine(std::set<int> j_t_m = {}, std::pair<int,int> o_m_p = std::make_pair(-1,-1), std::pair<int,int> n_m_p = std::make_pair(-1,-1))
    {jobs_to_move = j_t_m; old_machine_position = o_m_p; new_machine_position = n_m_p;}
    std::set<int> jobs_to_move;
    std::pair<int,int> old_machine_position;
    std::pair<int,int> new_machine_position;
};

class SwapBatches
{
    friend bool operator==(const SwapBatches& m1, const SwapBatches& m2);
    friend bool operator!=(const SwapBatches& m1, const SwapBatches& m2);
    friend bool operator<(const SwapBatches& m1, const SwapBatches& m2);
    friend std::ostream& operator<<(std::ostream& os, const SwapBatches& m);
    friend std::istream& operator>>(std::istream& is, SwapBatches& m);
public:
    SwapBatches(int mach = -1, int pos_1 = -1, int pos_2 = -1)
    { machine = mach; position_1 = pos_1; position_2 = pos_2; }
    int machine;
    int position_1;
    int position_2;
};

class InvertBatchesInMachine
{
    friend bool operator==(const InvertBatchesInMachine& m1, const InvertBatchesInMachine& m2);
    friend bool operator!=(const InvertBatchesInMachine& m1, const InvertBatchesInMachine& m2);
    friend bool operator<(const InvertBatchesInMachine& m1, const InvertBatchesInMachine& m2);
    friend std::ostream& operator<<(std::ostream& os, const InvertBatchesInMachine& m);
    friend std::istream& operator>>(std::istream& is, InvertBatchesInMachine& m);
public:
    InvertBatchesInMachine(int mach = -1, int pos_1 = -1, int pos_2 = -1)
    { machine = mach; position_1 = pos_1; position_2 = pos_2; }
    int machine;
    int position_1;
    int position_2;
};
