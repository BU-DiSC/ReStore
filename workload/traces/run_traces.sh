#!/bin/bash

# Define the list of executable files
EXECUTABLES=("Run_static" "Run_ideal" "Run_tLFU" "Run_tLRU" "Run_LRFU" "Run_EXD" "Run_RL" "Run_TEMP")
            #"Run_LogiReg" "Run_XGBfast")

# Define the list of argument groups
ARG_GROUPS=(
    "-workload=MSR_hm_1 
    -max_capacity_tier1=5000 -max_capacity_tier2=15000 -max_capacity_tier3=60000
    -read_time_tier1=38 -read_time_tier2=206 -read_time_tier3=514
    -asym_tier1=1.53 -asym_tier2=2.05 -asym_tier3=4.11
    -total_num_pages=51733 -total_num_reqs=2308560 
    -temp_incr_alpha=0.1 -temp_incr_buffersize=50000 -temp_drop_thrd=10000 -temp_drop_freqs=20000
    -RL_update_freqs=500 -RL_init_rounds=100
    -a_b_update_freq_s1=5 -num_elements_to_consider_s1=1000 
    -a_b_update_freq_s2=5 -num_elements_to_consider_s2=1000"
    # -num_threads_tier1=1 -num_threads_tier2=1 -num_threads_tier3=1

    # "-workload=MSR_wdev_0 
    # -max_capacity_tier1=5000 -max_capacity_tier2=15000 -max_capacity_tier3=60000
    # -read_time_tier1=38 -read_time_tier2=206 -read_time_tier3=514
    # -asym_tier1=1.53 -asym_tier2=2.05 -asym_tier3=4.11
    # -num_threads_tier1=1 -num_threads_tier2=1 -num_threads_tier3=1
    # -total_num_pages=79677 -total_num_reqs=695995 
    # -temp_incr_alpha=0.1 -temp_incr_buffersize=50000 -temp_drop_thrd=10000 -temp_drop_freqs=20000
    # -RL_update_freqs=500 -RL_init_rounds=100
    # -a_b_update_freq_s1=5 -num_elements_to_consider_s1=1000 
    # -a_b_update_freq_s2=5 -num_elements_to_consider_s2=1000"

    "-workload=TPCC 
    -max_capacity_tier1=2000000 -max_capacity_tier2=8000000 -max_capacity_tier3=20000000
    -total_num_pages=16842331 -total_num_reqs=62925641 
    -temp_incr_alpha=0.2 -temp_incr_buffersize=10000000 -temp_drop_thrd=1000000 -temp_drop_freqs=2000000
    -RL_update_freqs=1000 -RL_init_rounds=100
    -a_b_update_freq_s1=10 -num_elements_to_consider_s1=1000 
    -a_b_update_freq_s2=10 -num_elements_to_consider_s2=2000"

    "-workload=TPCE 
    -max_capacity_tier1=2000000 -max_capacity_tier2=8000000 -max_capacity_tier3=20000000
    -total_num_pages=19274163 -total_num_reqs=31884418 
    -temp_incr_alpha=0.2 -temp_incr_buffersize=10000000 -temp_drop_thrd=1000000 -temp_drop_freqs=2000000
    -RL_update_freqs=1000 -RL_init_rounds=100
    -a_b_update_freq_s1=10 -num_elements_to_consider_s1=1000 
    -a_b_update_freq_s2=10 -num_elements_to_consider_s2=2000"

    "-workload=thesios 
    -max_capacity_tier1=1500 -max_capacity_tier2=5000 -max_capacity_tier3=20000 
    -total_num_pages=16417 -total_num_reqs=95970071
    -temp_incr_alpha=0.05 -temp_incr_buffersize=10000 -temp_drop_thrd=5000 -temp_drop_freqs=10000
    -RL_update_freqs=500 -RL_init_rounds=100
    -a_b_update_freq_s1=5 -num_elements_to_consider_s1=500 
    -a_b_update_freq_s2=5 -num_elements_to_consider_s2=1000"

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