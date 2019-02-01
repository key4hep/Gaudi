import os
import matplotlib
matplotlib.use('PDF')
import matplotlib.pyplot as plt


def instancesVsTime(filename):
    logfile = open(filename)
    nOfInstances = {}
    runtime = {}
    # extract all info
    for line in logfile.readlines():
        if "I ran" in line:
            name = line.split("SUCCESS")[0]
            runtime[name] = float(line.split("runtime of")[1])
            if name in nOfInstances:
                number = nOfInstances[name]
            else:
                number = 0
            nOfInstances[name] = number + 1
    # now sort it
    x = []
    y = []
    for name in runtime:
        x.append(runtime[name])
        y.append(nOfInstances[name])
    fig = plt.figure()
    ax = fig.add_subplot(111)
    yo = ax.scatter(x, y)
    ax.grid(True)
    plt.xlabel("Runtime (s)")
    plt.ylabel("# instances")
    plt.title(
        "Requested algorithm instances with 10 events and 10 threads in parallel."
    )
    plt.savefig("test.pdf")


class TimingInfo(object):
    def __init__(self, name, time):
        name = name.rstrip(".time")
        threads, events, algos = name.split("_")[1:]
        self.threads = int(threads)
        self.events = int(events)
        self.algos = int(algos)
        self.time = float(time)


def prepareTimingPlots(config="BrunelScenario", path="../options"):

    # first read all the timings
    timings = []
    for filename in os.listdir(path):
        if filename.startswith(config) and filename.endswith("time"):
            ff = open(os.path.join(path, filename))
            time = ff.read().rstrip("\n")
            timing = TimingInfo(filename, time)
            timings.append(timing)
    # now prepare the various plots:
    #  o parallel algos vs. time (for fixed parallel events)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    times = []
    algos = []
    for timing in timings:
        if timing.events == 1:
            times.append(timing.time)
            algos.append(timing.algos)
    ax.plot(algos, times)
    plt.xlabel("Max algos in parallel")
    plt.ylabel("runtime (s)")
    plt.title("Brunel / CPUCruncher profiling.")

    plt.savefig("timing.pdf")


##########################
if __name__ == "__main__":

    #  instancesVsTime("/afs/cern.ch/work/h/hegner/hive/Gaudi/GaudiHive/options/log.log")
    prepareTimingPlots()
