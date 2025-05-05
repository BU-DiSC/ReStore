import pandas as pd
import numpy as np
# import matplotlib.pyplot as plt
from collections import Counter
import argparse

# Set up argument parser
parser = argparse.ArgumentParser(description="Extract page info from workload.")
parser.add_argument("-workload", type=str, required=True, help="Workload name (e.g., '5hf90_1e4_rw1_1e6')")
# Parse command-line arguments
args = parser.parse_args()
workload = args.workload  # use workload from command line
# workload = 'MSR_hm_1'

workload_access_freq_path = 'workload_' + workload + '_page_access_frequencies.csv'
freq=pd.read_csv(workload_access_freq_path)
print(freq['num_of_accesses'].value_counts())
# MSR_hm_1
# 1       25283
# 5        3996
# 7        3042
# 2        2817
# 6        2468
#         ...  
# 858         2
# 135         2
# 3755        2
# 1472        2
# 1422        2
# Name: num_of_accesses, Length: 798, dtype: int64
# Total number of pages:  51733
# Total number of hfpages:  7752 (>6)

# mds_0
# 1        718186
# 2         26888
# 3         12301
# 31        10011
# 4          8694
#           ...  
# 513           1
# 388           1
# 245           1
# 11634         1
# 2642          1
# Name: num_of_accesses, Length: 371, dtype: int64
# Total number of pages:  802284
# Total number of hfpages:  36215

# wdev_0
# 1       34453
# 6       19771
# 2        6147
# 7        3042
# 8        2609
#         ...  
# 1315        1
# 1275        1
# 209         1
# 327         1
# 422         1
# Name: num_of_accesses, Length: 195, dtype: int64
# Total number of pages:  79677
# Total number of hfpages:  8772 (>8)


# frequency_counts = Counter(freq['num_of_accesses'])
# frequencies = list(frequency_counts.keys())
# num_pages = list(frequency_counts.values())
# plt.figure(figsize=(10, 6))
# plt.bar(frequencies, num_pages, color='skyblue')
# # Add labels and title
# plt.xlabel('Frequency of Accesses')
# plt.ylabel('Number of Pages')
# plt.title('Page Access Frequency Distribution')
# # Show the plot
# plt.savefig('Page_access_freq_workload_MSR_hm_1.png',format='png',dpi=320, bbox_inches='tight')

# Write all page_ids to 'workload.allpageids'
all_page_ids = freq['page_id'].unique()
# shuffle the ids for random insertion
np.random.shuffle(all_page_ids)
print("Total number of pages: ", len(all_page_ids))

allpageid_path = 'workload_' + workload + '.allpageids'
with open(allpageid_path, 'w') as f_out:
    f_out.write(','.join(map(str, all_page_ids)))
print(f"all page IDs have been written to {allpageid_path}")

# Extract the hf page_ids
hf_page_ids = freq.loc[freq['num_of_accesses']>8]['page_id'].tolist()
print("Total number of hfpages: ", len(hf_page_ids))
# Write hf page_ids to 'workload.pageids'
hfpageid_path = 'workload_' + workload + '.pageids'
with open(hfpageid_path, 'w') as f_out:
    f_out.write(','.join(map(str, hf_page_ids)))
print(f"hf page IDs have been written to {hfpageid_path}")

# line_number=0
# target_page_id=[300,700,801530,2860075,11323838,20415828]
# with open('workload_TPCC.txt', 'r') as f:
#     for line in f:
#         line_number += 1  # Increment the line number as we read each line
#         action, page_id = line.split()  # Extract the action and page ID from the line
#         if int(page_id) in target_page_id:  # Check if the page ID matches the target
#             print(f"Page ID {page_id} occurs at line {line_number}")
