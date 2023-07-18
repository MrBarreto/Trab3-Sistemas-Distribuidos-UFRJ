#!/bin/bash

num_executions=64 
for ((i=1; i<=$num_executions; i++))
do
    ./agente.out $i 0 3 &  
done

wait  