import os
import re
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from matplotlib.ticker import FuncFormatter
from matplotlib.ticker import ScalarFormatter
import seaborn as sns
import numpy as np
from matplotlib.font_manager import FontProperties
# Provide the full path to the font file
font_path = '/Users/tiazh991/Library/Fonts/LinLibertine_R.otf'
prop28 = FontProperties(fname=font_path, size=28)
prop40 = FontProperties(fname=font_path, size=40)
prop = FontProperties(fname=font_path, size=50)


def extract_migrs_from_log(file_path):
    """Extract 'Number of page migrated from Tier2 to Tier1' and 'Tier3 to Tier2' from a log file, and raise an error if not found."""
    with open(file_path, 'r') as file:
        content = file.read()

    # Extract 'Number of page migrated from Tier2 to Tier1'
    migr_t2_t1_match = re.search(r'Number of page migrated from Tier2 to Tier1:\s+(\d+)', content)
    if not migr_t2_t1_match:
        # special cases for RL and RLs2
        migr_t2_t1_match = re.search(r'Number of page upgraded from Tier2 to Tier1:\s+(\d+)', content)
        if not migr_t2_t1_match:
            # raise ValueError(f"'Number of page migrated from Tier2 to Tier1' not found in file: {file_path}")
            # let migr = 0 for ideal
            migr_t2_t1 = 0
    if migr_t2_t1_match:
        migr_t2_t1 = int(migr_t2_t1_match.group(1))

    # Extract 'Number of page migrated from Tier3 to Tier2'
    migr_t3_t2_match = re.search(r'Number of page migrated from Tier3 to Tier2:\s+(\d+)', content)
    if not migr_t3_t2_match:
        # special cases for RL and RLs2
        migr_t3_t2_match = re.search(r'Number of page upgraded from Tier3 to Tier2:\s+(\d+)', content)
        if not migr_t3_t2_match:
            # raise ValueError(f"'Number of page migrated from Tier2 to Tier1' not found in file: {file_path}")
            # let migr = 0 for ideal
            migr_t3_t2 = 0
    if migr_t3_t2_match:
        migr_t3_t2 = int(migr_t3_t2_match.group(1))

    return migr_t2_t1, migr_t3_t2

def collect_data(directory):
    """Collect 'Number of page migrated from Tier2/3 to Tier1/2' data from all log files in the given directory."""
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
                migr_t2_t1, migr_t3_t2 = extract_migrs_from_log(log_file_path)
                times.append((migr_t2_t1, migr_t3_t2))  # Store as tuple
        data[setting] = times
    return data

def format_title(setting):
    """Format the title based on the setting string."""
    tier1, tier2 = map(int, setting.split('-'))
    tier1_percent = tier1 // 100
    tier2_percent = tier2 // 100
    return f'Tier1 {tier1_percent}%, Tier2 {tier2_percent}%'

# Custom formatter to show '1eX' notation instead of 'x10^X'
def scientific_notation_formatter(val, pos):
    return f'{val/1e4:.0f}'


def plot_data(*data_list):
    """Plot the collected data for multiple workloads with enhanced colors and stacked bars."""
    settings = sorted(data_list[0].keys(), key=lambda s: tuple(map(int, s.split('-'))))
    num_logs = len(next(iter(data_list[0].values())))  # Assuming all workloads have the same number of logs
    # print(num_logs)
    # original order after calling sorted(os.listdir()):
    # log_labels = ['EXD','LFU','LRFU','LRU','RLs2','TEMP','ideal','static']
    # new order of log_labels
    log_labels = ['tLFU','tLRU','LRFU','EXD','Logi','XGB','Oracle','TEMP','ReStore']
    num_logs = len(log_labels)
    # mapping from origin log orders to new orders
    labels = [1,3,2,0,4,7,8,6,5]
    percentage_list = [[]]*len(log_labels)
    workload_labels = ['5hf90','10hf90','10hf80']
    # workload_labels = ['Ch_Wl_1','Ch_Wl_2','Ch_Wl_3']

    # create a dictionary for num_migr_ratio of each policy against ReStore
    migr_ratio = {label: [] for label in log_labels}

    fig, axs = plt.subplots(nrows=3, ncols=2, figsize=(25, 15))
    plt.subplots_adjust(wspace=0.0, hspace=0.1)  # Adjust width and height spacing
    axs = axs.flatten()

    # Use seaborn color palette for better colors
    # colors = sns.color_palette("Set2")
    # # Define the darker colors by darkening the original colors
    # darker_colors = [sns.set_hls_values(c, l=0.45) if i in [0,5] else sns.set_hls_values(c, l=0.5) for i, c in enumerate(colors)]
    # colors = sns.color_palette("Set2")
    colors = ['gainsboro','gainsboro','gray','gray','darkseagreen','steelblue','lightslategray','salmon','black']
    # use patterns to reduce usage of colors
    patterns = ['\\', '.', '/', 'o', '', '', '', '', '']  # Hatching patterns
    # Define the darker colors by darkening the original colors
    darker_colors = [sns.set_hls_values(c, l=0.4) for c in colors]
    bar_width = 0.85 / num_logs  # Adjust the bar width based on the number of logs
    x = np.arange(len(workload_labels))  # Positions for the workloads on the x-axis

    for idx, setting in enumerate(settings):
        ax = axs[idx]
        
        for i, data in enumerate(data_list):
            migr_t2_t1 = [t[0] for t in data[setting]]
            migr_t3_t2 = [t[1] for t in data[setting]]

            for j in range(num_logs):
                # Plot migr_t2_t1 (lighter color)
                ax.bar(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                       migr_t2_t1[labels[j]]+migr_t3_t2[labels[j]], 
                       width=bar_width, 
                       color=colors[j],
                       hatch=patterns[j],  # Apply hatching patterns
                       edgecolor='black')

                # # Plot migr_t3_t2 (darker color)
                # ax.bar(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                #        migr_t3_t2[labels[j]], 
                #        width=bar_width, 
                #        bottom=migr_t2_t1[labels[j]], 
                #        color=darker_colors[j],
                #        edgecolor='black', 
                #        label=log_labels[j] if i == 0 and idx == 0 else "")
                
                # Calculate the migration ratio against ReStore, and append to the dictionary
                ratio = (migr_t3_t2[labels[j]]+migr_t2_t1[labels[j]]) / (migr_t3_t2[labels[8]]+migr_t2_t1[labels[8]])
                migr_ratio[log_labels[j]].append(round(ratio,2))

        # Set the labels and title
        ax.set_title(format_title(setting), fontproperties=prop)  # Set the title with the formatted setting
        # only plot xlable on bottom subplots
        if idx in [4,5]:
            ax.set_xlabel('Workloads', fontproperties=prop)
            ax.set_xticks(x)
            ax.set_xticklabels(workload_labels, fontproperties=prop)
        else:
            ax.set_xlabel('')
            ax.set_xticks([])
        if idx == 2:  # Only add ylabel to the first subplot
            ax.set_ylabel('Number of page migrations (log scale)', fontproperties=prop)
        # if idx not in [3,6]:  # Only add yticks to the first subplot
        #     ax.set_yticks([])
        # else:
        # Use ScalarFormatter and force scientific notation
        scalar_formatter = ScalarFormatter(useMathText=True)
        scalar_formatter.set_powerlimits((4, 4))  # Set the power limit for scientific notation
        ax.yaxis.set_major_formatter(scalar_formatter)
        # Modify the offset text (which shows x10^4) to '1e4'
        ax.yaxis.get_offset_text().set_visible(False)
        # ax.text(0.01, 1.01, '1e4', transform=ax.transAxes, fontsize=28)
        ax.set_yscale('log')
        ax.set_ylim(2e2, 1e6)
        ax.tick_params(axis='y', labelsize=28)
        # if idx == 0:  # Only add the legend to the first subplot
        #     ax.legend(fontsize=15, loc='upper right')

    
    plt.tight_layout()
    plt.savefig('Migration_capacity_tests_static_2x3_newcolor.pdf',format='pdf',dpi=600, bbox_inches='tight', pad_inches=0)
    # plt.show()

    return migr_ratio

# Main execution
if __name__ == "__main__":
    # Directories for each workload
    workload1_dir = 'Results_5hf90_1e4_rw1_1e6/capacity_tests_30-200-500'
    workload2_dir = 'Results_10hf90_1e4_rw1_1e6/capacity_tests_30-200-500'
    workload3_dir = 'Results_10hf80_1e4_rw1_1e6/capacity_tests_30-200-500'
    
    # Collect data
    data1 = collect_data(workload1_dir)
    # print(len(data1))
    data2 = collect_data(workload2_dir)
    # print(len(data2))
    data3 = collect_data(workload3_dir)
    # print(len(data3))
    
    # Plot the data
    migr_ratios = plot_data(data1, data2, data3)
    print(migr_ratios)
