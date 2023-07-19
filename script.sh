#!/bin/bash

num_executions=2 
for ((i=1; i<=$num_executions; i++))
do
    ./agente.out $i 2 10 &  
done

wait  