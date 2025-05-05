import os
import re
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from matplotlib.font_manager import FontProperties
# Provide the full path to the font file
font_path = '/Users/tiazh991/Library/Fonts/LinLibertine_R.otf'
prop = FontProperties(fname=font_path, size=42)


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
    """Collect 'Total Requests Time' and 'For loop time' data from all log files in the given directory."""
    data = {}
    for setting in os.listdir(directory):
        setting_dir = os.path.join(directory, setting)
        # Skip hidden directories (those that start with '.') and non-directories
        if setting.startswith('.') or not os.path.isdir(setting_dir):
            continue
        times = []
        # sort os.listdir to make sure it follows alphabet orders
        for log_file in sorted(os.listdir(setting_dir)):
            if log_file.endswith('.log'):
                log_file_path = os.path.join(setting_dir, log_file)
                total_time, loop_time = extract_times_from_log(log_file_path)
                times.append((total_time, loop_time))  # Store as tuple
        data[setting] = times
    return data

def format_title(setting):
    """Format the title based on the setting string."""
    tier1, tier2 = map(int, setting.split('-'))
    tier1_percent = tier1 // 100
    tier2_percent = tier2 // 100
    return f'Tier1 {tier1_percent}%, Tier2 {tier2_percent}%'

def plot_data(*data_list):
    """Plot the collected data for multiple workloads with enhanced colors and stacked bars."""
    settings = [sorted(data_list[i].keys(), key=lambda s: tuple(map(int, s.split('-')))) for i in range(len(data_list))]
    print(settings)
    num_logs = len(next(iter(data_list[0].values())))  # Assuming all workloads have the same number of logs
    # print(num_logs)
    # log_labels = ['EXD','LFU','LRFU','LRU','RLs2','TEMP','ideal']
    # new order of log_labels
    log_labels = ['tLFU','tLRU','LRFU','EXD','Logi','XGB','Oracle','TEMP','ReStore']
    num_logs = len(log_labels)
    # mapping from origin log orders to new orders
    labels = [1,3,2,0,4,7,8,6,5]
    percentage_list = [0]*len(log_labels)
    workload_labels = ['1e4-1e6','1e5-1e7','1e6-1e8','1e7-1e8']

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
            times_total = [t[0] for t in data[settings[i][0]]]  # Total Requests Time
            times_loop = [t[1] for t in data[settings[i][0]]]   # For loop time
            print('length of times_total: ',len(times_total))

            for j in range(num_logs):
                # Plot the For loop time (darker color)
                ax.bar(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                       times_loop[labels[j]]/times_total[8], 
                       width=bar_width, 
                       color=darker_colors[j],
                       # hatch=patterns[j],  # Apply hatching patterns
                       edgecolor='black')

                # Plot the Total Requests Time minus the For loop time (lighter color)
                ax.bar(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                       (times_total[labels[j]] - times_loop[labels[j]])/times_total[8], 
                       width=bar_width, 
                       bottom=times_loop[labels[j]]/times_total[8], 
                       color=colors[j], 
                       hatch=patterns[j],  # Apply hatching patterns
                       edgecolor='black',
                       label=log_labels[j] if i == 0 and idx == 0 else "")
                
                # record the percentage
                percentage_list[labels[j]] += times_total[labels[j]]/times_total[6]

        ax.axhline(y=1, xmin=0, xmax=0.2+len(workload_labels), linestyle='--', color='gray')
        # Set the labels and title
        # ax.set_title('Scalability tests', fontproperties=prop)  # Set the title with the formatted setting
        ax.set_xlabel('#pages-#operations', fontproperties=prop)
        ax.set_ylabel('Norm. Latency vs Oracle', fontproperties=prop)
        ax.set_ylim(0, 1.7)
        ax.set_yticks([0,0.5,1.0,1.5])
        ax.set_xticks(x)
        ax.set_xticklabels(workload_labels, fontproperties=prop)
        # if idx == 0:  # Only add the legend to the first subplot
        #     ax.legend(fontsize=30, loc='upper left', ncol=4)
        ax.tick_params(axis='y', labelsize=32)

    plt.tight_layout()
    plt.savefig('Results_scalability_tests_5hf90_new.pdf',format='pdf',dpi=600, bbox_inches='tight')
    # plt.show()

    percentage_list = [x/(len(workload_labels)*len(settings)) for x in percentage_list]
    return percentage_list

# Main execution
if __name__ == "__main__":
    # Directories for each workload
    workload1_dir = 'Results_5hf90_1e4_rw1_1e6/capacity_tests_30-200-500'
    workload2_dir = 'Results_5hf90_1e5_rw1_1e7/capacity_tests_30-200-500'
    workload3_dir = 'Results_5hf90_1e6_rw1_1e8/capacity_tests_30-200-500'
    workload4_dir = 'Results_5hf90_1e7_rw1_1e8/capacity_tests_30-200-500'
    
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
    policy_list = ['LFU','LRU','LRFU','EXD','Static','TEMP','ReStore']
    for i in range(len(policy_list)):
        print('Average percentage of', policy_list[i], 'over ideal: ', percentage_list[i])
