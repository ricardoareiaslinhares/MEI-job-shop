# This code was used to generate a 5x5 matrix
# Install library from https://github.com/Pabloo22/job_shop_lib

import argparse
from job_shop_lib.generation import GeneralInstanceGenerator

parser = argparse.ArgumentParser(description="Generate Job Shop Scheduling instance.")
parser.add_argument("output_file", help="Path to save the generated instance")
args = parser.parse_args()

generator = GeneralInstanceGenerator(
    duration_range=(1, 9), seed=42, num_jobs=4, num_machines=4
)

random_instance = generator.generate()

with open(args.output_file, "w") as f:
    f.write(f"{random_instance.num_jobs} {random_instance.num_machines}\n")
    for job in random_instance.jobs:
        line = " ".join(f"{task.machine_id} {task.duration}" for task in job)
        f.write(line + "\n")
