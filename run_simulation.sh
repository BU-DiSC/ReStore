#!/bin/bash

# Define the list of executable files
EXECUTABLES=("Run_static" "Run_ideal" "Run_tLFU" "Run_tLRU" "Run_LRFU" "Run_EXD" "Run_RL" "Run_TEMP")
            #"Run_LogiReg" "Run_XGBfast")

# Define the list of argument groups
ARG_GROUPS=(
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=2000"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=2000"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=2000"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=2000"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=2000"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=2000"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=2000"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=2000"
    # "-workload=5hf90_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=2000"

    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=10hf90_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"

    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=10hf80_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"

    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=20hf80_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"

    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90+10hf80_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"

    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90+10hf80+20hf80_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"

    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*10_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"

    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*20_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"

    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*50_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"

    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=100 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=400 -read_time_tier3=500"
    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=300 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=800 -read_time_tier3=500"
    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=600 -max_capacity_tier2=1600 -read_time_tier3=500"
    # "-workload=5hf90*100_1e4_rw1_1e6 -max_capacity_tier1=1200 -max_capacity_tier2=1600 -read_time_tier3=500"
    
    # Add more argument groups as needed
)



# Loop through each group of arguments
for args in "${ARG_GROUPS[@]}"; do
    # Loop through each executable
    for exe in "${EXECUTABLES[@]}"; do
        if [[ -x "$exe" ]]; then
            echo "Running $exe with arguments $args"

            # Extract workload and capacity values for log file naming
            workload=$(echo "$args" | grep -oP '(?<=-workload=)[^ ]+')
            max_capacity_tier1=$(echo "$args" | grep -oP '(?<=-max_capacity_tier1=)[^ ]+')
            max_capacity_tier2=$(echo "$args" | grep -oP '(?<=-max_capacity_tier2=)[^ ]+')
            read_time_tier1=$(echo "$args" | grep -oP '(?<=-read_time_tier1=)[^ ]+')
            read_time_tier2=$(echo "$args" | grep -oP '(?<=-read_time_tier2=)[^ ]+')
            read_time_tier3=$(echo "$args" | grep -oP '(?<=-read_time_tier3=)[^ ]+')

            log_dir="Results_${workload}/capacity_tests_${read_time_tier1}-${read_time_tier2}-${read_time_tier3}/${max_capacity_tier1}-${max_capacity_tier2}"
            log_file="${log_dir}/output_${workload}_$(echo $exe | sed 's/Run_//').log"

            # Create the log directory if it doesn't exist
            mkdir -p "$log_dir"

            # Run the executable in the background
            ./$exe $args &
            # record the system usage info
            # /usr/bin/time -v ./$exe $args >> "Simulations_system_usage.log" 2>&1 &

            # Get the PID of the background process
            pid=$!

            # Monitor the log file
            sleep 1
            while true; do
                if [[ -f "$log_file" && -s "$log_file" ]]; then
                    echo "Log file $log_file detected and is not empty. Terminating $exe."
                    kill -9 $pid
                    break
                fi
                sleep 1
            done

        else
            echo "Executable $exe not found or not executable."
        fi
    done
done


# # list of train_percentage values, only used for XGB&LogiReg
# TRAIN_PERCENTAGES=(1 2 5)
# # Loop through each train_percentage value first (optional)
# for per in "${TRAIN_PERCENTAGES[@]}"; do
#     # Loop through each group of arguments
#     for args in "${ARG_GROUPS[@]}"; do
#         # Loop through each executable
#         for exe in "${EXECUTABLES[@]}"; do
    
#             full_args="$args -train_percentage=$per"

#             if [[ -x "$exe" ]]; then
#                 echo -e "\nRunning $exe with arguments $full_args"

#                 # Extract workload and capacity values for log file naming
#                 workload=$(echo "$args" | grep -oP '(?<=-workload=)[^ ]+')
#                 max_capacity_tier1=$(echo "$args" | grep -oP '(?<=-max_capacity_tier1=)[^ ]+')
#                 max_capacity_tier2=$(echo "$args" | grep -oP '(?<=-max_capacity_tier2=)[^ ]+')
#                 read_time_tier1=$(echo "$args" | grep -oP '(?<=-read_time_tier1=)[^ ]+')
#                 read_time_tier2=$(echo "$args" | grep -oP '(?<=-read_time_tier2=)[^ ]+')
#                 read_time_tier3=$(echo "$args" | grep -oP '(?<=-read_time_tier3=)[^ ]+')

#                 log_dir="Results_${workload}/capacity_tests_${read_time_tier1}-${read_time_tier2}-${read_time_tier3}/${max_capacity_tier1}-${max_capacity_tier2}"
#                 # log_file="${log_dir}/output_${workload}_$(echo $exe | sed 's/Run_//').log"
#                 ## use this log file name for XGB
#                 log_file="${log_dir}/output_${workload}_$(echo $exe | sed 's/Run_//')_${per}%.log" 

#                 # Create the log directory if it doesn't exist
#                 mkdir -p "$log_dir"

#                 # Check if the log file exists and is not empty
#                 if [[ -f "$log_file" && -s "$log_file" ]]; then
#                     echo "Log file $log_file already exists and is not empty. Skipping execution of $exe."
#                 else
#                     echo "🚀 Running $exe since log file is missing or empty."

#                     # Run the executable in the background
#                     ./$exe $full_args &
#                     # record the system usage info
#                     # /usr/bin/time -v ./$exe $args >> "Simulations_system_usage.log" 2>&1 &

#                     # Get the PID of the background process
#                     pid=$!

#                     # Monitor the log file
#                     sleep 1
#                     while true; do
#                         if [[ -f "$log_file" && -s "$log_file" ]]; then
#                             echo "Log file $log_file detected and is not empty. Terminating $exe."
#                             kill -9 $pid
#                             break
#                         fi
#                         sleep 1
#                     done
#                 fi
#             else
#                 echo "Executable $exe not found or not executable."
#             fi
#         done
#     done
# done