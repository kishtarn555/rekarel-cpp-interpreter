#!/bin/bash
set -e
echo ====== TEST PASCAL ======

ROOT="$(git rev-parse --show-toplevel)"


for problem in "${ROOT}/test_suite/problems/"*; do
	problem_name=$(basename "${problem}")

	rekarel compile "${problem}/solution.kp" -o "${ROOT}/bin/${problem_name}.kx"

    echo -n "${problem_name} .................. RUNING                      "
    start_time=$(date +%s%N)

	for casename in "${problem}/cases"/*.in; do
        case_name=$(basename "${casename}")

		{
            ${ROOT}/bin/karel "${ROOT}/bin/${problem_name}.kx" < "${casename}" | diff -Naurw --ignore-blank-lines "${casename%.in}.out" -
        } &
	done
    wait
    # End the timer
    end_time=$(date +%s%N)
    
    # Calculate the elapsed time
    elapsed_time=$(( (end_time - start_time) / 1000000 ))
    echo -ne "\r${problem_name} .................. DONE ${elapsed_time} ms                               \n"
done