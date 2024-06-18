#include "OSP_helpers.hh"
#include <map>
#include <cmath>
#include <algorithm>

// if indipendence is on, then the six moves are indipendent from on another

void OSP_SolutionManager::RandomState(OSP_Output& st)
{
    //throw std::invalid_argument("Method RandomState not implemented yet.");
    GreedyState(st);
    // std::cout << "Done with greedy" << std::endl;
}

// Find a solution of the oven scheduling problem using a simple greedy algorithm
void OSP_SolutionManager::GreedyState(OSP_Output& st)
{
#if !defined(NDEBUG)
    std::cout << "Running greedy state" << std::endl;
#endif
    
    std::vector<std::pair<int, int>> job_to_batch_position;
    job_to_batch_position.resize(st.Jobs());
    
    int start = 0; // we always start at time 0, but in case we want to change this (e.g., dealing with particular format)
    
    // timespan between first and earliest start time
    int min_earliest_start = st.EarliestStartJob(0);
    int max_earliest_start = st.EarliestStartJob(0);
    for (int j = 0; j < in.Jobs(); ++j)
    {
        if (min_earliest_start > st.EarliestStartJob(j))
        {
            min_earliest_start = st.EarliestStartJob(j);
        }
        
        if (max_earliest_start < st.EarliestStartJob(j))
        {
            max_earliest_start = st.EarliestStartJob(j);
        }
    }
    
    int min_job_size = st.SizeJob(0);
    for (int j = 0; j < st.Jobs(); ++j)
    {
        if (min_job_size > st.SizeJob(j))
        {
            min_job_size = st.SizeJob(j);
        }
    }
    
    int earliest_time_span = max_earliest_start - min_earliest_start;
    
    int max_time_window = 1;
    int factor_time_window = (int) floor((double)earliest_time_span / (double)max_time_window);
    
    std::map<int, std::pair<int, bool>> current_shift_dict;
    for (int m = 0; m < st.Machines(); ++m)
    {
        int shift = GetCurrentShiftOnMachine(m, start, st.Intervals(), st.AvailabilityStartVector(m));
        bool on_shift = true;
        if (shift == -1 || st.AvailabilityEnd(m, shift) < start)
        {
            on_shift = false;
        }
        current_shift_dict[m] = std::make_pair(shift, on_shift);
    }
    
    std::map<int,Batch> last_batch_assignement_on_machine; // key is the machine
    std::vector<int> batch_count_per_machine;
    batch_count_per_machine.resize(st.Machines(), 0);
    int time = -1;
    std::set<int> available_jobs;
    std::map<int,int> job_scheduled_machine_dict;
    std::set<int> unscheduled_jobs;
    for (int j = 0; j < st.Jobs(); ++j)
    {
        unscheduled_jobs.insert(j);
    }
    
    while(unscheduled_jobs.size() > 0 && time <= st.Horizon())
    {
        // retrieve available job, that is job that are realised and not already scheduled
        while (available_jobs.empty())
        {
            time ++;
#if !defined(NDEBUG)
            std::cout << "Just update time to " << time << std::endl;
#endif
            for (int j : unscheduled_jobs)
            {
                if (st.EarliestStartJob(j) <= start + time)
                {
                    available_jobs.insert(j);
                }
            }
        }
        
        // update the current shift dictionary
        for (int m = 0; m < st.Machines(); ++m)
        {
            int old_shift = current_shift_dict[m].first;
            int new_shift = old_shift;
            int i = 1;
            // check if now we are in a shift over the old shift
            while(st.Intervals() > old_shift + i
                  && st.AvailabilityStart(m, old_shift + i) <= start + time)
            {
                new_shift ++;
                i ++;
            }
            bool on_shift = true;
            if (new_shift == -1 || st.AvailabilityEnd(m, new_shift) < start)
            {
                on_shift = false;
            }
            current_shift_dict[m] = std::make_pair(new_shift, on_shift);
        }
        
        // create a list of machines that are not available
        // (a) off-shift
        // (b) job is currently processing
        std::set<int> unavailable_machines;
        for (int m = 0; m < st.Machines(); ++m)
        {
            // if (last_batch_assignement_on_machine.count(m) == 1)
            if (last_batch_assignement_on_machine.find(m) != last_batch_assignement_on_machine.end())
            {
                if (last_batch_assignement_on_machine[m].end_time > start + time)
                {
                    unavailable_machines.insert(m);
                }
            }
            if (!current_shift_dict[m].second || st.AvailabilityEnd(m, current_shift_dict[m].first) < time)
            {
                unavailable_machines.insert(m);
            }
        }
        
        if (unavailable_machines.size() == st.Machines())
        {
            available_jobs.clear();
            continue;
        }
        
        while (available_jobs.size() > 0)
        {
            // update the current shift dictionary
            for (int m = 0; m < st.Machines(); ++m)
            {
                int old_shift = current_shift_dict[m].first;
                int new_shift = old_shift;
                int i = 1;
                // check if now we are in a shift over the old shift
                while(st.Intervals() > old_shift + i
                      && st.AvailabilityStart(m, old_shift + i) <= start + time)
                {
                    new_shift ++;
                    i ++;
                }
                bool on_shift = true;
                if (new_shift == -1 || st.AvailabilityEnd(m, new_shift) < start)
                {
                    on_shift = false;
                }
                current_shift_dict[m] = std::make_pair(new_shift, on_shift);
            }
            
            // create a list of machines that are not available
            // (a) off-shift
            // (b) job is currently processing
            std::set<int> unavailable_machines;
            for (int m = 0; m < st.Machines(); ++m)
            {
                // if (last_batch_assignement_on_machine.count(m) == 1)
                if (last_batch_assignement_on_machine.find(m) != last_batch_assignement_on_machine.end())
                {
                    if (last_batch_assignement_on_machine[m].end_time > start + time)
                    {
                        unavailable_machines.insert(m);
                    }
                }
                if (!current_shift_dict[m].second || st.AvailabilityEnd(m, current_shift_dict[m].first) < time)
                {
                    unavailable_machines.insert(m);
                }
            }
            
            
            int next_job = *available_jobs.begin();
            int end_next_job = st.LatestEndJob(next_job);
            int size_next_job = st.SizeJob(next_job);
#if !defined(NDEBUG)
            std::cout << "Avaible jobs at t " << time << " in the main procedure are: ";
            for (auto j : available_jobs)
            {
                std::cout << j << " ";
            }
            std::cout << std::endl;
#endif
            for (auto j : available_jobs)
            {
                if (end_next_job >= st.LatestEndJob(j))
                {
                    if (end_next_job > st.LatestEndJob(j))
                    {
                        next_job = j;
                        end_next_job = st.LatestEndJob(j); 
                        size_next_job = st.SizeJob(j);
                    }
                    else if (end_next_job == st.LatestEndJob(j) && size_next_job < st.SizeJob(j))
                    {
                        next_job = j;
                        end_next_job = st.LatestEndJob(j); 
                        size_next_job = st.SizeJob(j);
                    }
                    
                }
            }
#if !defined(NDEBUG)
            std::cout << "The main procedure selected job:  " << next_job << std::endl;
#endif
            // now that you have the next job, look if there is an available machine for the job among those possible for it
            std::set<int> available_machines;
            for (int m = 0; m < st.Machines(); ++m)
            {
                if (!(unavailable_machines.find(m) != unavailable_machines.end())//unavailable_machines.count(m) == 0
                    && st.IsMachineEligible(m, next_job)
                    && st.MaxCapacityMachine(m) >= st.SizeJob(next_job))
                {
                    available_machines.insert(m);
#if !defined(NDEBUG)
                    std::cout << "Machine " << m << " is available" << std::endl;
#endif
                }
            }
            
            if (available_machines.empty())
            {
                available_jobs.erase(next_job);
                continue;
            }
            
            // calculate setup times from previous for all available machines
            std::map<int,int> setup_times_for_machines = GetSetupTimes(st.AttributeJob(next_job),available_machines, last_batch_assignement_on_machine, batch_count_per_machine, st.InitialStatuses(), st.SetupTimes());
            
            // look for the best machines
#if !defined(NDEBUG)
            std::cout << "Looking for best machine for job: " << next_job << std::endl;
#endif
            int best_machine = FindBestMachine(time, st.MinTimeJob(next_job), setup_times_for_machines, available_machines, current_shift_dict, st.AvailabilityEndMatrix());
            
            // if you where not able to find a good machine, go on
            if (best_machine == -1)
            {
                
                available_jobs.erase(next_job);
                continue;
            }
            int shift = current_shift_dict[best_machine].first;
            
            // if you are here, now, this means that this job can be assigned to this machine in this moment in a proper batch

            
            unscheduled_jobs.erase(next_job);
            available_jobs.erase(next_job);

            int position_of_job = batch_count_per_machine[best_machine];
            batch_count_per_machine[best_machine] = batch_count_per_machine[best_machine] + 1;
            job_to_batch_position[next_job] = std::make_pair(best_machine, position_of_job);
#if !defined(NDEBUG)
            std::cout << "Job " << next_job << " inserted in " << best_machine << " // " << position_of_job + 1 << std::endl;
#endif
            
            job_scheduled_machine_dict[next_job] = best_machine;
            // int assigned_machine = best_machine;
            int batch_attribute = st.AttributeJob(next_job);
            int batch_size = st.SizeJob(next_job);
            int batch_set_up_time = setup_times_for_machines[best_machine];
            int batch_processing_time = st.MinTimeJob(next_job);
            Batch assigned_batch = {batch_size, batch_attribute, batch_processing_time, start + time + batch_set_up_time, start + time + batch_set_up_time + batch_processing_time, -1, batch_set_up_time}; // here the setup_cost is -1 because it has no sense to calculate it here
            
            last_batch_assignement_on_machine[best_machine] = assigned_batch;
            
#if !defined(NDEBUG)
            std::cout << "Looking for filling the batch" << std::endl;
#endif
            // add all the other batches
            FillBatch(
                      assigned_batch, last_batch_assignement_on_machine, unscheduled_jobs, job_to_batch_position, available_jobs,
                      
                      best_machine, position_of_job, next_job, st.MinTimeJob(next_job), st.MaxTimeJob(next_job), st.MaxCapacityMachine(best_machine), min_job_size, max_time_window, factor_time_window, shift,
                      
                      st.SizeJobs(), st.EligibleMachineMatrix(), st.EarliestStartJobs(), st.LatestEndJobs(), st.MinTimeJobs(), st.MaxTimeJobs(), st.AttributeJobs(), st.AvailabilityStartVector(best_machine), st.AvailabilityEndVector(best_machine)
                      );
#if !defined(NDEBUG)
            std::cout << "Finished filling the batch" << std::endl;
            std::cout << "Batch start " << last_batch_assignement_on_machine[best_machine].start_time << " end time " << last_batch_assignement_on_machine[best_machine].end_time << " (" << last_batch_assignement_on_machine[best_machine].batch_processing_time << ")" << std::endl;
#endif
            
        }
    }
    
    // if you are at this point you should have the job_to_batch_position, copy it into the st.job_to_batch_position;
    for (int j = 0; j < st.Jobs(); ++j)
    {
        int m = job_to_batch_position[j].first;
        int p = job_to_batch_position[j].second;
        st.ModifyJobToBatchPosition(j, m, p);
    }
#if !defined(NDEBUG)
    std::cout << "POPULATING SOLUTION" << std::endl;
#endif
    // call st.PopulateAllFromScratch;
    st.PopulateAllFromScratch();
    
}

// given a current time and a machine, find in which shift of the machine one currently is
int OSP_SolutionManager::GetCurrentShiftOnMachine(int m, int time, int intervals, std::vector<int> availability_start_vector)
{
    int selected = -1;
    for (int i = 0; i < intervals; ++i)
    {
        if (availability_start_vector[i] <= time)
        {
            selected = i;
        }
        else
        {
            break;
        }
    }
    return selected;
}

std::map<int,int> OSP_SolutionManager::GetSetupTimes(int next_attribute, std::set<int> available_machines, std::map<int,Batch>last_batch_assignement_on_machine, std::vector<int> batch_count_per_machine, std::vector<int> initial_status, std::vector<std::vector<int>> setup_times)
{
    std::map<int,int> setup_times_for_machines;
    
    // for every machine
    for (int m : available_machines)
    {
        int previous_attribute;
        // if the batch is at 0 for this machine, then it means it is the first one
        if(batch_count_per_machine[m] == 0)
        {
            // the initial setup time depends upon the initial_status of the machine
            previous_attribute = initial_status[m];
        }
        else
        {
            // the status depend on the attribute of the previous batch
            previous_attribute = last_batch_assignement_on_machine[m].attribute;
        }
        setup_times_for_machines[m] = setup_times[previous_attribute][next_attribute];
    }
    
    return setup_times_for_machines;
}

int OSP_SolutionManager::FindBestMachine(int time, int processing_time, std::map<int,int> setup_time_for_machines, std::set<int> available_machines, std::map<int, std::pair<int, bool>> current_shift_dict, std::vector<std::vector<int>> end_shifts)
{
    // bool found_machine = false;
    int best_machine = -1;
    int selected_machine = -1;
    int min_setup_time;
    while(true)
    {
        // if you do not have machines to scan anymore, then this means you did not find a good machine for the job
        if (available_machines.size() == 0)
        {
            return -1; // best_machine will be equal to 0
        }
        
        // from the available machines, pick up the one with the lower setup time
        selected_machine = *available_machines.begin();// the first
        min_setup_time = setup_time_for_machines[selected_machine];// the first
        for (int m : available_machines)
        {
            if (setup_time_for_machines[m] < min_setup_time)
            {
                selected_machine = m;
                min_setup_time = setup_time_for_machines[m];
            }
        }
        
        int shift = current_shift_dict[selected_machine].first; // retrieve the current shift that selected_machine is in
        
        // check whether setuptime and processing time can be done within the current shift
        if( time + min_setup_time + processing_time > end_shifts[selected_machine][shift])
        {
            // remove the machine from the possible machines
#if !defined(NDEBUG)
            std::cout << "Erasing machine " << selected_machine  << " (s " << shift << ") because of the shift on the machine ending --> Time " << time << " min_setup_time " << min_setup_time << " processing_time " << processing_time << " shift ending " << end_shifts[selected_machine][shift] << std::endl;
#endif
            available_machines.erase(selected_machine);
            setup_time_for_machines.erase(selected_machine);
        }
        else
        {
            best_machine = selected_machine;
#if !defined(NDEBUG)
            std::cout << "Found best machine, that is " << best_machine << std::endl;
#endif
            // return found_machine;
            break;
        }
    }
    return best_machine;
}

void OSP_SolutionManager::FillBatch(
                                             Batch &current_batch, std::map<int,Batch> &last_assignment_batch, std::set<int> &unscheduled_jobs, std::vector<std::pair<int, int>> &job_to_batch_position, std::set<int> &available_jobs,
                                             int current_machine, int current_position, int job_in_batch, int batch_min_time, int batch_max_time, int max_batch_capacity, int min_job_size, int max_time_window, int factor_time_window, int current_shift,
                                             std::vector<int> size_per_job, std::vector<std::vector<bool>> eligible_machine_matrix, std::vector<int> earliest_start_per_job, std::vector<int> latest_end_per_job, std::vector<int> min_time_per_job, std::vector<int> max_time_per_job, std::vector<int> attribute_per_job, std::vector<int> start_shift_machine, std::vector<int> end_shift_machine)
{
    int time_window = -1;
    std::set<int> job_available_for_batch;
    
#if !defined(NDEBUG)
    if (current_batch.end_time > latest_end_per_job[job_in_batch])
    {
        std::cout << "The batch is late." << std::endl;
    }
#endif
        
    // until you have enough space
    while(current_batch.size + min_job_size <= max_batch_capacity)
    {
        for (auto it = job_available_for_batch.begin(); it != job_available_for_batch.end();)
        {
            int j = *it;
            if (!
                (
                earliest_start_per_job[j] <= current_batch.start_time + (time_window * factor_time_window)  // job is available at batch start time + lookhaed window
                && eligible_machine_matrix[current_machine][j] // the assigned machin is eligible
                && attribute_per_job[j] == current_batch.attribute // matching attributes
                && max_time_per_job[j] >= batch_min_time // the min batch processing is not too long
                && min_time_per_job[j] <= batch_max_time // the max batch processing is not too short
                && size_per_job[j] + current_batch.size <= max_batch_capacity // the job can fit
                && earliest_start_per_job[j] + batch_min_time <= end_shift_machine[current_shift] // batch can be scheduled in assigned machine in current shift
                && earliest_start_per_job[j] + min_time_per_job[j] <= end_shift_machine[current_shift] //job can be scheduled in assigned machine in current shift
                && current_batch.start_time + batch_min_time <= end_shift_machine[current_shift] //batch can be scheduled in assigned machine in current shift
                && current_batch.start_time + min_time_per_job[j] <= end_shift_machine[current_shift] // job can be scheduled in assigned machine in current shift
                && (
                    current_batch.end_time > latest_end_per_job[job_in_batch] //unless jobInBatch already finishes too late, only consider jobs that will not force the batch to end late
                    || (
                        earliest_start_per_job[j] + min_time_per_job [j] <= latest_end_per_job[job_in_batch]
                        && earliest_start_per_job[j] + batch_min_time <= latest_end_per_job[job_in_batch]
                        && current_batch.start_time + min_time_per_job[j] <= latest_end_per_job[job_in_batch]
                        )
                    )
                
                ))
            {
                it = job_available_for_batch.erase(it);
            }
            else
            {
                ++it;
            }
        }
        // if no jobs, increase the time window
        // here this is only done one actually, since max_time_widnow = 1
        while(job_available_for_batch.empty() && time_window < max_time_window)
        {
            time_window ++;
                    
            // check if there are other jobs available that can be scheduled in the same batch
            for (int j : unscheduled_jobs)
            {
                if (
                    earliest_start_per_job[j] <= current_batch.start_time + (time_window * factor_time_window)  // job is available at batch start time + lookhaed window
                    && eligible_machine_matrix[current_machine][j] // the assigned machin is eligible
                    && attribute_per_job[j] == attribute_per_job[job_in_batch]// matching attributes
                    && max_time_per_job[j] >= batch_min_time // the min batch processing is not too long
                    && min_time_per_job[j] <= batch_max_time // the max batch processing is not too short
                    && size_per_job[j] + current_batch.size <= max_batch_capacity // the job can fit
                    && earliest_start_per_job[j] + batch_min_time <= end_shift_machine[current_shift] // batch can be scheduled in assigned machine in current shift
                    && earliest_start_per_job[j] + min_time_per_job[j] <= end_shift_machine[current_shift] //job can be scheduled in assigned machine in current shift
                    && current_batch.start_time + batch_min_time <= end_shift_machine[current_shift] //batch can be scheduled in assigned machine in current shift
                    && current_batch.start_time + min_time_per_job[j] <= end_shift_machine[current_shift] // job can be scheduled in assigned machine in current shift
                    && (
                        current_batch.end_time > latest_end_per_job[job_in_batch] //unless jobInBatch already finishes too late, only consider jobs that will not force the batch to end late
                        || (
                            earliest_start_per_job[j] + min_time_per_job[j] <= latest_end_per_job[job_in_batch]
                            && earliest_start_per_job[j] + batch_min_time <= latest_end_per_job[job_in_batch]
                            && current_batch.start_time + min_time_per_job[j] <= latest_end_per_job[job_in_batch]
                            )
                        )
                    
                    )
                {
                    job_available_for_batch.insert(j);
                }
            }
            
        }
        
        // pick the job with the earliest due date, then the large size
        // pay attention to the size again
        if (job_available_for_batch.empty() && time_window >= max_time_window)
        {
            return;
        }
        else if (job_available_for_batch.empty())
        {
            continue;
        }
        // std::cout << "job_available_for_batch " << job_available_for_batch.size() << std::endl;;
         
        int job_for_batch = *job_available_for_batch.begin();
        int size_job_for_batch = size_per_job[job_for_batch];
        int latest_end_job_for_batch = latest_end_per_job[job_for_batch];
        // for(int j : job_available_for_batch)
        for (std::set<int>::iterator it = job_available_for_batch.begin(); it != job_available_for_batch.end(); it++)
        {
            int j = *it;
#if !defined(NDEBUG)
            assert(size_per_job[j] + current_batch.size <= max_batch_capacity);
#endif
            if (latest_end_job_for_batch >= latest_end_per_job[j])
            {
                if (latest_end_job_for_batch == latest_end_per_job[j] && size_job_for_batch < size_per_job[j])
                {
                    job_for_batch = j;
                    size_job_for_batch = size_per_job[j];
                    latest_end_job_for_batch = latest_end_per_job[j];
                }
                if (latest_end_job_for_batch > latest_end_per_job[j])
                {
                    job_for_batch = j;
                    size_job_for_batch = size_per_job[j];
                    latest_end_job_for_batch = latest_end_per_job[j];
                }
            }
        }
        
        // remove the job from the unscheduled job
        unscheduled_jobs.erase(job_for_batch);
        available_jobs.erase(job_for_batch);
        job_available_for_batch.erase(job_for_batch);
        
        job_to_batch_position[job_for_batch] = std::make_pair(current_machine, current_position);
#if !defined(NDEBUG)
        std::cout << "job " << job_for_batch << " inserted in " << current_machine << " // " << current_position + 1 << std::endl;
#endif
        // update the batch
        current_batch.size = current_batch.size + size_per_job[job_for_batch];
        //update batch start time if necessary
        if (earliest_start_per_job[job_for_batch] > current_batch.start_time)
        {
            current_batch.start_time = earliest_start_per_job[job_for_batch];
        }
        //update minimal and maximal processing times of jobs in batch  if necessary
        if (min_time_per_job[job_for_batch] > batch_min_time)
        {
            batch_min_time = min_time_per_job[job_for_batch];
            current_batch.batch_processing_time = min_time_per_job[job_for_batch];
        }
        if (max_time_per_job[job_for_batch] < batch_max_time)
        {
            batch_max_time = max_time_per_job[job_for_batch];
        }
        //update batch processing time
        current_batch.end_time = current_batch.start_time + current_batch.batch_processing_time;
        
        last_assignment_batch[current_machine] = current_batch;
    }
    return;
}

bool OSP_SolutionManager::CheckConsistency(const OSP_Output& st) const
{
    // check batch size
    for (int m = 0; m < st.Machines(); ++m)
    {
        int size_machine = st.MaxCapacityMachine(m);
        for (int p = 0; p < st.GetBatchesPerMachine(m); ++p)
        {
            int size_batch = 0;
            std::set<int> jobs = st.GetJobsAtBatchPosition(m, p);
            for (int job_index : jobs)
            {
                // int job_index = jobs[j];
                size_batch += st.SizeJob(job_index);
            }
            if( size_batch > size_machine)
            {
                std::cerr << "Batch size is greater than machine capacity" << std::endl;
                return false;
            }
        }
    }
    // check they all have the same attribute
    for (int m = 0; m < st.Machines(); ++m)
    {
        for (int p = 0; p < st.GetBatchesPerMachine(m); ++p)
        {
            std::set<int> jobs = st.GetJobsAtBatchPosition(m, p);
            int attribute = st.AttributeJob(*jobs.begin());
            for (int job : jobs)
            {
                if( attribute !=  st.AttributeJob(job))
                {
                    std::cerr << "Attribute not ok" << std::endl;
                    return false;
                }
            }
        }
    }
    
    // violations on start time and end time of the jobs
    for (int m = 0; m < st.Machines(); ++m)
    {
        // std::cout << "Checking machine: " << m << std::endl;
        // std::cout << "Batches to check: " << st.GetBatchesPerMachine(m) << std::endl;
        for (int p = 0; p < st.GetBatchesPerMachine(m); ++p)
        {
            Batch batch = st.GetBatchCharacteristics(m, p);
            std::set<int> jobs = st.GetJobsAtBatchPosition(m, p);
            
            // std::cout << "Jobs in position " << p << " are :";
            // for (int j = 0; j < jobs.size(); ++j)
            // {
            //    std::cout << jobs[j] << " ";
            // }
            // std::cout << std::endl;
            
            if (batch.size > st.MaxCapacityMachine(m))
            {
                std::cerr << "Batch size is greater than machine capacity" << std::endl;
                return false;
            }
            
            for (int job : jobs)
            {
                // int job = jobs[j];
                if (batch.start_time < st.EarliestStartJob(job))
                {
                    std::cerr << "The batch start too early" << std::endl;
                    return false;
                }
                if (batch.batch_processing_time > st.MaxTimeJob(job))
                {
                    std::cerr << "The batch processing time (" << batch.batch_processing_time << ") is too much (m: " << m << " p: " << p << " j: " << job << ")" << std::endl;
                    return false;
                }
                if (batch.batch_processing_time < st.MinTimeJob(job))
                {
                    std::cerr << "The batch processing time (" << batch.batch_processing_time << ") is too little (m: " << m << " p: " << p << " j: " << job << ")" << std::endl;
                    return false;
                }
                if (!st.IsMachineEligible(m, job))
                {
                    std::cerr << "Job " << job << " is assigned to a wrong machine." << std::endl;
                    return false;
                }
                // if the batch is scheduled, then it must be inside one interval
                if (batch.start_time <= in.Horizon())
                {
                    bool found = false;
                    for (int i = 0; i < st.Intervals(); i++)
                    {
                        if (st.AvailabilityStart(m, i) + batch.setup_time <= batch.start_time &&
                            batch.end_time <= st.AvailabilityEnd(m, i) )
                        {
                            found = true;
                        }
                    }
                    if (!found)
                    {
                        std::cerr << "Problem with the scheduling of batch " << p << " in machine " << m << std::endl;
                        return false;
                    }
                }
            }
        }
    }
    
    return true;
}

void OSP_TotalSetUpTime::PrintViolations(const OSP_Output& st, std::ostream& os) const
{
    for(int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < st.GetBatchesPerMachine(m) ; ++p)
        {
            Batch batch = st.GetBatchCharacteristics(m, p);
            os << "Batch " << p << " at machine " << m << " requires a setup time of " << batch.setup_time << std::endl;
        }
    }
}

void OSP_TotalSetUpCost::PrintViolations(const OSP_Output& st, std::ostream& os) const
{
    for(int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < st.GetBatchesPerMachine(m) ; ++p)
        {
            Batch batch = st.GetBatchCharacteristics(m, p);
            os << "Batch " << p << " at machine " << m << " requires a setup cost of " << batch.setup_cost << std::endl;
        }
    }
}

void OSP_NumberOfTardyJobs::PrintViolations(const OSP_Output& st, std::ostream& os) const
{
    for(int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < st.GetBatchesPerMachine(m) ; ++p)
        {
            Batch batch = st.GetBatchCharacteristics(m, p);
            std::set<int> jobs = st.GetJobsAtBatchPosition(m, p);
            for (int job : jobs)
            {
                // int job = jobs[j];
                if (st.LatestEndJob(job) < batch.end_time)
                {
                    os << "Job " << job << " is late (should finish maximum at " << st.LatestEndJob(job) << " but it finishes at " << batch.end_time << ")" << std::endl;
                }
            }
        }
    }
}

void OSP_CumulativeBatchProcessingTime::PrintViolations(const OSP_Output& st, std::ostream& os) const
{
    for(int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < st.GetBatchesPerMachine(m) ; ++p)
        {
            Batch batch = st.GetBatchCharacteristics(m, p);
            os << "Batch  " << p << " at machine " << m << " has a processing time of " << batch.batch_processing_time << std::endl;
        }
    }
}

void OSP_NotScheduledBatches::PrintViolations(const OSP_Output& st, std::ostream& os) const
{
    for(int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < st.GetBatchesPerMachine(m) ; ++p)
        {
            Batch batch = st.GetBatchCharacteristics(m, p);
            if (batch.start_time > st.Horizon())
            {
                os << "Batch  " << p << " at machine " << m << " is not really scheduled." << std::endl;
            }
        }
    }
}

void OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer::RandomMove(const OSP_Output& st, SwapConsecutiveBatchesMove& mv) const
{
// #if !defined(NDEBUG)
//    std::cout << "OSP_SwapBatchesMoveNeighborhoodExplorer: In RandomMove" << std::endl;
// #endif
    do {
        AnyRandomMove(st, mv);
    } while (!FeasibleMove(st, mv));
}

bool OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer::FeasibleMove(const OSP_Output& st, const SwapConsecutiveBatchesMove& mv) const
{
    // #if !defined(NDEBUG)
    // std::cout << "OSP_SwapBatchesMoveNeighborhoodExplorer: In FeasibleMove" << std::endl;
    // #endif
    int m = mv.machine;
    int total_batches = st.GetBatchesPerMachine(m);
    
    return
    // check that p1 != p2
        mv.position_1 < mv.position_2
    // check that machine has more than one batch
        && total_batches > 1
    // check that the number of batches and the selected position are good
        && mv.position_2 < total_batches;
}

void OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer::AnyRandomMove(const OSP_Output& st, SwapConsecutiveBatchesMove& mv) const
{
    // select a random machine that has a number of batches greater than 1
    // #if !defined(NDEBUG)
    // std::cout << "OSP_SwapBatchesMoveNeighborhoodExplorer: In AnyRandomMove" << std::endl;
    // #endif
    bool at_least_one = false;
    for (int m = 0; m < st.Machines(); ++m)
    {
        if (st.GetBatchesPerMachine(m) > 1)
        {
            at_least_one = true;
            break;
        }
    }
    if (!at_least_one)
    {
        throw EmptyNeighborhood();
    }
    
    do
    {
        mv.machine = Random::Uniform<int>(0,st.Machines()-1);
    } while (st.GetBatchesPerMachine(mv.machine) <= 1);
    
    // select a random position between 0 and position - 2, that will be the first position
    int batch_in_machine = st.GetBatchesPerMachine(mv.machine);
    mv.position_1 = Random::Uniform<int>(0, batch_in_machine - 2);
    mv.position_2 = mv.position_1 + 1;
}

void OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer::MakeMove(OSP_Output& st, const SwapConsecutiveBatchesMove& mv) const
{
    // #if !defined(NDEBUG)
    // std::cout << "OSP_SwapBatchesMoveNeighborhoodExplorer: In MakeMove" << std::endl;
    // #endif
    st.SwapBatchesSameMachine(mv.machine, mv.position_1, mv.position_2); // this will change the Solution Data Structure
    st.CalculateAllCostsFromScratch(); // this will now calculate all costs from scratch. I am pretty sure that we can actually do something better here..
}

void OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer::FirstMove(const OSP_Output& st, SwapConsecutiveBatchesMove& mv) const
{
    mv.machine = -1;
    mv.position_1 = -1;
    mv.position_2 = -1;
    while(true)
    {
        mv.machine = mv.machine + 1;
        if (mv.machine >= st.Machines())
        {
            throw EmptyNeighborhood();
        }
        if (st.GetBatchesPerMachine(mv.machine) > 1 && mv.machine < st.Machines())
        {
            mv.position_1 = 0;
            mv.position_2 = 1;
            break;
        }
    }
#if !defined(NDEBUG)
    assert(FeasibleMove(st, mv));
#endif
}

bool OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer::NextMove(const OSP_Output& st, SwapConsecutiveBatchesMove& mv) const
{
    // if you still gave position in the machine, then go on with next
    if (mv.position_2 < st.GetBatchesPerMachine(mv.machine) - 1)
    {
        mv.position_1 = mv.position_2;
        mv.position_2 = mv.position_1 + 1;
    }
    // if you are done with this machine, advance one machine
        // if machine is greater than the number of machine, stop
        // if you don't have enough slot, then you should advance of one machine
    else
    {
        while (true)
        {
            mv.machine += 1;
            if (mv.machine >= st.Machines())
            {
                return false;
            }
            if (st.GetBatchesPerMachine(mv.machine) > 1)
            {
                mv.position_1 = 0;
                mv.position_2 = 1;
                break;
            }
        }
    }
    return true;
}


void OSP_BatchToNewPositionNeighborhoodExplorer::RandomMove(const OSP_Output& st, BatchToNewPositionMove& mv) const
{
    // std::cout << "OSP_BatchToNewPositionNeighborhoodExplorer: In RandomMove" << std::endl;
     do {
        AnyRandomMove(st, mv);
    } while (!FeasibleMove(st, mv));
}

bool OSP_BatchToNewPositionNeighborhoodExplorer::FeasibleMove(const OSP_Output& st, const BatchToNewPositionMove& mv) const
{
// #if !defined(NDEBUG)
//    std::cout << "OSP_BatchToNewPositionMoveNeighborhoodExplorer: In FeasibleMove" << std::endl;
// #endif
    // if the machine has more than one batch
    if (st.GetBatchesPerMachine(mv.machine) <= 1)
    {
        return false;
    }
    // new_position is different than old position (yes and no, for simplicity, we leave this aside for the complete neighb. case)
    // if (mv.new_position == mv.old_position)
    // {
    //    return false;
    // }
    return true;
}

void OSP_BatchToNewPositionNeighborhoodExplorer::AnyRandomMove(const OSP_Output& st, BatchToNewPositionMove& mv) const
{
// #if !defined(NDEBUG)
//    std::cout << "OSP_BatchToNewPositionMoveNeighborhoodExplorer: In RandomMove" << std::endl;
// #endif
    bool at_least_one = false;

    for (int m = 0; m < st.Machines(); ++m)
    {
        if (st.GetBatchesPerMachine(m) > 1)
        {
            at_least_one = true;
            break;
        }
    }
    if (!at_least_one)
    {
        throw EmptyNeighborhood();
    }
    do
    {
        mv.machine = Random::Uniform<int>(0,st.Machines()-1);
    } while (st.GetBatchesPerMachine(mv.machine) <= 1);
    // select a random position
    int batch_in_machine = st.GetBatchesPerMachine(mv.machine);
    mv.old_position = Random::Uniform<int>(0, batch_in_machine - 1);
    do {
        mv.new_position = Random::Uniform<int>(0, batch_in_machine - 1);
    } while (mv.new_position == mv.old_position);

}

void OSP_BatchToNewPositionNeighborhoodExplorer::MakeMove(OSP_Output& st, const BatchToNewPositionMove& mv) const
{
    st.InsertBatchToNewPosition(mv.machine, mv.old_position, mv.new_position);
    // recompute the costs
    st.CalculateAllCostsFromScratch(); // TODO: this could be more informative
}

void OSP_BatchToNewPositionNeighborhoodExplorer::FirstMove(const OSP_Output& st, BatchToNewPositionMove& mv) const
{
    /*
    mv.machine = -1;
    
    while(true)
    {
        mv.machine = mv.machine + 1;
        if (mv.machine == st.Machines())
        {
            throw EmptyNeighborhood();
        }
        if (st.GetBatchesPerMachine(mv.machine) > 1)
        {
            break;
        }
    }
    
    mv.old_position = 0;
    mv.new_position = 0;
     */
    // FIXME: implement me
    throw std::invalid_argument("Method FirstMove not implemented yet.");
}

bool OSP_BatchToNewPositionNeighborhoodExplorer::NextMove(const OSP_Output& st, BatchToNewPositionMove& mv) const
{
    /*
    int total_batches = st.GetBatchesPerMachine(mv.machine);
    if (mv.new_position < total_batches - 1)
    {
        mv.new_position = mv.new_position + 1;
    }
    else if (mv.new_position == total_batches - 1 && mv.old_position < total_batches - 1)
    {
        mv.new_position = 0;
        mv.old_position = mv.old_position + 1;
    }
    else if (mv.new_position == total_batches - 1 && mv.old_position == total_batches - 1)
    {
        while (true)
        {
            mv.machine += 1;
            if (mv.machine == st.Machines())
            {
                return false;
            }
            if (st.GetBatchesPerMachine(mv.machine) > 1)
            {
                mv.new_position = 0;
                mv.old_position = 0;
                break;
            }
        }
    }
    else
    {
        throw std::invalid_argument("Some errors occured in the NextMove of OSP_BatchToNewPositionNeighborhoodExplorer");
    }
    return true;
     */
    // FIXME: implement me
    throw std::invalid_argument("Method NextMove not implemented yet.");
}

void OSP_JobToExistingBatchNeighborhoodExplorer::RandomMove(const OSP_Output& st, JobToExistingBatch& mv) const
{
    // std::cout << "OSP_JobToExistingBatchNeighborhoodExplorer: In RandomMove" << std::endl;
    do {
        AnyRandomMove(st, mv);
    } while (!FeasibleMove(st, mv));
}

bool OSP_JobToExistingBatchNeighborhoodExplorer::FeasibleMove(const OSP_Output& st, const JobToExistingBatch& mv) const
{
    return st.IsJobCompatibleForBatch(mv.job, mv.new_machine, mv.new_position);
   // && (mv.old_machine != mv.new_machine && mv.old_position != mv.new_position);
}

void OSP_JobToExistingBatchNeighborhoodExplorer::AnyRandomMove(const OSP_Output& st, JobToExistingBatch& mv) const
{
    std::set<int> job_to_scan;
    for (int j = 0;  j < st.Jobs(); ++j)
    {
        job_to_scan.insert(j);
    }
    
    while (job_to_scan.size() > 0)
    {
        mv.job = -1;
        mv.new_machine = -1;
        mv.new_position = -1;
        mv.old_machine = -1;
        mv.old_position = -1;
        mv.other_possible_batches.clear();
        
        // randomly select a job from job_to_scan
        int k = Random::Uniform<int>(0, (int) job_to_scan.size() - 1);
        for (int job : job_to_scan)
        {
            mv.job = job;
            if (k == 0)
            {
                break;
            }
            k--;
        }
        // remove it from the set of job_to_scan
        job_to_scan.erase(mv.job);
        // update old machine and position
        mv.old_machine = st.GetJobToBatchPosition(mv.job).first;
        mv.old_position = st.GetJobToBatchPosition(mv.job).second;
        
        // retrieve feasible batches
        int attribute = st.AttributeJob(mv.job);
        mv.other_possible_batches = st.GetBatchesPerAttribute(attribute);
        for (auto it = mv.other_possible_batches.begin(); it != mv.other_possible_batches.end();)
        {
            std::pair<int,int> current_batch = *it; // Get the batch from the iterator

            if (!st.IsJobCompatibleForBatch(mv.job, current_batch.first, current_batch.second))
            {
                it = mv.other_possible_batches.erase(it); // Erase and get the next iterator
            }
            else
            {
                ++it; // Move to the next iterator
            }
        }
        
        // if feasible batches == 0
            // continue
        if (mv.other_possible_batches.size() == 0)
        {
            continue;
        }
        // retrieve a feasible bacth
        int y = Random::Uniform<int>(0, (int)mv.other_possible_batches.size() - 1);
        for (std::pair<int,int> batch : mv.other_possible_batches)
        {
            mv.new_machine = batch.first;
            mv.new_position = batch.second;
            if (y == 0)
            {
                break;
            }
            y--;
        }
        // remove it from feasible bacthes
        mv.other_possible_batches.erase(std::make_pair(mv.new_machine, mv.new_position));
        return;
    }
    
    throw(EmptyNeighborhood());
}

void OSP_JobToExistingBatchNeighborhoodExplorer::MakeMove(OSP_Output& st, const JobToExistingBatch& mv) const
{
    std::pair<int,int> old_machine_position = std::make_pair(mv.old_machine, mv.old_position);
    std::pair<int,int> new_machine_position = std::make_pair(mv.new_machine, mv.new_position);
    st.InsertJobInExistingBatch(mv.job, old_machine_position, new_machine_position);
    // update the costs
    st.CalculateAllCostsFromScratch();
}

void OSP_JobToExistingBatchNeighborhoodExplorer::FirstMove(const OSP_Output& st, JobToExistingBatch& mv) const
{
    mv.job = -1;
    while(true)
    {
        mv.job += 1;
        if (mv.job >= st.Jobs())
        {
            throw EmptyNeighborhood();
        }
        mv.old_machine = st.GetJobToBatchPosition(mv.job).first;
        mv.old_position = st.GetJobToBatchPosition(mv.job).second;
        int attribute = st.AttributeJob(mv.job);
        mv.other_possible_batches = st.GetBatchesPerAttribute(attribute);
        for (auto it = mv.other_possible_batches.begin(); it != mv.other_possible_batches.end();)
        {
            std::pair<int,int> current_batch = *it; // Get the batch from the iterator

            if (!st.IsJobCompatibleForBatch(mv.job, current_batch.first, current_batch.second))
            {
                it = mv.other_possible_batches.erase(it); // Erase and get the next iterator
            }
            else
            {
                ++it; // Move to the next iterator
            }
        }
        if (mv.other_possible_batches.size() == 0)
        {
            continue;
        }
        std::pair<int,int> new_machine_position = *mv.other_possible_batches.begin();
        mv.other_possible_batches.erase(new_machine_position);
        mv.new_machine = new_machine_position.first;
        mv.new_position = new_machine_position.second;
        break;
    }
}

bool OSP_JobToExistingBatchNeighborhoodExplorer::NextMove(const OSP_Output& st, JobToExistingBatch& mv) const
{
    if (!mv.other_possible_batches.empty())
    {
        std::pair<int,int> next_batch = *mv.other_possible_batches.begin();
        mv.other_possible_batches.erase(next_batch);
        mv.new_machine = next_batch.first;
        mv.new_position = next_batch.second;
    }
    else
    {
        while(true)
        {
            mv.job+=1;
            if(mv.job >= st.Jobs())
            {
                return false;
            }
            mv.old_machine = st.GetJobToBatchPosition(mv.job).first;
            mv.old_position = st.GetJobToBatchPosition(mv.job).second;
            int attribute = st.AttributeJob(mv.job);
            mv.other_possible_batches = st.GetBatchesPerAttribute(attribute);
            for (auto it = mv.other_possible_batches.begin(); it != mv.other_possible_batches.end();)
            {
                std::pair<int,int> current_batch = *it; // Get the batch from the iterator

                if (!st.IsJobCompatibleForBatch(mv.job, current_batch.first, current_batch.second))
                {
                    it = mv.other_possible_batches.erase(it); // Erase and get the next iterator
                }
                else
                {
                    ++it; // Move to the next iterator
                }
            }
            if (mv.other_possible_batches.size() == 0)
            {
                continue;
            }
            std::pair<int,int> new_machine_position = *mv.other_possible_batches.begin();
            mv.other_possible_batches.erase(new_machine_position);
            mv.new_machine = new_machine_position.first;
            mv.new_position = new_machine_position.second;
            break;
        }
    }
    return true;
}

void OSP_JobToNewBatchNeighborhoodExplorer::RandomMove(const OSP_Output& st, JobToNewBatch& mv) const
{
    // std::cout << "OSP_JobToNewBatchNeighborhoodExplorer: In RandomMove" << std::endl;
    do {
        AnyRandomMove(st, mv);
    } while (!FeasibleMove(st, mv));
}

bool OSP_JobToNewBatchNeighborhoodExplorer::FeasibleMove(const OSP_Output& st, const JobToNewBatch& mv) const
{
    // if the machine is not feasible for the job
    if (!st.IsMachineEligible(mv.new_position.first, mv.job))
    {
        return false;
    }
    // if job was alone in its old position && new machine is equal to the olde one, then the position must be different
    if (mv.is_alone && mv.old_position == mv.new_position)
    {
        return false;
    }
    // if machine capacity is not able to host the job
    if (st.MaxCapacityMachine(mv.new_position.first) < st.SizeJob(mv.job))
    {
        return false;
    }
    return true;
}

void OSP_JobToNewBatchNeighborhoodExplorer::AnyRandomMove(const OSP_Output& st, JobToNewBatch& mv) const
{
    // randomly select one job
    mv.machine_to_try_next.clear();
    mv.is_alone = false;
    mv.job = Random::Uniform<int>(0, st.Jobs()-1);
    mv.old_position = st.GetJobToBatchPosition(mv.job);
    if (st.GetJobsAtBatchPosition(mv.old_position.first, mv.old_position.second).size() == 1)
    {
        mv.is_alone = true;
    }
    // randomly select one position
    mv.machine_to_try_next = st.EligibleMachineSet(mv.job);
    int index_machine = Random::Uniform<int> (0, (int) (mv.machine_to_try_next.size() - 1));
    for (int machine : mv.machine_to_try_next)
    {
        mv.new_position.first = machine;
        if (index_machine == 0)
        {
            break;
        }
        index_machine--;
    }
    mv.machine_to_try_next.erase(mv.new_position.first);
    // if the machine has not enough space for the job --> this will be taken care by the feasibility checker
    if (mv.new_position.first == mv.old_position.first && mv.is_alone)
    {
        mv.new_position.second = Random::Uniform<int> (0, st.GetBatchesPerMachine(mv.new_position.first) - 1); // if the job is alone and you are working on the same machine, then this is equal to the case of moving the batch
    }
    else
    {
        // randomly select the position
        mv.new_position.second = Random::Uniform<int> (0, st.GetBatchesPerMachine(mv.new_position.first)); // here you do not subtract the last one, because you could also end up as the very last, therefore not yet existing position.
    }
    
}

void OSP_JobToNewBatchNeighborhoodExplorer::MakeMove(OSP_Output& st, const JobToNewBatch& mv) const
{
    // change the data structure
    st.InsertJobToNewBatch(mv.job, mv.old_position, mv.new_position, mv.is_alone);
    // change the costs
    st.CalculateAllCostsFromScratch();
}

void OSP_JobToNewBatchNeighborhoodExplorer::FirstMove(const OSP_Output& st, JobToNewBatch& mv) const
{
    mv.job = -1;
    while(true)
    {
        mv.job += 1;
        mv.is_alone = false;
        if (mv.job >= st.Jobs())
        {
            throw EmptyNeighborhood();
        }
        mv.old_position = st.GetJobToBatchPosition(mv.job);
        if (st.GetJobsAtBatchPosition(mv.old_position.first, mv.old_position.second).size() == 1)
        {
            mv.is_alone = true;
        }
        mv.machine_to_try_next = st.EligibleMachineSet(mv.job);
        bool found_machine = false;
        while(!found_machine)
        {
            if (mv.machine_to_try_next.size() > 0)
            {
                mv.new_position.first = *mv.machine_to_try_next.begin();
                mv.machine_to_try_next.erase(mv.new_position.first);
                if (st.SizeJob(mv.job) <= st.MaxCapacityMachine(mv.new_position.first))
                {
                    found_machine = true;
                }
            }
            else
            {
                break;
            }
        }
        if (!found_machine) // you may have not found the machine for the job, so you try with another job.
        {
            continue;
        }
        mv.new_position.second = 0;
        if (mv.is_alone && mv.new_position == mv.old_position && st.GetBatchesPerMachine(mv.new_position.first) > 2)
        {
            mv.new_position.second = 2;
            break;
        }
        else if (!mv.is_alone || (mv.is_alone && mv.new_position != mv.old_position))
        {
            break;
        }
        else
        {
            continue;
        }
        
    }
}

bool OSP_JobToNewBatchNeighborhoodExplorer::NextMove(const OSP_Output& st, JobToNewBatch& mv) const
{
    bool found_machine = false;
    if ((mv.is_alone && mv.old_position.first != mv.new_position.first && mv.new_position.second < st.GetBatchesPerMachine(mv.new_position.first))
        || (mv.is_alone && mv.old_position.first == mv.new_position.first && mv.new_position.second < st.GetBatchesPerMachine(mv.new_position.first)-1)
        || (!mv.is_alone && mv.new_position.second < st.GetBatchesPerMachine(mv.new_position.first))) // go on in the same machine
    {
        mv.new_position.second += 1;
        return true;
    }
    else if (mv.machine_to_try_next.size() > 0) // begin with a new machine
    {
        while(!found_machine)
        {
            if (mv.machine_to_try_next.size() > 0)
            {
                mv.new_position.first = *mv.machine_to_try_next.begin();
                mv.machine_to_try_next.erase(mv.new_position.first);
                if (st.SizeJob(mv.job) <= st.MaxCapacityMachine(mv.new_position.first))
                {
                    found_machine = true;
                }
            }
            else
            {
                break;
            }
            if (found_machine) // you may have not found the machine for the job, so you try with another job.
            {
                mv.new_position.second = 0;
                if (mv.is_alone && mv.new_position == mv.old_position && st.GetBatchesPerMachine(mv.new_position.first) > 2)
                {
                    mv.new_position.second = 2;
                    return true;
                }
                else if (!mv.is_alone || (mv.is_alone && mv.new_position != mv.old_position))
                {
                    return true;
                }
            }
        }
    }
    
    if (!found_machine)
    {
        while (true)
        {
            mv.job += 1;
            mv.is_alone = false;
            if (mv.job >= st.Jobs())
            {
                return false;
            }
            mv.old_position = st.GetJobToBatchPosition(mv.job);
            if (st.GetJobsAtBatchPosition(mv.old_position.first, mv.old_position.second).size() == 1)
            {
                mv.is_alone = true;
            }
            mv.machine_to_try_next = st.EligibleMachineSet(mv.job);
            found_machine = false;
            while(!found_machine)
            {
                if (mv.machine_to_try_next.size() > 0)
                {
                    mv.new_position.first = *mv.machine_to_try_next.begin();
                    mv.machine_to_try_next.erase(mv.new_position.first);
                    if (st.SizeJob(mv.job) <= st.MaxCapacityMachine(mv.new_position.first))
                    {
                        found_machine = true;
                    }
                }
                else
                {
                    break;
                }
            }
            if (!found_machine) // you may have not found the machine for the job, so you try with another job.
            {
                continue;
            }
            mv.new_position.second = 0;
            if (mv.is_alone && mv.new_position == mv.old_position && st.GetBatchesPerMachine(mv.new_position.first) > 2)
            {
                mv.new_position.second = 2;
                return true;
            }
            else if (!mv.is_alone || (mv.is_alone && mv.new_position != mv.old_position))
            {
                return true;
            }
        }
    }
    return false;
}

void OSP_BatchToNewMachineNeighborhoodExplorer::RandomMove(const OSP_Output& st, BatchToNewMachine& mv) const
{
    // std::cout << "OSP_BatchToNewMachineNeighborhoodExplorer: In RandomMove" << std::endl;
    do {
        AnyRandomMove(st, mv);
    } while (!FeasibleMove(st, mv));
}

bool OSP_BatchToNewMachineNeighborhoodExplorer::FeasibleMove(const OSP_Output& st, const BatchToNewMachine& mv) const
{
    // the new and old position are equal, because this would be not doing anything. Since all the jobs are good for that machine, you would have to move everything again to the same position
    if (mv.old_machine_position == mv.new_machine_position)
    {
        return false;
    }
    return true;
}

void OSP_BatchToNewMachineNeighborhoodExplorer::AnyRandomMove(const OSP_Output& st, BatchToNewMachine& mv) const
{
    mv.jobs_to_move = {};
    mv.old_machine_position = std::make_pair(-1,-1);
    mv.new_machine_position = std::make_pair(-1,-1);
    // randomly select one machine position
    int m;
    do
    {
        m = Random::Uniform<int> (0, (st.Machines() - 1));
    } while (st.GetBatchesPerMachine(m) <= 0);
    
    int p = Random::Uniform<int> (0, (st.GetBatchesPerMachine(m) - 1));
    mv.old_machine_position = std::make_pair(m,p);
    // in that, select a job
    std::set<int> possible_jobs = st.GetJobsAtBatchPosition(m, p);
    int j_index = Random::Uniform<int> (0, (int) (possible_jobs.size() - 1));
    int first_job;
    for (int job : possible_jobs)
    {
        first_job = job;
        if (j_index == 0)
        {
            break;
        }
        j_index--;
    }
    // randomly select a machine (and a position) that is ok with that job (must be different from the one you are currently in)
    std::set<int> eligible_machines =  st.EligibleMachineSet(first_job);
    int j_machine = Random::Uniform<int> (0, (int) (eligible_machines.size() - 1));
    for (int machine : eligible_machines)
    {
        mv.new_machine_position.first = machine;
        if (j_machine == 0)
        {
            break;
        }
        j_machine--;
    }
    if (mv.old_machine_position.first == mv.new_machine_position.first)
    {
        mv.new_machine_position.second = Random::Uniform<int> (0, (int) (st.GetBatchesPerMachine(mv.new_machine_position.first) - 1));
    }
    else
    {
        mv.new_machine_position.second = Random::Uniform<int> (0, (int) (st.GetBatchesPerMachine(mv.new_machine_position.first)));
    }
    // look if in the same batch, you have other jobs that are ok with that machine (you don't need to check the attribute or other things, because if they are together this means that they are compatible). But check the overall size
    int new_batch_size = st.SizeJob(first_job);
    mv.jobs_to_move.insert(first_job);
    for (int job : possible_jobs)
    {
        if (st.IsMachineEligible(mv.new_machine_position.first, job)
            && new_batch_size + st.SizeJob(job) <= st.MaxCapacityMachine(mv.new_machine_position.first))
        {
            mv.jobs_to_move.insert(job);
            new_batch_size = new_batch_size + st.SizeJob(job);
        }
    }
}

void OSP_BatchToNewMachineNeighborhoodExplorer::MakeMove(OSP_Output& st, const BatchToNewMachine& mv) const
{
    // update the data structures
    st.InsertBatchToNewMachine(mv.jobs_to_move, mv.old_machine_position, mv.new_machine_position);
    
    // update the costs
    st.CalculateAllCostsFromScratch();
}
void OSP_BatchToNewMachineNeighborhoodExplorer::FirstMove(const OSP_Output& st, BatchToNewMachine& mv) const
{
    // FIXME: implement me
    throw std::invalid_argument("Method FirstMove not implemented yet.");
}

bool OSP_BatchToNewMachineNeighborhoodExplorer::NextMove(const OSP_Output& st, BatchToNewMachine& mv) const
{
    // FIXME: implement me
    throw std::invalid_argument("Method NextMove not implemented yet.");
    return false;
}

void OSP_SwapBatchesNeighborhoodExplorer::RandomMove(const OSP_Output& st, SwapBatches& mv) const
{
    // std::cout << "OSP_SwapBatchesNeighborhoodExplorer: In RandomMove" << std::endl;
    do {
        AnyRandomMove(st, mv);
    } while (!FeasibleMove(st, mv));
}

bool OSP_SwapBatchesNeighborhoodExplorer::FeasibleMove(const OSP_Output& st, const SwapBatches& mv) const
{
    // the machine must have more than one batch
    if (st.GetBatchesPerMachine(mv.machine) <= 1)
    {
        return false;
    }
    // the position should be one greater than the other
    if (mv.position_1 >= mv.position_2)
    {
        return false;
    }
    return true;
}

void OSP_SwapBatchesNeighborhoodExplorer::AnyRandomMove(const OSP_Output& st, SwapBatches& mv) const
{
    // randomly select a machine with more than one batch
    bool at_least_one = false;
    for (int m = 0; m < st.Machines(); ++m)
    {
        if (st.GetBatchesPerMachine(m) > 1)
        {
            at_least_one = true;
            break;
        }
    }
    if (!at_least_one)
    {
        throw EmptyNeighborhood();
    }
    do
    {
        mv.machine = Random::Uniform<int>(0,st.Machines()-1);
    } while (st.GetBatchesPerMachine(mv.machine) <= 1);
    // select a random position between 0 and position - 2, that will be the first position
    int batch_in_machine = st.GetBatchesPerMachine(mv.machine);
    mv.position_1 = Random::Uniform<int>(0, batch_in_machine - 2);
    mv.position_2 = Random::Uniform<int>(mv.position_1 + 1, batch_in_machine - 1);
}

void OSP_SwapBatchesNeighborhoodExplorer::MakeMove(OSP_Output& st, const SwapBatches& mv) const
{
    // Update the data structures
    st.SwapBatchesSameMachine(mv.machine, mv.position_1, mv.position_2);
    // Update the costs
    st.CalculateAllCostsFromScratch();
}

void OSP_SwapBatchesNeighborhoodExplorer::FirstMove(const OSP_Output& st, SwapBatches& mv) const
{
    // FIXME: implement me
    throw std::invalid_argument("Method FirstMove not implemented yet.");
}

bool OSP_SwapBatchesNeighborhoodExplorer::NextMove(const OSP_Output& st, SwapBatches& mv) const
{
    // FIXME: implement me
    throw std::invalid_argument("Method NextMove not implemented yet.");
    return false;
}

void OSP_InvertBatchesInMachineNeighborhoodExplorer::RandomMove(const OSP_Output& st, InvertBatchesInMachine& mv) const
{
    // std::cout << "OSP_InvertBatchesInMachineNeighborhoodExplorer: In RandomMove" << std::endl;
    do {
        AnyRandomMove(st, mv);
    } while (!FeasibleMove(st, mv));
}

bool OSP_InvertBatchesInMachineNeighborhoodExplorer::FeasibleMove(const OSP_Output& st, const InvertBatchesInMachine& mv) const
{
    // it is infeasible if the machine has not more than one batch and if position 1 comes after or is equal position 2
    if (st.GetBatchesPerMachine(mv.machine) <= 1
        || mv.position_1 >= mv.position_2)
    {
        return false;
    }
    return true;
}

void OSP_InvertBatchesInMachineNeighborhoodExplorer::AnyRandomMove(const OSP_Output& st, InvertBatchesInMachine& mv) const
{
    // randomly select a machine with more than one batch
    bool at_least_one = false;
    for (int m = 0; m < st.Machines(); ++m)
    {
        if (st.GetBatchesPerMachine(m) > 1)
        {
            at_least_one = true;
            break;
        }
    }
    if (!at_least_one)
    {
        throw EmptyNeighborhood();
    }
    do
    {
        mv.machine = Random::Uniform<int>(0,st.Machines()-1);
    } while (st.GetBatchesPerMachine(mv.machine) <= 1);
    // select a random position between 0 and position - 2, that will be the first position
    int batch_in_machine = st.GetBatchesPerMachine(mv.machine);
    mv.position_1 = Random::Uniform<int>(0, batch_in_machine - 2);
    mv.position_2 = Random::Uniform<int>(mv.position_1 + 1, batch_in_machine - 1);
}

void OSP_InvertBatchesInMachineNeighborhoodExplorer::MakeMove(OSP_Output& st, const InvertBatchesInMachine& mv) const
{
    // update the data structure of the solution
    st.InverseBatchesInMachine(mv.machine, mv.position_1, mv.position_2);
    // update the costs
    st.CalculateAllCostsFromScratch();
}

void OSP_InvertBatchesInMachineNeighborhoodExplorer::FirstMove(const OSP_Output& st, InvertBatchesInMachine& mv) const
{
    // FIXME: implement me
    throw std::invalid_argument("Method FirstMove not implemented yet.");
}

bool OSP_InvertBatchesInMachineNeighborhoodExplorer::NextMove(const OSP_Output& st, InvertBatchesInMachine& mv) const
{
    // FIXME: implement me
    throw std::invalid_argument("Method NextMove not implemented yet.");
    return false;
}



/* ============================================================================ */
void Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer::RandomMove(const OSP_Output& st, BatchToNewMachine& mv) const
{
    // std::cout << "OSP_BatchToNewMachineNeighborhoodExplorer: In RandomMove" << std::endl;
    do {
        AnyRandomMove(st, mv);
    } while (!FeasibleMove(st, mv));
}

bool Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer::FeasibleMove(const OSP_Output& st, const BatchToNewMachine& mv) const
{
    // the new and old position are equal, because this would be not doing anything. Since all the jobs are good for that machine, you would have to move everything again to the same position
    if (mv.old_machine_position == mv.new_machine_position)
    {
        return false;
    }
    return true;
}

void Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer::AnyRandomMove(const OSP_Output& st, BatchToNewMachine& mv) const
{
    std::set<std::pair<int,int>> possible_batches;
    for(int m  = 0; m < st.Machines(); ++m)
    {
        for (int p = 0; p < st.GetBatchesPerMachine(m); ++p)
        {
            if (st.GetJobsAtBatchPosition(m, p).size() > 1)
            {
                possible_batches.insert(std::make_pair(m,p));
            }
        }
    }
    while(possible_batches.size() > 0)
    {
        mv.jobs_to_move = {};
        mv.old_machine_position = std::make_pair(-1,-1);
        mv.new_machine_position = std::make_pair(-1,-1);
        int index_batch = Random::Uniform<int> (0, (int) (possible_batches.size() - 1));
        for (std::pair<int,int> batch : possible_batches)
        {
            mv.old_machine_position = batch;
            if (index_batch == 0)
            {
                break;
            }
            index_batch--;
        }
        possible_batches.erase(mv.old_machine_position);
        std::set<int> jobs_to_analyse = st.GetJobsAtBatchPosition(mv.old_machine_position.first, mv.old_machine_position.second);
        while(jobs_to_analyse.size() > 1)
        {
            mv.jobs_to_move = {};
            mv.new_machine_position = std::make_pair(-1,-1);
            int j_index = Random::Uniform<int> (0, (int) (jobs_to_analyse.size() - 1));
            int first_job;
            for (int job : jobs_to_analyse)
            {
                first_job = job;
                if (j_index == 0)
                {
                    break;
                }
                j_index--;
            }
            // randomly select a machine (and a position) that is ok with that job (must be different from the one you are currently in)
            jobs_to_analyse.erase(first_job);
            std::set<int> eligible_machines =  st.EligibleMachineSet(first_job);
            eligible_machines.erase(mv.old_machine_position.first);
            if (eligible_machines.size() == 0)
            {
                continue;
            }
            int j_machine = Random::Uniform<int> (0, (int) (eligible_machines.size() - 1));
            for (int selected_machine : eligible_machines)
            {
                mv.new_machine_position.first = selected_machine;
                if (j_machine == 0)
                 {
                     break;
                 }
                 j_machine--;
            }
            if (mv.old_machine_position.first == mv.new_machine_position.first)
            {
                mv.new_machine_position.second = Random::Uniform<int> (0, (int) (st.GetBatchesPerMachine(mv.new_machine_position.first) - 1));
            }
            else
            {
                mv.new_machine_position.second = Random::Uniform<int> (0, (int) (st.GetBatchesPerMachine(mv.new_machine_position.first)));
            }
            int new_batch_size = st.SizeJob(first_job);
            mv.jobs_to_move.insert(first_job);
            // jobs_to_analyse.erase(first_job);
            for (int selected_job : jobs_to_analyse)
            {
                if (selected_job != first_job && st.IsMachineEligible(mv.new_machine_position.first, selected_job)
                  && new_batch_size + st.SizeJob(selected_job) <= st.MaxCapacityMachine(mv.new_machine_position.first))
                  {
                      mv.jobs_to_move.insert(selected_job);
                      new_batch_size = new_batch_size + st.SizeJob(selected_job);
                  }
            }
            if (mv.jobs_to_move.size() > 1)
            {
                for (int selected_job : mv.jobs_to_move)
                {
                    if (selected_job!=first_job)
                        jobs_to_analyse.erase(selected_job);
                }
                return;
            }
            else
            {
                continue;
            }
        }
    }
    throw EmptyNeighborhood();
}

void Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer::MakeMove(OSP_Output& st, const BatchToNewMachine& mv) const
{
    // update the data structures
    st.InsertBatchToNewMachine(mv.jobs_to_move, mv.old_machine_position, mv.new_machine_position);
    
    // update the costs
    st.CalculateAllCostsFromScratch();
}
void Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer::FirstMove(const OSP_Output& st, BatchToNewMachine& mv) const
{
    // FIXME: implement me
    throw std::invalid_argument("Method FirstMove not implemented yet.");
}

bool Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer::NextMove(const OSP_Output& st, BatchToNewMachine& mv) const
{
    // FIXME: implement me
    throw std::invalid_argument("Method NextMove not implemented yet.");
    return false;
}
