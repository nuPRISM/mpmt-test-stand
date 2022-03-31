#!/bin/bash


echo "Starting repetition of 10 loops with gantry"


for i in {1..20}
do

    # Move X minus
    echo 'starting X minus'
    odbedit -c "set /Equipment/Motors00/Settings/Destination[1] -40000"
    odbedit -c "set /Equipment/Motors00/Settings/Move[1] y"
    sleep 50
    
    # Move Y minus
    echo 'starting Y minus'
    odbedit -c "set /Equipment/Motors00/Settings/Destination[2] -40000"
    odbedit -c "set /Equipment/Motors00/Settings/Move[2] y"
    sleep 50
    
    # Move X plus
    echo 'start X plus'
    odbedit -c "set /Equipment/Motors00/Settings/Destination[1] 40000"
    odbedit -c "set /Equipment/Motors00/Settings/Move[1] y"
    sleep 50

    
    # Move Y plus
    echo 'start Y plus'
    odbedit -c "set /Equipment/Motors00/Settings/Destination[2] 40000"
    odbedit -c "set /Equipment/Motors00/Settings/Move[2] y"
    sleep 50
    
done






