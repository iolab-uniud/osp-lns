from input import Instance
from solution import Solution,Batch
from initial_solution import greedy_solution

import time
import copy
import logging
import random
import numpy
import minizinc
import tempfile as tf
import subprocess
from datetime import timedelta

class LNS:
    def __init__(self, seed : int, input : Instance, type_initial_solution : str, k_min : int, k_max : int, architecture : str, repair_5 : float, k_batches : float, k_batches_attribute : float,k_batches_same_machine : float):
        """
        input: input as per Instance class.
        type_initial_solution: the initial solution we want to use. "greedy" -> per Lackner et al. (2023).
        k: when applying the destroy_k_given_batches, how many batches do you want to destroy.
        """
        random.seed(seed)
        self.input = input
        self.initial_solution = Solution(input)
        if type_initial_solution == "greedy":
            greedy_solution(self.input.complete_file_name, self.initial_solution, seed)
            self.initial_solution.populate_data_structure_from_scratch()
            logging.debug(f"Initial solution is: {self.initial_solution}")
        else:
            raise NotImplementedError
        self.name_for_cplex_model = f"model_{seed}_{self.input.name.split('R')[0]}"
        self.k_min = k_min
        self.k_max = k_max
        self.k = k_min
        self.architecture = architecture
        self.opl_locations = {}

        self.repair_5 = repair_5
        self.k_batches = k_batches 
        self.k_batches_attribute = k_batches_attribute
        self.k_batches_same_machine = k_batches_same_machine
    
    def destroy(self, solution : Solution) -> (str,list,set,set,Solution):
        start = time.time()
        # selector = random.randint(1,3) # random.randint(1,3)
        selector = numpy.random.choice([1,2,3], 1, p=[self.k_batches_same_machine, self.k_batches_attribute, self.k_batches])[0]
        
        if selector == 0:
            jobs_to_remove,machines_to_relax,batches_to_relax =  self.destroy_most_expensive_machine(solution)
            logging.info(f"Destroy, {time.time() - start}, k_bacthes_most_exp_m,{len(jobs_to_remove)}")
            return "k_bacthes_most_exp_m",jobs_to_remove,machines_to_relax,batches_to_relax,solution
        elif selector == 1:
            jobs_to_remove,machines_to_relax,batches_to_relax = self.destroy_k_random_batches_from_random_machine(solution)
            logging.info(f"Destroy, {time.time() - start}, k_batches_from_random_m, {len(jobs_to_remove)}")
            return "k_batches_from_random_m",jobs_to_remove,machines_to_relax,batches_to_relax,solution
        elif selector == 2:
            jobs_to_remove,machines_to_relax,batches_to_relax = self.destroy_k_random_batches_with_same_attribute(solution)
            logging.info(f"Destroy, {time.time() - start}, k_batches_from_random_attr, {len(jobs_to_remove)}")
            return "k_batches_from_random_attr",jobs_to_remove,machines_to_relax,batches_to_relax,solution
        elif selector == 3:
            jobs_to_remove,machines_to_relax,batches_to_relax = self.destroy_k_random_batches(solution)
            logging.info(f"Destroy, {time.time() - start}, k_batches, {len(jobs_to_remove)}")
            return "k_batches",jobs_to_remove,machines_to_relax,batches_to_relax,solution
        return
        

    def destroy_one_random_machine(self, solution : Solution):
        machine_to_destroy = random.choice(list(solution.machine_to_batch.keys()))
        logging.info(f"Destroy random machine schedule: {machine_to_destroy} ({len(solution.jobs_to_machine[machine_to_destroy])} jobs).")
        return solution.jobs_to_machine[machine_to_destroy], {machine_to_destroy}, set(solution.machine_to_batch[machine_to_destroy])
    
    def destroy_most_expensive_machine(self, solution : Solution) -> list:
        machine_to_destroy = list(solution.machine_to_batch.keys())[0]
        cost_max = solution.calculate_total_cost_per_machine(machine_to_destroy)
        for m in solution.machine_to_batch.keys():
            current_c = solution.calculate_total_cost_per_machine(m)
            if current_c > cost_max:
                cost_max = current_c
                machine_to_destroy = m
        # select the highest objective
        inside_k = self.k
        if len(solution.machine_to_batch[machine_to_destroy]) < self.k:
            inside_k = len(solution.machine_to_batch[machine_to_destroy])
        batches_to_destroy = random.sample(list(solution.machine_to_batch[machine_to_destroy]), k = inside_k)
        to_destroy = list()
        for batch in batches_to_destroy:
            to_destroy = to_destroy + solution.jobs_to_batch[batch]
        logging.info(f"Destroy k random batches from most expensive machine ({machine_to_destroy}) ({cost_max}): {batches_to_destroy} ({len(to_destroy)} jobs).")
        # print("to destroy ", to_destroy)
        return to_destroy, {machine_to_destroy},{}
    
    def destroy_k_random_batches(self, solution : Solution):
        inside_k = self.k
        if len(solution.jobs_to_batch.keys()) < self.k:
            inside_k = len(solution.jobs_to_batch.keys())
        batches_to_destroy = random.sample(list(solution.jobs_to_batch.keys()),inside_k)
        to_destroy = list()
        machines_to_relax = set()
        for batch in batches_to_destroy:
            to_destroy = to_destroy + solution.jobs_to_batch[batch]
            machines_to_relax.add(batch[0])
        logging.info(f"Destroy random k random batches: {batches_to_destroy} ({len(to_destroy)} jobs)")
        return to_destroy,machines_to_relax,{}
    
    def destroy_k_random_batches_from_random_machine(self, solution : Solution):
        machine_to_destroy = random.choice(list(solution.machine_to_batch.keys()))
        inside_k = self.k
        if len(solution.machine_to_batch[machine_to_destroy]) < self.k:
            inside_k = len(solution.machine_to_batch[machine_to_destroy])
        batches_to_destroy = random.sample(list(solution.machine_to_batch[machine_to_destroy]), k = inside_k)
        to_destroy = list()
        for batch in batches_to_destroy:
            to_destroy = to_destroy + solution.jobs_to_batch[batch]
        logging.info(f"Destroy k random batches from random machine ({machine_to_destroy} ): {batches_to_destroy} ({len(to_destroy)} jobs).")
        # print("to destroy ", to_destroy)
        return to_destroy, {machine_to_destroy},{}
    
    def destroy_k_random_batches_with_same_attribute(self, solution : Solution):
        attribute_to_destroy = random.choice(list(solution.batches_per_attribute.keys()))
        inside_k = self.k
        if len(solution.batches_per_attribute[attribute_to_destroy]) < self.k:
            inside_k = len(solution.batches_per_attribute[attribute_to_destroy])
        batches_to_destroy = random.sample(list(solution.batches_per_attribute[attribute_to_destroy]), inside_k)
        to_destroy = list()
        machines_to_relax = set()
        for batch in batches_to_destroy:
            to_destroy = to_destroy + solution.jobs_to_batch[batch]
            machines_to_relax.add(batch[0])
        logging.info(f"Destroy k random batches with same attribute ({attribute_to_destroy}): {batches_to_destroy} ({len(to_destroy)} jobs).")
        # print("to destroy ", to_destroy)
        return to_destroy, machines_to_relax,{}
    
    def repair(self, solution : Solution, destroyer : str, to_modify : list, machine_to_relax : set, batches_to_relax :set) -> Solution:
        start = time.time()
        possible_repairs = ["gurobi","chuffed","cp-optimizer"] # here add others
        selected_repair = numpy.random.choice([1,2], 1, p=[1-self.repair_5, self.repair_5])[0]
        # selected_repair = random.randint(1,2)#"cp-optimizer" # here modify
        # if selected_repair in ["chuffed","gurobi"]:
        #     self.repair_minizinc(solution,to_modify,machine_to_relax,batches_to_relax,destroyer,selected_repair)
        # elif selected_repair == "cp-optimizer":
        #     self.repair_cp_optimizer(solution,to_modify,machine_to_relax,batches_to_relax,destroyer,selected_repair)
        if selected_repair == 1:
            self.repair_cp_optimizer(solution,to_modify,machine_to_relax,batches_to_relax,destroyer,selected_repair)
        else:
            self.repair_cp_optimizer_2(solution,to_modify,machine_to_relax,batches_to_relax,destroyer,selected_repair)
        logging.info(f"Reparation, {selected_repair}, {destroyer}, {time.time() - start}")
        return solution
    
    def repair_cp_optimizer(self, solution : Solution, to_modify : list, machine_to_relax : set, batches_to_relax : set, destroyer : str, solver_string : str) -> Solution:
        start = time.time()
        list_of_commands = list()

        for job in self.input.jobs.keys():
            if job in to_modify:
                for other_machine in self.input.machines.keys():
                    if other_machine not in machine_to_relax:
                                constr = "subject to {" + f"!presenceOf(jobOnMach[{job}][{other_machine}]); " + "}"
                                list_of_commands.append(constr)
                continue
            elif solution.assignement_job_to_oven[job].assigned_oven in machine_to_relax:
                # here you need to relax the machine schedule, meaning that since you are moving its other batches also the others should be "free" of having a new start. However, they cannot change their processing time 
                m = solution.assignement_job_to_oven[job].assigned_oven
                p = solution.assignement_job_to_oven[job].position_in_oven
                if job == solution.batch_characteristics_dictionary[(m,p)].representative_job:
                    constr = "subject to {" + f"lengthOf(jobOnMach[{job}][{m}]) == {solution.batch_characteristics_dictionary[(m,p)].processing_time}; presenceOf(jobOnMach[{job}][{m}]);" + "}"
                    list_of_commands.append(constr)
                    # for all machine different from this one, you have to prevent the assignment
                    for other_machine in self.input.machines.keys():
                        if other_machine != m:
                            constr = "subject to {" + f"!presenceOf(jobOnMach[{job}][{other_machine}]); " + "}"
                            list_of_commands.append(constr)
                    constr = "subject to {" + f"presenceOf(job[{job}]); " + "}"
                    list_of_commands.append(constr)
                else:
                    constr = "subject to {" + f"inBatchWithJob[{job}] == {solution.batch_characteristics_dictionary[(m,p)].representative_job};" + "}"
                    list_of_commands.append(constr) 
                    constr = "subject to {" + f"!presenceOf(job[{job}]); " + "}"
                    list_of_commands.append(constr)      
            else:
                m = solution.assignement_job_to_oven[job].assigned_oven
                p = solution.assignement_job_to_oven[job].position_in_oven
                if job == solution.batch_characteristics_dictionary[(m,p)].representative_job:
                    constr = "subject to {" + f"lengthOf(jobOnMach[{job}][{m}]) == {solution.batch_characteristics_dictionary[(m,p)].processing_time}; presenceOf(jobOnMach[{job}][{m}]);  startOf(jobOnMach[{job}][{m}]) == {solution.batch_characteristics_dictionary[(m,p)].start_time}; endOf(jobOnMach[{job}][{m}]) == {solution.batch_characteristics_dictionary[(m,p)].end_time};" + "}"
                    list_of_commands.append(constr)
                    for other_machine in self.input.machines.keys():
                        if other_machine != m:
                            constr = "subject to {" + f"!presenceOf(jobOnMach[{job}][{other_machine}]); " + "}"
                            list_of_commands.append(constr)
                    constr = "subject to {" + f"presenceOf(job[{job}]); " + "}"
                    list_of_commands.append(constr)
                else:
                    constr = "subject to {" + f"inBatchWithJob[{job}] == {solution.batch_characteristics_dictionary[(m,p)].representative_job};" + "}"
                    list_of_commands.append(constr)   
                    constr = "subject to {" + f"!presenceOf(job[{job}]); " + "}"
                    list_of_commands.append(constr) 
        
        for m in solution.machine_to_batch.keys():
            if m in machine_to_relax:
                for b_1 in range(1, len(solution.machine_to_batch[m]) + 1):
                    repr_1 = solution.batch_characteristics_dictionary[(m,b_1)].representative_job
                    if repr_1 in to_modify:
                        continue
                    for b_2 in range(1, len(solution.machine_to_batch[m]) + 1):
                        repr_2 = solution.batch_characteristics_dictionary[(m,b_2)].representative_job
                        if (repr_2 in to_modify):
                            continue
                        elif b_1 < b_2:
                            constr = "subject to {" + f" startOf(jobOnMach[{repr_1}][{m}]) <= startOf(jobOnMach[{repr_2}][{m}]) ; endOf(jobOnMach[{repr_1}][{m}]) <= startOf(jobOnMach[{repr_2}][{m}]);" + "}"
                            list_of_commands.append(constr)     
                   

        
        mod_file = tf.NamedTemporaryFile(prefix=f"{self.name_for_cplex_model}_{self.iteration}", suffix='.mod')

        model = """
using CP;

//note: any execute block placed before the objective or constraints declaration is part of
//preprocessing; other blocks are part of postprocessing.
execute {
var p = cp.param;
p.TimeLimit = 120; //Limits the CPU time spent solving before terminating a search. In seconds.
//p.FailLimit = 20000; //Limits the number of failures that can occur before terminating the search.
p.Workers = 1; //Sets the number of workers to run in parallel to solve your model. The default value auto corresponds to the number of CPUs available.
//p.FailureDirectedSearchEmphasis = 0.5; 
//p.SearchType = "Restart"; //DepthFirst, Restart, MultiPoint or Auto
p.RelativeOptimalityTolerance = 1e-3; // changed for LNS
}

//---------Input------------------------------------------------------------------------------------------------------

int LengthSchedulingHorizon = ...; 
range Times = 0..LengthSchedulingHorizon;

//attributes
int nAttributes = ...; //number of attributes
range Attributes = 1..nAttributes;
int SetupCosts[0..nAttributes][Attributes] = ...; //setup_costs[i,j] costs for switching from attribute i to attribute j
int SetupTimes[0..nAttributes][Attributes] = ...; //setup_times[i,j] time for switching from attribute i to attribute j
tuple triplet { int a1; int a2; int setupTime; };
{ triplet } SetupTimeTriplets = {<i,j,SetupTimes[i][j]> | i in Attributes, j in Attributes};

//machines
int nMachines = ...; //number of machines
range Machines = 1..nMachines;
int MinCap[Machines] = ...; //minimum capacity per machine
int MaxCap[Machines] = ...; //maximum capacity per machine
int initState[Machines] = ...; //initial state of every machine
int nShifts = ...; //maximum number of shifts per machine
range Shifts = 1..nShifts;
int ShiftStartTimes[Machines][Shifts] = ...; //machine availability start times
int ShiftEndTimes[Machines][Shifts] = ...; //machine availability end times

//jobs
int nJobs = ...; //number of jobs
range Jobs = 1..nJobs;
{int} EligibleMachines[Jobs] = ...; //set of eligible machines for every job
int EarliestStart[Jobs] = ...; //earliest start date for every job
int LatestEnd[Jobs] = ...; //latest end date for every job
int MinTime[Jobs] = ...; //minimum time in machine for every job
int MaxTime[Jobs] = ...; //maximum time in machine for every job
int JobSize[Jobs] = ...; //occupied space in machine for every job
int Attribute[Jobs] = ...; //attribute of every job


//constants needed for the calculation of the objective function
int upper_bound_integer_objective = ...;
int mult_factor_total_runtime = ...;
int mult_factor_finished_toolate = ...;
int mult_factor_total_setuptimes = ...;
int mult_factor_total_setupcosts = ...;
int running_time_bound = ...;
int min_duration = ...;
int max_duration = ...;
int max_setup_time = ...;
int max_setup_cost = ...;

//---------Modelling of machine availability times -------------------------------------------------------------------

//Intensity step functions for machine availabilty times (see scheduling tutorial, chapter 4)
tuple Step {
int v; //availability value of machine (0 or 100)
key int x; //date up to which the availability has this value
};
//tuples need to be sorted for stepwise function
sorted {Step} Steps[m in Machines] =
{ <100, ShiftEndTimes[m][s]> | s in Shifts } union
{ <0, ShiftStartTimes[m][s]> | s in Shifts } union
{ <0, LengthSchedulingHorizon>} ;//off-shift after last on-shift until end of scheduling horizon
stepFunction AvailabilityTimes[m in Machines] =
stepwise (s in Steps[m]) { s.v -> s.x; 0 }; //last value is value of stepwise funtion after final step

//---------Decision Variables-------------------------------------------------------------------
//---------Jobs: 
dvar interval job [j in Jobs] optional in EarliestStart[j]..LengthSchedulingHorizon
	size MinTime[j]..MaxTime[j]; 
dvar interval jobOnMach [j in Jobs][m in Machines] optional in EarliestStart[j]..LengthSchedulingHorizon
	size MinTime[j]..MaxTime[j]
	intensity AvailabilityTimes[m];

//if job is not scheduled, pointer to job that is scheduled
dvar int inBatchWithJob[Jobs] in 0..nJobs;	


//interval variables for setups between jobs/batches
//setupTime[j][m] is setup before jobOnMach[j][m]
dvar interval setupTime [j in Jobs][m in Machines] optional in Times 
	size 0..max_setup_time
	intensity AvailabilityTimes[m];
	
	
//sequence variables for every machine
dvar sequence machines[m in Machines] in
	all( j in Jobs) jobOnMach [j][m] 
	types all( j in Jobs) Attribute[j];
	
//tardy jobs
dvar boolean tardy[Jobs];

//---------Objective Function & Evaluation of Solution-------------------------------------------------------------------
dexpr int totalNumberOfBatches = sum (m in Machines, j in Jobs)
	presenceOf(jobOnMach[j][m]) ;

////components of objective function 
dexpr int runningTimeOvens = sum (m in Machines, j in Jobs)
	lengthOf(jobOnMach[j][m]);
dexpr int numberOfTardyJobs = sum( j in Jobs ) 
   	tardy[j];
dexpr int totalSetupCosts = 
sum (m in Machines)
  (sum (j in Jobs)
		SetupCosts[typeOfPrev(machines[m], jobOnMach[j][m], initState[m], 0)][Attribute[j]]);
 	
dexpr int objective = 
	mult_factor_total_runtime * runningTimeOvens
	+
	mult_factor_finished_toolate * numberOfTardyJobs
	+ 
	mult_factor_total_setupcosts* totalSetupCosts
	;

minimize objective;
   

//--------- Constraints -------------------------------------------------------------------
subject to {

//bounds on objective components  
objective <= upper_bound_integer_objective;
runningTimeOvens <= running_time_bound;
numberOfTardyJobs <= nJobs;
totalSetupCosts <= nJobs * max_setup_cost;

//if job is present, it is scheduled on exactly one machine
//and is scheduled to eligible machine
forall (j in Jobs){
  alternative(job[j], all(m in Machines) jobOnMach[j][m]);
  alternative(job[j], all(m in EligibleMachines[j]) jobOnMach[j][m]);
}

//job is either scheduled or pointer is set to some other job 
//that is scheduled and has lower index
forall (j in Jobs, i in Jobs){
  (presenceOf(job[j]) && inBatchWithJob[j]==0)
  ||
  (!presenceOf(job[j]) && inBatchWithJob[j]>0);
}
forall (j in Jobs, i in Jobs){
  (inBatchWithJob[j]==i)
  =>
  (presenceOf(job[i]) && i<j);
}

//machine capacities may not be exceeded  
forall (m in Machines, j in Jobs)
  ctMachineCapcity : 
    presenceOf(jobOnMach[j][m])
  	*(JobSize[j] + sum(i in Jobs)(JobSize[i]*(inBatchWithJob[i]==j)))
    <= MaxCap[m]
;

//jobs can only be processed in the same batch if they have the same attribute,
//if processing times are compatible
//and start is after earliest start
forall (j in Jobs, i in Jobs){
  inBatchWithJob[j]==i
  =>
  (Attribute[i]==Attribute[j]
  &&
  lengthOf(job[i]) <= MaxTime[j]
  &&
  lengthOf(job[i]) >= MinTime[j]
  &&
  startOf(job[i]) >= EarliestStart[j]);
}

//jobs can only be processed together with other job if assigned machine is eligible
forall (i in Jobs, j in Jobs){
  inBatchWithJob[j] == i
  =>
  (sum (m in EligibleMachines[j])
  	presenceOf(jobOnMach[i][m]))
  == 1;
}

//batches on the same machine may not overlap 
forall(m in Machines)
	noOverlap(machines[m], SetupTimeTriplets, true);
 


//schedule setups between jobs
forall (j in Jobs, m in Machines){
  //setup is present iff job is present on machine
  presenceOf(jobOnMach[j][m]) == presenceOf(setupTime[j][m]);  
  //setup ends exactly at start of following batch
  //usage: endAtStart(a, b, delay) forces endTime(a) + delay == startTime(b)
  //is only effective if both a and b is present, otherwise the constraint is alsways fulfilled
  ctSetupDirectlyBeforeBatch:
  endAtStart(setupTime[j][m], jobOnMach[j][m], 0); 
  //setup starts after preceeding batch
  endOfPrev(machines[m], jobOnMach[j][m] , 0 , 0) <= startOf(setupTime[j][m]);
}

//length of setup times
forall (m in Machines, j in Jobs){
	  ctSetupLengthWhenPresent:
	  //usage: typeOfPrev(sequenceVar seq, intervalVar interval, int firstValue = 0, int absentValue = 0)
	  // firstValue: value to return if interval variable interval is the first one in seq.
      //absentValue: value to return if interval variable interval becomes absent. 
	  (lengthOf(setupTime[j][m]) == SetupTimes[typeOfPrev(machines[m], jobOnMach[j][m], initState[m], 0)][Attribute[j]]);
}

forall (j in Jobs, m in Machines){
  	//usage of forbidExtent: whenever interval variable jobOnMach[j][m] is present, 
  	//it cannot overlap a point t where AvailabilityTimes[m](t) = 0
	forbidExtent(jobOnMach[j][m], AvailabilityTimes[m]);
	forbidExtent(setupTime[j][m], AvailabilityTimes[m]);
}


//Redundant constraint: restrict the number of batches
//(at most as many batches as there are jobs)
totalNumberOfBatches <= nJobs;

//define tardy jobs
forall (j in Jobs, i in Jobs){
  (presenceOf(job[j]) && endOf(job[j]) > LatestEnd[j])
  || 
  (inBatchWithJob[j]==i && endOf(job[i]) > LatestEnd[j])
  =>
  tardy[j]==1;
}
}



//--------- Output -------------------------------------------------------------------

execute {
//when is job scheduled
for (var j in Jobs){
  for (var m in Machines){
      	if (jobOnMach[j][m].present)
      		writeln ("RJ " + j + " > " + m + " > " + jobOnMach[j][m].start + " > " + jobOnMach[j][m].end );
  }
}
}	
// inBatchWithJob
execute {
//when is job scheduled
for (var j in Jobs){
    if (inBatchWithJob[j]!=0)
        writeln ("IBW " + j + " > " + inBatchWithJob[j] );
}
}

//create ouput for evaluation of results
execute {
  writeln ("objective = " + objective + "; ");
}

"""

        model_new = model + " ".join(list_of_commands)
        with open(mod_file.name, 'w') as f:
            f.write(model_new)
        # call
        try:
            cmd = [
                self.opl_locations[self.architecture],
                mod_file.name,
                self.input.complete_file_name.split(".dzn")[0] + ".dat"
            ] 
            result = subprocess.run(cmd, stdout=subprocess.PIPE, text=True)

        finally:
            mod_file.close()

        # if  time.time() - start > 170.0:
        #     with open("model_broken.mod", 'w') as f:
        #         f.write(model_new)
        #         raise KeyError
        # print("========= RESULT STDOUT ", result.stdout)
        # decode
        bc = self.reconstruct_batches(result.stdout)
        # put in the format you need
        solution.assignement_job_to_oven = dict()
        for b in bc.keys():
            for job in bc[b]:
                solution.assignement_job_to_oven[job] = Batch(b[0], b[1])
        solution.populate_data_structure_from_scratch()
        return solution
    
    def repair_cp_optimizer_2(self, solution : Solution, to_modify : list, machine_to_relax : set, batches_to_relax : set, destroyer : str, solver_string : str) -> Solution:
        start = time.time()
        list_of_commands = list()

        for job in self.input.jobs.keys():
            if job in to_modify:
                for other_machine in self.input.machines.keys():
                    if other_machine not in machine_to_relax:
                                constr = "subject to {" + f"!presenceOf(jobOnMach[{job}][{other_machine}]); " + "}"
                                list_of_commands.append(constr)
                continue
            elif solution.assignement_job_to_oven[job].assigned_oven in machine_to_relax:
                # here you need to relax the machine schedule, meaning that since you are moving its other batches also the others should be "free" of having a new start. However, they cannot change their processing time 
                m = solution.assignement_job_to_oven[job].assigned_oven
                p = solution.assignement_job_to_oven[job].position_in_oven
                if job == solution.batch_characteristics_dictionary[(m,p)].representative_job:
                    constr = "subject to {" + f"lengthOf(jobOnMach[{job}][{m}]) == {solution.batch_characteristics_dictionary[(m,p)].processing_time}; presenceOf(jobOnMach[{job}][{m}]);" + "}"
                    list_of_commands.append(constr)
                    # for all machine different from this one, you have to prevent the assignment
                    for other_machine in self.input.machines.keys():
                        if other_machine != m:
                            constr = "subject to {" + f"!presenceOf(jobOnMach[{job}][{other_machine}]); " + "}"
                            list_of_commands.append(constr)
                    constr = "subject to {" + f"presenceOf(job[{job}]); " + "}"
                    list_of_commands.append(constr)
                else:
                    constr = "subject to {" + f"inBatchWithJob[{job}] == {solution.batch_characteristics_dictionary[(m,p)].representative_job};" + "}"
                    list_of_commands.append(constr) 
                    constr = "subject to {" + f"!presenceOf(job[{job}]); " + "}"
                    list_of_commands.append(constr)      
            else:
                m = solution.assignement_job_to_oven[job].assigned_oven
                p = solution.assignement_job_to_oven[job].position_in_oven
                if job == solution.batch_characteristics_dictionary[(m,p)].representative_job:
                    constr = "subject to {" + f"lengthOf(jobOnMach[{job}][{m}]) == {solution.batch_characteristics_dictionary[(m,p)].processing_time}; presenceOf(jobOnMach[{job}][{m}]);  startOf(jobOnMach[{job}][{m}]) == {solution.batch_characteristics_dictionary[(m,p)].start_time}; endOf(jobOnMach[{job}][{m}]) == {solution.batch_characteristics_dictionary[(m,p)].end_time};" + "}"
                    list_of_commands.append(constr)
                    for other_machine in self.input.machines.keys():
                        if other_machine != m:
                            constr = "subject to {" + f"!presenceOf(jobOnMach[{job}][{other_machine}]); " + "}"
                            list_of_commands.append(constr)
                    constr = "subject to {" + f"presenceOf(job[{job}]); " + "}"
                    list_of_commands.append(constr)
                else:
                    constr = "subject to {" + f"inBatchWithJob[{job}] == {solution.batch_characteristics_dictionary[(m,p)].representative_job};" + "}"
                    list_of_commands.append(constr)   
                    constr = "subject to {" + f"!presenceOf(job[{job}]); " + "}"
                    list_of_commands.append(constr) 
        
        for m in solution.machine_to_batch.keys():
            if m in machine_to_relax:
                for b_1 in range(1, len(solution.machine_to_batch[m]) + 1):
                    repr_1 = solution.batch_characteristics_dictionary[(m,b_1)].representative_job
                    if repr_1 in to_modify:
                        continue
                    for b_2 in range(1, len(solution.machine_to_batch[m]) + 1):
                        repr_2 = solution.batch_characteristics_dictionary[(m,b_2)].representative_job
                        if (repr_2 in to_modify):
                            continue
                        elif b_1 < b_2:
                            constr = "subject to {" + f" startOf(jobOnMach[{repr_1}][{m}]) <= startOf(jobOnMach[{repr_2}][{m}]) ; endOf(jobOnMach[{repr_1}][{m}]) <= startOf(jobOnMach[{repr_2}][{m}]);" + "}"
                            list_of_commands.append(constr)     
                   

        
        mod_file = tf.NamedTemporaryFile(prefix=f"{self.name_for_cplex_model}_{self.iteration}", suffix='.mod')

        model = """
using CP;

//note: any execute block placed before the objective or constraints declaration is part of
//preprocessing; other blocks are part of postprocessing.
execute {
var p = cp.param;
p.TimeLimit = 120; //Limits the CPU time spent solving before terminating a search. In seconds.
//p.FailLimit = 20000; //Limits the number of failures that can occur before terminating the search.
p.Workers = 1; //Sets the number of workers to run in parallel to solve your model. The default value auto corresponds to the number of CPUs available.
//p.FailureDirectedSearchEmphasis = 0.5; 
//p.SearchType = "Restart"; //DepthFirst, Restart, MultiPoint or Auto
p.RelativeOptimalityTolerance = 0.05; // changed for LNS
}

//---------Input------------------------------------------------------------------------------------------------------

int LengthSchedulingHorizon = ...; 
range Times = 0..LengthSchedulingHorizon;

//attributes
int nAttributes = ...; //number of attributes
range Attributes = 1..nAttributes;
int SetupCosts[0..nAttributes][Attributes] = ...; //setup_costs[i,j] costs for switching from attribute i to attribute j
int SetupTimes[0..nAttributes][Attributes] = ...; //setup_times[i,j] time for switching from attribute i to attribute j
tuple triplet { int a1; int a2; int setupTime; };
{ triplet } SetupTimeTriplets = {<i,j,SetupTimes[i][j]> | i in Attributes, j in Attributes};

//machines
int nMachines = ...; //number of machines
range Machines = 1..nMachines;
int MinCap[Machines] = ...; //minimum capacity per machine
int MaxCap[Machines] = ...; //maximum capacity per machine
int initState[Machines] = ...; //initial state of every machine
int nShifts = ...; //maximum number of shifts per machine
range Shifts = 1..nShifts;
int ShiftStartTimes[Machines][Shifts] = ...; //machine availability start times
int ShiftEndTimes[Machines][Shifts] = ...; //machine availability end times

//jobs
int nJobs = ...; //number of jobs
range Jobs = 1..nJobs;
{int} EligibleMachines[Jobs] = ...; //set of eligible machines for every job
int EarliestStart[Jobs] = ...; //earliest start date for every job
int LatestEnd[Jobs] = ...; //latest end date for every job
int MinTime[Jobs] = ...; //minimum time in machine for every job
int MaxTime[Jobs] = ...; //maximum time in machine for every job
int JobSize[Jobs] = ...; //occupied space in machine for every job
int Attribute[Jobs] = ...; //attribute of every job


//constants needed for the calculation of the objective function
int upper_bound_integer_objective = ...;
int mult_factor_total_runtime = ...;
int mult_factor_finished_toolate = ...;
int mult_factor_total_setuptimes = ...;
int mult_factor_total_setupcosts = ...;
int running_time_bound = ...;
int min_duration = ...;
int max_duration = ...;
int max_setup_time = ...;
int max_setup_cost = ...;

//---------Modelling of machine availability times -------------------------------------------------------------------

//Intensity step functions for machine availabilty times (see scheduling tutorial, chapter 4)
tuple Step {
int v; //availability value of machine (0 or 100)
key int x; //date up to which the availability has this value
};
//tuples need to be sorted for stepwise function
sorted {Step} Steps[m in Machines] =
{ <100, ShiftEndTimes[m][s]> | s in Shifts } union
{ <0, ShiftStartTimes[m][s]> | s in Shifts } union
{ <0, LengthSchedulingHorizon>} ;//off-shift after last on-shift until end of scheduling horizon
stepFunction AvailabilityTimes[m in Machines] =
stepwise (s in Steps[m]) { s.v -> s.x; 0 }; //last value is value of stepwise funtion after final step

//---------Decision Variables-------------------------------------------------------------------
//---------Jobs: 
dvar interval job [j in Jobs] optional in EarliestStart[j]..LengthSchedulingHorizon
	size MinTime[j]..MaxTime[j]; 
dvar interval jobOnMach [j in Jobs][m in Machines] optional in EarliestStart[j]..LengthSchedulingHorizon
	size MinTime[j]..MaxTime[j]
	intensity AvailabilityTimes[m];

//if job is not scheduled, pointer to job that is scheduled
dvar int inBatchWithJob[Jobs] in 0..nJobs;	


//interval variables for setups between jobs/batches
//setupTime[j][m] is setup before jobOnMach[j][m]
dvar interval setupTime [j in Jobs][m in Machines] optional in Times 
	size 0..max_setup_time
	intensity AvailabilityTimes[m];
	
	
//sequence variables for every machine
dvar sequence machines[m in Machines] in
	all( j in Jobs) jobOnMach [j][m] 
	types all( j in Jobs) Attribute[j];
	
//tardy jobs
dvar boolean tardy[Jobs];

//---------Objective Function & Evaluation of Solution-------------------------------------------------------------------
dexpr int totalNumberOfBatches = sum (m in Machines, j in Jobs)
	presenceOf(jobOnMach[j][m]) ;

////components of objective function 
dexpr int runningTimeOvens = sum (m in Machines, j in Jobs)
	lengthOf(jobOnMach[j][m]);
dexpr int numberOfTardyJobs = sum( j in Jobs ) 
   	tardy[j];
dexpr int totalSetupCosts = 
sum (m in Machines)
  (sum (j in Jobs)
		SetupCosts[typeOfPrev(machines[m], jobOnMach[j][m], initState[m], 0)][Attribute[j]]);
 	
dexpr int objective = 
	mult_factor_total_runtime * runningTimeOvens
	+
	mult_factor_finished_toolate * numberOfTardyJobs
	+ 
	mult_factor_total_setupcosts* totalSetupCosts
	;

minimize objective;
   

//--------- Constraints -------------------------------------------------------------------
subject to {

//bounds on objective components  
objective <= upper_bound_integer_objective;
runningTimeOvens <= running_time_bound;
numberOfTardyJobs <= nJobs;
totalSetupCosts <= nJobs * max_setup_cost;

//if job is present, it is scheduled on exactly one machine
//and is scheduled to eligible machine
forall (j in Jobs){
  alternative(job[j], all(m in Machines) jobOnMach[j][m]);
  alternative(job[j], all(m in EligibleMachines[j]) jobOnMach[j][m]);
}

//job is either scheduled or pointer is set to some other job 
//that is scheduled and has lower index
forall (j in Jobs, i in Jobs){
  (presenceOf(job[j]) && inBatchWithJob[j]==0)
  ||
  (!presenceOf(job[j]) && inBatchWithJob[j]>0);
}
forall (j in Jobs, i in Jobs){
  (inBatchWithJob[j]==i)
  =>
  (presenceOf(job[i]) && i<j);
}

//machine capacities may not be exceeded  
forall (m in Machines, j in Jobs)
  ctMachineCapcity : 
    presenceOf(jobOnMach[j][m])
  	*(JobSize[j] + sum(i in Jobs)(JobSize[i]*(inBatchWithJob[i]==j)))
    <= MaxCap[m]
;

//jobs can only be processed in the same batch if they have the same attribute,
//if processing times are compatible
//and start is after earliest start
forall (j in Jobs, i in Jobs){
  inBatchWithJob[j]==i
  =>
  (Attribute[i]==Attribute[j]
  &&
  lengthOf(job[i]) <= MaxTime[j]
  &&
  lengthOf(job[i]) >= MinTime[j]
  &&
  startOf(job[i]) >= EarliestStart[j]);
}

//jobs can only be processed together with other job if assigned machine is eligible
forall (i in Jobs, j in Jobs){
  inBatchWithJob[j] == i
  =>
  (sum (m in EligibleMachines[j])
  	presenceOf(jobOnMach[i][m]))
  == 1;
}

//batches on the same machine may not overlap 
forall(m in Machines)
	noOverlap(machines[m], SetupTimeTriplets, true);
 


//schedule setups between jobs
forall (j in Jobs, m in Machines){
  //setup is present iff job is present on machine
  presenceOf(jobOnMach[j][m]) == presenceOf(setupTime[j][m]);  
  //setup ends exactly at start of following batch
  //usage: endAtStart(a, b, delay) forces endTime(a) + delay == startTime(b)
  //is only effective if both a and b is present, otherwise the constraint is alsways fulfilled
  ctSetupDirectlyBeforeBatch:
  endAtStart(setupTime[j][m], jobOnMach[j][m], 0); 
  //setup starts after preceeding batch
  endOfPrev(machines[m], jobOnMach[j][m] , 0 , 0) <= startOf(setupTime[j][m]);
}

//length of setup times
forall (m in Machines, j in Jobs){
	  ctSetupLengthWhenPresent:
	  //usage: typeOfPrev(sequenceVar seq, intervalVar interval, int firstValue = 0, int absentValue = 0)
	  // firstValue: value to return if interval variable interval is the first one in seq.
      //absentValue: value to return if interval variable interval becomes absent. 
	  (lengthOf(setupTime[j][m]) == SetupTimes[typeOfPrev(machines[m], jobOnMach[j][m], initState[m], 0)][Attribute[j]]);
}

forall (j in Jobs, m in Machines){
  	//usage of forbidExtent: whenever interval variable jobOnMach[j][m] is present, 
  	//it cannot overlap a point t where AvailabilityTimes[m](t) = 0
	forbidExtent(jobOnMach[j][m], AvailabilityTimes[m]);
	forbidExtent(setupTime[j][m], AvailabilityTimes[m]);
}


//Redundant constraint: restrict the number of batches
//(at most as many batches as there are jobs)
totalNumberOfBatches <= nJobs;

//define tardy jobs
forall (j in Jobs, i in Jobs){
  (presenceOf(job[j]) && endOf(job[j]) > LatestEnd[j])
  || 
  (inBatchWithJob[j]==i && endOf(job[i]) > LatestEnd[j])
  =>
  tardy[j]==1;
}
}



//--------- Output -------------------------------------------------------------------

execute {
//when is job scheduled
for (var j in Jobs){
  for (var m in Machines){
      	if (jobOnMach[j][m].present)
      		writeln ("RJ " + j + " > " + m + " > " + jobOnMach[j][m].start + " > " + jobOnMach[j][m].end );
  }
}
}	
// inBatchWithJob
execute {
//when is job scheduled
for (var j in Jobs){
    if (inBatchWithJob[j]!=0)
        writeln ("IBW " + j + " > " + inBatchWithJob[j] );
}
}

//create ouput for evaluation of results
execute {
  writeln ("objective = " + objective + "; ");
}

"""

        model_new = model + " ".join(list_of_commands)
        with open(mod_file.name, 'w') as f:
            f.write(model_new)
        # call
        try:
            cmd = [
                self.opl_locations[self.architecture],
                mod_file.name,
                self.input.complete_file_name.split(".dzn")[0] + ".dat"
            ] 
            result = subprocess.run(cmd, stdout=subprocess.PIPE, text=True)

        finally:
            mod_file.close()

        # if  time.time() - start > 170.0:
        #     with open("model_broken.mod", 'w') as f:
        #         f.write(model_new)
        #         raise KeyError
#        print("========= RESULT STDOUT ", result.stdout)
        # decode
        bc = self.reconstruct_batches(result.stdout)
        # put in the format you need
        solution.assignement_job_to_oven = dict()
        for b in bc.keys():
            for job in bc[b]:
                solution.assignement_job_to_oven[job] = Batch(b[0], b[1])
        solution.populate_data_structure_from_scratch()
#	print(solution.total_cost)
        return solution

    def repair_minizinc(self, solution : Solution, to_modify : list, machine_to_relax : set, batches_to_relax : set, destroyer : str, solver_string : str) -> Solution:
        # try to call minizinc
        solv = minizinc.Solver.lookup(solver_string)
        solver_to_model = {
            "chuffed":"exact-methods/models/MiniZinc/batch-pos/direct-batch-pos-default.mzn",
            "gurobi":"exact-methods/models/MiniZinc/batch-pos/direct-batch-pos-default.mzn",
            } # TODO: add others
        mod = minizinc.Model(solver_to_model[solver_string])
        ins = minizinc.Instance(solver=solv,model=mod)
        ins.add_file(self.input.complete_file_name)

        fixed_variables = []
        if destroyer == "CompleteMachine":
            for batch in solution.jobs_to_batch.keys():
                all_jobs = solution.jobs_to_batch[batch]
                m = batch[0]
                p = batch[1]
                if m in batches_to_relax:
                    continue
                # fix what is related to the jobs
                for j in all_jobs:
                    fixed_variables.append(
                        f"""constraint batch_for_job[{j}] = {p};"""
                    )
                    fixed_variables.append(
                        f"""constraint machine_for_job[{j}] = {m}; """
                    )
                    fixed_variables.append(
                        f"""constraint job_in_batch[{m},{p},{j}] = 1; """
                    )
                    for other_batch in solution.jobs_to_batch.keys():
                        if other_batch != batch:
                            fixed_variables.append(
                                f"""constraint job_in_batch[{other_batch[0]},{other_batch[1]},{j}] = 0; """
                            )
                # fix what is related to the batches
                fixed_variables.append(
                    f"""constraint start_times[{m},{p}] = {solution.batch_characteristics_dictionary[batch].start_time}; """
                )
                fixed_variables.append(
                    f"""constraint duration[{m},{p}] = {solution.batch_characteristics_dictionary[batch].processing_time}; """
                )
                fixed_variables.append(
                    f"""constraint empty_batch[{m},{p}] = 0; """
                )
                fixed_variables.append(
                    f"""constraint attribute_for_batch[{m},{p}] = {solution.batch_characteristics_dictionary[batch].attribute}; """
                )
        if destroyer == "BatchBased":
            batches_to_fix = set()
            for job in solution.assignement_job_to_oven.keys():
                if job in to_modify:
                    continue
                fixed_variables.append(f"constraint job_in_batch[{solution.assignement_job_to_oven[job].assigned_oven},{solution.assignement_job_to_oven[job].position_in_oven},{job}] = 1;")

                # %late_job[j,ma,ba]=1 if job is in batch ba on machine ba and is late
                # if solution.input.jobs[job].latest_end < solution.batch_characteristics_dictionary[solution.assignement_job_to_oven[job].assigned_oven,solution.assignement_job_to_oven[job].position_in_oven].end_time:
                #     fixed_variables.append(f"constraint late_job[{job},{solution.assignement_job_to_oven[job].assigned_oven},{solution.assignement_job_to_oven[job].position_in_oven}] = 1;")
                # else:
                #     fixed_variables.append(f"constraint late_job[{job},{solution.assignement_job_to_oven[job].assigned_oven},{solution.assignement_job_to_oven[job].position_in_oven}] = 0;")

                batches_to_fix.add((solution.assignement_job_to_oven[job].assigned_oven,solution.assignement_job_to_oven[job].position_in_oven))
            
                for batch in solution.jobs_to_batch.keys():
                    if batch != (solution.assignement_job_to_oven[job].assigned_oven,solution.assignement_job_to_oven[job].position_in_oven):
                        fixed_variables.append(f"constraint job_in_batch[{batch[0]},{batch[1]},{job}] = 0;")
            
            for batch in batches_to_fix:
                # start_times[ma,ba] is start time of batch ba on machine ma
                # fixed_variables.append(f"constraint start_times[{batch[0]},{batch[1]}] = {solution.batch_characteristics_dictionary[batch].start_time};")
                # duration[ma,ba] is duration of batch ba on machine ma
                fixed_variables.append(f"constraint duration[{batch[0]},{batch[1]}] = {solution.batch_characteristics_dictionary[batch].processing_time};")
                # attribute_for_batch[ma,ba] is attribute of all jobs in batch ba on machine ma
                fixed_variables.append(f"constraint attribute_for_batch [{batch[0]},{batch[1]}] = {solution.batch_characteristics_dictionary[batch].attribute};")
                # empty_batch[ma,ba]=1 if the batch ba on machine ma is empty
                fixed_variables.append(f"constraint empty_batch [{batch[0]},{batch[1]}] = 0;")


        # batches_to_fix = set()
        # for job in solution.assignement_job_to_oven.keys():
        #     if job in to_modify:
        #         continue
        #     if solution.assignement_job_to_oven[job].assigned_oven in machine_to_relax: 
        #         fixed_variables.append(f"constraint exists(k in 1..n)( job_in_batch[{solution.assignement_job_to_oven[job].assigned_oven}, k, {job}] = 1 /\ empty_batch [{solution.assignement_job_to_oven[job].assigned_oven},k] = 0) ;")

        #         fixed_variables.append(f"constraint machine_for_job[{job}] = {solution.assignement_job_to_oven[job].assigned_oven};")

        #         for batch in solution.jobs_to_batch.keys():
        #             if batch[0] != solution.assignement_job_to_oven[job].assigned_oven:
        #                 fixed_variables.append(f"constraint job_in_batch[{batch[0]},{batch[1]},{job}] = 0;")
        #     else: # the machine is untouched, so you fix it
        #         fixed_variables.append(f"constraint job_in_batch[{solution.assignement_job_to_oven[job].assigned_oven},{solution.assignement_job_to_oven[job].position_in_oven},{job}] = 1;")
        #         fixed_variables.append(f"constraint machine_for_job[{job}] = {solution.assignement_job_to_oven[job].assigned_oven};")
        #         fixed_variables.append(f"constraint batch_for_job[{job}] = {solution.assignement_job_to_oven[job].position_in_oven};")
        #         # %late_job[j,ma,ba]=1 if job is in batch ba on machine ba and is late
        #         if solution.input.jobs[job].latest_end < solution.batch_characteristics_dictionary[solution.assignement_job_to_oven[job].assigned_oven,solution.assignement_job_to_oven[job].position_in_oven].end_time:
        #             fixed_variables.append(f"constraint late_job[{job},{solution.assignement_job_to_oven[job].assigned_oven},{solution.assignement_job_to_oven[job].position_in_oven}] = 1;")
        #         else:
        #             fixed_variables.append(f"constraint late_job[{job},{solution.assignement_job_to_oven[job].assigned_oven},{solution.assignement_job_to_oven[job].position_in_oven}] = 0;")

        #         batches_to_fix.add((solution.assignement_job_to_oven[job].assigned_oven,solution.assignement_job_to_oven[job].position_in_oven))
        
        #         for batch in solution.jobs_to_batch.keys():
        #             if batch != (solution.assignement_job_to_oven[job].assigned_oven,solution.assignement_job_to_oven[job].position_in_oven):
        #                 fixed_variables.append(f"constraint job_in_batch[{batch[0]},{batch[1]},{job}] = 0;")
        
        # for batch in batches_to_fix:
        #     # start_times[ma,ba] is start time of batch ba on machine ma
        #     fixed_variables.append(f"constraint start_times[{batch[0]},{batch[1]}] = {solution.batch_characteristics_dictionary[batch].start_time};")
        #     # duration[ma,ba] is duration of batch ba on machine ma
        #     fixed_variables.append(f"constraint duration[{batch[0]},{batch[1]}] = {solution.batch_characteristics_dictionary[batch].processing_time};")
        #     # attribute_for_batch[ma,ba] is attribute of all jobs in batch ba on machine ma
        #     fixed_variables.append(f"constraint attribute_for_batch [{batch[0]},{batch[1]}] = {solution.batch_characteristics_dictionary[batch].attribute};")
        #     # empty_batch[ma,ba]=1 if the batch ba on machine ma is empty
        #     fixed_variables.append(f"constraint empty_batch [{batch[0]},{batch[1]}] = 0;")
            
        fixed_variables_str = " ".join(fixed_variables)
        # print(fixed_variables_str)
        ins.add_string(fixed_variables_str)
        logging.info("Calling minizinc solver.")
        result = ins.solve(timeout=timedelta(seconds=180))
        logging.info("Minizinc finished.")
        solution.assignement_job_to_oven = dict()
        for job in self.input.jobs.keys():
            index = job - 1
            solution.assignement_job_to_oven[job] = Batch(result["machine_for_job"][index], result["batch_for_job"][index])
        logging.info(f"Minizinc result --> {result['objective']} - checking purpose")
        solution.populate_data_structure_from_scratch()
        return solution

    def solve_simple(self, timeout : int):
        self.start_time = time.time()
        # logging.info(f"Start procedure.")
        
        def time_left(): 
            return timeout - (time.time() - self.start_time)
        
        # current best solution are equal to the initial solution
        self.best_solution = copy.deepcopy(self.initial_solution)
        self.best_cost = self.best_solution.get_total_cost

        # logging.info(f"Initialization completed.")
        
        self.iteration = 0
        self.idle_iteration = 0
        logging.info(f"It: {self.iteration}, current_cost: {self.best_cost}, best_cost: {self.best_cost}")
        while time_left() > 0:
            self.iteration += 1
            self.idle_iteration += 1
            
            # destroy of the incumbent solution and repair the incumbent solution
            destroyer, jobs_to_remove, machines_to_relax, batches_to_relax, sol = self.destroy(copy.deepcopy(self.best_solution))
            current_solution = self.repair(sol, destroyer, jobs_to_remove, machines_to_relax, batches_to_relax)
            current_cost =  current_solution.get_total_cost  

            # if it is improving - always, then update also the best
            if current_cost < self.best_cost:
                self.best_solution = copy.deepcopy(current_solution)
                self.best_cost = current_cost
                self.idle_iteration = 0
                self.k = self.k_min
            elif self.idle_iteration > 5 and self.k < self.k_max:
                self.k = self.k + 1
            
            logging.info(f"It: {self.iteration}, current_cost: {current_cost}, best_cost: {self.best_cost}")

        self.end_time = time.time()
        # logging.info(f"End procedure.")
        # TODO: add a series of checker
        assert self.best_cost == self.best_solution.get_total_cost

    def solve_simple_n_max(self, timeout : int):
        self.start_time = time.time()
        # logging.info(f"Start procedure.")
        
        def time_left(): 
            return timeout - (time.time() - self.start_time)
        
        # current best solution are equal to the initial solution
        self.best_solution = copy.deepcopy(self.initial_solution)
        self.best_cost = self.best_solution.get_total_cost

        # logging.info(f"Initialization completed.")
        
        self.iteration = 0
        self.idle_iteration = 0
        self.last_change = 0
        logging.info(f"It: {self.iteration}, current_cost: {self.best_cost}, best_cost: {self.best_cost}")
        while time_left() > 0:
            self.iteration += 1
            self.idle_iteration += 1
            self.last_change += 1
            
            # destroy of the incumbent solution and repair the incumbent solution
            destroyer, jobs_to_remove, machines_to_relax, batches_to_relax, sol = self.destroy(copy.deepcopy(self.best_solution))
            current_solution = self.repair(sol, destroyer, jobs_to_remove, machines_to_relax, batches_to_relax)
            current_cost =  current_solution.get_total_cost  

            # if it is improving - always, then update also the best
            if current_cost < self.best_cost:
                self.best_solution = copy.deepcopy(current_solution)
                self.best_cost = current_cost
                self.idle_iteration = 0
                self.last_change = 0
                self.k = self.k_min
            elif self.idle_iteration > 5 and self.k < self.k_max and self.last_change > 5:
                self.k = self.k + 1
                self.last_change = 0
            
            logging.info(f"It: {self.iteration}, current_cost: {current_cost}, best_cost: {self.best_cost}")

        self.end_time = time.time()
        # logging.info(f"End procedure.")
        # TODO: add a series of checker
        assert self.best_cost == self.best_solution.get_total_cost
        print(self.best_cost)
        logging.info(f"End: {self.iteration}, best_cost: {self.best_cost}, oven_running_time: {self.best_solution.cumulative_process_time}")

    def solve_sa(self):
        raise NotImplementedError
    
    def reconstruct_batches(self, output_text):
            # Initialize variables to store job scheduling information
            job_processing_times = {}
            job_processed_with = {}

            # Flag to indicate if objective section is reached
            objective_reached = False

            # Iterate over each line in the text
            for line in output_text.split('\n'):
                # Check if the line starts with "OBJECTIVE"
                if line.strip().startswith('OBJECTIVE'):
                    objective_reached = True
                # Process lines after the objective section
                if objective_reached:
                    # Check if the line starts with "RJ"
                    if line.strip().startswith('RJ'):
                        # Split the line and extract job, machine, start, and end processing
                        parts = line.strip().split(' > ')
                        job = parts[0].split()[1]
                        machine = parts[1]
                        start_processing = parts[2]
                        end_processing = parts[3]
                        # Store job processing times
                        job_processing_times[job] = (machine, start_processing, end_processing)
                    # Check if the line starts with "IBW"
                    elif line.strip().startswith('IBW'):
                        # Split the line and extract job and the job it's processed with
                        parts = line.strip().split()
                        job = parts[1]
                        processed_with = parts[3]
                        # Store job processed with information
                        job_processed_with[job] = processed_with
            
            start_times_per_machine = {}
            for m in self.input.machines.keys():# FIXME: number machines
                start_times_per_machine[m] = set()

            dict_batch_composition = dict()
            for job in job_processing_times.keys():
                start_times_per_machine[int(job_processing_times[job][0])].add(int(job_processing_times[job][1]))
                dict_batch_composition[int(job)] = {"batches":[int(job)], "machine":int(job_processing_times[job][0]), "start":int(job_processing_times[job][1])}

            for job, processed_with in job_processed_with.items():
                dict_batch_composition[int(processed_with)]["batches"].append(int(job))

            bc = dict()
            for batch in dict_batch_composition:
                ls = list(start_times_per_machine[dict_batch_composition[batch]["machine"]])
                ls.sort()
                pos = ls.index(dict_batch_composition[batch]["start"]) + 1
                bc[(dict_batch_composition[batch]["machine"],pos)] = dict_batch_composition[batch]["batches"]

            return bc
