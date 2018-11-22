#!/usr/bin/env python3
import csv
import numpy as np
import sys
import scipy.stats as stats
import matplotlib.pyplot as plt


def main():
    if sys.argv[1:]:
        filename = sys.argv[1]
    else:
        raise Exception("Missing file name argument")
    with open(filename, "r") as f: 
        data_list = list(csv.reader(f))
    # convert all values to floats
    converted_data_list = []
    for row in data_list:
        converted_row = []
        for item in row:
            converted_row.append(float(item))
        converted_data_list.append(converted_row)
    data = np.array(converted_data_list)
    mu, std = stats.norm.fit(data.tolist())

    # Plot the histogram.
    plt.hist(data, bins=25, density=True, alpha=0.6, color='g')

    # Plot the PDF.
    xmin, xmax = plt.xlim()
    x = np.linspace(xmin, xmax, 100)
    p = stats.norm.pdf(x, mu, std)
    plt.plot(x, p, 'k', linewidth=2)
    title = "Fit results: mu = %.2f,  std = %.2f" % (mu, std)
    plt.title(title)
    plt.savefig('foo.png')

    print("Length:    %d" % int(len(data)))
    print("Min:       %1.2f" % int(data.min()))
    print("Max:       %1.2f" % int(data.max()))
    print("Mean:      %1.2f" % float(data.mean()))
    print("StdDev:    %1.2f" % float(data.std()))
    print("Var:       %1.2f" % float(data.var()))
    print("Peak2Peak: %1.2f" % int(data.ptp()))

if __name__ == '__main__':
    main()
