# Plotting the Results

Each script reads the experiment logs from `Results_<workload>/...` folders in this directory and writes a PDF figure. See [Reproducing the paper's experiments](../README.md#reproducing-the-papers-experiments) in the main README for which results each script needs.

## Usage

After running the experiments, move the `Results_<workload>` folders (created in the directory where `restore` was run, by default the repository root) into this directory and run the corresponding script:

```bash
pip install matplotlib seaborn numpy
mv ../Results_* .
python3 plot_capacity_test.py
```

The paper figures use the Linux Libertine font; set `RESTORE_PLOT_FONT=/path/to/LinLibertine_R.otf` to use it (otherwise a default serif font is used).
