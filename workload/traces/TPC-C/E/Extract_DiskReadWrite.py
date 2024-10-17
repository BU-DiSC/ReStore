# collect 'DiskRead' and 'DiskWrite items from the trace
diskrw=[]
with open("W2K8.TPCE.10-18-2007.05-46-PM.trace.csv", 'r') as temp_f:
    for line in temp_f.readlines():
        cleaned_elements = [element.strip() for element in line.split(",")]
        if cleaned_elements[0] in ['DiskRead', 'DiskWrite'] :
            diskrw.append(cleaned_elements)
import csv
with open("W2K8.TPCE.10-18-2007.05-46-PM.trace_DiskReadWrite.csv", 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerows(diskrw)

import pandas as pd
diskrw_df = pd.read_csv("W2K8.TPCE.10-18-2007.05-46-PM.trace_DiskReadWrite.csv", header=None)
diskrw_df=diskrw_df[2:]
diskrw_df[8]=diskrw_df[8].astype(int)
diskrw_df[5]=diskrw_df[5].apply(lambda x: int(x, 16))
diskrw_df[6]=diskrw_df[6].apply(lambda x: int(x, 16))
print(diskrw_df[5].describe().apply(lambda x: '%.3f' % x))
# count        15259272.000
# mean     108909896866.809
# std       73580840794.498
# min             97792.000
# 25%       47724054016.000
# 50%      105576369664.000
# 75%      147326553600.000
# max      344996269056.000
# Name: 5, dtype: object