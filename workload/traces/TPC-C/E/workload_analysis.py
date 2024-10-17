import pandas as pd
import matplotlib.pyplot as plt
from collections import Counter

freq=pd.read_csv('workload_TPCE_page_access_frequencies.csv')
print(freq['num_of_accesses'].value_counts())
# num_of_accesses
# 1       11289544
# 2        5105035
# 3        1858147
# 4         652545
# 5         224704
# 6          81703
# 7          30779
# 8          13678
# 9           7292
# 10          4444
# 11          2865
# 12          1638
# 13           907
# 14           420
# 15           163
# 16            80
# 399           47
# 398           28
# 400           24
# 397           21
# 401           15
# 396           13
# 17            10
# 395            7
# 394            5
# 18             4
# 391            3
# 393            3
# 381            3
# 378            3
# 1199           2
# 389            2
# 387            2
# 19             2
# 20             2
# 379            2
# 390            1
# 1200           1
# 377            1
# 382            1
# 1198           1
# 97             1
# 384            1
# 392            1
# 25             1
# 88             1
# 67             1
# 38             1
# 383            1
# 24             1
# 34             1
# 96             1
# 106            1
# 66             1
# 100            1
# 60             1
# 37             1
# Name: count, dtype: int64
frequency_counts = Counter(freq['num_of_accesses'])
frequencies = list(frequency_counts.keys())
num_pages = list(frequency_counts.values())
plt.figure(figsize=(10, 6))
plt.bar(frequencies, num_pages, color='skyblue')
# Add labels and title
plt.xlabel('Frequency of Accesses')
plt.ylabel('Number of Pages')
plt.title('Page Access Frequency Distribution')
# Show the plot
plt.savefig('Page_access_freq_workload_TPCE.png',format='png',dpi=320, bbox_inches='tight')

# Write all page_ids to 'workload.allpageids'
all_page_ids = freq['page_id'].unique()
print("Total number of pages: ", len(all_page_ids))
# Total number of pages: 19274163
with open('workload_TPCE.allpageids', 'w') as f_out:
    f_out.write(','.join(map(str, all_page_ids)))
print(f"all page IDs have been written to workload_TPCE.pageids")

# Extract the hf page_ids
hf_page_ids = freq.loc[freq['num_of_accesses']>2]['page_id'].tolist()
print("Total number of hfpages: ", len(hf_page_ids))
# Total number of hfpages: 2879584
# Write hf page_ids to 'workload.pageids'
with open('workload_TPCE.pageids', 'w') as f_out:
    f_out.write(','.join(map(str, hf_page_ids)))
print(f"hf page IDs have been written to workload_TPCE.pageids")

# line_number=0
# target_page_id=[]
# with open('workload_TPCE.txt', 'r') as f:
#     for line in f:
#         line_number += 1  # Increment the line number as we read each line
#         action, page_id = line.split()  # Extract the action and page ID from the line
#         if int(page_id) in target_page_id:  # Check if the page ID matches the target
#             print(f"Page ID {page_id} occurs at line {line_number}")
