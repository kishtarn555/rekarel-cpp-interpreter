#!/bin/bash
echo ====== TEST RTE ======

ROOT="$(git rev-parse --show-toplevel)"

for problem in "${ROOT}/test_suite/rte/"*; do
    problem_name=$(basename "${problem}")

    rekarel compile "${problem}/code.kcode" -o "${ROOT}/bin/${problem_name}.kx"

    echo "...... ${problem_name} ......"
    start_time=$(date +%s%N)
    for casename in "${problem}/cases"/*.in; do
        case_name=$(basename "${casename}")

        echo ">>> CASE: ${case_name} <<<"
        
        expected_stdout="${casename%.in}.out"
        expected_stderr="${casename%.in}.stderr"
        expected_signal="${casename%.in}.signal"

        # Temporary files to capture the outputs
        actual_stdout=$(mktemp)
        actual_stderr=$(mktemp)

        # Run the command and capture stdout, stderr, and the exit status
        ${ROOT}/bin/karel "${ROOT}"/bin/${problem_name}.kx <"$casename" >"$actual_stdout" 2>"$actual_stderr"
        exit_code=$?

        # Compare stdout
        if ! diff -Naurw --ignore-blank-lines "$expected_stdout" "$actual_stdout"; then
            echo "> STDOUT does not match for $case_name !!!!"
        fi
        echo "------------------------------------"

        # Compare stderr
        if ! diff -Naurw --ignore-blank-lines "$expected_stderr" "$actual_stderr"; then
            echo "> STDERR does not match for $case_name !!!!"
        fi
        echo "------------------------------------"

        # Compare the exit signal
        expected_exit_code=$(cat "$expected_signal")
        if [ "$exit_code" -ne "$expected_exit_code" ]; then
            echo "> Exit signal does not match for $case_name. Expected $expected_exit_code, got $exit_code !!!!"
        fi
        echo "------------------------------------"

        # Clean up temporary files
        rm "$actual_stdout" "$actual_stderr"
        
    done
    wait
    # End the timer
    end_time=$(date +%s%N)

    # Calculate the elapsed time
    elapsed_time=$(( (end_time - start_time) / 1000000 ))
    echo  "${problem_name} .................. DONE ${elapsed_time} seconds"
    echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
    echo ""
done