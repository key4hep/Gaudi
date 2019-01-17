import os
import sys
import random
import string
import json
import networkx as nx

from Gaudi.Configuration import INFO
from Configurables import GaudiSequencer, CPUCruncher


def _buildFilePath(filePath):

    if not os.path.exists(filePath):
        __fullFilePath__ = os.path.realpath(
            os.path.join(
                os.environ.get('GAUDIHIVEROOT', ''), "data", filePath))
        if not os.path.exists(__fullFilePath__):
            print "\nERROR: invalid file path '%s'. It must be either absolute, or relative to '$GAUDIHIVEROOT/data/'." % filePath
            sys.exit(1)
    else:
        __fullFilePath__ = filePath

    return __fullFilePath__


class UniformTimeValue(object):
    """A class to manage uniform algorithm timing"""

    def __init__(self, avgRuntime, varRuntime=0):

        self.avgRuntime = avgRuntime
        self.varRuntime = varRuntime

    def get(self, algoName=''):
        """Get time and its variance (in a tuple) for a given algorithm name"""

        return self.avgRuntime, self.varRuntime


class RealTimeValue(object):
    """A class to manage real algorithm timing"""

    def __init__(self, path, defaultTime, factor=1):
        """
        defaultTime -- run time, assigned to an algorithm if no time is found in provided timing library
                       (and it will also be scaled by the 'factor' argument)
        """

        self.path = os.path.realpath(_buildFilePath(path))
        self.factor = factor
        self.defaultTime = defaultTime  # typically 0.05s
        self.varRuntime = 0

        self.file = open(self.path)
        self.timings = json.load(self.file)

    def get(self, algoName=''):
        """Get time for a given algorithm name"""

        if algoName in self.timings:
            time = float(self.timings[algoName])
        else:
            capAlgoName = algoName[0].upper() + algoName[1:len(algoName)]

            if capAlgoName in self.timings:
                time = float(self.timings[capAlgoName])
            else:
                time = self.defaultTime
                print "WARNING: TimiNg for %s (or %s) not found in the provided library, using default one: %s" % (
                    algoName, capAlgoName, time)

        time = time * self.factor

        return time, self.varRuntime


class UniformBooleanValue(object):
    def __init__(self, value):

        self.value = value

    def get(self):

        return self.value


class RndBiasedBooleanValue(object):
    """Provides randomly ordered set of boolean values with requested proportion of True and False."""

    def __init__(self, pattern, seed=None):
        """
        Keyword arguments:
        pattern -- either a dictionary describing proportion of True and False (e.g., {True:5,False:15}), or
                   a list/tuple containing a pattern to be used as-is (e.g., [False,True,True,False])
        seed -- an int, long or other hashable object to initialize random number generator (passed to random.shuffle as-is)
        """

        if isinstance(pattern, dict):
            proportion = pattern

            length = proportion[True] + proportion[False]
            if length <= 0:
                raise "ERROR: Wrong set length requested: %i " % length

            self.pattern = [False for i in range(proportion[False])
                            ] + [True for i in range(proportion[True])]

            if seed is not None:
                random.seed(seed)

            random.shuffle(self.pattern)

        elif isinstance(pattern, (list, tuple)):
            self.pattern = pattern
        else:
            raise "ERROR: unknown pattern type"

        self.generator = self._create_generator(self.pattern)

    def _create_generator(self, pattern):

        for b in pattern:
            yield b

    def get(self):

        return next(self.generator)

    def get_pattern(self):

        return self.pattern


class CruncherSequence(object):
    """Constructs the sequence tree of CPUCrunchers with provided control flow and data flow precedence rules."""

    unique_sequencers = []
    dupl_seqs = {}
    OR_sequencers = []
    unique_algos = []
    dupl_algos = {}

    unique_data_objects = []

    def __init__(self,
                 timeValue,
                 IOboolValue,
                 sleepFraction,
                 cfgPath,
                 dfgPath,
                 topSequencer,
                 showStat=False,
                 timeline=False,
                 outputLevel=INFO):
        """
        Keyword arguments:
        timeValue -- timeValue object to set algorithm execution time
        IOboolValue -- *BooleanValue object to set whether an algorithm has to experience IO-bound execution
        cfgPath -- relative to $GAUDIHIVEROOT/data path to GRAPHML file with control flow dependencies
        dfgPath -- relative to $GAUDIHIVEROOT/data path to GRAPHML file with data flow dependencies
        showStat -- print out statistics on precedence graph
        """

        self.timeValue = timeValue
        self.IOboolValue = IOboolValue
        self.sleepFraction = sleepFraction

        self.cfg = nx.read_graphml(_buildFilePath(cfgPath))
        self.dfg = nx.read_graphml(_buildFilePath(dfgPath))

        self.enableTimeline = timeline

        self.outputLevel = outputLevel

        # Generate control flow part
        self.sequencer = self._generate_sequence(topSequencer)

        if showStat:
            import pprint

            print "\n===== Statistics on Algorithms ====="
            print "Total number of algorithm nodes: ", len(
                self.unique_algos) + sum(
                    [self.dupl_algos[i] - 1 for i in self.dupl_algos])
            print "Number of unique algorithms: ", len(self.unique_algos)
            print "  -->", len(
                self.dupl_algos
            ), "of them being re-used with the following distribution: ", [
                self.dupl_algos[i] for i in self.dupl_algos
            ]
            # pprint.pprint(dupl_algos)

            print "\n===== Statistics on Sequencers ====="
            print "Total number of sequencers: ", len(
                self.unique_sequencers) + sum(
                    [self.dupl_seqs[i] - 1 for i in self.dupl_seqs])
            print "Number of unique sequencers: ", len(self.unique_sequencers)
            print "  -->", len(
                self.dupl_seqs
            ), "of them being re-used with the following distribution: ", [
                self.dupl_seqs[i] for i in self.dupl_seqs
            ]
            # pprint.pprint(dupl_seqs)
            print "Number of OR-sequencers: ", len(self.OR_sequencers)

            print "\n===== Statistics on DataObjects ====="
            print "Number of unique DataObjects: ", len(
                self.unique_data_objects)
            # pprint.pprint(self.unique_data_objects)
            print

    def get(self):

        return self.sequencer

    def _declare_data_deps(self, algo_name, algo):
        """ Declare data inputs and outputs for a given algorithm. """

        # Declare data inputs
        for inNode, outNode in self.dfg.in_edges(algo_name):
            dataName = inNode
            if dataName not in self.unique_data_objects:
                self.unique_data_objects.append(dataName)

            if dataName not in algo.inpKeys:
                algo.inpKeys.append(dataName)

        # Declare data outputs
        for inNode, outNode in self.dfg.out_edges(algo_name):
            dataName = outNode
            if dataName not in self.unique_data_objects:
                self.unique_data_objects.append(dataName)

            if dataName not in algo.outKeys:
                algo.outKeys.append(dataName)

    def _generate_sequence(self, name, seq=None):
        """ Assemble the tree of sequencers. """

        if not seq:
            seq = GaudiSequencer(name, ShortCircuit=False)

        for n in self.cfg[name]:
            if '/' in n:
                algo_type, algo_name = n.split('/')
            else:
                algo_type = 'GaudiAlgorithm'
                algo_name = n

            if algo_type in ['GaudiSequencer', 'AthSequencer', 'ProcessPhase']:
                if algo_name in ['RecoITSeq', 'RecoOTSeq', 'RecoTTSeq']:
                    continue

                if n not in self.unique_sequencers:
                    self.unique_sequencers.append(n)
                else:
                    if n not in self.dupl_seqs:
                        self.dupl_seqs[n] = 2
                    else:
                        self.dupl_seqs[n] += 1

                seq_daughter = GaudiSequencer(algo_name, OutputLevel=INFO)
                if self.cfg.node[n].get('ModeOR') == 'True':
                    self.OR_sequencers.append(n)
                    seq_daughter.ModeOR = True
                # if self.cfg.node[n].get('Lazy') == 'False':
                #    print "Non-Lazy - ", n
                seq_daughter.ShortCircuit = False
                if seq_daughter not in seq.Members:
                    seq.Members += [seq_daughter]
                    # iterate deeper
                    self._generate_sequence(n, seq_daughter)
            else:
                #rndname = ''.join(random.choice(string.lowercase) for i in range(5))
                #if algo_name in unique_algos: algo_name = algo_name + "-" + rndname
                if n not in self.unique_algos:
                    self.unique_algos.append(n)
                else:
                    if n not in self.dupl_algos:
                        self.dupl_algos[n] = 2
                    else:
                        self.dupl_algos[n] += 1

                avgRuntime, varRuntime = self.timeValue.get(algo_name)
                algo_daughter = CPUCruncher(
                    n,
                    OutputLevel=self.outputLevel,
                    varRuntime=varRuntime,
                    avgRuntime=avgRuntime,
                    SleepFraction=self.sleepFraction
                    if self.IOboolValue.get() else 0.,
                    Timeline=self.enableTimeline)

                self._declare_data_deps(n, algo_daughter)

                if algo_daughter not in seq.Members:
                    seq.Members += [algo_daughter]

        return seq
