import os
import numpy as np
import pandas as pd

dfs = []
for i in range(21):
    filename = f"Google_Thesios/cluster1_16TB_20240115_data-{i:05d}-of-00100"
    if os.path.exists(filename):
        df = pd.read_csv(filename, sep=',', header=0)
        dfs.append(df)
    else:
        print(f"File {filename} does not exist.")
thesios = pd.concat(dfs, ignore_index=True)

# extract disk-only workload
thesios_disk = thesios.loc[thesios['cache_hit']!=1]

# Extract necessary columns from the DataFrame
offsets = thesios_disk['file_offset'].to_numpy()
sizes = thesios_disk['request_io_size_bytes'].to_numpy()
op_types = thesios_disk['op_type'].to_numpy()

# Calculate start and end pages
start_pages = offsets // 4096
end_pages = (offsets + sizes) // 4096

# Determine actions based on op_type
actions = np.where(op_types == 'WRITE', 'W', np.where(op_types == 'READ', 'R', 'Unknown'))

# Check for unknown operations
if 'Unknown' in actions:
    raise ValueError('Unknown operation type detected')

# Generate the page numbers and corresponding actions
list_page = []
list_act = []
for start_page, end_page, action in zip(start_pages, end_pages, actions):
    list_page.extend(range(start_page, end_page + 1))
    list_act.extend([action] * (end_page - start_page + 1))

# Save to a .txt file
with open('workload_thesios.txt', 'w') as file:
    for action, page in zip(list_act, list_page):
        file.write(f"{action} {page}\n")