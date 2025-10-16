from collections import defaultdict

# Set to track unique page IDs globally (across all input files)
global_unique_pages = set()  
global_total_reqs = 0       # Global counter for total lines in all output files
global_total_reads = 0       # Global counter for 'R' (Read) actions
global_total_writes = 0      # Global counter for 'W' (Write) actions

# List of input files
input_files = ['ssdtrace-00', 'ssdtrace-01']  # Add more file names as needed
output_file = 'workload_YCSB.txt'  # Output file to write the transformed log

with open(output_file, 'w') as f_out:
    for input_file in input_files:
        unique_pages = set()  # Set to track unique page IDs per file
        total_reqs = 0       # Counter for total lines in the current output file
        total_reads = 0       # Counter for 'R' (Read) actions per file
        total_writes = 0      # Counter for 'W' (Write) actions per file

        print(f"Processing {input_file}...")

        with open(input_file, 'r') as f_in:
            num_lines = 0
            for line in f_in:
                num_lines += 1
                # Uncomment the following if you want to limit the number of lines
                # if num_lines > 10000000:
                #     break

                parts = line.split()
                # Extract action: 'W' is Write ('W') and 'R' is Read ('R')
                if 'W' in parts and 'Q' in parts:
                    action = 'W'
                    total_writes += 1  # Increment write counter
                    global_total_writes += 1
                elif 'R' in parts and 'Q' in parts:
                    action = 'R'
                    total_reads += 1  # Increment read counter
                    global_total_reads += 1
                else:
                    continue  # skip other lines

                # Extract starting page and size
                starting_sector = int(parts[7])  # Starting sector number
                sector_count = int(parts[9])  # Number of sectors spanned (after '+')

                # Calculate ending sector
                ending_sector = starting_sector + sector_count

                # Convert sector range to page range
                starting_page = starting_sector // 8  # 8 sectors per page (4KB page)
                ending_page = ending_sector // 8

                # Generate page IDs starting from the offset
                for page_id in range(starting_page, ending_page+1):
                    f_out.write(f"{action} {page_id}\n")
                    total_reqs += 1  # Increment line count
                    global_total_reqs += 1  # Global increment
                    unique_pages.add(page_id)  # Add page to the set of unique pages
                    global_unique_pages.add(page_id)  # Track globally

        # Print summary statistics for this file
        print(f"File: {input_file}")
        print(f"Total number of unique pages in {input_file}: {len(unique_pages)}")
        print(f"Total number of requests in {input_file}: {total_reqs}")
        print(f"Total number of 'R' (Read) actions in {input_file}: {total_reads}")
        print(f"Total number of 'W' (Write) actions in {input_file}: {total_writes}")
        print()  # Blank line for readability between file statistics

# Print global summary statistics across all files
print(f"Global total number of unique pages: {len(global_unique_pages)}")
print(f"Global total number of requests in the output file: {global_total_reqs}")
print(f"Global total number of 'R' (Read) actions: {global_total_reads}")
print(f"Global total number of 'W' (Write) actions: {global_total_writes}")

# Total number of unique pages: 61960292
# Total number of lines in the file: 92728323
# Total number of 'R' (Read) actions: 92447244
# Total number of 'W' (Write) actions: 281079
