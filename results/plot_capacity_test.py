import os
import re
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from matplotlib import scale as mscale
from matplotlib import transforms as mtransforms
from matplotlib.ticker import FixedLocator, FuncFormatter, ScalarFormatter
from matplotlib.font_manager import FontProperties
# Provide the full path to the font file
font_path = '/Users/tiazh991/Library/Fonts/LinLibertine_R.otf'
prop28 = FontProperties(fname=font_path, size=28)
prop40 = FontProperties(fname=font_path, size=40)
prop = FontProperties(fname=font_path, size=50)


# Define the custom scale class
class ExpScale(mscale.ScaleBase):
    """
    Scales data as exp(y / 1e8).
    """
    name = 'exp_scale'

    def __init__(self, axis, **kwargs):
        super().__init__(axis)
        self.base = 1e8  # Define your base here

    def get_transform(self):
        return self.ExpTransform(self.base)

    def set_default_locators_and_formatters(self, axis):
        axis.set_major_locator(FixedLocator([0, 1e8, 2e8, 3e8, 4e8]))
        axis.set_major_formatter(FuncFormatter(lambda x, pos: f'{x / 1e8:.0f}e8'))

    class ExpTransform(mtransforms.Transform):
        input_dims = output_dims = 1

        def __init__(self, base):
            mtransforms.Transform.__init__(self)
            self.base = base

        def transform_non_affine(self, a):
            # Apply the exponential transformation: exp(y / 1e8)
            return np.exp(a / self.base)

        def inverted(self):
            return ExpScale.InvertedExpTransform(self.base)

    class InvertedExpTransform(mtransforms.Transform):
        input_dims = output_dims = 1

        def __init__(self, base):
            mtransforms.Transform.__init__(self)
            self.base = base

        def transform_non_affine(self, a):
            # Apply the inverse transformation: log(a) * 1e8
            return np.log(a) * self.base

        def inverted(self):
            return ExpScale.ExpTransform(self.base)

# Register the custom scale with matplotlib
mscale.register_scale(ExpScale)


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
    percentage_list = [[] for _ in range(len(log_labels))]
    workload_labels = ['5hf90','10hf90','10hf80']
    # workload_labels = ['Ch_Wl_1','Ch_Wl_2','Ch_Wl_3']

    fig, axs = plt.subplots(nrows=3, ncols=2, figsize=(25, 15))
    plt.subplots_adjust(wspace=0.0, hspace=0.06)  # Adjust width and height spacing
    axs = axs.flatten()

    # Use seaborn color palette for better colors
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
            times_total = [t[0] for t in data[setting]]  # Total Requests Time
            times_loop = [t[1] for t in data[setting]]   # For loop time

            for j in range(num_logs):
                # Plot the For loop time (darker color)
                ax.bar(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                       times_loop[labels[j]],   # use new order
                       width=bar_width, 
                       color=darker_colors[j],
                       edgecolor='black')

                # Plot the Total Requests Time minus the For loop time (lighter color)
                ax.bar(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                       times_total[labels[j]] - times_loop[labels[j]],   # use new order
                       width=bar_width, 
                       bottom=times_loop[labels[j]],   # use new order
                       color=colors[j],
                       hatch=patterns[j],  # Apply hatching patterns
                       edgecolor='black', 
                       label=log_labels[j] if i == 0 and idx == 8 else "")
                
                # # Add percentage(/ideal) above each bar
                # ax.text(x[i] + j*bar_width - (num_logs-1)*bar_width/2, 
                #         times_total[labels[j]], #+ 1e5,  # Slight offset to place the text above the bar
                #         round(times_total[labels[j]]/times_total[6],2),   # Format to 1 decimal places
                #         ha='center', 
                #         va='bottom', 
                #         fontsize=11, 
                #         color='black')
                
                # record the percentage
                if data: #== data1:
                    percentage_list[labels[j]].append(times_total[labels[j]]/times_total[6])

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
        # Only add ylabel to the first subplot
        if idx == 2:
            ax.set_ylabel('Runtime (microseconds)', fontproperties=prop)
        # use different ylim
        if idx in [0]:
            ax.set_ylim(0, 8.0*1e8)
        elif idx in [1,2]:
            ax.set_ylim(0, 4*1e8)
        else:
            ax.set_ylim(0, 2*1e8)
        # if idx != 0:  # Only add yticks to the first subplot
            # ax.set_yticks([])
        if idx in [3,4,5]:
            ax.set_yticks([0,1e8,2e8])
        # Use ScalarFormatter and force scientific notation
        scalar_formatter = ScalarFormatter(useMathText=True)
        scalar_formatter.set_powerlimits((8, 8))  # Set the power limit for scientific notation
        ax.yaxis.set_major_formatter(scalar_formatter)
        # Modify the offset text (which shows x10^8) to '1e8'
        ax.yaxis.get_offset_text().set_visible(False)
        ax.text(0.01, 1.01, '1e8', transform=ax.transAxes, fontsize=28)
        # ax.set_yscale('exp_scale')
        # ax.set_yticks([])
        # ax.set_ylim(0, 2e8)
        ax.tick_params(axis='y', labelsize=28)
        # ax.yaxis.offsetText.set_fontsize(28)
        # if idx == 8:  # Only add the legend to the first subplot
        #     ax.legend(fontsize=25, loc='upper right', ncol=3)

    plt.tight_layout()
    plt.savefig('Results_capacity_tests_static_2x3_newcolor.pdf',format='pdf',dpi=600, bbox_inches='tight', pad_inches=0)
    # plt.savefig('Results_capacity_tests_changings.png',format='png',dpi=320, bbox_inches='tight')
    plt.show()

    return percentage_list

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
    percentage_list = plot_data(data1, data2, data3)
    print(len(percentage_list))
    print(len(percentage_list[0]))
    policy_list = ['tLFU','tLRU','LRFU','EXD','Logi','XGB','Oracle','TEMP','ReStore']
    labels = [1,3,2,0,4,7,8,6,5]
    for i in range(len(policy_list)):
        avg_percentage = sum(percentage_list[i])/len(percentage_list[i])
        print('Average percentage of', policy_list[labels[i]], 'over Static: ', avg_percentage)
