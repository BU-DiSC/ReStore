import os
import re
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from matplotlib.font_manager import FontProperties
# Provide the full path to the font file
# font_path = '/Users/tiazh991/Library/Fonts/LinLibertine_R.otf'
# prop = FontProperties(fname=font_path, size=42)


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
    data = []
    # Sort os.listdir to make sure it follows alphabetic order
    for log_file in sorted(os.listdir(directory)):
        if log_file.endswith('.log'):
            log_file_path = os.path.join(directory, log_file)
            total_time, loop_time = extract_times_from_log(log_file_path)
            data.append((total_time, loop_time))  # Store as tuple
    return data


def plot_data(*data_list):
    """Plot the collected data for multiple workloads with enhanced colors and stacked bars."""
    # num_logs = len(next(iter(data_list[0])))  # Assuming all workloads have the same number of logs
    # log_labels = ['Sibyl','tLFU','tLRU','LRFU','EXD','Logi','XGB','Oracle','TEMP','ReStore']
    log_labels = ['Sibyl','tLFU','tLRU','LRFU','EXD','Logi','XGB','TEMP','ReStore']
    num_logs = len(log_labels)
    # mapping from origin log orders to new orders
    # labels = [6,1,3,2,0,4,8,9,7,5]
    labels = [6,1,3,2,0,4,8,7,5]
    percentage_list = [0] * len(log_labels)
    workload_labels = ['hm_1_ST','wdev_0_ST','hm_1_Con','wdev_0_Con']

    fig, ax = plt.subplots(figsize=(20, 8))  # Only one subplot now, not an array of subplots

    # Use seaborn color palette for better colors
    # colors = sns.color_palette("Set2")
    # # Define the darker colors by darkening the original colors
    # darker_colors = [sns.set_hls_values(c, l=0.4) for c in colors]
    # colors = ['moccasin','gainsboro','gainsboro','gray','gray','darkseagreen','steelblue','lightslategray','salmon','black']
    colors = ['moccasin','gainsboro','gainsboro','gray','gray','darkseagreen','steelblue','salmon','black']
    # use patterns to reduce usage of colors
    # patterns = ['','\\', '.', '/', 'o', '', '', '', '', '']  # Hatching patterns
    patterns = ['','\\', '.', '/', 'o', '', '', '', '']  # Hatching patterns
    # Define the darker colors by darkening the original colors
    darker_colors = [sns.set_hls_values(c, l=0.3) for c in colors]

    bar_width = 0.85 / num_logs  # Adjust the bar width based on the number of logs
    x = np.arange(len(workload_labels))  # Positions for the workloads on the x-axis


    for i, data in enumerate(data_list):
        times_total = [t[0] for t in data]  # Total Requests Time
        times_loop = [t[1] for t in data]   # For loop time
        print(times_total)

        for j in range(num_logs):
            # # Plot the For loop time (darker color)
            # ax.bar(x[i] + j * bar_width - (num_logs - 1) * bar_width / 2,
            #        times_loop[labels[j]],
            #        width=bar_width,
            #        color=darker_colors[j],
            #        # hatch=patterns[j],  # Apply hatching patterns
            #        edgecolor='black')

            # Plot the Total Requests Time minus the For loop time (lighter color)
            ax.bar(x[i] + j * bar_width - (num_logs - 1) * bar_width / 2,
                   times_total[labels[j]] / times_total[6],
                   width=bar_width,
                   #bottom=times_loop[labels[j]] / times_total[8],
                   color=colors[j], 
                   hatch=patterns[j],  # Apply hatching patterns
                   edgecolor='black',
                   label=log_labels[j] if i == 0 else "")

            # record the percentage
            # percentage_list[labels[j]] += times_total[labels[j]]/times_total[8]

    ax.axhline(y=1, xmin=0, xmax=0.2+len(workload_labels), linestyle='--', color='gray')
    # Set the labels and title
    # ax.set_title('Results on workloads from real I/O traces', fontsize=40)  # Set the title
    ax.set_xlabel('Workloads', fontsize=40)
    ax.set_ylabel('Norm. Latency vs Sibyl', fontsize=40)
    ax.set_ylim(0,1.9)
    ax.set_yticks([0,0.5,1,1.5])
    ax.set_xticks(x)
    ax.set_xticklabels(workload_labels, fontsize=40)
    ax.legend(fontsize=33, loc='upper right', ncol=3)
    ax.tick_params(axis='y', labelsize=32)

    plt.tight_layout()
    plt.savefig('Results_Sibyl_traces.pdf', format='pdf', dpi=320, bbox_inches='tight')
    # plt.show()


# Main execution
if __name__ == "__main__":
    # Directories for each workload
    workload1_dir = 'Results_MSR_hm_1/num_threads111/5000-15000'
    workload2_dir = 'Results_MSR_wdev_0/num_threads111/5000-15000'
    workload3_dir = 'Results_MSR_hm_1/num_threads842/5000-15000'
    workload4_dir = 'Results_MSR_wdev_0/num_threads842/5000-15000'


    # Collect data
    data1 = collect_data(workload1_dir)
    data2 = collect_data(workload2_dir)
    data3 = collect_data(workload3_dir)
    data4 = collect_data(workload4_dir)

    # Plot the data
    plot_data(data1, data2, data3, data4)
