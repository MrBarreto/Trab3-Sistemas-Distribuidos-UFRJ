#!/bin/bash

num_executions=64 # Number of times to execute the program
for ((i=1; i<=$num_executions; i++))
do
    ./agente.out $i 0 3 &  # Execute agent.out in the background
done

wait  # Wait for all background processes to finish