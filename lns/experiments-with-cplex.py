from solution import Solution

import tempfile as tf
import subprocess
import json
import logging
import copy

model ="""

using CP;

//note: any execute block placed before the objective or constraints declaration is part of
//preprocessing; other blocks are part of postprocessing.
execute {
var p = cp.param;
p.TimeLimit = 3600; //Limits the CPU time spent solving before terminating a search. In seconds.
//p.FailLimit = 20000; //Limits the number of failures that can occur before terminating the search.
p.Workers = 1; //Sets the number of workers to run in parallel to solve your model. The default value auto corresponds to the number of CPUs available.
//p.FailureDirectedSearchEmphasis = 0.5; 
//p.SearchType = "Restart"; //DepthFirst, Restart, MultiPoint or Auto
p.RelativeOptimalityTolerance = 1e-8; //same value as used for minizinc mip solvers
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

subject to {lengthOf(jobOnMach[1][2]) == 7; presenceOf(jobOnMach[1][2]); startOf(jobOnMach[1][2]) == 9; endOf(jobOnMach[1][2]) == 16;} subject to {!presenceOf(jobOnMach[1][1]);} subject to {lengthOf(jobOnMach[5][1]) == 10; presenceOf(jobOnMach[5][1]);} subject to {!presenceOf(jobOnMach[5][2]);} subject to {lengthOf(jobOnMach[6][1]) == 4; presenceOf(jobOnMach[6][1]);} subject to {!presenceOf(jobOnMach[6][2]);} subject to {lengthOf(jobOnMach[7][2]) == 2; presenceOf(jobOnMach[7][2]); startOf(jobOnMach[7][2]) == 18; endOf(jobOnMach[7][2]) == 20;} subject to {!presenceOf(jobOnMach[7][1]);} subject to {inBatchWithJob[8] == 5;} subject to {inBatchWithJob[9] == 1}
"""

logging.debug(f"Start greedy procedure.")
mod_file = tf.NamedTemporaryFile(prefix='cp_model', suffix='.mod')



with open(mod_file.name, 'w') as f:
    f.write(model)

try:
    cmd = [
        "/Applications/CPLEX_Studio_Community2211/opl/bin/arm64_osx/oplrun",
        mod_file.name,
        "instances/instancesUC2/01NewRandomOvenSchedulingInstance-n10-k2-a2--2904-10.37.39.dat"
    ] 
    result = subprocess.run(cmd, stdout=subprocess.PIPE, text=True)

finally:
    mod_file.close()

def reconstruct_batches(output_text):
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
    for m in [1,2]:# FIXME: number machines
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

# Example usage:
output_text = result.stdout
print(output_text)

# Call the function to reconstruct batches and scheduling
bc = reconstruct_batches(output_text)



print(bc)


