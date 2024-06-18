from input import Instance
from large_neighborhood_search import LNS

import click
import logging
import random

logger = logging.getLogger(__name__)
# logging.basicConfig(level=logging.INFO, format='%(asctime)-15s [%(process)d:%(processName)s] %(module)s:%(lineno)d %(levelname)s: %(message)s')

def extract_number(instance_name):
    instance_name = instance_name.split("/")[-1]
    if "NewRandomOvenSchedulingInstance" in instance_name:
        return int(instance_name.split("NewRandom")[0])
    return int(instance_name.split("Random")[0])

@click.command()
@click.option("--instance", "-i", type=str, required=True, help="instance of the problem.")
@click.option("--timeout", "-t", type=int, required=True, help="timeout for the lns.")
@click.option("--random_seed", "-s", type=int, default=42, help="random seed for replicability. Default 42.")
@click.option("--verbose", "-v", type=bool, default=False, help="whether or not you want to log lns exectuion. Default false")
@click.option("--architecture", "-a", type=str, default="lagiovanna", help="server you are using")
@click.option("--folder", "-f", type=str, help="folder where you want to store the log file")
@click.option("--repair_5", "-r", type=float, required=True, help="probability of selecting repair operator with 0.05 gap")
@click.option("--k_batches", type=float, required=True, help="probability of selecting destroy operator as destroy random batches")
@click.option("--k_batches_attribute", type=float, required=True, help="probability of selecting destroy operator as destroy random batches with the same attribute")
@click.option("--k_batches_same_machine", type=float, required=True, help="probability of selecting destroy operator as destroy random batchesin the same machine")
@click.option("--tuning", type=bool, default=False, help="tuning")
def calling_lns(instance, timeout, random_seed, verbose, architecture, folder, repair_5, k_batches, k_batches_attribute,k_batches_same_machine,tuning):
    random.seed(random_seed)

    ins_code_for_logger = extract_number(instance_name=instance)


    if verbose: 
        logging.basicConfig(level=logging.DEBUG, format='%(asctime)-15s; %(message)s')
    else:
        if tuning:
            logging.basicConfig(level=logging.WARNING, format='%(asctime)-15s; %(message)s')
        elif folder is None:
            logging.basicConfig(level=logging.INFO, format='%(asctime)-15s; %(message)s')
        else:
            logging.basicConfig(filename = f"{folder}/results_{ins_code_for_logger}_{timeout}_{random_seed}.txt", filemode="w",level=logging.INFO, format='%(asctime)-15s; %(message)s')
        
    input = Instance(instance)
    # lns = LNS(random_seed,input,"greedy",2,5,architecture) # runner 7
    # lns.solve_simple(timeout) # runner 7
    lns = LNS(random_seed,input,"greedy",2,7,architecture,repair_5, k_batches, k_batches_attribute,k_batches_same_machine) # runner 10
    lns.solve_simple_n_max(timeout)  # runner 10

    # TODO: add k to the command line

if __name__ == "__main__":
    calling_lns()
