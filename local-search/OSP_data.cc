#include "OSP_data.hh"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>

OSP_Input::OSP_Input(std::string file_name)
{
    FileFormat ff = FindFileFormat(file_name);
    if (ff == FileFormat::DZN)
    {
        ReadDznFormat(file_name);
    }
    else
    {
        throw std::invalid_argument("Method not implemented for file " + file_name);
    }
}

FileFormat OSP_Input::FindFileFormat(std::string file_name) const
{
    // DZN, DAT, JSON
    if (file_name.find("dzn") != std::string::npos)
    {
        return FileFormat::DZN;
    }
    else if (file_name.find("dat") != std::string::npos)
    {
        return FileFormat::DAT;
    }
    else if (file_name.find("json") != std::string::npos)
    {
        return FileFormat::JSON;
    }
    throw std::invalid_argument("Unknown file format for file " + file_name);
}

void OSP_Input::ReadDznFormat(std::string file_name)
{
    const int LEN = 256;
    std::string tmp_str;
    char tmp_char;
    int tmp_int;
        
    std::ifstream is(file_name);
    if (!is)
    {
        throw std::invalid_argument("Cannot open instance file " + file_name);
    }
    
    is.ignore(LEN,'=');
    is >> horizon;
#if !defined(NDEBUG)
    std::cout << "horizon: " << horizon << std::endl;
#endif
    
    is.ignore(LEN,'=');
    is >> attributes;
#if !defined(NDEBUG)
    std::cout << "attributes: " << attributes << std::endl;
#endif
        
    setup_costs.resize(attributes, std::vector<int>(attributes));
    is.ignore(LEN, '[');
    for (int r = 0; r < attributes; ++r)
    {
        is >> tmp_char; // takes out |
        for (int c = 0; c < attributes; ++c)
        {
            is >> setup_costs[r][c];
            is >> tmp_char; // takes out ,
        }
    }
    // remove the line of all 0 (this was used just for the minizinc model)
    is >> tmp_char >> tmp_int >> tmp_char;
    
#if !defined(NDEBUG)
    std::cout << "setup costs" << std::endl;
    for (int r = 0; r < attributes; ++r)
    {
        for (int c = 0; c < attributes; ++c)
        {
            std::cout << setup_costs[r][c] << " ";
        }
        std::cout << std::endl;
    }
#endif
    
    setup_times.resize(attributes, std::vector<int>(attributes));
    is.ignore(LEN, '[');
    for (int r = 0; r < attributes; ++r)
    {
        is >> tmp_char; // skipping |
        for (int c = 0; c < attributes; ++c)
        {
            is >> setup_times[r][c];
            is >> tmp_char; // skipping ,
        }
    }
    // remove the line of all 0 (this was used just for the minizinc model)
    is >> tmp_char >> tmp_int >> tmp_char;
#if !defined(NDEBUG)
    std::cout << "setup times" << std::endl;
    for (int r = 0; r < attributes; ++r)
    {
        for (int c = 0; c < attributes; ++c)
        {
            std::cout << setup_times[r][c] << " ";
        }
        std::cout << std::endl;
    }
#endif
    
    is.ignore(LEN, '=');
    is >> machines;
#if !defined(NDEBUG)
    std::cout << "machines: " << machines << std::endl;
#endif
    
    min_cap.resize(machines);
    is.ignore(LEN, '[');
    for (int m = 0; m < machines; ++m)
    {
        is >> min_cap[m];
        is >> tmp_char; // skipping ,
    }
#if !defined(NDEBUG)
    std::cout << "min cap: ";
    for (int m = 0; m < machines; ++m)
    {
        std::cout << min_cap[m] << " ";
    }
    std::cout << std::endl;
#endif
    
    max_cap.resize(machines);
    is.ignore(LEN, '[');
    for (int m = 0; m < machines; ++m)
    {
        is >> max_cap[m];
        is >> tmp_char; // skipping ,
    }
#if !defined(NDEBUG)
    std::cout << "max cap: ";
    for (int m = 0; m < machines; ++m)
    {
        std::cout << max_cap[m] << " ";
    }
    std::cout << std::endl;
#endif
    
    initial_state.resize(machines);
    is.ignore(LEN, '[');
    for (int m = 0; m < machines; ++m)
    {
        is >> tmp_int;
        initial_state[m]  = tmp_int - 1; // -1 because in cpp we start with 0, so activities goes from 0 to activities-1
        is >> tmp_char; // skipping ,
    }
#if !defined(NDEBUG)
    std::cout << "initial state: ";
    for (int m = 0; m < machines; ++m)
    {
        std::cout << initial_state[m] << " ";
    }
    std::cout << std::endl;
#endif
    
    is.ignore(LEN, '=');
    is >> intervals;
#if !defined(NDEBUG)
    std::cout << "intervals: " << intervals << std::endl;
#endif
    
    m_a_s.resize(machines, std::vector<int>(intervals));
    is.ignore(LEN, '[');
    for (int m = 0; m < machines; ++m)
    {
        is >> tmp_char; // skipping |
        for (int i = 0; i < intervals; ++i)
        {
            is >> m_a_s[m][i];
            is >> tmp_char; // skipping ,
        }
    }
#if !defined(NDEBUG)
    std::cout << "start of machines intervals: " << std::endl;
    for (int m = 0; m < machines; ++m)
    {
        for (int i = 0; i < intervals; ++i)
        {
            std::cout << m_a_s[m][i] << " ";
        }
        std::cout << std::endl;
    }
#endif
    
    m_a_e.resize(machines, std::vector<int>(intervals));
    is.ignore(LEN, '[');
    for (int m = 0; m < machines; ++m)
    {
        is >> tmp_char; // skipping |
        for (int i = 0; i < intervals; ++i)
        {
            is >> m_a_e[m][i];
            is >> tmp_char; // skipping ,
        }
    }
#if !defined(NDEBUG)
    std::cout << "end of machines intervals: " << std::endl;
    for (int m = 0; m < machines; ++m)
    {
        for (int i = 0; i < intervals; ++i)
        {
            std::cout << m_a_e[m][i] << " ";
        }
        std::cout << std::endl;
    }
#endif
    
    is.ignore(LEN, '=');
    is >> jobs;
#if !defined(NDEBUG)
    std::cout << "jobs: " << jobs << std::endl;
#endif
    
    eligible_machine_set.resize(jobs);
    is.ignore(LEN, '=');
    for (int j = 0; j < jobs; ++j)
    {
        is >> tmp_char >> tmp_char;
        while (tmp_char != '}')
        {
            is >> tmp_int;
            eligible_machine_set[j].insert(tmp_int - 1); // -1 because we are counting from 0, so machines goes from 0 to machine - 1
            is >> tmp_char;
            if (tmp_char == '}')
            {
                break;
            }
        }
    }
#if !defined(NDEBUG)
    std::cout << "eligible machines for jobs" << std::endl;
    for (int j = 0; j < jobs; ++j)
    {
        for (auto s : eligible_machine_set[j])
        {
            std::cout << s << " ";
        }
        std::cout << std::endl;
    }
#endif
    
    // redundant datastructure
    eligible_machine_matrix.resize(machines, std::vector<bool>(jobs, false));
    for (int j = 0; j < jobs; ++j)
    {
        for (auto m : eligible_machine_set[j])
        {
            eligible_machine_matrix[m][j] = true;
        }
    }
    
    earliest_start.resize(jobs);
    is.ignore(LEN, '[');
    for (int j = 0; j < jobs; ++j)
    {
        is >> earliest_start[j];
        is >> tmp_char;
    }
#if !defined(NDEBUG)
    std::cout << "earliest start for job: ";
    for (int j=0; j < jobs; ++j)
    {
        std::cout << earliest_start[j] << " ";
    }
    std::cout << std::endl;
#endif
    
    latest_end.resize(jobs);
    is.ignore(LEN, '[');
    for (int j = 0; j < jobs; ++j)
    {
        is >> latest_end[j];
        is >> tmp_char;
    }
#if !defined(NDEBUG)
    std::cout << "latest end for job: ";
    for (int j=0; j < jobs; ++j)
    {
        std::cout << latest_end[j] << " ";
    }
    std::cout << std::endl;
#endif
    
    min_time.resize(jobs);
    is.ignore(LEN, '[');
    for (int j = 0; j < jobs; ++j)
    {
        is >> min_time[j];
        is >> tmp_char;
    }
#if !defined(NDEBUG)
    std::cout << "min time for job: ";
    for (int j=0; j < jobs; ++j)
    {
        std::cout << min_time[j] << " ";
    }
    std::cout << std::endl;
#endif
    
    max_time.resize(jobs);
    is.ignore(LEN, '[');
    for (int j = 0; j < jobs; ++j)
    {
        is >> max_time[j];
        is >> tmp_char;
    }
#if !defined(NDEBUG)
    std::cout << "max time for job: ";
    for (int j=0; j < jobs; ++j)
    {
        std::cout << max_time[j] << " ";
    }
    std::cout << std::endl;
#endif
    
    size.resize(jobs);
    is.ignore(LEN, '[');
    for (int j = 0; j < jobs; ++j)
    {
        is >> size[j];
        is >> tmp_char;
    }
#if !defined(NDEBUG)
    std::cout << "size for job: ";
    for (int j=0; j < jobs; ++j)
    {
        std::cout << size[j] << " ";
    }
    std::cout << std::endl;
#endif
    
    attribute.resize(jobs);
    is.ignore(LEN, '[');
    for (int j = 0; j < jobs; ++j)
    {
        is >> tmp_int;
        attribute[j] = tmp_int - 1; // because we start counting from one, therefore attributes goes from 0 to attributes-1
        is >> tmp_char;
    }
#if !defined(NDEBUG)
    std::cout << "attribute for job: ";
    for (int j=0; j < jobs; ++j)
    {
        std::cout << attribute[j] << " ";
    }
    std::cout << std::endl;
#endif
    
    is.ignore(LEN, '=');
    is >> upper_bound_integer_objective;
#if !defined(NDEBUG)
    std::cout << "upper bound integer objective: " << upper_bound_integer_objective << std::endl;
#endif
    
    is.ignore(LEN, '=');
    is >> mult_factor_total_runtime;
#if !defined(NDEBUG)
    std::cout << "mult. factor total runtime: " << mult_factor_total_runtime << std::endl;
#endif
    
    is.ignore(LEN, '=');
    is >> mult_factor_finished_toolate;
#if !defined(NDEBUG)
    std::cout << "mult. factor finished too late: " << mult_factor_finished_toolate << std::endl;
#endif
    
    is.ignore(LEN, '=');
    is >> mult_factor_total_setuptimes;
#if !defined(NDEBUG)
    std::cout << "mult. factor total setup times: " << mult_factor_total_setuptimes << std::endl;
#endif
    
    is.ignore(LEN, '=');
    is >> mult_factor_total_setupcosts;
#if !defined(NDEBUG)
    std::cout << "mult. factor total setup costs: " << mult_factor_total_setupcosts << std::endl;
#endif
    
    is.ignore(LEN, '=');
    is >> running_time_bound;
#if !defined(NDEBUG)
    std::cout << "running time bound: " << running_time_bound << std::endl;
#endif
    
    is.close();
    
}

std::ostream& operator<<(std::ostream& os, const OSP_Input& in)
{
    os << "CURRENTLY YOU ARE PRINTING THE INSTANCE CONSIDERING THE FACT THAT CPP COUNTS FROM 0." << std::endl;
    os << "Machines: " << in.machines << std::endl;
    os << "Jobs: " << in.jobs << std::endl;
    os << "Attributes: " << in.attributes << std::endl;
    os << "Intervals: " << in.intervals << std::endl;
    os << "Horizon: " << in.horizon << std::endl;
    os << "Setup costs:";
    for (int r = 0; r < in.attributes; ++r)
    {
        for (int c = 0; c < in.attributes; ++c)
        {
            os << " " << in.setup_costs[r][c] << " ";
        }
        os << "//";
    }
    os << std::endl;
    os << "Setup times:";
    for (int r = 0; r < in.attributes; ++r)
    {
        for (int c = 0; c < in.attributes; ++c)
        {
            os << " " << in.setup_times[r][c] << " ";
        }
        os << "//";
    }
    os << std::endl;
    os << "Max capacity per machine: ";
    for(int m = 0; m < in.machines; ++m)
    {
        os << in.max_cap[m] << " ";
    }
    os << std::endl;
    os << "Initial state per machine: ";
    for(int m = 0; m < in.machines; ++m)
    {
        os << in.initial_state[m] << " ";
    }
    os << std::endl;
    os << "Start and end of machine intervals: " << std::endl;
    for (int m = 0; m < in.machines; ++m)
    {
        os << "Machine " << m << ": ";
        for (int i = 0; i < in.intervals; ++i)
        {
            os << in.m_a_s[m][i] << " " << in.m_a_e[m][i] << " // ";
        }
        os << std::endl;
    }
    for (int j = 0; j < in.jobs; ++j)
    {
        os << "Charactieristics of job " << j << ": size " << in.size[j] << ", attribute " << in.attribute[j] << ", earliest start " << in.earliest_start[j] << ", latest end " << in.latest_end[j] << ", processing timing " << in.min_time[j] << "-->" << in.max_time[j] << std::endl;
    }
    os << "Upper bound to the integer objective: " << in.upper_bound_integer_objective << std::endl;
    os << "Mult. factor total run time: " << in.mult_factor_total_runtime << std::endl;
    os << "Mult. factor finished too late: " << in.mult_factor_finished_toolate << std::endl;
    os << "Mult. factor total set up times: " << in.mult_factor_total_setuptimes << std::endl;
    os << "Mult. factor total set up costs: " << in.mult_factor_total_setupcosts << std::endl;
    os << "Running time bound: " << in.running_time_bound << std::endl;
    return os;
}

OSP_Output::OSP_Output(const OSP_Input& my_in)
: in(my_in), 
job_to_batch_position(in.Jobs()),
batches_per_machine(in.Machines(), 0),
jobs_at_batch_position(in.Machines()),
batch_characteristics(in.Machines()),
batches_per_attribute(in.Attributes()),
number_tardy_jobs(0),
total_set_up_time(0),
total_set_up_cost(0),
cumulative_batch_processing_time(0),
not_scheduled_batches(0)
{}

OSP_Output& OSP_Output::operator=(const OSP_Output& out)
{
    // FIXME: check this is everything
    job_to_batch_position = out.job_to_batch_position;
    batches_per_machine = out.batches_per_machine;
    jobs_at_batch_position = out.jobs_at_batch_position;
    batch_characteristics = out.batch_characteristics;
    batches_per_attribute = out.batches_per_attribute;
    number_tardy_jobs = out.number_tardy_jobs;
    total_set_up_time = out.total_set_up_time;
    total_set_up_cost = out.total_set_up_cost;
    cumulative_batch_processing_time = out.cumulative_batch_processing_time;
    not_scheduled_batches = out.not_scheduled_batches;
    return *this;
}

void OSP_Output::PopulateAllFromScratch()
{
    // FIXME: check if this order is ok
#if !defined(NDEBUG)
    std::cout << "About to populate batches per machine" << std::endl;
#endif
    PopulateBatchesPerMachine();
#if !defined(NDEBUG)
    std::cout << "About to populate jobs at batch position" << std::endl;
#endif
    PopulateJobsAtBatchPosition();
#if !defined(NDEBUG)
    std::cout << "About to populate jobs at batch characteristics" << std::endl;
#endif
    PopulateBatchCharacteristics();
#if !defined(NDEBUG)
    std::cout << "About to populate jobs at batch per attribute" << std::endl;
#endif
    PopulateBatchesPerAttribute();
    
    CalculateAllCostsFromScratch();
}

// Actually, you don't need this because this is redundant with other information, anyway, might be useful
void OSP_Output::PopulateBatchesPerMachine()
{
    // position_per_machine counts how many batch are there for a given machine
    batches_per_machine.resize(in.Machines(), 0); // so we now this it ready
    for (int j = 0; j < in.Jobs(); ++j)
    {
        int mach = job_to_batch_position[j].first;
        int pos = job_to_batch_position[j].second;
        if (batches_per_machine[mach] <= pos)
        {
            batches_per_machine[mach] = pos +1;
        }
    }
}


void OSP_Output::PopulateJobsAtBatchPosition()
{
    jobs_at_batch_position.resize(in.Machines());
    for (int j = 0; j < in.Jobs(); ++j)
    {
        int mach = job_to_batch_position[j].first;
        int pos = job_to_batch_position[j].second;
        if (jobs_at_batch_position[mach].size() <= pos)
        {
            jobs_at_batch_position[mach].resize(pos+1);
        }
        jobs_at_batch_position[mach][pos].insert(j);
    }
    // FIXME: check if here it is pos or pos + 1
    // throw std::invalid_argument("Method not implemented");
}

void OSP_Output::PopulateBatchCharacteristics()
{
    batch_characteristics.clear();
    batch_characteristics.resize(in.Machines());
    for (int m = 0; m < in.Machines(); ++m)
    {
        batch_characteristics[m].resize(batches_per_machine[m]);
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            batch_characteristics[m][p] = CalculateBatchProperties(m, p);
        }
    }
}

void OSP_Output::PopulateBatchesPerAttribute()
{
    batches_per_attribute.clear();
    batches_per_attribute.resize(in.Attributes());
    for (int a = 0; a < in.Attributes(); ++a)
    {
        batches_per_attribute[a].clear();
    }
    for (int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            int a = batch_characteristics[m][p].attribute;
            batches_per_attribute[a].insert(std::make_pair(m, p));
        }
    }
}

Batch OSP_Output::CalculateBatchProperties(int m, int p)
{
    // retrieve the vector of jobs related to this batch
    std::set<int> jobs = jobs_at_batch_position[m][p];
    if (jobs.size() == 0)
    {
        throw std::invalid_argument("jobs should be populated here");
    }
    // determine the batch processing time and the size
    int size = 0;
    int batch_processing_time = in.MinTimeJob(*jobs.begin());
    int earliest_start = in.EarliestStartJob(*jobs.begin());
    for (int job : jobs)
    {
        if (batch_processing_time < in.MinTimeJob(job))
        {
            batch_processing_time = in.MinTimeJob(job);
        }
        if (earliest_start < in.EarliestStartJob(job))
        {
            earliest_start = in.EarliestStartJob(job);
        }
        size += in.SizeJob(job);
    }
    // get the attributes
    int attribute = in.AttributeJob(*jobs.begin());
#if !defined(NDEBUG)
    for (int job : jobs)
    {
        // std::cout << "Calculating batch properties: j " << job << " of att " << in.AttributeJob(job) << " in " << attribute << std::endl;
        assert(in.AttributeJob(job) == attribute);
    }
#endif
    
    // get infos on the previous batch
    int previous_pos = p - 1;
    int previous_attribute, previous_start_time, previous_end_time;
    if (previous_pos > -1)
    {
        previous_attribute = batch_characteristics[m][previous_pos].attribute;
        previous_start_time = batch_characteristics[m][previous_pos].start_time;
        previous_end_time = batch_characteristics[m][previous_pos].end_time;
    }
    else
    {
        previous_attribute = in.InitialStateMachine(m);
        previous_start_time = 0;
        previous_end_time = 0;
    }
    
    int setup_time = in.SetUpTime(previous_attribute, attribute);
    int setup_cost = in.SetUpCost(previous_attribute, attribute);
    int start_time, end_time;
    if (previous_start_time > in.Horizon()) // this means the previous batch was not scheduled
    {
        start_time = in.Horizon() + 1;
        end_time = in.Horizon() + 1;
    }
    else
    {
        start_time = CalculateBatchStartTime(m, earliest_start, setup_time, batch_processing_time, previous_end_time);
        end_time = start_time + batch_processing_time;
    }
    return {size, attribute, batch_processing_time, start_time, end_time, setup_cost, setup_time};
}

int OSP_Output::CalculateBatchStartTime(int machine, int earliest_start, int setup_time, int processing_time, int previous_end)
{
    // check the earliest possible start
    if (previous_end + setup_time > earliest_start)
    {
        earliest_start = previous_end + setup_time;
    }
    
    // check if it can hypothetically fit
    if (earliest_start + processing_time > in.Horizon())
    {
        return in.Horizon() + 1;
    }
    
    int start_in_machine = CalculateEarliestSuitableMachineIntervalStart(machine, earliest_start, setup_time, processing_time);
    
    return start_in_machine;
}

int OSP_Output::CalculateEarliestSuitableMachineIntervalStart(int machine, int earliest_start, int setup_time, int processing_time)
{
    int interval_index = -1;
    int earliest_start_in_interval = earliest_start;
    
    // find the first interval in list of machine availability intervals in which
    // (a) there is enough space for both setup time and processing time
    // (b) is late enough for earliestStartTime
    for(int s = 0; s < in.Intervals(); ++s)
    {
        // the batch cannot start before max(earliest_start, machine.AvailabilityStart[i] + setup_time)
        if(in.AvailabilityStart(machine, s) + setup_time > earliest_start)
        {
            earliest_start_in_interval = in.AvailabilityStart(machine, s) + setup_time;
        }
        // check if there is enough space in the interval
        if(earliest_start_in_interval - setup_time >= in.AvailabilityStart(machine, s)
           && earliest_start_in_interval + processing_time <= in.AvailabilityEnd(machine, s))
        {
            interval_index = s;
            break;
        }
    }
    // if no interval was found, return the DummyValue
    if (interval_index == - 1)
    {
        return in.Horizon() + 1;
    }
    else
    {
        return earliest_start_in_interval;
    }
}

void OSP_Output::CalculateAllCostsFromScratch()
{
    // CalculateTotalSetUpTime();
    CalculateTotalSetUpCost();
    CalculateNumberOfTardyJobs();
    CalculateCumulativeBatchProcessingTime();
    CalculateNotScheduledBatches();
}

void OSP_Output::CalculateTotalSetUpTime()
{
    total_set_up_time = 0;
    for (int m = 0; m < in.Machines(); ++m)
    {
        for (Batch batch : batch_characteristics[m])
        {
            if (batch.start_time <= in.Horizon())
            {
                total_set_up_time += batch.setup_time;
            }
        }
    }
}

void OSP_Output::CalculateTotalSetUpCost()
{
    total_set_up_cost = 0;
    for (int m = 0; m < in.Machines(); ++m)
    {
        for (Batch batch : batch_characteristics[m])
        {
            if (batch.start_time <= in.Horizon())
            {
                total_set_up_cost += batch.setup_cost;
            }
        }
    }
}

void OSP_Output::CalculateNumberOfTardyJobs()
{
    number_tardy_jobs = 0;
    for (int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            Batch batch = batch_characteristics[m][p];
            if (batch.start_time <= in.Horizon())
            {
                std::set<int> jobs = jobs_at_batch_position[m][p];
                for (int job : jobs)
                {
                    if (in.LatestEndJob(job) < batch.end_time)
                    {
                        number_tardy_jobs += 1;
                    }
                }
            }
        }
    }
}

void OSP_Output::CalculateCumulativeBatchProcessingTime()
{
    cumulative_batch_processing_time = 0;
    for (int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            Batch batch = batch_characteristics[m][p];
            if (batch.start_time <= in.Horizon())
            {
                cumulative_batch_processing_time += batch.batch_processing_time;
            }
        }
    }
}

void OSP_Output::CalculateNotScheduledBatches()
{
    not_scheduled_batches = 0;
    for (int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            Batch batch = batch_characteristics[m][p];
            if (batch.start_time > in.Horizon())
            {
                not_scheduled_batches += jobs_at_batch_position[m][p].size();
            }
        }
    }
}

void OSP_Output::CheckerForBatchCharacteristicsUpdate()
{
    // this is just to check you are modifying the entire batch_characteristics stucture
    std::vector<std::vector<Batch>> to_debug_batch_characteristics;
    to_debug_batch_characteristics.resize(in.Machines());
    for (int m = 0; m < in.Machines(); ++m)
    {
        to_debug_batch_characteristics[m].resize(batches_per_machine[m]);
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            to_debug_batch_characteristics[m][p] = CalculateBatchProperties(m,p);
        }
    }
    for (int m = 0; m < in.Machines(); ++m)
    {
    
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            Batch b1 = to_debug_batch_characteristics[m][p] ;
            Batch b2 = batch_characteristics[m][p];
            assert(b1.size == b2.size
                   && b1.attribute == b2.attribute
                   && b1.batch_processing_time == b2.batch_processing_time
                   && b1.start_time == b2.start_time
                   && b1.end_time == b2.end_time
                   && b1.setup_cost == b2.setup_cost
                   && b1.setup_time == b2.setup_time);
            /*
             if (b1.size != b2.size
                || b1.attribute != b2.attribute
                || b1.batch_processing_time != b2.batch_processing_time
                || b1.start_time != b2.start_time
                || b1.end_time != b2.end_time
                || b1.setup_cost != b2.setup_cost
                || b1.setup_time != b2.setup_time)
            {
                std::cout << "Big problems with CheckerForBatchCharacteristicsUpdate" << std::endl;
            }
             */
            
        }
    }
}

void OSP_Output::CheckerForBatchesPerAttributeUpdate()
{
    std::vector<std::set<std::pair<int,int>>> to_debug;
    to_debug.resize(in.Attributes());
    for (int m = 0; m < in.Machines(); ++m)
    {
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            int a = batch_characteristics[m][p].attribute;
            to_debug[a].insert(std::make_pair(m, p));
        }
    }
    for (int a = 0; a < in.Attributes(); ++a)
    {
        // std::cout << "attribute " << a << std::endl;
        assert(to_debug.size() == batches_per_attribute.size());
        assert(to_debug[a] == batches_per_attribute[a]);
        // if (to_debug.size() != batches_per_attribute.size()
        //    || to_debug[a] != batches_per_attribute[a])
        // {
        //    std::cout << "Big problems with CheckerForBatchesPerAttributeUpdate" << std::endl;
        // }
    }
}

void OSP_Output::CheckerForJobsAtBatchPositionUpdate()
{
    std::vector<std::vector<std::set<int>>> to_debug;
    to_debug.resize(in.Machines());
    for (int j = 0; j < in.Jobs(); ++j)
    {
        int mach = job_to_batch_position[j].first;
        int pos = job_to_batch_position[j].second;
        if (to_debug[mach].size() <= pos)
        {
            to_debug[mach].resize(pos+1);
        }
        to_debug[mach][pos].insert(j);
    }
    
    
    for (int m = 0; m < in.Machines(); ++m)
    {
        assert(jobs_at_batch_position[m].size() == to_debug[m].size());
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            assert(to_debug[m][p] == jobs_at_batch_position[m][p]);
        }
    }
}

void OSP_Output::CheckForNumberofBatchesUpdate()
{
    std::vector<int> to_debug;
    to_debug.resize(in.Machines(), 0); // so we now this it ready
    for (int j = 0; j < in.Jobs(); ++j)
    {
        int mach = job_to_batch_position[j].first;
        int pos = job_to_batch_position[j].second;
        if (to_debug[mach] <= pos)
        {
            to_debug[mach] = pos +1;
        }
    }
    for (int m = 0; m < in.Machines(); ++m)
    {
        assert(to_debug[m] == batches_per_machine[m]);
    }
}

void OSP_Output::SwapBatchesSameMachine (int m, int p1, int p2)
{
    std::set<int> jobs_1 = jobs_at_batch_position[m][p1];
    std::set<int> jobs_2 = jobs_at_batch_position[m][p2];
    
    // modify the job_to_batch_position
    for (int job : jobs_1)
    {
        job_to_batch_position[job].second = p2;
    }
    for (int job : jobs_2)
    {
        job_to_batch_position[job].second = p1;
    }
    
    // modify jobs_at_batch_position
    jobs_at_batch_position[m][p1].clear();
    jobs_at_batch_position[m][p1] = jobs_2;
    jobs_at_batch_position[m][p2].clear();
    jobs_at_batch_position[m][p2] = jobs_1;
    
    // modify batches_per_attribute
    // atribute of what was in position_1
    int a1 = AttributeJob(*jobs_1.begin());
    int a2 = AttributeJob(*jobs_2.begin());
    batches_per_attribute[a1].erase(std::make_pair(m, p1));
    batches_per_attribute[a2].erase(std::make_pair(m, p2));
    batches_per_attribute[a1].insert(std::make_pair(m, p2));
    batches_per_attribute[a2].insert(std::make_pair(m, p1));
    
    // modify batch_characteristics
    for (int p = p1; p < batches_per_machine[m]; ++p)
    {
        batch_characteristics[m][p] = CalculateBatchProperties(m, p);
    }
    
#if !defined(NDEBUG)
    // this is just to check you are modifying the entire batch_characteristics stucture
    CheckerForBatchCharacteristicsUpdate();
    CheckerForBatchesPerAttributeUpdate();
    CheckerForJobsAtBatchPositionUpdate();
    CheckForNumberofBatchesUpdate();
#endif
}

void OSP_Output::InsertBatchToNewPosition (int m, int o_p, int n_p)
{
    // std::cout << o_p << "-->" << n_p << std::endl;
    if (o_p == n_p)
    {
#if !defined(NDEBUG)
        CheckerForBatchCharacteristicsUpdate();
        CheckerForBatchesPerAttributeUpdate();
        CheckForNumberofBatchesUpdate();
#endif
        return;
    }
    else if (o_p < n_p)
    {
        // std::cout << "o_p < n_p" << std::endl;
        // do not have to change the number of batches per machine
        std::set<int> to_put_at_end = jobs_at_batch_position[m][o_p];
        for (int p = o_p; p < n_p; ++p)
        {
            std::set<int> to_anticipate = jobs_at_batch_position[m][p+1];
            jobs_at_batch_position[m][p].clear();
            jobs_at_batch_position[m][p] = to_anticipate;
        }
        jobs_at_batch_position[m][n_p].clear();
        jobs_at_batch_position[m][n_p] = to_put_at_end;
        
        // change the job position
        for (int p = 0; p < batches_per_machine[m]; ++p)
        {
            std::set<int> jobs = jobs_at_batch_position[m][p];
            for (int j : jobs)
            {
                job_to_batch_position[j] = std::make_pair(m,p);
            }
        }
        for (int p = o_p; p < batches_per_machine[m]; ++p)
        {
            std::pair<int,int> pos = std::make_pair(m, p);
            int o_a = batch_characteristics[m][p].attribute;
            batches_per_attribute[o_a].erase(pos);
            batch_characteristics[m][p] = CalculateBatchProperties(m, p);
            int n_a = batch_characteristics[m][p].attribute;
            batches_per_attribute[n_a].insert(pos);
        }
    }
    else if (o_p > n_p)
    {
        // std::cout << "o_p > n_p" << std::endl;
        std::set<int> to_put_at_beginning = jobs_at_batch_position[m][o_p];
        int attribute = in.AttributeJob(*(to_put_at_beginning.begin()));
        batches_per_attribute[attribute].erase(std::make_pair(m,o_p));
        for (int p = o_p; p > n_p; --p)
        {
            std::set<int> to_delay = jobs_at_batch_position[m][p-1];
            attribute = in.AttributeJob(*(to_delay.begin()));
            batches_per_attribute[attribute].erase(std::make_pair(m,p-1));
            jobs_at_batch_position[m][p].clear();
            jobs_at_batch_position[m][p] = to_delay;
            batches_per_attribute[attribute].insert(std::make_pair(m,p));
        }
        jobs_at_batch_position[m][n_p].clear();
        jobs_at_batch_position[m][n_p] = to_put_at_beginning;
        attribute = in.AttributeJob(*(to_put_at_beginning.begin()));
        batches_per_attribute[attribute].insert(std::make_pair(m,n_p));
        // change the job position
        for (int p = n_p; p <= o_p; ++p)
        {
            std::set<int> jobs = jobs_at_batch_position[m][p];
            for (int j : jobs)
            {
                job_to_batch_position[j] = std::make_pair(m,p);
            }
        }
        for (int p = n_p; p < batches_per_machine[m]; ++p)
        {
            // std::pair<int,int> pos = std::make_pair(m, p);
            //int o_a = batch_characteristics[m][p].attribute;
            //batches_per_attribute[o_a].erase(pos);
            batch_characteristics[m][p] = CalculateBatchProperties(m, p);
            //int n_a = batch_characteristics[m][p].attribute;
            // batches_per_attribute[n_a].insert(pos);
        }
    }
    
#if !defined(NDEBUG)
    // this is just to check you are modifying the entire batch_characteristics stucture
    CheckerForBatchCharacteristicsUpdate();
    CheckerForBatchesPerAttributeUpdate();
    CheckerForJobsAtBatchPositionUpdate();
    CheckForNumberofBatchesUpdate();
#endif
}


bool OSP_Output::IsJobCompatibleForBatch(int job, int machine, int position) const
{
    // std::cout << "in is compatible for batch" << std::endl;
    
    Batch batch = batch_characteristics[machine][position];
    if(job_to_batch_position[job].first == machine && job_to_batch_position[job].second == position)
    {
        return false;
    }
    // std::cout << "machine for job" << std::endl;
    // the new batch should be on a machine eligible to the job.
    if (!in.IsMachineEligible(machine, job))
    {
        // std::cout << "machine " << machine << " not eligible for job " << job << std::endl;
        return false;
    }
    // std::cout << "attribute" << std::endl;
    // the new batch should have the same attribute
    if (in.AttributeJob(job) != batch.attribute)
    {
        // std::cout << "batch attribute " << batch.attribute << " not eligible for attribute " << in.AttributeJob(job)  <<  " of job " << job << std::endl;
        return false;
    }
    // std::cout << "processing time" << std::endl;
    // the new batch should be ok with minimal and maximal processing time
    bool matching_processing_time = false;
    // std::cout << "batch processing time is " << batch.batch_processing_time << std::endl;
    if (in.MinTimeJob(job) <= batch.batch_processing_time && batch.batch_processing_time <= in.MaxTimeJob(job))
    {
        matching_processing_time = true;
    }
    else if (in.MinTimeJob(job) > batch.batch_processing_time) // if the current processing time is too short, check if it can be extended
    {
        std::set<int> jobs = jobs_at_batch_position[machine][position];
        
        int first_job = *jobs.begin();
        int min_max_processing_time = in.MaxTimeJob(first_job);
        for (int current : jobs)
        {
            if (min_max_processing_time > in.MaxTimeJob(current))
            {
                min_max_processing_time = in.MaxTimeJob(current);
            }
        }
        if (in.MinTimeJob(job) <= min_max_processing_time)
        {
            matching_processing_time = true;
        }
    }
    if (!matching_processing_time)
    {
        // std::cout << "batch processing time not mathcin" << std::endl;
        return false;
    }
    // std::cout << "size" << std::endl;
    // the new batch should have enough capacity
    if (batch.size + in.SizeJob(job) > in.MaxCapacityMachine(machine))
    {
        // std::cout << "batch size time not matching" << std::endl;
        return false;
    }
    return true;
}

void OSP_Output::InsertJobInExistingBatch(int job, std::pair<int, int> old_machine_position, std::pair<int, int> new_machine_position)
{
    // two cases: the job is alone in the previous batch or the job is not alone
    bool is_alone = false;
    if (jobs_at_batch_position[old_machine_position.first][old_machine_position.second].size() == 1)
    {
        is_alone = true;
    }
    if (is_alone)
    {
        // in this case you are alone
        // you need to put forward the old batches
        int attr = in.AttributeJob(job);
        batches_per_attribute[attr].erase(std::make_pair(old_machine_position.first, old_machine_position.second));
        int total_batches = batches_per_machine[old_machine_position.first];
        for(int p = old_machine_position.second; p < total_batches -1; ++p)
        {
            std::set<int> jobs_to_anticipate = jobs_at_batch_position[old_machine_position.first][p+1];
            jobs_at_batch_position[old_machine_position.first][p].clear();
            jobs_at_batch_position[old_machine_position.first][p] = jobs_to_anticipate;
            for (int j : jobs_to_anticipate)
            {
                job_to_batch_position[j].second = p;
            }
            attr = in.AttributeJob(*jobs_to_anticipate.begin());
            batches_per_attribute[attr].erase(std::make_pair(old_machine_position.first, p+1));
            batches_per_attribute[attr].insert(std::make_pair(old_machine_position.first, p));
        }
        jobs_at_batch_position[old_machine_position.first].resize(total_batches - 1);
        batch_characteristics[old_machine_position.first].resize(total_batches - 1);
        batches_per_machine[old_machine_position.first] = batches_per_machine[old_machine_position.first] - 1;
        total_batches = batches_per_machine[old_machine_position.first];
        for (int p = old_machine_position.second; p < total_batches; ++p)
        {
            batch_characteristics[old_machine_position.first][p] = CalculateBatchProperties(old_machine_position.first, p);
        }
        // if you are on the same machine and the new machine is after the old, then you have to decrease of one the new position
        if (new_machine_position.first == old_machine_position.first && new_machine_position.second > old_machine_position.second)
        {
            new_machine_position.second = new_machine_position.second -1;
        }
    }
    else
    {
        jobs_at_batch_position[old_machine_position.first][old_machine_position.second].erase(job);
        for (int p = old_machine_position.second; p < batches_per_machine[old_machine_position.first]; ++p)
        {
            batch_characteristics[old_machine_position.first][p] = CalculateBatchProperties(old_machine_position.first, p);
        }
    }
    // now here you insert the job in the new position
    jobs_at_batch_position[new_machine_position.first][new_machine_position.second].insert(job);
    job_to_batch_position[job] = new_machine_position;
    for (int p = new_machine_position.second;  p < batches_per_machine[new_machine_position.first]; ++p)
    {
        batch_characteristics[new_machine_position.first][p] = CalculateBatchProperties(new_machine_position.first, p);
    }
#if !defined(NDEBUG)
    // this is just to check you are modifying the entire batch_characteristics stucture
    CheckerForBatchCharacteristicsUpdate();
    CheckerForBatchesPerAttributeUpdate();
    CheckerForJobsAtBatchPositionUpdate();
    CheckForNumberofBatchesUpdate();
#endif
}

void OSP_Output::InsertJobToNewBatch (int job, std::pair<int,int> old_machine_position, std::pair<int,int> new_machine_position, bool is_alone)
{
    if (is_alone && old_machine_position.first == new_machine_position.first)
    {
        //std::cout << "case alone and same machine " << std::endl;
        // in this case we are simply inserting the batch to a new position
        InsertBatchToNewPosition(old_machine_position.first, old_machine_position.second, new_machine_position.second);
    }
    else // if it is going to a new machine
    {
        // you take care of the old machine
        if (is_alone) // if in the old machine, the job is alone
        {
            // std::cout << "case alone and NOT same machine " << std::endl;
            // update attributes per batch
            // update jobs at position
            // update position of all the jobs
            // decrease the number of batches
            // recalculate all the batches
            // move everything of one slot, starting from o_p to batches-1
            int attr = in.AttributeJob(job);
            batches_per_attribute[attr].erase(std::make_pair(old_machine_position.first, old_machine_position.second));
            int total_batches = batches_per_machine[old_machine_position.first];
            for(int p = old_machine_position.second; p < total_batches -1; ++p)
            {
                std::set<int> jobs_to_anticipate = jobs_at_batch_position[old_machine_position.first][p+1];
                jobs_at_batch_position[old_machine_position.first][p].clear();
                jobs_at_batch_position[old_machine_position.first][p] = jobs_to_anticipate;
                for (int j : jobs_to_anticipate)
                {
                    job_to_batch_position[j].second = p;
                }
                attr = in.AttributeJob(*jobs_to_anticipate.begin());
                batches_per_attribute[attr].erase(std::make_pair(old_machine_position.first, p+1));
                batches_per_attribute[attr].insert(std::make_pair(old_machine_position.first, p));
            }
            jobs_at_batch_position[old_machine_position.first].resize(total_batches - 1);
            batch_characteristics[old_machine_position.first].resize(total_batches - 1);
            batches_per_machine[old_machine_position.first] = batches_per_machine[old_machine_position.first] - 1;
            total_batches = batches_per_machine[old_machine_position.first];
            for (int p = old_machine_position.second; p < total_batches; ++p)
            {
                batch_characteristics[old_machine_position.first][p] = CalculateBatchProperties(old_machine_position.first, p);
            }
        }
        else // else
        {
            // std::cout << "case not alone" << std::endl;
            // remove the batch from the old jobs at batch
            jobs_at_batch_position[old_machine_position.first][old_machine_position.second].erase(job);
            // recalculate all batch characteristics
            for (int p = old_machine_position.second; p < batches_per_machine[old_machine_position.first]; ++p)
            {
                batch_characteristics[old_machine_position.first][p] = CalculateBatchProperties(old_machine_position.first, p);
            }
        }
        
        // at this point you need to update the new machine
        // you insert it at the end (everywhere!!)
        int end_position = batches_per_machine[new_machine_position.first];
        jobs_at_batch_position[new_machine_position.first].resize(end_position + 1);
        jobs_at_batch_position[new_machine_position.first][end_position].insert(job);
        batches_per_machine[new_machine_position.first] = batches_per_machine[new_machine_position.first] + 1;
#if !defined(NDEBUG)
        assert(batches_per_machine[new_machine_position.first] == (int) jobs_at_batch_position[new_machine_position.first].size());
#endif
        job_to_batch_position[job] = std::make_pair(new_machine_position.first, end_position);
        batches_per_attribute[in.AttributeJob(job)].insert(std::make_pair(new_machine_position.first, end_position));
        batch_characteristics[new_machine_position.first].resize(batches_per_machine[new_machine_position.first]);
        batch_characteristics[new_machine_position.first][end_position] = CalculateBatchProperties(new_machine_position.first,end_position);
        // batch_characteristics[new_machine_position.first].push_back(CalculateBatchProperties(new_machine_position.first,end_position));
        // now you take that job and you insert where necessary
        InsertBatchToNewPosition(new_machine_position.first, end_position, new_machine_position.second);
    
    }
#if !defined(NDEBUG)
    // std::cout << this << std::endl;
    // this is just to check you are modifying the entire batch_characteristics stucture
    CheckerForBatchCharacteristicsUpdate();
    CheckerForBatchesPerAttributeUpdate();
    CheckerForJobsAtBatchPositionUpdate();
    CheckForNumberofBatchesUpdate();
#endif
}

void OSP_Output::InsertBatchToNewMachine (std::set<int> jobs_to_move, std::pair<int,int> old_position, std::pair<int,int> new_position)
{
    // if the two machines are equal, then this is a case where the move correspons to an insertion
    if (old_position.first == new_position.first)
    {
        InsertBatchToNewPosition(old_position.first, old_position.second, new_position.second);
    }
    // if you are moving just one job, then this is a case where you are moving job to new batch
    else if (jobs_to_move.size() == 1)
    {
        bool is_alone = false;
        if (jobs_at_batch_position[old_position.first][old_position.second].size() == 1)
        {
            is_alone = true;
        }
        InsertJobToNewBatch(*jobs_to_move.begin(), old_position, new_position, is_alone);
    }
    // at this point you could have two cases, you are moving some of the jobs or you are moving the entire batch to a new position
    else
    {
        // you are moving the entire batch
        if (jobs_to_move.size() == jobs_at_batch_position[old_position.first][old_position.second].size())
        {
            int attr = batch_characteristics[old_position.first][old_position.second].attribute;
            batches_per_attribute[attr].erase(std::make_pair(old_position.first, old_position.second));
            int total_batches = batches_per_machine[old_position.first];
            for(int p = old_position.second; p < total_batches -1; ++p)
            {
                std::set<int> jobs_to_anticipate = jobs_at_batch_position[old_position.first][p+1];
                jobs_at_batch_position[old_position.first][p].clear();
                jobs_at_batch_position[old_position.first][p] = jobs_to_anticipate;
                for (int j : jobs_to_anticipate)
                {
                    job_to_batch_position[j].second = p;
                }
                attr = in.AttributeJob(*jobs_to_anticipate.begin());
                batches_per_attribute[attr].erase(std::make_pair(old_position.first, p+1));
                batches_per_attribute[attr].insert(std::make_pair(old_position.first, p));
            }
            jobs_at_batch_position[old_position.first].resize(total_batches - 1);
            batch_characteristics[old_position.first].resize(total_batches - 1);
            batches_per_machine[old_position.first] = batches_per_machine[old_position.first] - 1;
            total_batches = batches_per_machine[old_position.first];
            for (int p = old_position.second; p < total_batches; ++p)
            {
                batch_characteristics[old_position.first][p] = CalculateBatchProperties(old_position.first, p);
            }
        }
        else
        {
            // remove the jobs for jobs_at_batch_position
            for (int j : jobs_to_move)
            {
                jobs_at_batch_position[old_position.first][old_position.second].erase(j);
            }
            // recalculate the batch_characteristics
            int total_batches = batches_per_machine[old_position.first];
            for (int p = old_position.second; p < total_batches; ++p)
            {
                batch_characteristics[old_position.first][p] = CalculateBatchProperties(old_position.first, p);
            }
        }
        // now you add the new thing at the end of the machine
        // now you put it in the right place of a new machine
        int end_position = batches_per_machine[new_position.first];
        jobs_at_batch_position[new_position.first].resize(end_position + 1);
        jobs_at_batch_position[new_position.first][end_position].clear();
        jobs_at_batch_position[new_position.first][end_position] = jobs_to_move;
        batches_per_machine[new_position.first] = batches_per_machine[new_position.first] + 1;
#if !defined(NDEBUG)
        assert(batches_per_machine[new_position.first] == (int) jobs_at_batch_position[new_position.first].size());
#endif
        for (int j : jobs_to_move)
        {
            job_to_batch_position[j] = std::make_pair(new_position.first, end_position);
        }
        batches_per_attribute[in.AttributeJob(*jobs_to_move.begin())].insert(std::make_pair(new_position.first, end_position));
        batch_characteristics[new_position.first].resize(batches_per_machine[new_position.first]);
        batch_characteristics[new_position.first][end_position] = CalculateBatchProperties(new_position.first,end_position);
        // batch_characteristics[new_machine_position.first].push_back(CalculateBatchProperties(new_machine_position.first,end_position));
        // now you take that job and you insert where necessary
        InsertBatchToNewPosition(new_position.first, end_position, new_position.second);
    }
    
#if !defined(NDEBUG)
    // this is just to check you are modifying the entire batch_characteristics stucture
    CheckerForBatchCharacteristicsUpdate();
    CheckerForBatchesPerAttributeUpdate();
    CheckerForJobsAtBatchPositionUpdate();
    CheckForNumberofBatchesUpdate();
#endif
}

void OSP_Output::InverseBatchesInMachine(int m, int p_1, int p_2)
{
    // if position 2 is the consecutive of position 1, than you could simply swap the two positions
    if (p_1 + 1 == p_2)
    {
        SwapBatchesSameMachine(m, p_1, p_2);
    }
    else // otherwise you need to invert everything
    {
        // remove the batches from the attribute
        for (int p = p_1; p <= p_2; ++p)
        {
            int a = batch_characteristics[m][p].attribute;
            batches_per_attribute[a].erase(std::make_pair(m, p));
        }
        // reverse jobs at batch position
        std::reverse(jobs_at_batch_position[m].begin() + p_1, jobs_at_batch_position[m].begin() + p_2 + 1);

        // update job to batch position and attributes
        for (int p = p_1; p <= p_2; ++p)
        {
            std::pair<int,int> machine_position = std::make_pair(m, p);
            std::set<int> jobs = jobs_at_batch_position[m][p];
            int a = in.AttributeJob(*jobs.begin());
            for (int job : jobs)
            {
                job_to_batch_position[job] = machine_position;
            }
            batches_per_attribute[a].insert(machine_position);
        }
        
        // update batch characteristics
        for (int p = p_1; p < batches_per_machine[m]; ++p)
        {
            batch_characteristics[m][p] = CalculateBatchProperties(m, p);
        }
    }
#if !defined(NDEBUG)
    // this is just to check you are modifying the entire batch_characteristics stucture
    CheckerForBatchCharacteristicsUpdate();
    CheckerForBatchesPerAttributeUpdate();
    CheckerForJobsAtBatchPositionUpdate();
    CheckForNumberofBatchesUpdate();
#endif
}

std::ostream& operator<< (std::ostream& os, const OSP_Output& out)
{
    /*
    for (int j = 0; j < out.Jobs(); ++j)
    {
        os << "j " << j << " m " << out.job_to_batch_position[j].first << " p " << out.job_to_batch_position[j].second << std::endl;
    }
    */
    /*
    os << "This is the print to compare with the warm start (therefore you have a plus one): " << std::endl;
    */
    // os << "horizon is " << out.Horizon() << std::endl;
    
    os << "\"batch_for_job\": [";
    for (int j = 0; j < out.Jobs(); ++j)
    {
        os << out.job_to_batch_position[j].second + 1 << ",";
    }
    os << "],"<< std::endl;
    os << "\"machine_for_job\": [";
    for (int j = 0; j < out.Jobs(); ++j)
    {
        os << out.job_to_batch_position[j].first + 1 << ",";
    }
    os <<  "],"<< std::endl;
    
    os << "\"start_time\": [" ;
    for (int m = 0; m < out.Machines() ; ++m)
    {
        for (int p = 0; p < out.GetBatchesPerMachine(m); ++p)
        {
            Batch batch = out.GetBatchCharacteristics(m, p);
            os << batch.start_time << ",";
        }
    }
    os <<  "],"<< std::endl;
    /*
    os << "batch size: " ;
    for (int m = 0; m < out.Machines() ; ++m)
    {
        for (int p = 0; p < out.GetBatchesPerMachine(m); ++p)
        {
            Batch batch = out.GetBatchCharacteristics(m, p);
            os << batch.size << " ";
        }
    }
    os << std::endl;
    */
    os << "\"batch_processing_time\": [" ;
    for (int m = 0; m < out.Machines() ; ++m)
    {
        for (int p = 0; p < out.GetBatchesPerMachine(m); ++p)
        {
            Batch batch = out.GetBatchCharacteristics(m, p);
            os << batch.batch_processing_time << ",";
        }
    }
    os <<  "],"<< std::endl;
    
    os << "\"tardy_jobs_are\": [" ;
    for (int j = 0; j < out.Jobs() ; ++j)
    {
        int m = out.GetJobToBatchPosition(j).first;
        int p = out.GetJobToBatchPosition(j).second;
        Batch batch = out.GetBatchCharacteristics(m, p);
        if (batch.end_time > out.LatestEndJob(j))
        {
            os << j + 1 << ",";
        }
    }
    os <<  "],"<< std::endl;
    
    /*
    os << "batches per machines: ";
    for(int m = 0; m < out.Machines(); ++m)
    {
        os << m << ": " << out.GetBatchesPerMachine(m) << std::endl;
    }
    
    os << "batches per attribute: ";
    for (int a = 0; a < out.Attributes(); ++a)
    {
        os << a << ": ";
        for (auto b : out.GetBatchesPerAttribute(a))
        {
            os << b.first << "//" << b.second  << " ";
        }
        os << std::endl;
    }
    
    os << "jobs at batch position" << std::endl;
    for (int m = 0; m < out.Machines(); ++m)
    {
        for (int p = 0; p < out.GetBatchesPerMachine(m); ++p)
        {
            os << m << " // " << p << ": ";
            for (int j : out.GetJobsAtBatchPosition(m, p))
            {
                os << j << " ";
            }
            os << std::endl;
        }
    }
  
    // costs "\"tardy jobs are\":
     */
    os << "\"n_tardy_jobs\": " << out.GetNumberOfTardyJobs() << "," << std::endl;
    os << "\"cumulative_processing_times\": " << out.GetCumulativeBatchProcessingTime() << "," << std::endl;
    // os << "Total setup time: " << out.GetTotalSetUpTime() << std::endl;
    os << "\"set_up_costs\": " << out.GetTotalSetUpCost() << "," << std::endl;
    os << "\"n_not_scheduled_jobs\": " << out.GetNotScheduledBatches() << std::endl;

    
     return os;
}

std::istream& operator>>(std::istream& is, OSP_Output& out)
{
    // FIXME: add checkers
    char tmp_char;
    int j, m, p;
    do{
        is >> tmp_char >> j >> tmp_char >> m >> tmp_char >> p;
        out.job_to_batch_position[j] = std::make_pair(m, p);
    } while( j != out.Jobs());
    
    out.PopulateAllFromScratch();
    return is;
}

bool operator==(const OSP_Output& out1, const OSP_Output& out2)
{
    for (int i = 0; i < out1.Jobs(); ++i)
    {
        if (out1.job_to_batch_position[i] != out2.job_to_batch_position[i])
        {
            return false;
        }
    }
    return true;
}

bool operator==(const SwapConsecutiveBatchesMove& m1, const SwapConsecutiveBatchesMove& m2)
{
    return m1.machine == m2.machine
       && (m1.position_1 == m2.position_1
           || m1.position_1 == m2.position_2
           || m1.position_2 == m2.position_2
           || m1.position_2 == m2.position_1);
    // FIXME: actually, since we are constructing the moves taking as position_2 simply position_1 + 1, the checker on the second is not necessary
}

bool operator!=(const SwapConsecutiveBatchesMove& m1, const SwapConsecutiveBatchesMove& m2)
{
    return m1.machine != m2.machine
        && (m1.position_1 != m2.position_1
            && m1.position_1 != m2.position_2
            && m1.position_2 != m2.position_2
            && m1.position_2 != m2.position_1);
    // FIXME: actually, since we are constructing the moves taking as position_2 simply position_1 + 1, the checker on the second is not necessary
}

bool operator<(const SwapConsecutiveBatchesMove& m1, const SwapConsecutiveBatchesMove& m2)
{
    return m1.machine < m2.machine
        || (m1.machine == m2.machine && m1.position_1 < m2.position_1)
        || (m1.machine == m2.machine && m1.position_1 == m2.position_1 && m1.position_2 < m2.position_2);
    // this is actually just a case for the frequency based components. It is here more for historical purposes.
}

std::ostream& operator<<(std::ostream& os, const SwapConsecutiveBatchesMove& m)
{
    os << m.machine << ": " << m.position_1 << " <--> " << m.position_2 << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, SwapConsecutiveBatchesMove& m)
{
    return is;
}

// FIXME: double check the operators for BatchToNewPositionMove

bool operator==(const BatchToNewPositionMove& m1, const BatchToNewPositionMove& m2)
{
    return m1.machine == m2.machine
        && m1.old_position == m2.old_position
        && m1.new_position == m2.new_position;
}

bool operator!=(const BatchToNewPositionMove& m1, const BatchToNewPositionMove& m2)
{
    return m1.machine != m2.machine
        || (m1.machine == m2.machine && m1.old_position != m2.old_position)
        || (m1.machine == m2.machine && m1.old_position == m2.old_position && m1.new_position != m2.new_position);
}

bool operator<(const BatchToNewPositionMove& m1, const BatchToNewPositionMove& m2)
{
    return m1.machine < m2.machine;
    // this is actually just a case for the frequency based components. It is here more for historical purposes.
}

std::ostream& operator<<(std::ostream& os, const BatchToNewPositionMove& m)
{
    os << m.machine << ": " << m.old_position << " --> " << m.new_position << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, BatchToNewPositionMove& m)
{
    return is;
}


bool operator==(const JobToExistingBatch& m1, const JobToExistingBatch& m2)
{
    return m1.job == m2.job;
        // && m1.new_machine == m2.new_machine
        // && m1.new_position == m2.old_position;
}

bool operator!=(const JobToExistingBatch& m1, const JobToExistingBatch& m2)
{
    return m1.job != m2.job ;
      //  || (m1.job == m2.job && m1.new_machine != m2.new_machine)
      //  || (m1.job == m2.job && m1.new_machine == m2.new_machine && m1.new_position != m2.old_position);
}

bool operator<(const JobToExistingBatch& m1, const JobToExistingBatch& m2)
{
    return m1.job < m2.job;
    // this is actually just a case for the frequency based components. It is here more for historical purposes.
}

std::ostream& operator<<(std::ostream& os, const JobToExistingBatch& m)
{
    os << m.job << ": <" << m.old_machine << "," << m.old_position << "> --> <" << m.new_machine << "," << m.new_position << ">" << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, JobToExistingBatch& m)
{
    return is;
}

bool operator==(const JobToNewBatch& m1, const JobToNewBatch& m2)
{
    return m1.job == m2.job;
        // && m1.old_position == m2.old_position
        // && m1.new_position == m2.new_position;
}

bool operator!=(const JobToNewBatch& m1, const JobToNewBatch& m2)
{
    return m1.job != m2.job;
        // || (m1.job == m2.job && m1.old_position != m2.old_position)
        // || (m1.job == m2.job && m1.old_position == m2.old_position && m1.new_position != m2.new_position);
}

bool operator<(const JobToNewBatch& m1, const JobToNewBatch& m2)
{
    return m1.job < m2.job;
}

std::ostream& operator<<(std::ostream& os, const JobToNewBatch& m)
{
    os << m.job << ": <" << m.old_position.first << "," << m.old_position.second << "> --> <" << m.new_position.first << "," << m.new_position.second << ">" << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, JobToNewBatch& m)
{
    return is;
}


bool operator==(const Batch& b1, const Batch& b2)
{
    return
    b1.size == b2.size
    && b1.attribute == b2.attribute
    && b1.batch_processing_time == b2.batch_processing_time
    && b1.start_time == b2.start_time
    && b1.end_time == b2.end_time
    && b1.setup_cost == b2.setup_cost
    && b1.setup_time == b2.setup_time;
}

bool operator==(const BatchToNewMachine& m1, const BatchToNewMachine& m2)
{
    return m1.jobs_to_move == m2.jobs_to_move
        && m1.old_machine_position == m2.old_machine_position
        && m1.new_machine_position == m2.new_machine_position;
}

bool operator!=(const BatchToNewMachine& m1, const BatchToNewMachine& m2)
{
    return m1.jobs_to_move.size() != m2.jobs_to_move.size()
        || m1.jobs_to_move != m2.jobs_to_move
        || (m1.jobs_to_move == m2.jobs_to_move && m1.old_machine_position != m2.old_machine_position)
        || (m1.jobs_to_move == m2.jobs_to_move && m1.old_machine_position == m2.old_machine_position && m1.new_machine_position != m2.new_machine_position);
}

bool operator<(const BatchToNewMachine& m1, const BatchToNewMachine& m2)
{
    return m1.jobs_to_move.size() < m2.jobs_to_move.size();
}

std::ostream& operator<<(std::ostream& os, const BatchToNewMachine& m)
{
    os << "{ ";
    for (int j : m.jobs_to_move)
    {
        os << j << " ";
    }
    os << " } : <" << m.old_machine_position.first << "," << m.old_machine_position.second << "> --> <" << m.new_machine_position.first
    << "," << m.new_machine_position.second << ">" << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, BatchToNewMachine& mv)
{
    return is;
}

bool operator==(const SwapBatches& m1, const SwapBatches& m2)
{
    return m1.machine == m2.machine
       && m1.position_1 == m2.position_1
       && m1.position_2 == m2.position_2;
}

bool operator!=(const SwapBatches& m1, const SwapBatches& m2)
{
    return m1.machine != m2.machine
        || m1.position_1 != m2.position_1
        || m1.position_2 != m2.position_2;
}

bool operator<(const SwapBatches& m1, const SwapBatches& m2)
{
    return m1.machine < m2.machine
        || (m1.machine == m2.machine && m1.position_1 < m2.position_1)
        || (m1.machine == m2.machine && m1.position_1 == m2.position_1 && m1.position_2 < m2.position_2);
    // this is actually just a case for the frequency based components. It is here more for historical purposes.
}

std::ostream& operator<<(std::ostream& os, const SwapBatches& m)
{
    os << m.machine << ": " << m.position_1 << " <--> " << m.position_2 << std::endl;
    return os;
}


std::istream& operator>>(std::istream& is, SwapBatches& m)
{
    return is;
}

bool operator==(const InvertBatchesInMachine& m1, const InvertBatchesInMachine& m2)
{
    return m1.machine == m2.machine
       && m1.position_1 == m2.position_1
       && m1.position_2 == m2.position_2;
}

bool operator!=(const InvertBatchesInMachine& m1, const InvertBatchesInMachine& m2)
{
    return m1.machine != m2.machine
        || m1.position_1 != m2.position_1
        || m1.position_2 != m2.position_2;
}

bool operator<(const InvertBatchesInMachine& m1, const InvertBatchesInMachine& m2)
{
    return m1.machine < m2.machine
        || (m1.machine == m2.machine && m1.position_1 < m2.position_1)
        || (m1.machine == m2.machine && m1.position_1 == m2.position_1 && m1.position_2 < m2.position_2);
    // this is actually just a case for the frequency based components. It is here more for historical purposes.
}

std::ostream& operator<<(std::ostream& os, const InvertBatchesInMachine& m)
{
    os << m.machine << ": " << m.position_1 << " <--> " << m.position_2 << std::endl;
    return os;
}


std::istream& operator>>(std::istream& is, InvertBatchesInMachine& m)
{
    return is;
}
