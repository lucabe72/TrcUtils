This is a pre-release of TrcUtils, a set of filters allowing to handle
traces generate by different tools, converting them between various formats,
extracting statistics, and visualising them in varions way.

Currently, 4 filters are provided:
- import: convert traces from ftrace (old and new format), rtsim, Fiasco
          (with a ukernel patch), and X server (with a patch) to the TrcUtils
          internal format
- export: convert traces from the TrcUtils internal format to rtsim, or
          generate xfig plots from the trace
- stats: extract various statistics from a trace in TrcUtils format
- visual: live visualisation of the tasks contained in a trace

To compile the filters, just type "make".

All the filters provide a help message explaining the command line options.

The "Scripts" directory contains some example scripts showing how to use
the various filters.
