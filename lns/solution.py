from typing import Any
from input import Instance

from functools import total_ordering 
import copy
import logging

@total_ordering
class Batch:
    """
    A batch is represented by an oven (assigned_oven) and a specific position in the oven (position_in_oven).
    """
    def __init__(self, assigned_oven : int, position_in_oven: int):
        self.assigned_oven = assigned_oven
        self.position_in_oven = position_in_oven
    
    def __eq__(self, other):
        return (self.assigned_oven == other.assigned_oven and  self.position_in_oven == other.position_in_oven)
    
    def __lt__(self, other):
        return (self.assigned_oven < other.assigned_oven) or (self.assigned_oven == other.assigned_oven and  self.position_in_oven < other.position_in_oven) 

class BatchCharacteristic:
    """
    Batch datastructure.
    """
    def __init__(self, start_time : int, processing_time : int, end_time: int, attribute : int, setup_cost : int, representative_job :int):
        self.start_time = start_time
        self.processing_time = processing_time
        self.end_time = end_time
        self.attribute = attribute
        self.setup_cost = setup_cost
        self.representative_job = representative_job
    
    def __repr__(self) -> str:
        return f"Batch: process from {self.start_time} to {self.end_time} ({self.processing_time}), with attribute {self.attribute}"

class Solution:
    def __init__(self, input : Instance):
        self.input = input
        self.assignement_job_to_oven = dict()
    
    def __repr__(self) -> str:
        solution = list()
        for j in self.assignement_job_to_oven.keys():
            solution.append(f" ({j},{self.assignement_job_to_oven[j].assigned_oven},{self.assignement_job_to_oven[j].position_in_oven})")
        return " ".join(solution) + f" --> {self.total_cost} ({self.total_setup_cost},{self.cumulative_process_time},{self.tardy_jobs},{self.not_scheduled_job_count})" 

    def add_assignement(self, job_code: int, assigned_oven : int, position_in_oven: int):
        """
        Given a job, we assign to the job the oven at which it is scheduled and the position
        """
        self.assignement_job_to_oven[job_code] = Batch(assigned_oven, position_in_oven)
    
    def populate_data_structure_from_scratch(self):
        """
        Redundant data structure and cost. The base of all the process is assignement_job_to_oven, from that we build the schedule and hence we are able to calculate the costs.
        """
        self.map_jobs_to_batch_and_machine()
        self.populate_batch_characteristics()
        self.calculate_total_cost()

    
    def map_jobs_to_batch_and_machine(self):
        """
        Given the list of jobs, create a dictionary (m,i) = [j_1, etc.] so that with key (m,i) reports as value the jobs processed by machine m in position i. 
        You can have a maximum of num_jobs positions.
        Also, we save the jobs per machine
        """ 
        self.jobs_to_batch = dict()
        self.jobs_to_machine = dict()
        # for every job in assignement_job_to_oven.keys()
        for j in self.assignement_job_to_oven.keys():
            if ((self.assignement_job_to_oven[j].assigned_oven, self.assignement_job_to_oven[j].position_in_oven)) in self.jobs_to_batch:
                self.jobs_to_batch[(self.assignement_job_to_oven[j].assigned_oven, self.assignement_job_to_oven[j].position_in_oven)].append(j)
            else:
                self.jobs_to_batch[(self.assignement_job_to_oven[j].assigned_oven, self.assignement_job_to_oven[j].position_in_oven)] = [j]
            if self.assignement_job_to_oven[j].assigned_oven in self.jobs_to_machine:
                self.jobs_to_machine[self.assignement_job_to_oven[j].assigned_oven].append(j)
            else:
                self.jobs_to_machine[self.assignement_job_to_oven[j].assigned_oven] = [j]

    def populate_batch_characteristics(self):
        """
        Given all the batches, calculate their characteristics
        """
        batches = sorted(self.jobs_to_batch.keys())
        self.batch_characteristics_dictionary = dict()
        self.machine_to_batch = dict()
        self.batches_per_attribute = dict()
        previous_machine = 0
        previous_position = 0
        for batch_position in batches:
            machine_id = batch_position[0]
            position = batch_position[1]
            if machine_id in self.machine_to_batch.keys():
                self.machine_to_batch[machine_id].append(batch_position)
            else:
                self.machine_to_batch[machine_id] = [batch_position]
            jobs_in_batch = self.jobs_to_batch[batch_position]
            if (len(jobs_in_batch) == 0):
                continue
            previous_batch = None
            if previous_machine == machine_id:
                previous_batch = self.batch_characteristics_dictionary[(previous_machine,previous_position)]
            self.batch_characteristics_dictionary[(machine_id, position)] = self.get_batch_properties(machine_id, position, previous_batch)
            # logging.debug(f"populate_batch_characteristics // {machine_id} - {position} -> {self.batch_characteristics_dictionary[(machine_id, position)]}")
            previous_machine = batch_position[0]
            previous_position = batch_position[1]

    def get_batch_properties(self, oven : int, position : int, previous_batch : (BatchCharacteristic or None)):
        """
        Given the coordinates of a batch and its predecessor, calculate its characteristics. If no job is there, then return None.
        """
        jobs_in_batch = self.jobs_to_batch[(oven, position)] 
        if len(jobs_in_batch) == 0:
            return None
        # determine the batch processing time
        max_min_time = self.input.jobs[jobs_in_batch[0]].min_time
        for j in jobs_in_batch:
            if max_min_time < self.input.jobs[j].min_time:
                max_min_time = self.input.jobs[j].min_time
        batch_processing_time = max_min_time 
        batch_start_time = self.calculate_batch_start_time(jobs_in_batch, previous_batch, batch_processing_time, oven)
        if batch_start_time is None:
            batch_start_time = self.input.horizon + 1
            batch_processing_time = 0
        
        batch_end_time = batch_start_time + batch_processing_time

        batch_attribute = self.input.jobs[jobs_in_batch[0]].attribute

        if batch_attribute in self.batches_per_attribute.keys():
            self.batches_per_attribute[batch_attribute].add((oven, position))
        else:
            self.batches_per_attribute[batch_attribute] = {(oven, position)}

        setup_cost_batch = 0
        if previous_batch is None:
            # setup seconds between batch attribute and initial state of the oven
            setup_cost_batch = self.input.setup_costs[self.input.machines[oven].initial_state - 1][batch_attribute - 1]
        else: 
            setup_cost_batch = self.input.setup_costs[previous_batch.attribute - 1][batch_attribute - 1]
        
        jobs_in_batch.sort()
        representative_job = jobs_in_batch[0]

        return BatchCharacteristic(batch_start_time,batch_processing_time,batch_end_time,batch_attribute,setup_cost_batch,representative_job)
    
    def calculate_batch_start_time(self, jobs : list, previous_batch : BatchCharacteristic, processing_time : int, machine : int):
        """
        Calculate the start of the corrent batch, given the list of jobs of this batch, the previous batch, the required processing time for the current batch, and oven associated with the current batch
        """
        # if the previous batch could not be scheduled, then also this one cannot be scheduled
        if previous_batch != None and previous_batch.start_time > self.input.horizon:
            return None
        
        earliest_start_batch = self.input.jobs[jobs[0]].earliest_start
        for j in jobs:
            if self.input.jobs[j].earliest_start > earliest_start_batch:
                earliest_start_batch = self.input.jobs[j].earliest_start
        
        
        previous_batch_end = 0
        setup_time_between_batches = 0
        batch_attribute = self.input.jobs[jobs[0]].attribute

        if previous_batch is None:
            # setup seconds between batch attribute and initial state of the oven
            setup_time_between_batches = self.input.setup_times[self.input.machines[machine].initial_state - 1][batch_attribute -1]
        else: 
            previous_batch_end = previous_batch.end_time
            setup_time_between_batches = self.input.setup_times[previous_batch.attribute - 1][batch_attribute -1 ]
        
        if previous_batch_end + setup_time_between_batches > earliest_start_batch:
            earliest_start_batch = previous_batch_end + setup_time_between_batches
        
        if earliest_start_batch + processing_time > self.input.horizon:
            return None

        return self.calculate_earliest_suitable_machine_interval_start(machine, earliest_start_batch, processing_time,setup_time_between_batches)

    
    def calculate_earliest_suitable_machine_interval_start(self, machine : int, earliest_start_batch : int, processing_time : int, setup_time : int):
        interval_index = -1

        earliest_start_interval = earliest_start_batch

        for s in self.input.machines[machine].shifts.keys():
            if self.input.machines[machine].shifts[s].start + setup_time > earliest_start_batch:
                earliest_start_interval = self.input.machines[machine].shifts[s].start + setup_time
            
            if (self.input.machines[machine].shifts[s].start <= (earliest_start_interval - setup_time)) and ((earliest_start_interval + processing_time) <= self.input.machines[machine].shifts[s].end):
                interval_index = s
                break
        
        if interval_index == -1:
            return None
        
        return earliest_start_interval
              
    def calculate_total_cost(self):
        self.tardy_jobs = 0
        for batch in self.batch_characteristics_dictionary.keys():
            jj = self.jobs_to_batch[batch]
            for w in jj:
                characteristics = self.batch_characteristics_dictionary[batch]
                if characteristics.start_time <= self.input.horizon:
                    if characteristics.end_time > self.input.jobs[w].latest_end:
                        self.tardy_jobs = self.tardy_jobs + 1
        
        self.total_setup_cost = 0
        self.cumulative_process_time = 0
        self.not_scheduled_job_count = 0
        for batch in self.batch_characteristics_dictionary.keys():
            self.cumulative_process_time = self.cumulative_process_time + self.batch_characteristics_dictionary[batch].processing_time
            if self.batch_characteristics_dictionary[batch].start_time <= self.input.horizon:
                self.total_setup_cost = self.total_setup_cost + self.batch_characteristics_dictionary[batch].setup_cost
            else:
                self.not_scheduled_job_count = self.not_scheduled_job_count + len(self.jobs_to_batch[batch])

        self.total_cost = (self.cumulative_process_time * self.input.weight_cumulative_process_time) + (self.tardy_jobs * self.input.weight_tardy_jobs) + (self.total_setup_cost * self.input.weight_setup_cost) + (self.not_scheduled_job_count * self.input.weight_non_scheduled_jobs)

    @property
    def get_total_cost(self) -> int:
        return self.total_cost
    
    def calculate_total_cost_per_machine(self, m : int) -> int:
        cnt_tardy = 0
        cnt_process_time = 0
        cnt_total_setup_cost = 0
        cnt_not_scheduled = 0
        for job_code in self.jobs_to_machine[m]:
            characteristics = self.batch_characteristics_dictionary[(self.assignement_job_to_oven[job_code].assigned_oven, self.assignement_job_to_oven[job_code].position_in_oven)]
            if characteristics.start_time <= self.input.horizon:
                if characteristics.end_time > self.input.jobs[job_code].latest_end:
                    cnt_tardy = cnt_tardy + 1
        for batch in self.machine_to_batch[m]:
            cnt_process_time = cnt_process_time + self.batch_characteristics_dictionary[batch].processing_time
            if self.batch_characteristics_dictionary[batch].start_time <= self.input.horizon:
                cnt_total_setup_cost = cnt_total_setup_cost + self.batch_characteristics_dictionary[batch].setup_cost
            else:
                cnt_not_scheduled = cnt_not_scheduled + len(self.jobs_to_batch[batch])
    
        return (cnt_process_time * self.input.weight_cumulative_process_time) + (cnt_tardy * self.input.weight_tardy_jobs) + (cnt_total_setup_cost * self.input.weight_setup_cost) + (cnt_not_scheduled * self.input.weight_non_scheduled_jobs)
