from pymzn import dzn2dict
from logger import log

class Job:
    """Characteristics of a job are its id (from 1 to n), its eligible machines (eligible_machines), the earliestt start (earliest_start), the minimuma and maximum treatment time (min_time, max_time), its size (size), and its attribute (attribute)"""
    def __init__(self, code : int, eligbile_machines : dict, earliest_start : int, latest_end : int, min_time : int, max_time : int, size : int, attribute : int):
        self.code = code
        self.eligible_machines = eligbile_machines
        self.earliest_start = earliest_start
        self.latest_end = latest_end
        self.min_time = min_time
        self.max_time = max_time
        self.size = size
        self.attribute = attribute

    def __repr__(self) -> str:
        return f"Job {self.code} -> machines: {self.eligible_machines}, early_start: {self.earliest_start}, latest_end: {self.latest_end}, min_time: {self.min_time}, max_time: {self.max_time}, size: {self.size}, attribute: {self.attribute}"

class Shift:
    """Characteristics of a shift are: its start and its end"""

    def __init__(self,start : int, end : int):
        self.start = start
        self.end = end

    def __repr__(self) -> str:
        return f"{self.start}-{self.end}"

class Machine:
    """Characteristics of a machine are: its id (from 1 to m), the capacity (max_capacity), the initial state (init_state), the shifts (shifts)"""

    def __init__(self, code : int, initial_state : int, max_capacity : int):
        self.code = code
        self.initial_state = initial_state
        self.max_capacity = max_capacity
        self.shifts = dict()
    
    def __repr__(self) -> str:
        return f"Machine {self.code} -> initial_state: {self.initial_state}, max_capacity: {self.max_capacity}, shifts: {self.shifts}"

    def add_shift(self, shift : int,start : int, end : int):
        self.shifts[shift] = Shift(start,end)

class Instance:
    def __init__(self, file_name : str):
        self.complete_file_name = file_name
        self.name = file_name.split("/")[-1]
        
        if ".dzn" in file_name:
            self.parse_mzn(file_name)
        else:
            raise ValueError
    
    def __repr__(self) -> str:
        return f"{self.name}"
    
    def parse_mzn(self, file_name : str):
        log.debug(f"Reading instance - {self.name}")

        self.data = dzn2dict(file_name)

        self.horizon = self.data["l"]
        self.n_shifts = self.data["s"]
        self.n_attributes = self.data["a"]
        
        self.n_machines = self.data["m"]
        self.machines = dict() 
        # TODO: add checker - everything that regard the machine should have length == n_machines
        for i in range(0, self.n_machines):
            self.machines[i+1] = Machine(i+1, self.data["initState"][i], self.data["max_cap"][i])
        # TODO: len(data["m_a_s"] == len(data["m_a_e"] == machines * shifts
        # to each machine add its shift 
            # here you have to pay attention, cause in the minizinc format this is a list... 
        for i in range(0,len(self.data["m_a_s"])):
            m = 1 + ( (i // self.n_shifts) % self.n_machines)
            s = 1 + (i % self.n_shifts)
            self.machines[m].add_shift(s, self.data["m_a_s"][i],self.data["m_a_e"][i])

        self.n_jobs = self.data["n"]
        self.jobs = dict()
        # TODO: add checker - everything that regard the machine should have length == n_machines
        for i in range(0, self.n_jobs):
            job_id = i + 1
            self.jobs[job_id]= Job(job_id, self.data["eligible_machine"][i], self.data["earliest_start"][i], self.data["latest_end"][i], self.data["min_time"][i], self.data["max_time"][i], self.data["size"][i], self.data["attribute"][i])

        
        # setup cost and times are given in a matrix
        self.setup_costs = [[0] * self.n_attributes for _ in range(self.n_attributes)]
        self.setup_times = [[0] * self.n_attributes for _ in range(self.n_attributes)]
        index = 0
        for i in range(0,self.n_attributes):
            for j in range(0,self.n_attributes):
                self.setup_costs[i][j] = self.data["setup_costs"][index]
                self.setup_times[i][j] = self.data["setup_times"][index]
                index += 1

        # bounds and weights
        self.weight_tardy_jobs = self.data["mult_factor_finished_toolate"]
        self.weight_cumulative_process_time = self.data["mult_factor_total_runtime"]
        self.weight_setup_cost = self.data["mult_factor_total_setupcosts"]
        self.weight_non_scheduled_jobs = 2*(self.weight_tardy_jobs + self.weight_cumulative_process_time + self.weight_setup_cost)       

    def compute_features(self):
        pass


