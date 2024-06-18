#include "OSP_helpers.hh"

#include <chrono>
#include <string>
#include <cmath>

using namespace std::chrono;

using namespace EasyLocal::Debug;

int main(int argc, const char* argv[])
{
#if !defined(NDEBUG)
    std::cout << "This code is running in DEBUG mode" << std::endl;
#endif
    ParameterBox main_parameters("main", "Main Program options");

    // The set of arguments added are the following:
    // TODO: greedy_solution should be a flag
    Parameter<int> greedy_solution("greedy_solution", "Only run the greedy heuristic", main_parameters);
    Parameter<int> irace("irace", "Irace version, means that the output (only the cost) will be printed", main_parameters);
    Parameter<std::string> instance("instance", "Input instance", main_parameters);
    Parameter<unsigned int> seed("seed", "Random seed", main_parameters);
    Parameter<std::string> method("method", "Solution method (empty for tester)", main_parameters);
    Parameter<std::string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
    Parameter<std::string> output_file("output_file", "Name of the output file, otherwise the output is only printed", main_parameters);
    
    Parameter<double> theta_min("theta_min", "Multiplier for lower bound to the number of iterations of the ts", main_parameters);
    Parameter<double> theta_max("theta_min", "Multiplier for upper bound to the number of iterations of the ts", main_parameters);
    Parameter<double> theta("theta", "Multiplier to the number of iterations of the ts", main_parameters);

    Parameter<double> ita("ita", "Multiplier to the steps in the lahc", main_parameters);
    
    // Parameter<double> swap_rate("swap_rate", "Rate for the swap batches in the same machine move", main_parameters);
    // Parameter<double> insert_rate("insert_rate", "Rate for the insertion of a batch in a new position in the same machine", main_parameters);
    // Parameter<double> job_existing_batch_rate("job_existing_batch_rate", "Rate for the insertion of a job in an existing batch", main_parameters);
    // Parameter<double> jobs_new_batch_rate("jobs_new_batch_rate", "Rate for the insertion of a job in a new batch", main_parameters);
    // Parameter<double> inverse_rate("inverse_rate", "Rate for the inverse move", main_parameters);
    
    Parameter<double> swap_param("swap_param", "Parameter for the swap batches in the same machine move", main_parameters);
    Parameter<double> insert_param("insert_param", "Parameter for the insertion of a batch in a new position in the same machine", main_parameters);
    Parameter<double> job_existing_batch_param("job_existing_batch_param", "Parameter for the insertion of a job in an existing batch", main_parameters);
    Parameter<double> single_job_new_batch_param("single_job_new_batch_param", "Parameter for the insertion of a job in a new batch", main_parameters);
    Parameter<double> jobs_new_batch_param("jobs_new_batch_param", "Parameter for the insertion of more jobs in a new batch", main_parameters);
    Parameter<double> inverse_param("inverse_param", "Parameter for the inverse move", main_parameters);
    
    double swap_rate, insert_rate, job_existing_batch_rate, jobs_new_batch_rate, inverse_rate, single_job_rate;
    
    CommandLineParameters::Parse(argc, argv, false, true);
    
    // setting the weights for neighborhoods
    if (method == std::string("SATBWITHOUTSWAP") || method == std::string("LAHCWITHOUTSWAP"))
    {
        double weights_total = insert_param + job_existing_batch_param + jobs_new_batch_param + inverse_param + single_job_new_batch_param;
        
        insert_rate = insert_param / weights_total;
        job_existing_batch_rate = job_existing_batch_param / weights_total;
        jobs_new_batch_rate = jobs_new_batch_param / weights_total;
        inverse_rate = inverse_param / weights_total;
        single_job_rate = single_job_new_batch_param / weights_total;
    }
    else if (method == std::string("SATBWITHOUTJOBNEWBATCH") || method == std::string("LAHCWITHOUTJOBNEWBATCH"))
    {
        double weights_total = insert_param + job_existing_batch_param + jobs_new_batch_param + inverse_param + swap_param;
        
        insert_rate = insert_param / weights_total;
        job_existing_batch_rate = job_existing_batch_param / weights_total;
        jobs_new_batch_rate = jobs_new_batch_param / weights_total;
        inverse_rate = inverse_param / weights_total;
        swap_rate = swap_param / weights_total;
    }
    else if (method == std::string("SATBWITHOUTJOBEXISTINGBATCH") || method == std::string("LAHCWITHOUTJOBEXISTINGBATCH"))
    {
        double weights_total = insert_param + single_job_new_batch_param + jobs_new_batch_param + inverse_param + swap_param;
        
        insert_rate = insert_param / weights_total;
        single_job_rate = single_job_new_batch_param / weights_total;
        jobs_new_batch_rate = jobs_new_batch_param / weights_total;
        inverse_rate = inverse_param / weights_total;
        swap_rate = swap_param / weights_total;
    }
    else if (method == std::string("SATBWITHOUTMULTIPLEJOBS") || method == std::string("LAHCWITHOUTMULTIPLEJOBS"))
    {
        double weights_total = insert_param + single_job_new_batch_param + job_existing_batch_param + inverse_param + swap_param;
        
        insert_rate = insert_param / weights_total;
        single_job_rate = single_job_new_batch_param / weights_total;
        job_existing_batch_rate = job_existing_batch_param / weights_total;
        inverse_rate = inverse_param / weights_total;
        swap_rate = swap_param / weights_total;
    }
    else if (method == std::string("SATBWITHOUTINVERT") || method == std::string("LAHCWITHOUTINVERT"))
    {
        double weights_total = insert_param + single_job_new_batch_param + job_existing_batch_param + jobs_new_batch_param + swap_param;
        
        insert_rate = insert_param / weights_total;
        single_job_rate = single_job_new_batch_param / weights_total;
        job_existing_batch_rate = job_existing_batch_param / weights_total;
        jobs_new_batch_rate = jobs_new_batch_param / weights_total;
        swap_rate = swap_param / weights_total;
    }
    else if (method == std::string("SATBWITHOUTINSERT") || method == std::string("LAHCWITHOUTINSERT"))
    {
        double weights_total = inverse_param + single_job_new_batch_param + job_existing_batch_param + jobs_new_batch_param + swap_param;
        
        inverse_rate = inverse_param / weights_total;
        single_job_rate = single_job_new_batch_param / weights_total;
        job_existing_batch_rate = job_existing_batch_param / weights_total;
        jobs_new_batch_rate = jobs_new_batch_param / weights_total;
        swap_rate = swap_param / weights_total;
    }
    else if (method == std::string("SATB6MULTI") || method == std::string("TOKENRINGTSSA") )
    {
        double weights_total = 3.171;
        swap_rate = swap_param / weights_total;
        insert_rate = insert_param / weights_total;
        job_existing_batch_rate = job_existing_batch_param / weights_total;
        jobs_new_batch_rate = jobs_new_batch_param / weights_total;
        inverse_rate = inverse_param / weights_total;
        single_job_rate = single_job_new_batch_param / weights_total;
       //  std::cout << swap_rate + insert_rate + job_existing_batch_rate + jobs_new_batch_rate + inverse_rate << " coming from: " << swap_rate << " " << insert_rate << " " << job_existing_batch_rate << " " << jobs_new_batch_rate << " " << inverse_rate << std::endl;
    }
    else if (method == std::string("LAHC6MULTI") && swap_param.IsSet() && insert_param.IsSet() && job_existing_batch_param.IsSet() && jobs_new_batch_param.IsSet() && inverse_param.IsSet() && single_job_new_batch_param.IsSet())
    {
        double weights_total = 2.18;
        swap_rate = swap_param / weights_total;
        insert_rate = insert_param / weights_total;
        job_existing_batch_rate = job_existing_batch_param / weights_total;
        jobs_new_batch_rate = jobs_new_batch_param / weights_total;
        inverse_rate = inverse_param / weights_total;
        single_job_rate = single_job_new_batch_param / weights_total;
       //  std::cout << swap_rate + insert_rate + job_existing_batch_rate + jobs_new_batch_rate + inverse_rate << " coming from: " << swap_rate << " " << insert_rate << " " << job_existing_batch_rate << " " << jobs_new_batch_rate << " " << inverse_rate << std::endl;
    }
    else if (method == std::string("SATBPATAT"))
    {
        double weights_total = insert_param + swap_param + job_existing_batch_param + jobs_new_batch_param ;
        insert_rate = insert_param / weights_total;
        swap_rate = swap_param / weights_total;
        job_existing_batch_rate = job_existing_batch_param / weights_total;
        jobs_new_batch_rate = jobs_new_batch_param / weights_total;
       //  std::cout << swap_rate + insert_rate + job_existing_batch_rate + jobs_new_batch_rate + inverse_rate << " coming from: " << swap_rate << " " << insert_rate << " " << job_existing_batch_rate << " " << jobs_new_batch_rate << " " << inverse_rate << std::endl;
    }
    /*
    if (swap_rate + insert_rate + job_existing_batch_rate + jobs_new_batch_rate + inverse_rate > 1.000000000000000000001)
    {
        std::cout << "Error: with neighborhood percentages" << std::endl;
        std::cout << swap_rate + insert_rate + job_existing_batch_rate + jobs_new_batch_rate + inverse_rate << " coming from: " << swap_rate << " " << insert_rate << " " << job_existing_batch_rate << " " << jobs_new_batch_rate << " " << inverse_rate << std::endl;
        return 1;
    }
     */

    if (!instance.IsSet())
    {
        std::cout << "Error: --main::instance filename option must always be set" << std::endl;
        return 1;
    }
    OSP_Input in(instance);

    // cerr << in << endl;
    if (seed.IsSet())
    {
        Random::SetSeed(seed);
    }
    // TODO: this should be parameters
    int w_p = 4;
    int w_sc = 1;
    int w_t = 100;
    int w_sum = w_p + w_sc + w_t;
    int n = in.Jobs();
    int sum_min_t = 0;
    for (int j = 0; j < n; ++j)
    {
        sum_min_t = sum_min_t + in.MinTimeJob(j);
    }
    int avg_t = std::ceil((double) sum_min_t / (double) n);
    int max_setupcost = in.SetUpCost(0, 0);
    for (int a_1 = 0; a_1 < in.Attributes(); ++a_1)
    {
        for (int a_2 = 0; a_2 < in.Attributes(); ++a_2)
        {
            if (max_setupcost < in.SetUpCost(a_1, a_2))
            {
                max_setupcost = in.SetUpCost(a_1, a_2);
            }
        }
    }
    int maxi = std::max(1, max_setupcost);
    int C = std::lcm(maxi,avg_t);
    // std::cout << sum_min_t << " " << avg_t << " " << maxi << " " << C << " " << w_sum << std::endl; 
    int hard_constraint_multiplier = C * n * w_sum;
    //std::cout << hard_constraint_multiplier << std::endl;
    
    if (greedy_solution.IsSet())
    {
        OSP_SolutionManager OSP_sm(in);
        OSP_Output st(in);
        auto start = high_resolution_clock::now();
        OSP_sm.GreedyState(st);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        // std::cout << "greedy performed in: " << duration.count() << "microseconds" << std::endl;
        OSP_sm.CheckConsistency(st);
        int cost = st.GetNumberOfTardyJobs() * st.MultFactorFinishedTooLate() + st.GetCumulativeBatchProcessingTime() * st.MultFactorTotalRunTime() + st.GetTotalSetUpCost() * st.MultFactorTotalSetUpCosts();
        // write the output on the file passed in the command line
        if (output_file.IsSet())
        {
            std::ofstream os(static_cast<std::string>(output_file).c_str());
            os << "{\"solution\": {" << st <<  "}, "
            << "\"cost\": " <<  cost  <<  ", "
            << "\"time_microsec\": " << duration.count() << "} " << std::endl;
            os.close();
        }
        else
        {
            std::cout << "{\"solution\": {" << st <<  "}, "
            << "\"cost\": " <<  cost  <<  ", "
            << "\"time_microsec\": " << duration.count() << "} " << std::endl;
        }
        return(0);
    }
    
    
    // cost components: second parameter is the cost, third is the type (true -> hard, false -> soft)
    OSP_TotalSetUpCost cc1(in,in.MultFactorTotalSetUpCosts(),false);
    OSP_NumberOfTardyJobs cc2(in, in.MultFactorFinishedTooLate(), false);
    OSP_CumulativeBatchProcessingTime cc3(in,in.MultFactorTotalRunTime(),false);
    OSP_NotScheduledBatches cc4(in,2*hard_constraint_multiplier,true);
    
    // helpers
    OSP_SolutionManager OSP_sm(in);
    OSP_SwapConsecutiveBatchesMoveNeighborhoodExplorer OSP_scbmnhe(in,OSP_sm);
    OSP_BatchToNewPositionNeighborhoodExplorer OSP_btnpnhe(in,OSP_sm);
    OSP_JobToExistingBatchNeighborhoodExplorer OSP_jtebnhe(in,OSP_sm);
    OSP_JobToNewBatchNeighborhoodExplorer OSP_jtnbnhe(in,OSP_sm);
    OSP_BatchToNewMachineNeighborhoodExplorer OSP_btnmnhe(in,OSP_sm);
    OSP_SwapBatchesNeighborhoodExplorer OSP_sbnhe(in,OSP_sm);
    OSP_InvertBatchesInMachineNeighborhoodExplorer OSP_ibimnhe(in,OSP_sm);
    Decoupled_OSP_BatchToNewMachineNeighborhoodExplorer OSP_decouples(in,OSP_sm);
    
    // All cost components must be added to the state manager
    OSP_sm.AddCostComponent(cc1);
    OSP_sm.AddCostComponent(cc2);
    OSP_sm.AddCostComponent(cc3);
    OSP_sm.AddCostComponent(cc4);
  
    // All cost components must be added to the neighborhood explorer
    OSP_scbmnhe.AddCostComponent(cc1);
    OSP_scbmnhe.AddCostComponent(cc2);
    OSP_scbmnhe.AddCostComponent(cc3);
    OSP_scbmnhe.AddCostComponent(cc4);
    
    OSP_btnpnhe.AddCostComponent(cc1);
    OSP_btnpnhe.AddCostComponent(cc2);
    OSP_btnpnhe.AddCostComponent(cc3);
    OSP_btnpnhe.AddCostComponent(cc4);
    
    OSP_jtebnhe.AddCostComponent(cc1);
    OSP_jtebnhe.AddCostComponent(cc2);
    OSP_jtebnhe.AddCostComponent(cc3);
    OSP_jtebnhe.AddCostComponent(cc4);
    
    OSP_jtnbnhe.AddCostComponent(cc1);
    OSP_jtnbnhe.AddCostComponent(cc2);
    OSP_jtnbnhe.AddCostComponent(cc3);
    OSP_jtnbnhe.AddCostComponent(cc4);
    
    /*
    OSP_btnmnhe.AddCostComponent(cc1);
    OSP_btnmnhe.AddCostComponent(cc2);
    OSP_btnmnhe.AddCostComponent(cc3);
    OSP_btnmnhe.AddCostComponent(cc4);
     */
    
    OSP_sbnhe.AddCostComponent(cc1);
    OSP_sbnhe.AddCostComponent(cc2);
    OSP_sbnhe.AddCostComponent(cc3);
    OSP_sbnhe.AddCostComponent(cc4);
    
    OSP_ibimnhe.AddCostComponent(cc1);
    OSP_ibimnhe.AddCostComponent(cc2);
    OSP_ibimnhe.AddCostComponent(cc3);
    OSP_ibimnhe.AddCostComponent(cc4);
    
    OSP_decouples.AddCostComponent(cc1);
    OSP_decouples.AddCostComponent(cc2);
    OSP_decouples.AddCostComponent(cc3);
    OSP_decouples.AddCostComponent(cc4);
    
    
    // multi-neigborhoods
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_scbmnhe), decltype(OSP_btnpnhe), decltype(OSP_jtebnhe), decltype(OSP_jtnbnhe)>
        OSP_original_multi(in, OSP_sm, "SA - Multi as PATAT", OSP_scbmnhe, OSP_btnpnhe, OSP_jtebnhe, OSP_jtnbnhe, 
                           {swap_rate, insert_rate, job_existing_batch_rate, jobs_new_batch_rate});
    
    //SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    //decltype(OSP_scbmnhe), decltype(OSP_btnpnhe), decltype(OSP_jtebnhe), decltype(OSP_btnmnhe)>
    //    OSP_original_multi_with_shift(in, OSP_sm, "SA - Multi as PATAT", OSP_scbmnhe, OSP_btnpnhe, OSP_jtebnhe, OSP_btnmnhe,
    //                       {swap_rate, insert_rate, job_existing_batch_rate, jobs_new_batch_rate});
    
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_btnpnhe), decltype(OSP_jtebnhe), decltype(OSP_jtnbnhe), decltype(OSP_decouples),decltype(OSP_sbnhe),decltype(OSP_ibimnhe)>
        OSP_multi_six(in, OSP_sm, "SA - Multi six", OSP_btnpnhe, OSP_jtebnhe, OSP_jtnbnhe, OSP_decouples, OSP_sbnhe, OSP_ibimnhe,
                      {insert_rate, job_existing_batch_rate, single_job_rate ,jobs_new_batch_rate, swap_rate, inverse_rate});
    
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_btnpnhe), decltype(OSP_jtebnhe), decltype(OSP_jtnbnhe), decltype(OSP_decouples),decltype(OSP_ibimnhe)>
        OSP_multi_without_swap(in, OSP_sm, "SA - without swap", OSP_btnpnhe, OSP_jtebnhe, OSP_jtnbnhe, OSP_decouples, OSP_ibimnhe,
                      {insert_rate, job_existing_batch_rate, single_job_rate ,jobs_new_batch_rate, inverse_rate});
    
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_btnpnhe), decltype(OSP_jtebnhe), decltype(OSP_sbnhe), decltype(OSP_decouples),decltype(OSP_ibimnhe)>
        OSP_multi_without_new_job(in, OSP_sm, "SA - without new job", OSP_btnpnhe, OSP_jtebnhe,  OSP_sbnhe, OSP_decouples, OSP_ibimnhe,
                      {insert_rate, job_existing_batch_rate, swap_rate ,jobs_new_batch_rate, inverse_rate});
    
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_btnpnhe), decltype(OSP_jtnbnhe), decltype(OSP_sbnhe), decltype(OSP_decouples),decltype(OSP_ibimnhe)>
        OSP_multi_without_existing(in, OSP_sm, "SA - without existing job", OSP_btnpnhe, OSP_jtnbnhe, OSP_sbnhe, OSP_decouples, OSP_ibimnhe,
                      {insert_rate, single_job_rate, swap_rate ,jobs_new_batch_rate, inverse_rate});
    
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_btnpnhe), decltype(OSP_jtnbnhe), decltype(OSP_sbnhe), decltype(OSP_jtebnhe),decltype(OSP_ibimnhe)>
        OSP_multi_without_multiple_jobs(in, OSP_sm, "SA - without multiple jobs", OSP_btnpnhe, OSP_jtnbnhe, OSP_sbnhe, OSP_jtebnhe, OSP_ibimnhe,
                      {insert_rate, single_job_rate, swap_rate ,job_existing_batch_rate, inverse_rate});
    
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_btnpnhe), decltype(OSP_jtnbnhe), decltype(OSP_sbnhe), decltype(OSP_jtebnhe),decltype(OSP_decouples)>
        OSP_multi_without_invert(in, OSP_sm, "SA - without invert", OSP_btnpnhe, OSP_jtnbnhe, OSP_sbnhe, OSP_jtebnhe, OSP_decouples,
                      {insert_rate, single_job_rate, swap_rate ,job_existing_batch_rate, jobs_new_batch_rate});
    
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_ibimnhe), decltype(OSP_jtnbnhe), decltype(OSP_sbnhe), decltype(OSP_jtebnhe),decltype(OSP_decouples)>
        OSP_multi_without_insert(in, OSP_sm, "SA - without insert", OSP_ibimnhe, OSP_jtnbnhe, OSP_sbnhe, OSP_jtebnhe, OSP_decouples,
                      {inverse_rate, single_job_rate, swap_rate ,job_existing_batch_rate, jobs_new_batch_rate});
    
    
    // SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    // decltype(OSP_btnpnhe), decltype(OSP_jtebnhe), decltype(OSP_btnmnhe),decltype(OSP_sbnhe),decltype(OSP_ibimnhe)>
    //    OSP_multi_five(in, OSP_sm, "SA - Multi five", OSP_btnpnhe, OSP_jtebnhe, OSP_btnmnhe, OSP_sbnhe, OSP_ibimnhe,
    //    {insert_rate, job_existing_batch_rate, jobs_new_batch_rate, swap_rate, inverse_rate});
    
    // SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    // decltype(OSP_btnpnhe), decltype(OSP_jtebnhe), decltype(OSP_jtnbnhe),decltype(OSP_sbnhe),decltype(OSP_ibimnhe)>
    //    OSP_multi_five_one_job(in, OSP_sm, "SA - Multi five only one job", OSP_btnpnhe, OSP_jtebnhe, OSP_jtnbnhe, OSP_sbnhe, OSP_ibimnhe,
    //    {insert_rate, job_existing_batch_rate, jobs_new_batch_rate, swap_rate, inverse_rate});

    /*
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_scbmnhe), decltype(OSP_jtebnhe), decltype(OSP_jtnbnhe)>
    OSP_multi_three_batch(in, OSP_sm, "TS - Multi three batch", OSP_scbmnhe, OSP_jtebnhe, OSP_jtnbnhe, {1/3, 1/3, 1/3});
    
    OSP_multi_three_batch.AddInverseFunction<SwapConsecutiveBatchesMove,SwapConsecutiveBatchesMove>([](const SwapConsecutiveBatchesMove &scm_1, const SwapConsecutiveBatchesMove &scm_2){
        return scm_1.position_1 == scm_2.position_1
        || scm_1.position_1 == scm_2.position_2
        || scm_1.position_2 == scm_2.position_1
        || scm_1.position_2 == scm_2.position_2;
    });
    OSP_multi_three_batch.AddInverseFunction<JobToNewBatch,JobToNewBatch>([](const JobToNewBatch &jnbm_1, const JobToNewBatch &jnbm_2){
        return jnbm_1.old_position == jnbm_2.old_position
            || jnbm_1.old_position == jnbm_2.new_position
            || jnbm_1.new_position == jnbm_2.old_position
            || jnbm_1.new_position == jnbm_2.new_position;
    });
    OSP_multi_three_batch.AddInverseFunction<JobToExistingBatch,JobToExistingBatch>([](const JobToExistingBatch &jebm_1, const JobToExistingBatch &jebm_2){
        std::pair<int,int> jebm_1_old_position = std::make_pair(jebm_1.old_machine,jebm_1.old_position);
        std::pair<int,int> jebm_1_new_position = std::make_pair(jebm_1.new_machine,jebm_1.new_position);
        std::pair<int,int> jebm_2_old_position = std::make_pair(jebm_2.old_machine,jebm_2.old_position);
        std::pair<int,int> jebm_2_new_position = std::make_pair(jebm_2.new_machine,jebm_2.new_position);
        return jebm_1_old_position == jebm_2_old_position
            || jebm_1_old_position == jebm_2_new_position
            || jebm_1_new_position == jebm_2_old_position
            || jebm_1_new_position == jebm_2_new_position;
    });
    OSP_multi_three_batch.AddInverseFunction<SwapConsecutiveBatchesMove,JobToNewBatch>([](const SwapConsecutiveBatchesMove &sm, const JobToNewBatch &jnbm){
        std::pair<int,int> sm_1_position_1 = std::make_pair(sm.machine,sm.position_1);
        std::pair<int,int> sm_1_position_2 = std::make_pair(sm.machine,sm.position_2);
        return sm_1_position_1 == jnbm.old_position
            || sm_1_position_1 == jnbm.new_position
            || sm_1_position_2 == jnbm.old_position
            || sm_1_position_2 == jnbm.new_position;
    });
    OSP_multi_three_batch.AddInverseFunction<SwapConsecutiveBatchesMove,JobToExistingBatch>([](const SwapConsecutiveBatchesMove &sm, const JobToExistingBatch &jebm){
        std::pair<int,int> sm_1_position_1 = std::make_pair(sm.machine,sm.position_1);
        std::pair<int,int> sm_1_position_2 = std::make_pair(sm.machine,sm.position_2);
        std::pair<int,int> jebm_2_old_position = std::make_pair(jebm.old_machine,jebm.old_position);
        std::pair<int,int> jebm_2_new_position = std::make_pair(jebm.new_machine,jebm.new_position);
        return sm_1_position_1 == jebm_2_old_position
            || sm_1_position_1 == jebm_2_new_position
            || sm_1_position_2 == jebm_2_old_position
            || sm_1_position_2 == jebm_2_new_position;
    });
    OSP_multi_three_batch.AddInverseFunction<JobToNewBatch,JobToExistingBatch>([](const JobToNewBatch &jnbm, const JobToExistingBatch &jebm){
        std::pair<int,int> jebm_2_old_position = std::make_pair(jebm.old_machine,jebm.old_position);
        std::pair<int,int> jebm_2_new_position = std::make_pair(jebm.new_machine,jebm.new_position);
        return jnbm.old_position == jebm_2_old_position
            || jnbm.old_position == jebm_2_new_position
            || jnbm.new_position == jebm_2_old_position
            || jnbm.new_position == jebm_2_new_position;
    });
    // this are the same as before but with inverted combinations
    OSP_multi_three_batch.AddInverseFunction<JobToNewBatch,SwapConsecutiveBatchesMove>([](const JobToNewBatch &jnbm,const SwapConsecutiveBatchesMove &sm){
        std::pair<int,int> sm_1_position_1 = std::make_pair(sm.machine,sm.position_1);
        std::pair<int,int> sm_1_position_2 = std::make_pair(sm.machine,sm.position_2);
        return sm_1_position_1 == jnbm.old_position
            || sm_1_position_1 == jnbm.new_position
            || sm_1_position_2 == jnbm.old_position
            || sm_1_position_2 == jnbm.new_position;
    });
    OSP_multi_three_batch.AddInverseFunction<JobToExistingBatch,SwapConsecutiveBatchesMove>([](const JobToExistingBatch &jebm,const SwapConsecutiveBatchesMove &sm){
        std::pair<int,int> sm_1_position_1 = std::make_pair(sm.machine,sm.position_1);
        std::pair<int,int> sm_1_position_2 = std::make_pair(sm.machine,sm.position_2);
        std::pair<int,int> jebm_2_old_position = std::make_pair(jebm.old_machine,jebm.old_position);
        std::pair<int,int> jebm_2_new_position = std::make_pair(jebm.new_machine,jebm.new_position);
        return sm_1_position_1 == jebm_2_old_position
            || sm_1_position_1 == jebm_2_new_position
            || sm_1_position_2 == jebm_2_old_position
            || sm_1_position_2 == jebm_2_new_position;
    });
    OSP_multi_three_batch.AddInverseFunction<JobToExistingBatch,JobToNewBatch>([](const JobToExistingBatch &jebm,const JobToNewBatch &jnbm){
        std::pair<int,int> jebm_2_old_position = std::make_pair(jebm.old_machine,jebm.old_position);
        std::pair<int,int> jebm_2_new_position = std::make_pair(jebm.new_machine,jebm.new_position);
        return jnbm.old_position == jebm_2_old_position
            || jnbm.old_position == jebm_2_new_position
            || jnbm.new_position == jebm_2_old_position
            || jnbm.new_position == jebm_2_new_position;
    });
    */
    
    SetUnionNeighborhoodExplorer<OSP_Input, OSP_Output, DefaultCostStructure<int>,
    decltype(OSP_jtebnhe), decltype(OSP_jtnbnhe)>
    OSP_multi_two_job(in, OSP_sm, "TS - Multi two job", OSP_jtebnhe, OSP_jtnbnhe, {1/2, 1/2});
    OSP_multi_two_job.AddInverseFunction<JobToExistingBatch,JobToExistingBatch>([](const JobToExistingBatch &m_1,const JobToExistingBatch &m_2){
        return m_1.job == m_2.job;
    });
    OSP_multi_two_job.AddInverseFunction<JobToNewBatch,JobToNewBatch>([](const JobToNewBatch &m_1,const JobToNewBatch &m_2){
        return m_1.job == m_2.job;
    });
    OSP_multi_two_job.AddInverseFunction<JobToExistingBatch,JobToNewBatch>([](const JobToExistingBatch &m_1,const JobToNewBatch &m_2){
        return m_1.job == m_2.job;
    });
    OSP_multi_two_job.AddInverseFunction<JobToNewBatch,JobToExistingBatch>([](const JobToNewBatch &m_1,const JobToExistingBatch &m_2){
        return m_1.job == m_2.job;
    });

  // runners
    // HillClimbing<OSP_Input, OSP_Output, SwapConsecutiveBatchesMove, DefaultCostStructure<int>> OSP_hc_scbm(in, OSP_sm, OSP_scbmnhe, "OSP_hc_scbm");
    // HillClimbing<OSP_Input, OSP_Output, BatchToNewPositionMove, DefaultCostStructure<int>> OSP_hc_btnp(in, OSP_sm, OSP_btnpnhe, "OSP_hc_btnp");
    // HillClimbing<OSP_Input, OSP_Output, JobToExistingBatch, DefaultCostStructure<int>> OSP_hc_jteb(in, OSP_sm, OSP_jtebnhe, "OSP_hc_jteb");
    // HillClimbing<OSP_Input, OSP_Output, JobToNewBatch, DefaultCostStructure<int>> OSP_hc_jtnb(in, OSP_sm, OSP_jtnbnhe, "OSP_hc_jtnb");
    // HillClimbing<OSP_Input, OSP_Output, BatchToNewMachine, DefaultCostStructure<int>> OSP_hc_btnm(in, OSP_sm, OSP_btnmnhe, "OSP_hc_btnm");
    // HillClimbing<OSP_Input, OSP_Output, SwapBatches, DefaultCostStructure<int>> OSP_hc_sb(in, OSP_sm, OSP_sbnhe, "OSP_hc_sb");
    // HillClimbing<OSP_Input, OSP_Output, InvertBatchesInMachine, DefaultCostStructure<int>> OSP_hc_ibim(in, OSP_sm, OSP_ibimnhe, "OSP_hc_ibim");
    
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_original_multi)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_as_patat(in, OSP_sm, OSP_original_multi, "SATBPATAT");
    // SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_seven)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_multi_seven(in, OSP_sm, OSP_multi_seven, "SATB7MULTI");
    // SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_five)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_multi_five(in, OSP_sm, OSP_multi_five, "SATB5MULTI");
    // SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_five_one_job)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_multi_five_one_job(in, OSP_sm, OSP_multi_five_one_job, "SATB5MULTIONEJOB");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_six)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_multi_six(in, OSP_sm, OSP_multi_six, "SATB6MULTI");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_without_swap)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_without_swap(in, OSP_sm, OSP_multi_without_swap, "SATBWITHOUTSWAP");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_without_new_job)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_without_new_job(in, OSP_sm, OSP_multi_without_new_job, "SATBWITHOUTNEWJOB");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_without_existing)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_without_existing(in, OSP_sm, OSP_multi_without_existing, "SATBWITHOUTEXISTING");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_without_multiple_jobs)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_without_multiple(in, OSP_sm, OSP_multi_without_multiple_jobs, "SATBWITHOUTMULTIPLE");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_without_invert)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_without_invert(in, OSP_sm, OSP_multi_without_invert, "SATBWITHOUTINVERT");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_without_insert)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_without_insert(in, OSP_sm, OSP_multi_without_insert, "SATBWITHOUTINSERT");
    
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_sbnhe)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_only_swap(in, OSP_sm, OSP_sbnhe, "SATBONLYSWAP");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_jtnbnhe)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_only_job_new(in, OSP_sm, OSP_jtnbnhe, "SATBONLYJOBNEW");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_jtebnhe)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_only_job_existing(in, OSP_sm, OSP_jtebnhe, "SATBONLYJOBEXISTING");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_decouples)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_only_multiple(in, OSP_sm, OSP_decouples, "SATBONLYMULTIPLEJOBS");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_ibimnhe)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_only_invert(in, OSP_sm, OSP_ibimnhe, "SATBONLYINVERT");
    SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_btnpnhe)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_only_insert(in, OSP_sm, OSP_btnpnhe, "SATBONLYINSERT");

    // SimulatedAnnealingEvaluationBased<OSP_Input, OSP_Output, decltype(OSP_multi_five)::MoveType, DefaultCostStructure<int>> OSP_sa_eval_based_multi_five(in, OSP_sm, OSP_multi_five, "SAEB5MULTI");
    // SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_three_for_tb)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_multi_three_tb(in, OSP_sm, OSP_multi_three_for_tb, "SATB3MULTI");
    // SimulatedAnnealingTimeBased<OSP_Input, OSP_Output, decltype(OSP_multi_five_consecutive)::MoveType, DefaultCostStructure<int>> OSP_sa_time_based_multi_five_consecutive(in, OSP_sm, OSP_multi_five_consecutive, "SATB5MULTIC");
    LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_multi_six)::MoveType, DefaultCostStructure<int>> OSP_lahc_multi_six(in, OSP_sm, OSP_multi_six, "LAHC6MULTI");
    
    LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_multi_without_swap)::MoveType, DefaultCostStructure<int>> OSP_lahc_without_swap(in, OSP_sm, OSP_multi_without_swap, "LAHCWITHOUTSWAP");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_multi_without_new_job)::MoveType, DefaultCostStructure<int>> OSP_lahc_without_new_job(in, OSP_sm, OSP_multi_without_new_job, "LAHCWITHOUTNEWJOB");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_multi_without_existing)::MoveType, DefaultCostStructure<int>> OSP_lahc_without_existing(in, OSP_sm, OSP_multi_without_existing, "LAHCWITHOUTEXISTING");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_multi_without_multiple_jobs)::MoveType, DefaultCostStructure<int>> OSP_lahc_without_multiple(in, OSP_sm, OSP_multi_without_multiple_jobs, "LAHCWITHOUTMULTIPLE");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_multi_without_invert)::MoveType, DefaultCostStructure<int>> OSP_lahc_without_invert(in, OSP_sm, OSP_multi_without_invert, "LAHCWITHOUTINVERT");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_multi_without_insert)::MoveType, DefaultCostStructure<int>> OSP_lahc_without_insert(in, OSP_sm, OSP_multi_without_insert, "LAHCWITHOUTINSERT");
        
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_sbnhe)::MoveType, DefaultCostStructure<int>> OSP_lahc_only_swap(in, OSP_sm, OSP_sbnhe, "LAHCONLYSWAP");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_jtnbnhe)::MoveType, DefaultCostStructure<int>> OSP_lahc_only_job_new(in, OSP_sm, OSP_jtnbnhe, "LAHCONLYJOBNEW");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_jtebnhe)::MoveType, DefaultCostStructure<int>> OSP_lahc_only_job_existing(in, OSP_sm, OSP_jtebnhe, "LAHCONLYJOBEXISTING");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_decouples)::MoveType, DefaultCostStructure<int>> OSP_lahc_only_multiple(in, OSP_sm, OSP_decouples, "LAHCONLYMULTIPLEJOBS");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_ibimnhe)::MoveType, DefaultCostStructure<int>> OSP_lahc_only_invert(in, OSP_sm, OSP_ibimnhe, "LAHCONLYINVERT");
        LateAcceptanceHillClimbing<OSP_Input, OSP_Output, decltype(OSP_btnpnhe)::MoveType, DefaultCostStructure<int>> OSP_lahc_only_insert(in, OSP_sm, OSP_btnpnhe, "LAHCONLYINSERT");
    
    TabuSearch<OSP_Input, OSP_Output, decltype(OSP_multi_two_job)::MoveType, DefaultCostStructure<int>> OSP_ts_two_move_job_fixed_length(in, OSP_sm, OSP_multi_two_job, "TS2MULTIJOBFLTL",OSP_multi_two_job.InverseFunction());
    TabuSearch<OSP_Input, OSP_Output, decltype(OSP_jtnbnhe)::MoveType, DefaultCostStructure<int>> OSP_ts_to_new_batch(in, OSP_sm, OSP_jtnbnhe, "TS1JOBTONEW");
    TabuSearch<OSP_Input, OSP_Output, decltype(OSP_jtebnhe)::MoveType, DefaultCostStructure<int>> OSP_ts_to_existing_batch(in, OSP_sm, OSP_jtebnhe, "TS1JOBTOEXISTING");
    // TabuSearch<OSP_Input, OSP_Output, decltype(OSP_multi_two_job)::MoveType, DefaultCostStructure<int>> OSP_ts_two_move_job_gendreau(in, OSP_sm, OSP_multi_two_job, "TS2MULTIJOBGTL",OSP_multi_two_job.InverseFunction());
    // TabuSearch<OSP_Input, OSP_Output, decltype(OSP_multi_three_batch)::MoveType, DefaultCostStructure<int>> OSP_ts_three_move_batch(in, OSP_sm, OSP_multi_three_batch, "TS3MULTIBATCH",OSP_multi_three_batch.InverseFunction());
    // TabuSearch<OSP_Input, OSP_Output, decltype(OSP_scbmnhe)::MoveType, DefaultCostStructure<int>> OSP_ts_swap_batches(in, OSP_sm, OSP_scbmnhe, "TS1SWAPBATCH");
    
    SteepestDescent<OSP_Input, OSP_Output, decltype(OSP_multi_two_job)::MoveType, DefaultCostStructure<int>> OSP_sd_two_move(in, OSP_sm, OSP_multi_two_job, "SD2MULTI");
    
    TokenRingSearch<OSP_Input, OSP_Output> token_ring_ts_sa(in, OSP_sm, "TOKENRINGTSSA");
    token_ring_ts_sa.AddRunner(OSP_ts_two_move_job_fixed_length);
    token_ring_ts_sa.AddRunner(OSP_sa_time_based_multi_six);

    // tester
    Tester<OSP_Input, OSP_Output, DefaultCostStructure<int>> tester(in,OSP_sm);
    
    MoveTester<OSP_Input, OSP_Output, SwapConsecutiveBatchesMove, DefaultCostStructure<int>> swap_batches_move_test(in,OSP_sm,OSP_scbmnhe, "SwapConsecutiveBatches move", tester);
    MoveTester<OSP_Input, OSP_Output, BatchToNewPositionMove, DefaultCostStructure<int>> batch_to_ne_position_move_test(in,OSP_sm,OSP_btnpnhe, "BatchToNewPosition move", tester);
    MoveTester<OSP_Input, OSP_Output, JobToExistingBatch, DefaultCostStructure<int>> job_to_existing_batch_move_test(in,OSP_sm,OSP_jtebnhe, "JobToExistingBatch move", tester);
    MoveTester<OSP_Input, OSP_Output, JobToNewBatch, DefaultCostStructure<int>> job_to_new_batch_move_test(in,OSP_sm,OSP_jtnbnhe, "JobToNewBatch move", tester);
    // MoveTester<OSP_Input, OSP_Output, BatchToNewMachine, DefaultCostStructure<int>> job_to_new_machine_move_test(in,OSP_sm,OSP_btnmnhe, "BatchToNewMachine move", tester);
    MoveTester<OSP_Input, OSP_Output, SwapBatches, DefaultCostStructure<int>> swap_non_cons_batches_move_test(in,OSP_sm,OSP_sbnhe, "SwapBatches move", tester);
    MoveTester<OSP_Input, OSP_Output, InvertBatchesInMachine, DefaultCostStructure<int>> invert_bathes_in_machine(in,OSP_sm,OSP_ibimnhe, "InvertBatchesInMachine move", tester);
    MoveTester<OSP_Input, OSP_Output, BatchToNewMachine, DefaultCostStructure<int>> job_to_new_machine_move_test(in,OSP_sm,OSP_decouples, "JobsToNewMachineDecoupled move", tester);
    
    // MoveTester<OSP_Input, OSP_Output, decltype(OSP_original_multi)::MoveType, DefaultCostStructure<int>> original_multi_move_test(in,OSP_sm,OSP_original_multi, "PATAT multi-neighb move", tester);
    // MoveTester<OSP_Input, OSP_Output, decltype(OSP_multi_seven)::MoveType, DefaultCostStructure<int>> multi_7_move_test(in,OSP_sm,OSP_multi_seven, "Multi-neighb 7 move", tester);
    // MoveTester<OSP_Input, OSP_Output, decltype(OSP_multi_five)::MoveType, DefaultCostStructure<int>> multi_5_move_test(in,OSP_sm,OSP_multi_five, "Multi-neighb 5 move", tester);
    
    
    SimpleLocalSearch<OSP_Input, OSP_Output, DefaultCostStructure<int>> OSP_solver(in, OSP_sm, "OSP_solver");
    if (!CommandLineParameters::Parse(argc, argv, true, false))
    {
        return 1;
    }
    
    if (!method.IsSet())
    { // If no search method is set -> enter in the tester
        if (init_state.IsSet())
        {
            tester.RunMainMenu(init_state);
        }
        else
        {
            tester.RunMainMenu();
        }
    }
    else
    {
        Runner<OSP_Input, OSP_Output, DefaultCostStructure<int>> *used_runner = nullptr;
        // if (method == std::string("OSP_hc_scbm"))
        // {
        //     OSP_solver.SetRunner(OSP_hc_scbm);
        // }
        // else if (method == std::string("OSP_hc_btnp"))
        // {
        //     OSP_solver.SetRunner(OSP_hc_btnp);
        // }
        // else if (method == std::string("OSP_hc_jteb"))
        // {
        //     OSP_solver.SetRunner(OSP_hc_jteb);
        // }
        // else if (method == std::string("OSP_hc_jtnb"))
        // {
        //     OSP_solver.SetRunner(OSP_hc_jtnb);
        // }
        //else if (method == std::string("OSP_hc_btnm"))
        //{
        //    OSP_solver.SetRunner(OSP_hc_btnm);
        //}
        if (method ==  std::string("SATBPATAT"))
        {
            used_runner = &OSP_sa_time_based_as_patat;
            OSP_solver.SetRunner(OSP_sa_time_based_as_patat);
        }
        //else if (method == std::string("OSP_hc_sb"))
        //{
        //    OSP_solver.SetRunner(OSP_hc_sb);
        //}
        //else if (method == std::string("OSP_hc_ibim"))
        //{
        //    OSP_solver.SetRunner(OSP_hc_ibim);
        //}
        //else if (method == std::string("SATB5MULTI"))
        //{
        //    OSP_solver.SetRunner(OSP_sa_time_based_multi_five);
        //}
        //else if (method == std::string("SATB5MULTIONEJOB"))
        //{
        //    OSP_solver.SetRunner(OSP_sa_time_based_multi_five_one_job);
        //}
        else if (method == std::string("SATB6MULTI"))
        {
            used_runner = &OSP_sa_time_based_multi_six;
            OSP_solver.SetRunner(OSP_sa_time_based_multi_six);
        }
        else if (method == std::string("SATBWITHOUTSWAP"))
        {
            used_runner = &OSP_sa_time_based_without_swap;
            OSP_solver.SetRunner(OSP_sa_time_based_without_swap);
        }
        else if (method == std::string("SATBWITHOUTNEWJOB"))
        {
            used_runner = &OSP_sa_time_based_without_new_job;
            OSP_solver.SetRunner(OSP_sa_time_based_without_new_job);
        }
        else if (method == std::string("SATBWITHOUTEXISTING"))
        {
            used_runner = &OSP_sa_time_based_without_existing;
            OSP_solver.SetRunner(OSP_sa_time_based_without_existing);
        }
        else if (method == std::string("SATBWITHOUTMULTIPLE"))
        {
            used_runner = &OSP_sa_time_based_without_multiple;
            OSP_solver.SetRunner(OSP_sa_time_based_without_multiple);
        }
        else if (method == std::string("SATBWITHOUTINVERT"))
        {
            used_runner = &OSP_sa_time_based_without_invert;
            OSP_solver.SetRunner(OSP_sa_time_based_without_invert);
        }
        else if (method == std::string("SATBWITHOUTINSERT"))
        {
            used_runner = &OSP_sa_time_based_without_insert;
            OSP_solver.SetRunner(OSP_sa_time_based_without_insert);
        }
        else if (method == std::string("SATBONLYINSERT"))
        {
            used_runner = &OSP_sa_time_based_only_insert;
            OSP_solver.SetRunner(OSP_sa_time_based_only_insert);
        }
        else if (method == std::string("SATBONLYSWAP"))
        {
            used_runner = &OSP_sa_time_based_only_swap;
            OSP_solver.SetRunner(OSP_sa_time_based_only_swap);
        }
        else if (method == std::string("SATBONLYJOBNEW"))
        {
            used_runner = &OSP_sa_time_based_only_job_new;
            OSP_solver.SetRunner(OSP_sa_time_based_only_job_new);
        }
        else if (method == std::string("SATBONLYJOBEXISTING"))
        {
            used_runner = &OSP_sa_time_based_only_job_existing;
            OSP_solver.SetRunner(OSP_sa_time_based_only_job_existing);
        }
        else if (method == std::string("SATBONLYMULTIPLEJOBS"))
        {
            used_runner = &OSP_sa_time_based_only_multiple;
            OSP_solver.SetRunner(OSP_sa_time_based_only_multiple);
        }
        else if (method == std::string("SATBONLYINVERT"))
        {
            used_runner = &OSP_sa_time_based_only_invert;
            OSP_solver.SetRunner(OSP_sa_time_based_only_invert);
        }
        else if (method == std::string("LAHCWITHOUTSWAP"))
        {
            used_runner = &OSP_lahc_without_swap;
            OSP_solver.SetRunner(OSP_lahc_without_swap);
        }
        else if (method == std::string("LAHCWITHOUTNEWJOB"))
        {
            used_runner = &OSP_lahc_without_new_job;
            OSP_solver.SetRunner(OSP_lahc_without_new_job);
        }
        else if (method == std::string("LAHCWITHOUTEXISTING"))
        {
            used_runner = &OSP_lahc_without_existing;
            OSP_solver.SetRunner(OSP_lahc_without_existing);
        }
        else if (method == std::string("LAHCWITHOUTMULTIPLE"))
        {
            used_runner = &OSP_lahc_without_multiple;
            OSP_solver.SetRunner(OSP_lahc_without_multiple);
        }
        else if (method == std::string("LAHCWITHOUTINVERT"))
        {
            used_runner = &OSP_lahc_without_invert;
            OSP_solver.SetRunner(OSP_lahc_without_invert);
        }
        else if (method == std::string("LAHCWITHOUTINSERT"))
        {
            used_runner = &OSP_lahc_without_insert;
            OSP_solver.SetRunner(OSP_lahc_without_insert);
        }
        else if (method == std::string("LAHCONLYINSERT"))
        {
            used_runner = &OSP_lahc_only_insert;
            OSP_solver.SetRunner(OSP_lahc_only_insert);
        }
        else if (method == std::string("LAHCONLYSWAP"))
        {
            used_runner = &OSP_lahc_only_swap;
            OSP_solver.SetRunner(OSP_lahc_only_swap);
        }
        else if (method == std::string("LAHCONLYJOBNEW"))
        {
            used_runner = &OSP_lahc_only_job_new;
            OSP_solver.SetRunner(OSP_lahc_only_job_new);
        }
        else if (method == std::string("LAHCONLYJOBEXISTING"))
        {
            used_runner = &OSP_lahc_only_job_existing;
            OSP_solver.SetRunner(OSP_lahc_only_job_existing);
        }
        else if (method == std::string("LAHCONLYMULTIPLEJOBS"))
        {
            used_runner = &OSP_lahc_only_multiple;
            OSP_solver.SetRunner(OSP_lahc_only_multiple);
        }
        else if (method == std::string("LAHCONLYINVERT"))
        {
            used_runner = &OSP_lahc_only_invert;
            OSP_solver.SetRunner(OSP_lahc_only_invert);
        }
        else if (method == std::string("TS2MULTIJOBFLTL"))
        {
            if (!theta.IsSet())
            {
                std::cout << "Error: using TS2MULTIJOBFLTL, set --main::theta" << std::endl;
                return 1;
            }
            unsigned int tabu_size = (unsigned int) std::floor(theta * in.Jobs());
            OSP_ts_two_move_job_fixed_length.SetParameter("min_tenure", tabu_size);
            OSP_ts_two_move_job_fixed_length.SetParameter("max_tenure", tabu_size);
            used_runner = &OSP_ts_two_move_job_fixed_length;
            OSP_solver.SetRunner(OSP_ts_two_move_job_fixed_length);
        }
        else if (method == std::string("TS1JOBTONEW"))
        {
            if (!theta.IsSet())
            {
                std::cout << "Error: using TS1JOBTONEW, set --main::theta" << std::endl;
                return 1;
            }
            unsigned int tabu_size = (unsigned int) std::floor(theta * in.Jobs());
            OSP_ts_to_new_batch.SetParameter("min_tenure", tabu_size);
            OSP_ts_to_new_batch.SetParameter("max_tenure", tabu_size);
            used_runner = &OSP_ts_to_new_batch;
            OSP_solver.SetRunner(OSP_ts_to_new_batch);
        }
        else if (method == std::string("TS1JOBTOEXISTING"))
        {
            if (!theta.IsSet())
            {
                std::cout << "Error: using TS1JOBTOEXISTING, set --main::theta" << std::endl;
                return 1;
            }
            unsigned int tabu_size = (unsigned int) std::floor(theta * in.Jobs());
            OSP_ts_to_existing_batch.SetParameter("min_tenure", tabu_size);
            OSP_ts_to_existing_batch.SetParameter("max_tenure", tabu_size);
            used_runner = &OSP_ts_to_existing_batch;
            OSP_solver.SetRunner(OSP_ts_to_existing_batch);
        }
        // else if (method == std::string("TS2MULTIJOBGTL"))
        // {
        //    if (!theta_min.IsSet() || !theta_max.IsSet())
        //    {
        //        std::cout << "Error: using TS2MULTIJOBGTL, set --main::theta_min and --main::theta_max" << std::endl;
        //       return 1;
        //   }
        //   unsigned int tabu_size_min = (unsigned int) std::floor(theta_min * in.Jobs());
        //   unsigned int tabu_size_max = (unsigned int) std::floor(theta_max * in.Jobs());
        //    OSP_ts_two_move_job_fixed_length.SetParameter("min_tenure", tabu_size_min);
        //    OSP_ts_two_move_job_fixed_length.SetParameter("max_tenure", tabu_size_max);
        //    used_runner = &OSP_ts_two_move_job_gendreau;
        //    OSP_solver.SetRunner(OSP_ts_two_move_job_gendreau);
        // }
        // else if (method == std::string("TS3MULTIBATCH"))
        // {
        //    used_runner = &OSP_ts_three_move_batch;
        //    OSP_solver.SetRunner(OSP_ts_three_move_batch);
        // }
        else if (method == std::string("SD2MULTI"))
        {
            used_runner = &OSP_sd_two_move;
            OSP_solver.SetRunner(OSP_sd_two_move);
        }
        //else if (method == std::string("TS1SWAPBATCH"))
        //{
        //    used_runner = &OSP_ts_swap_batches;
        //    OSP_solver.SetRunner(OSP_ts_swap_batches);
        //}
        else if (method == std::string("LAHC6MULTI"))
        {
            if (!ita.IsSet())
            {
                std::cout << "Error: using LAHC6MULTI, set --main::ita" << std::endl;
                return 1;
            }
            unsigned int step_size = (unsigned int) std::floor(ita * in.Jobs());
            OSP_lahc_multi_six.SetParameter("steps", step_size);
            used_runner = &OSP_lahc_multi_six;
            OSP_solver.SetRunner(OSP_lahc_multi_six);
        }
        else if (method == std::string("TOKENRINGTSSA"))
        {
            if (!theta.IsSet())
            {
                std::cout << "Error: using TS2MULTIJOBFLTL, set --main::theta" << std::endl;
                return 1;
            }
            std::cerr << "token ring: " << std::endl;
            std::cerr << "theta " << theta << std::endl;
            std::cerr << "jobs " << in.Jobs() << std::endl;
            std::cerr << swap_rate + insert_rate + job_existing_batch_rate + jobs_new_batch_rate + inverse_rate << " coming from: " << swap_rate << " " << insert_rate << " " << job_existing_batch_rate << " " << jobs_new_batch_rate << " " << inverse_rate << std::endl;
            unsigned int tabu_size = (unsigned int) std::floor(theta * in.Jobs());
            OSP_ts_two_move_job_fixed_length.SetParameter("min_tenure", tabu_size);
            OSP_ts_two_move_job_fixed_length.SetParameter("max_tenure", tabu_size);
        }
        else
        {
            throw std::invalid_argument("Unknown method" + std::string(method));
        }
        SolverResult<OSP_Input,OSP_Output,DefaultCostStructure<int>> result(in);
        
        if (method == std::string("TOKENRINGTSSA"))
        {
            result = token_ring_ts_sa.Solve();
        }
        else
        {
            result = OSP_solver.Solve();
        }
        // result is a tuple: 0: solution, 1: number of violations, 2: total cost, 3: computing time
        OSP_Output out = result.output;
        
        if(irace.IsSet())
        {
            std::cout << (double)result.cost.total/hard_constraint_multiplier << std::endl;
        }
        else if (output_file.IsSet() && method == std::string("TOKENRINGTSSA"))
        { // write the output on the file passed in the command line
            std::ofstream os(static_cast<std::string>(output_file).c_str());
            os << "{\"solution\": {" << out <<  "}, "
               << "\"cost\": " <<  result.cost.total <<  ", "
               << "\"time\": " << result.running_time << ", "
               << "\"seed\": " << Random::GetSeed() << "} " << std::endl;
            os.flush();
            os.close();
        }
        else if (method == std::string("TOKENRINGTSSA"))
        { // write the solution in the standard output
            std::cout << "{\"solution\": {" << out <<  "}, "
               << "\"cost\": " <<  result.cost.total <<  ", "
               << "\"time\": " << result.running_time << ", "
               << "\"seed\": " << Random::GetSeed() << "} " << std::endl;
        }
        else if (output_file.IsSet())
        { // write the output on the file passed in the command line
            std::ofstream os(static_cast<std::string>(output_file).c_str());
            os 
                << "{\"solution\": {" << out <<  "}, "
               << "{\"cost\": " <<  result.cost.total <<  ", "
               << "\"time\": " << result.running_time << ", "
               << "\"total_iterations\": " << used_runner->iteration << ", "
               << "\"iteration_of_best\": " << used_runner->iteration_of_best << ", "
               << "\"seed\": " << Random::GetSeed() << "} " << std::endl;
            os.flush();
            os.close();
        }
        else
        { // write the solution in the standard output
            std::cout << "{\"solution\": {" << out <<  "}, "
               << "\"cost\": " <<  result.cost.total <<  ", "
               << "\"time\": " << result.running_time << ", "
               << "\"total_iterations\": " << used_runner->iteration << ", "
               << "\"iteration_of_best\": " << used_runner->iteration_of_best << ", "
               << "\"seed\": " << Random::GetSeed() << "} " << std::endl;
        }
    }

#if !defined(NDEBUG)
    std::cout << "This code is running in DEBUG mode" << std::endl;
#endif
  return 0;
}
