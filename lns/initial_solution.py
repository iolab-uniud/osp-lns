from solution import Solution

import tempfile as tf
import subprocess
import json
import logging
import random

def greedy_solution(filename : str, initial_solution : Solution, seed : int):
    """
    Given the filename of the input (complete with the path where the file is located), the function calls the C++ code that calculate the greedy solution as per Lackner et al. (2023).
    The function relies on subprocess and temporary files.
    """
    logging.debug(f"Start greedy procedure.")
    log_file = tf.NamedTemporaryFile(prefix=f'initial_solution_{initial_solution.input.name}_{seed}', suffix='.json')
    try:
        cmd = [
            "./bin/osp",
            "--main::instance", filename,
            "--main::greedy_solution", str(1),
            "--main::seed", str(42),
            "--main::output_file", log_file.name,
        ] 
        result = subprocess.run(cmd, capture_output=False, text=True)
        with open(log_file.name, 'r') as file:
        # Read the file content and remove trailing commas
            content = file.read().replace(",]","]")
        # Parse the modified content
        try:
            data = json.loads(content)
        except json.JSONDecodeError as e:
            print(f"Error parsing JSON: {e}")
            return None
    finally:
        log_file.close()
    # given the dictionary "data", populate the solution data structure
    job_position = data["solution"]["batch_for_job"] # this is a list, that, for every job, reports its position in the machine
    job_oven = data["solution"]["machine_for_job"] # this is a list that, for every job, reports the oven where it is processed
    for j in range(0, len(job_position)):
        job_code = j + 1
        initial_solution.add_assignement(job_code,job_oven[j],job_position[j])
    
    logging.debug(f"End greedy procedure.")
    
