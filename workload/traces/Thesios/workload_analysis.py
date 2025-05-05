from collections import defaultdict
import csv

unique_pages = set()  # Set to track unique page IDs
total_lines = 0       # Counter for total lines in the file
total_reads = 0       # Counter for 'R' (Read) actions
total_writes = 0      # Counter for 'W' (Write) actions
page_access_count = defaultdict(int)  # Dictionary to track access frequencies

with open('workload_thesios.txt', 'r') as f_in:
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

# Total number of unique pages: 16417
# Total number of lines in the file: 95970071
# Total number of 'R' (Read) actions: 57308269
# Total number of 'W' (Write) actions: 38661802

# Write page access counts to a CSV file, if specified
csv_output_file = 'workload_thesios_page_access_frequencies.csv'
with open(csv_output_file, 'w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(['page_id', 'num_of_accesses'])
    for page_id, access_count in page_access_count.items():
        csv_writer.writerow([page_id, access_count])
print(f"Page access frequencies have been written to {csv_output_file}")


from collections import Counter
import pandas as pd
import matplotlib.pyplot as plt
freq = pd.read_csv('Google_Thesios/workload_thesios_page_access_frequencies.csv')
frequency_counts = Counter(freq['num_of_accesses'])
frequencies = list(frequency_counts.keys())
num_pages = list(frequency_counts.values())
plt.figure(figsize=(10, 6))
plt.bar(frequencies, num_pages, color='skyblue')
# Add labels and title
plt.xlabel('Frequency of Accesses')
plt.xlim(0,45)
plt.xticks([0,7,10,12,17,22,25,28,31,36,44], 
           labels=[0,7,10,12,17,22,25,28,31,'35+',101954])
plt.ylabel('Number of Pages')
plt.title('Distribution of Page Access Frequency')
# Show the plot
plt.show()
# svae fig
plt.savefig('Google_Thesios/Page_num_accesses_dist_Thesios.png', format='png', dpi=320, bbox_inches='tight')
