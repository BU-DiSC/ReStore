from collections import defaultdict
import csv
import pandas as pd

# read DiskReadWrite record
diskrw_df=pd.read_csv('W2K8.TPCE.10-18-2007.05-46-PM.trace_DiskReadWrite.csv',header=None)
# Preprocessing
diskrw_df=diskrw_df[2:]
diskrw_df[8]=diskrw_df[8].astype(int)
diskrw_df[5]=diskrw_df[5].apply(lambda x: int(x, 16))
diskrw_df[6]=diskrw_df[6].apply(lambda x: int(x, 16))
# select record offset < 100GB
selected_io=diskrw_df#.loc[diskrw_df[5]<100*1024**3]


# Step 1: Initialize data structures
unique_pages = set()  # Set to track unique page IDs
page_access_count = defaultdict(int)  # Dictionary to track access frequencies
total_reqs = 0        # Counter for total lines in the output file
total_reads = 0       # Counter for 'R' (Read) actions
total_writes = 0      # Counter for 'W' (Write) actions

# # Process the subset_io to gather all unique page IDs
# for line in range(len(selected_io)):
#     parts = selected_io[line:line+1]
#     offset = int(parts[5]) - 1114112
#     size = int(parts[6])
#     start_page = offset // 4096
#     num_pages = size // 4096
    
#     # Collect all page IDs for this I/O operation
#     for page_id in range(start_page, start_page + num_pages):
#         unique_pages.add(page_id)

# # Step 2: Create a mapping from original page IDs to sequential IDs (because of integar range in C++)
# sorted_unique_pages = sorted(unique_pages)
# page_id_mapping = {page_id: new_id for new_id, page_id in enumerate(sorted_unique_pages)}

# Step 3: Reprocess the I/O operations and write to the output file with reassigned page IDs
with open('workload_TPCE.txt', 'w') as f_out:
    for line in range(len(selected_io)):
        # select the ith line of df
        parts = selected_io[line:line+1]
        
        # Extract action: 'DiskWrite' is Write ('W') and 'DiskRead' is Read ('R')
        if parts[0].item() == 'DiskWrite':
            action = 'W'
            total_writes += 1  # Increment write counter
        elif parts[0].item() == 'DiskRead':
            action = 'R'
            total_reads += 1  # Increment read counter
        else:
            raise ValueError('Unknown operation')
        
        # Extract offset and size
        offset = int(parts[5]) - 97792  # The offset is at column 5, start from min(offset)
        size = int(parts[6])  # The size is at column 6
        
        start_page = offset // 4096
        # Calculate the number of pages (size / 4KB)
        num_pages = size // 4096
        
        # Generate page IDs starting from the offset and write reassigned page IDs to the file
        for page_id in range(start_page, start_page + num_pages):
            # reassigned_page_id = page_id_mapping[page_id]  # Get the reassigned page ID
            f_out.write(f"{action} {page_id}\n")
            total_reqs += 1  # Increment line count
            unique_pages.add(page_id)
            page_access_count[page_id] += 1  # Increment the page access count

# Step 4: Write page access frequencies to CSV file
with open('workload_TPCE_page_access_frequencies.csv', 'w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(['page_id', 'num_of_accesses'])
    for page_id, access_count in page_access_count.items():
        csv_writer.writerow([page_id, access_count])

# Step 5: Print the summary statistics
print(f"Total number of unique pages: {len(unique_pages)}")
print(f"Total number of requests in the output file: {total_reqs}")
print(f"Total number of 'R' (Read) actions: {total_reads}")
print(f"Total number of 'W' (Write) actions: {total_writes}")

# Total number of unique pages: 19274163
# Total number of requests in the output file: 31884418
# Total number of 'R' (Read) actions: 14014644
# Total number of 'W' (Write) actions: 1244628