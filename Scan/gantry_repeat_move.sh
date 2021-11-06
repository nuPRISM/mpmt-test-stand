#!/bin/bash



for i in {1..10}
do

    # Move X minus
    echo 'starting X minus'
    odbedit -c "set /Equipment/Motors00/Settings/Destination[1] -20000"
    odbedit -c "set /Equipment/Motors00/Settings/Move[1] y"
    sleep 10
    odbedit -c "set /Equipment/Motors00/Settings/Move[1] n"
    
    # Move Y minus
    echo 'starting Y minus'
    odbedit -c "set /Equipment/Motors00/Settings/Destination[2] -20000"
    odbedit -c "set /Equipment/Motors00/Settings/Move[2] y"
    sleep 10
    odbedit -c "set /Equipment/Motors00/Settings/Move[2] n"
    
    # Move X plus
    echo 'start X plus'
    odbedit -c "set /Equipment/Motors00/Settings/Destination[1] 20000"
    odbedit -c "set /Equipment/Motors00/Settings/Move[1] y"
    sleep 10
    odbedit -c "set /Equipment/Motors00/Settings/Move[1] n"
    
    # Move Y plus
    echo 'start Y plus'
    odbedit -c "set /Equipment/Motors00/Settings/Destination[2] 20000"
    odbedit -c "set /Equipment/Motors00/Settings/Move[2] y"
    sleep 10
    odbedit -c "set /Equipment/Motors00/Settings/Move[2] n"
    
done






