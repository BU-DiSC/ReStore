from collections import defaultdict
import csv
import pandas as pd

# read DiskReadWrite record
diskrw_df=pd.read_csv('MSR-Cambridge/hm_1.csv', header=None,
                      names=['Timestamp','Hostname','DiskNumber','Type','Offset','Size','ResponseTime'])

# # check if there's a write before read
# written_offsets = set()
# read_was_written_before = []
# for index, row in diskrw_df.iterrows():
#     if row["Type"] == "Read":
#         read_was_written_before.append(row["Offset"] in written_offsets)
#     else:
#         written_offsets.add(row["Offset"])
#         read_was_written_before.append(row["Offset"] in written_offsets)
# diskrw_df["Read_Offset_Was_Written_Before"] = read_was_written_before

# Preprocessing
# select record offset < 100GB
selected_io=diskrw_df#.loc[diskrw_df[5]<100*1024**3]


# Step 1: Initialize data structures
unique_pages = set()  # Set to track unique page IDs
page_access_count = defaultdict(int)  # Dictionary to track access frequencies
total_reqs = 0        # Counter for total lines in the output file
total_reads = 0       # Counter for 'R' (Read) actions
total_writes = 0      # Counter for 'W' (Write) actions


# Step 3: Reprocess the I/O operations and write to the output file with reassigned page IDs
with open('workload_MSR_hm_1.txt', 'w') as f_out:
    for line in range(len(selected_io)):
        # select the ith line of df
        parts = selected_io[line:line+1]
        
        # Extract action: 'DiskWrite' is Write ('W') and 'DiskRead' is Read ('R')
        if parts['Type'].item() == 'Write':
            action = 'W'
            total_writes += 1  # Increment write counter
        elif parts['Type'].item() == 'Read':
            action = 'R'
            total_reads += 1  # Increment read counter
        else:
            raise ValueError('Unknown operation')
        
        # Extract offset and size
        # offset = int(parts['Offset']) - 24576 #(hm_1)  # The offset is at column 5, start from min(offset)
        offset = int(parts['Offset']) - 16384 #(mds_0)  # The offset is at column 5, start from min(offset)
        size = int(parts['Size'])  # The size is at column 6
        
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
with open('workload_MSR_hm_1_page_access_frequencies.csv', 'w', newline='') as csv_file:
    csv_writer = csv.writer(csv_file)
    csv_writer.writerow(['page_id', 'num_of_accesses'])
    for page_id, access_count in page_access_count.items():
        csv_writer.writerow([page_id, access_count])

# Step 5: Print the summary statistics
print(f"Total number of unique pages: {len(unique_pages)}")
print(f"Total number of requests in the output file: {total_reqs}")
print(f"Total number of 'R' (Read) actions: {total_reads}")
print(f"Total number of 'W' (Write) actions: {total_writes}")

# hm_1
# Total number of unique pages: 51733
# Total number of requests in the output file: 2308560
# Total number of 'R' (Read) actions: 580896
# Total number of 'W' (Write) actions: 28415

# wdev_0_trim
# Total number of unique pages: 79677
# Total number of requests in the output file: 695995
# Total number of 'R' (Read) actions: 64068
# Total number of 'W' (Write) actions: 240102