#!/bin/bash

num_executions=128

for ((i=1; i<=$num_executions; i++))
do
    ./agente.out $i 0 3 &  
done

wait  