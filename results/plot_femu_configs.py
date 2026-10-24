import os
import re
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from matplotlib.font_manager import FontProperties
prop = FontProperties(size=42)


def extract_times_from_log(file_path):
    """Extract 'Total Requests Time' and 'For loop time' from a log file, and raise an error if 'For loop time' is not found."""
    with open(file_path, 'r') as file:
        content = file.read()

    # Extract 'For loop time' in microseconds
    loop_time_match = re.search(r'For loop time:\s+(\d+)\s+microseconds', content)
    if not loop_time_match:
        raise ValueError(f"'For loop time' not found in file: {file_path}")
    loop_time = int(loop_time_match.group(1))

    # Extract 'Total Requests Time' in microseconds
    total_time_match = re.search(r'Total requests time:\s+(\d+)\s+microseconds', content)
    if not total_time_match:
        raise ValueError(f"'Total Requests Time' not found in file: {file_path}")
    total_time = int(total_time_match.group(1))

    return total_time, loop_time

def collect_data(directory):
    """Collect 'Total Requests Time' and 'Total [POLICY] runtime' data from all log files in the given directory."""
    data = {}
    times = []
    # sort os.listdir to make sure it follows alphabet orders
    for log_file in sorted(os.listdir(directory)):
        if log_file.endswith('.log'):
            log_file_path = os.path.join(directory, log_file)
            total_time, loop_time = extract_times_from_log(log_file_path)
            times.append((total_time, loop_time))  # Store as tuple
    # Use the directory name as the key
    setting_name = os.path.basename(directory)
    data[setting_name] = times
    return data

def format_title(setting):
    """Format the title based on the setting string."""
    tier1, tier2 = map(int, setting.split('-'))
    tier1_percent = tier1 // 100
    tier2_percent = tier2 // 100
    return f'Tier1 {tier1_percent}%, Tier2 {tier2_percent}%'

def plot_data(*data_list):
    """Plot the collected data for multiple workloads with enhanced colors and stacked bars."""
    # Since we now have a simpler structure, just get the first (and only) key from each data dict
    settings = [list(data.keys()) for data in data_list]
    print(settings)
    num_logs = len(data_list[0][list(data_list[0].keys())[0]])  # Get number of logs from first data entry
    # print(num_logs)
    # log_labels = ['EXD','LFU','LRFU','LRU','RLs2','TEMP','ideal']
    # new order of log_labels
    log_labels = ['tLFU','tLRU','LRFU','EXD','Logi','XGB','Oracle','TEMP','ReStore']
    num_logs = len(log_labels)
    # mapping from origin log orders to new orders
    labels = [1,3,2,0,4,7,8,6,5]
    percentage_list = [0]*len(log_labels)
    workload_labels = ['config1','config2','config3','default']

    fig, axs = plt.subplots(nrows=1, ncols=1, figsize=(20, 8))
    # axs = axs.flatten()

    # Use seaborn color palette for better colors
    # colors = sns.color_palette("Set2")
    # # Define the darker colors by darkening the original colors
    # darker_colors = [sns.set_hls_values(c, l=0.4) for c in colors]
    colors = ['gainsboro','gainsboro','gray','gray','darkseagreen','steelblue','lightslategray','salmon','black']
    # use patterns to reduce usage of colors
    patterns = ['\\', '.', '/', 'o', '', '', '', '', '']  # Hatching patterns
    # Define the darker colors by darkening the original colors
    darker_colors = [sns.set_hls_values(c, l=0.3) for c in colors]

    bar_width = 0.85 / num_logs  # Adjust the bar width based on the number of logs
    x = np.arange(len(workload_labels))  # Positions for the workloads on the x-axis

    for idx, setting in enumerate(settings):
        ax = axs#[idx]

        for i, data in enumerate(data_list):
            # Get the first (and only) key from the data dictionary
            key = list(data.keys())[0]
            times_total = [t[0] for t in data[key]]  # Total Requests Time
            times_loop = [t[1] for t in data[key]]   # For loop time
            print('length of times_total: ',len(times_total))

            for j in range(num_logs):
                # Plot the for loop time (darker color)
                ax.bar(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                       times_loop[labels[j]],
                       width=bar_width, 
                       color=darker_colors[j],
                       # hatch=patterns[j],  # Apply hatching patterns
                       edgecolor='black')

                # Plot the Total Requests Time minus the for loop time (lighter color)
                ax.bar(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                       (times_total[labels[j]] - times_loop[labels[j]]),
                       width=bar_width, 
                       bottom=times_loop[labels[j]],
                       color=colors[j], 
                       hatch=patterns[j],  # Apply hatching patterns
                       edgecolor='black',
                       label=log_labels[j] if i == 0 and idx == 0 else "")
                
                # record the percentage
                percentage_list[labels[j]] += times_total[labels[j]]/times_total[8]

        ax.axhline(y=1, xmin=0, xmax=0.2+len(workload_labels), linestyle='--', color='gray')
        # Set the labels and title
        # ax.set_title('Scalability tests', fontsize=40)  # Set the title with the formatted setting
        # ax.set_xlabel('Configurations', fontsize=40)
        ax.set_ylabel('Runtime (microseconds)', fontsize=40)
        ax.set_ylim(0, 450*1e6)
        ax.set_yticks([0,0.8*1e8,1.6*1e8,2.4*1e8,3.2*1e8,4.0*1e8])
        ax.set_xticks(x)
        ax.set_xticklabels(workload_labels, fontsize=32)
        # if idx == 0:  # Only add the legend to the first subplot
        ax.legend(fontsize=30, loc='upper left', ncol=5)
        ax.tick_params(axis='y', labelsize=32)
        ax.yaxis.offsetText.set_fontsize(32)

    plt.tight_layout()
    plt.savefig('Results_femu_4config.pdf',format='pdf',dpi=600, bbox_inches='tight')
    # plt.show()

    percentage_list = [x/(len(workload_labels)*len(settings)) for x in percentage_list]
    return percentage_list

# Main execution
if __name__ == "__main__":
    # Directories for each workload
    workload1_dir = 'config1/Results_MSR_wdev_0/capacity_tests_7-200-500/5000-15000'
    workload2_dir = 'config2/Results_MSR_wdev_0/capacity_tests_30-200-500/5000-15000'
    workload3_dir = 'config3/Results_MSR_wdev_0/capacity_tests_7-200-500/5000-15000'
    workload4_dir = 'default/Results_MSR_wdev_0/capacity_tests_30-200-500/5000-15000'
    
    # Collect data
    data1 = collect_data(workload1_dir)
    # print(len(data1))
    data2 = collect_data(workload2_dir)
    # print(len(data2))
    data3 = collect_data(workload3_dir)
    # print(len(data3))
    data4 = collect_data(workload4_dir)
    # print(len(data3))
    # data5 = collect_data(workload5_dir)
    # # print(len(data3))
    # data6 = collect_data(workload6_dir)
    # # print(len(data3))
    # data7 = collect_data(workload7_dir)
    # print(len(data3))


    # Plot the data
    percentage_list = plot_data(data1, data2, data3, data4)#, data5, data6, data7)
    policy_list = ['tLFU','tLRU','LRFU','EXD','Logi','XGB','Oracle','TEMP','ReStore']
    for i in range(len(policy_list)):
        print('Average percentage of', policy_list[i], 'over ideal: ', percentage_list[i])
