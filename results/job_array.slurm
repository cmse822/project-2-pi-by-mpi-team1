#!/bin/bash 
#SBATCH --array=0-2
#SBATCH --time=00:05:00
#SBATCH --nodes=16
#SBATCH --ntasks-per-node=1
#SBATCH --job-name=pi_mc
#SBATCH --mail-user=dalfaver@msu.edu
#SBATCH --output=job_array_%A_%a.out
#SBATCH --error=job_array_%A_%a.err

num_darts=(1e3 1e6 1e9)
tmpfile=$(mktemp)
cd ~/project-2-pi-by-mpi-team1
pi_val=$(mpiexec -n 2 /usr/bin/time -f "%e" -o $tmpfile ./pi 200 ${num_darts[$SLURM_ARRAY_TASK_ID]})
echo "${SLURM_NNODES},${num_darts[$SLURM_ARRAY_TASK_ID]},$(cat $tmpfile),${pi_val}"
