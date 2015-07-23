import os, random, string, json
import networkx as nx

from Gaudi.Configuration import INFO, DEBUG
from Configurables import GaudiSequencer, CPUCruncher

class UniformTimeValue(object):
    """A class to manage uniform algorithm timing"""

    def __init__(self, avgRuntime, varRuntime = 0):

        self.avgRuntime = avgRuntime
        self.varRuntime = varRuntime

    def get(self, algoName = ''):
        """Get time and its variance (in a tuple) for a given algorithm name"""

        return self.avgRuntime, self.varRuntime

class RealTimeValue(object):
    """A class to manage real algorithm timing"""

    def __init__(self, path, defaultTime, factor = 1):
        """
        defaultTime -- run time, assigned to an algorithm if no time is found in provided timing library
                       (and it will also be scaled by the 'factor' argument)
        """

        self.path = path
        self.factor = factor
        self.defaultTime = defaultTime # typically 0.05s
        self.varRuntime = 0

        self.file=open(self.path)
        self.timings=json.load(self.file)


    def get(self, algoName = ''):
        """Get time for a given algorithm name"""

        if algoName in self.timings:
            time = float(self.timings[algoName])
        else:
            capAlgoName = algoName[0].upper() + algoName[1:len(algoName)]

            if capAlgoName in self.timings:
                time = float(self.timings[capAlgoName])
            else:
                time = self.defaultTime
                print "WARNING: TimiNg for %s (or %s) not found in the provided library, using default one: %s" %(algoName,capAlgoName,time)

        time = time * self.factor

        return time, self.varRuntime

class UniformBooleanValue(object):

    def __init__(self, value):

        self.value = value

    def get(self):

        return self.value

class RndBiased10BooleanValue(object):
    """
    Provides randomly distributed boolean value with True taking only 10%.
    The distribution has only 276 values and is reproducible, if no pattern re-generation is requested.
    """

    # 276 values, biased as 90% to 10%
    builtinPattern = [False, False, True, False, False, False, False, False, False, False, False, False, False, False, False, True, False, False, True, False, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, False, False, False, False, False, False, True, False, True, False, True, False, False, False, False, False, False, False, False, True, False, True, False, False, True, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, False, True, False, False, False, False, True, True, False, False, False, False, False, False, False, False, False, False, True, True, False, True, False, False, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, False, False, False, True, False, False, False, True, False, False, False, False, False, False, False, False, True, False, False, False, False, False, False, False, False, False, False, False, True, False]
    pattern = []

    def __init__(self, useBuiltinPattern=True):

        if useBuiltinPattern:
            self.pattern = self.builtinPattern
        else:
            # 276 values, biased as 90% to 10%
            self.pattern = [False for i in range(248)] + [True for i in range(28)]
            random.shuffle(self.pattern)

        self.generator = self._create_generator(self.pattern)

    def _create_generator(self, pattern):

        for b in pattern:
            yield b

    def get(self):

        return next(self.generator)

    def get_pattern(self):

        return self.pattern


class CruncherSequence(object):
    """CPUCruncher-as-algorithm precedence sequence with real control flow and data flow dependencies of a Brunel workflow."""

    unique_sequencers=[]
    dupl_seqs={}
    OR_sequencers=[]
    unique_algos=[]
    dupl_algos={}

    unique_data_objects = []

    def __init__(self, timeValue, IOboolValue, cfgPath=None, dfgPath=None, showStat=False, algoDebug = False):
        """
        Keyword arguments:
        timeValue -- timeValue object to set algorithm execution time
        IOboolValue -- *BooleanValue object to set whether an algorithm has to experience IO-bound execution
        cfgPath -- absolute path to GRAPHML file with control flow dependencies (if None then built-in CF graph is used)
        dfgPath -- absolute path to GRAPHML file with data flow dependencies (if None then built-in DF graph is used)
        showStat -- print out statistics on precedence graph
        """

        self.timeValue = timeValue
        self.IOboolValue = IOboolValue

        if not cfgPath: cfgPath = "cf_dependencies.graphml"
        if not dfgPath: dfgPath = "data_dependencies.graphml"
        __location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__)))
        self.cfg = nx.read_graphml(os.path.join(__location__, cfgPath))
        self.dfg = nx.read_graphml(os.path.join(__location__, dfgPath))

        self.algoDebug = algoDebug

        # Generate control flow part
        self.sequencer = self._generate_sequence('GaudiSequencer/BrunelSequencer')

        avgRuntime, varRuntime = self.timeValue.get("DstWriter")
        dstwriter = CPUCruncher("DstWriter",
                                OutputLevel = DEBUG if self.algoDebug else INFO,
                                shortCalib = True,
                                varRuntime = varRuntime,
                                avgRuntime = avgRuntime)

        # Generate data flow part
        self._attach_io_data_objects("DstWriter", dstwriter)
        self.sequencer.Members += [dstwriter]

        if showStat:
            import pprint

            print "\n===== Statistics on Algorithms ====="
            print "Total number of algorithm nodes: ", len(self.unique_algos) + sum([self.dupl_algos[i]-1 for i in self.dupl_algos])
            print "Number of unique algorithms: ", len(self.unique_algos)
            print "  -->", len(self.dupl_algos), "of them being re-used with the following distribution: ", [self.dupl_algos[i] for i in self.dupl_algos]
            #pprint.pprint(dupl_algos)

            print "\n===== Statistics on Sequencers ====="
            print "Total number of sequencers: ", len(self.unique_sequencers) + sum([self.dupl_seqs[i]-1 for i in self.dupl_seqs])
            print "Number of unique sequencers: ", len(self.unique_sequencers)
            print "  -->", len(self.dupl_seqs), "of them being re-used with the following distribution: ", [self.dupl_seqs[i] for i in self.dupl_seqs]
            #pprint.pprint(dupl_seqs)
            print "Number of OR-sequencers: ", len(self.OR_sequencers)

            print "\n===== Statistics on DataObjects ====="
            print "Number of unique DataObjects: ", len(self.unique_data_objects)
            #pprint.pprint(self.unique_data_objects)
            print

    def get(self):

        return self.sequencer

    def _attach_io_data_objects(self, algo_name, algo):

        #print "===============================", algo_name, "========================================"

        i=0
        cache=[]
        for in_n, out_n in self.dfg.in_edges(algo_name):
            s = "input_" + str(i)
            addr = str(self.dfg.get_edge_data(in_n, out_n)['name'])#.lstrip('/Event/')
            #print "addr: ", addr, self.dfg.get_edge_data(in_n, out_n)['name']
            if not addr:
                #print "   empty output string, skipping"
                continue # if Path is "/Event". how to set it?
            if addr == '/Event': continue#addr = '/Event/TEMP'
            if addr not in cache:
                getattr(algo.Inputs, s).Path = addr
                if addr not in self.unique_data_objects: self.unique_data_objects.append(addr)
            else:
                pass#print "   has such an input already, skipping..", addr
            cache.append(addr)
            i += 1

        j=0
        cache=[]
        for out_n, in_n in self.dfg.out_edges(algo_name):
            s = "output_" + str(j)
            addr = str(self.dfg.get_edge_data(out_n, in_n)['name'])#.lstrip('/Event/')
            if not addr:
                #print "   empty output string, skipping"
                continue # if Path is "/Event". how to set it?
            if addr == '/Event': continue#addr = '/Event/TEMP'
            if addr not in cache:
                getattr(algo.Outputs, s).Path = addr
                if addr not in self.unique_data_objects: self.unique_data_objects.append(addr)
            else:
                pass#print "   has such an output already, skipping..", addr
            cache.append(addr)
            j += 1


    def _generate_sequence(self, name, seq=None):

        if not seq:
            seq = GaudiSequencer(name, ShortCircuit = False)
            avgRuntime, varRuntime = self.timeValue.get("Framework")
            framework = CPUCruncher("Framework",
                                    OutputLevel = DEBUG if self.algoDebug else INFO,
                                    shortCalib = True,
                                    varRuntime = varRuntime,
                                    avgRuntime = avgRuntime,
                                    SleepFraction = 0.5)
            framework.Outputs.output_0.Path = '/Event/DAQ/RawEvent'
            framework.Outputs.output_2.Path = '/Event/DAQ/ODIN'
            seq.Members += [framework]

        for n in self.cfg[name]:
            if '/' in n:
                algo_type, algo_name = n.split('/')
            else:
                algo_type = 'GaudiAlgorithm'
                algo_name = n

            if algo_type in ['GaudiSequencer', 'ProcessPhase']:
                if algo_name in ['RecoITSeq','RecoOTSeq','RecoTTSeq']: continue

                if n not in self.unique_sequencers:
                    self.unique_sequencers.append(n)
                else:
                    if n not in self.dupl_seqs: self.dupl_seqs[n] = 2
                    else: self.dupl_seqs[n] += 1

                seq_daughter=GaudiSequencer(algo_name, OutputLevel=INFO )
                if self.cfg.node[n].get('ModeOR') == 'True':
                    self.OR_sequencers.append(n)
                    seq_daughter.ModeOR = True
                #if self.cfg.node[n].get('Lazy') == 'False':
                #    print "Non-Lazy - ", n
                seq_daughter.ShortCircuit = False
                if seq_daughter not in seq.Members:
                    seq.Members += [seq_daughter]
                    # iterate deeper
                    self._generate_sequence(n,seq_daughter)
            else:
                #rndname = ''.join(random.choice(string.lowercase) for i in range(5))
                #if algo_name in unique_algos: algo_name = algo_name + "-" + rndname
                if n not in self.unique_algos:
                    self.unique_algos.append(n)
                else:
                    if n not in self.dupl_algos: self.dupl_algos[n] = 2
                    else: self.dupl_algos[n] += 1

                avgRuntime, varRuntime = self.timeValue.get(algo_name)
                algo_daughter = CPUCruncher(algo_name,
                                OutputLevel = DEBUG if self.algoDebug else INFO,
                                #Outputs = ['/Event/DAQ/ODIN'],
                                shortCalib = True,
                                varRuntime = varRuntime,
                                avgRuntime = avgRuntime,
                                SleepFraction = 0.5 if self.IOboolValue.get() else 0.)

                self._attach_io_data_objects(algo_name, algo_daughter)

                if algo_daughter not in seq.Members:
                    seq.Members += [algo_daughter]

        return seq
