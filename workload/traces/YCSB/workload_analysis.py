import csv
from collections import defaultdict, Counter
import pandas as pd
import matplotlib.pyplot as plt

def analyze_page_rw_log(input_file, csv_output_file=None):
    unique_pages = set()  # Set to track unique page IDs
    total_lines = 0       # Counter for total lines in the file
    total_reads = 0       # Counter for 'R' (Read) actions
    total_writes = 0      # Counter for 'W' (Write) actions
    page_access_count = defaultdict(int)  # Dictionary to track access frequencies
    
    with open(input_file, 'r') as f_in:
        for line in f_in:
            parts = line.split()
            if len(parts) != 2:
                continue  # Skip any malformed lines
            
            action = parts[0]  # 'R' or 'W'
            page_id = int(parts[1])  # Page ID
            
            # Count the action
            if action == 'R':
                total_reads += 1
            elif action == 'W':
                total_writes += 1
            
            # Update total lines and unique page set
            total_lines += 1
            unique_pages.add(page_id)
            
            # Increment the page access count
            page_access_count[page_id] += 1
    
    # Print the summary statistics
    print(f"Total number of unique pages: {len(unique_pages)}")
    print(f"Total number of lines in the file: {total_lines}")
    print(f"Total number of 'R' (Read) actions: {total_reads}")
    print(f"Total number of 'W' (Write) actions: {total_writes}")
    
    # Total number of unique pages: 61960292
    # Total number of lines in the file: 92728323
    # Total number of 'R' (Read) actions: 92447244
    # Total number of 'W' (Write) actions: 281079

    # Write page access counts to a CSV file, if specified
    if csv_output_file:
        with open(csv_output_file, 'w', newline='') as csv_file:
            csv_writer = csv.writer(csv_file)
            csv_writer.writerow(['page_id', 'num_of_accesses'])
            for page_id, access_count in page_access_count.items():
                csv_writer.writerow([page_id, access_count])
        print(f"Page access frequencies have been written to {csv_output_file}")

# Example usage
input_file = 'workload_YCSB.txt'  # Path to your page read/write log file
csv_output_file = 'workload_YCSB_page_access_frequencies.csv'  # Output CSV file for page access frequencies
# analyze_page_rw_log(input_file, csv_output_file)



# Step 1: Load the CSV file
csv_file = 'workload_YCSB_page_access_frequencies.csv'  # Path to your CSV file
data = pd.read_csv(csv_file)

# Write all page_ids to 'workload.allpageids'
all_page_ids = data['page_id'].unique()
with open('workload_YCSB.allpageids', 'w') as f_out:
    f_out.write(','.join(map(str, all_page_ids)))

print(f"all page IDs have been written to workload_YCSB.allpageids")

print(data['num_of_accesses'].value_counts())
# num_of_accesses
# 1     37380886
# 2     19089211
# 3      4833606
# 4       616014
# 5        39340
# 6         1216
# 7           17
# 13           2

# Step 2: Sort the data by 'num_of_accesses' in descending order
sorted_data = data.sort_values(by='num_of_accesses', ascending=False)

# Extract the hf page_ids
hf_page_ids = data.loc[data['num_of_accesses']>2]['page_id'].tolist()

# Write hf page_ids to 'workload.pageids'
with open('workload_YCSB.pageids', 'w') as f_out:
    f_out.write(','.join(map(str, hf_page_ids)))

print(f"hf page IDs have been written to workload_YCSB.pageids")

# Step 2: Count the frequencies of accesses
# The 'num_of_accesses' column represents how many times each page was accessed
frequency_counts = Counter(data['num_of_accesses'])

# Step 3: Prepare data for plotting
# x-axis will be the access frequencies
# y-axis will be the count of how many pages have each access frequency
frequencies = list(frequency_counts.keys())
num_pages = list(frequency_counts.values())

# Step 4: Plot the bar plot
plt.figure(figsize=(10, 6))
plt.bar(frequencies, num_pages, color='skyblue')

# Add labels and title
plt.xlabel('Frequency of Accesses')
plt.ylabel('Number of Pages')
plt.title('Page Access Frequency Distribution')

# Show the plot
plt.show()
plt.savefig('Page_access_freq_workload_YCSB.png',format='png',dpi=320, bbox_inches='tight')