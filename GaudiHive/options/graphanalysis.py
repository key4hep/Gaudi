from pygraph.classes.digraph import digraph
from pygraph.algorithms.cycles import find_cycle
from pygraph.algorithms.accessibility import connected_components
from pygraph.algorithms.critical import critical_path


def read_graph_from_json(filename):
    data = open(filename).read()
    workflow = eval(data)
    gr = digraph()
    # create all nodes
    for algo in workflow["algorithms"]:
        # to differentiate algos from equally named products
        name = algo["name"] + "_algo"
        gr.add_nodes([name])
        for input in algo["inputs"]:
            if input == "":
                input = "dummy"
            if not gr.has_node(input):
                gr.add_nodes([input])
            if not gr.has_edge((input, name)):
                gr.add_edge((input, name), wt=0)
        for output in algo["outputs"]:
            if output == "":
                output = "dummy"
            if not gr.has_node(output):
                gr.add_nodes([output])
            if not gr.has_edge((name, output)):
                # the output edge gets the weigth corresponding to the runtime
                gr.add_edge((name, output), wt=algo["runtimes_wall"][0] * 100)
    return gr


def analyze_and_fix_cycles(gr):
    has_loop = True
    n_cycles = 0
    while has_loop:
        cycle = find_cycle(gr)
        if len(cycle) == 0:
            has_loop = False
            continue
        n_cycles += 1
        print cycle
        print "Removed loop by deleting edge (%s,%s)" % (cycle[-1], cycle[0])
        gr.del_edge((cycle[-1], cycle[0]))
    print "\nIN TOTAL %i CYCLES\n" % (n_cycles)
    return n_cycles > 0  # whether it needed to fix cycles


def analyze_connected_componets(gr):
    cc = connected_components(gr)
    cc_size = {}
    for i in cc.values():
        cc_size[i] = 0
    for k, v in cc.iteritems():
        cc_size[v] = cc_size[v] + 1
    print "Connected components have the following size:"
#    for k,v in cc_size.iteritems():
#      print "%i : %i" %(k,v)
    print "NUMBER OF CONNECTED COMPONENTS: %i" % (len(cc_size.keys()))


def analyze_critical_path(gr):
    total_time = 0
    critical_time = 0
    cp = critical_path(gr)
    # calculate total time
    for edge in gr.edges():
        total_time += gr.edge_weight(edge)

    # calculate time of critical path
    edges = [tuple(cp[i:i + 2]) for i in range(0, len(cp))]
    for edge in edges:
        critical_time += gr.edge_weight(edge)

    print "Total time   : %s" % total_time
    print "Critical path: %s" % critical_time
    print "POSSIBLE SPEEDUP: %s" % (total_time / critical_time)


def print_graph_to_json(gr, filename):
    algorithms = {}  # still make it known to workflow
    known_names = set()
    for edge in gr.edges():
        if edge[0].endswith("_algo"):
            algoname = edge[0].rstrip("_algo")
            product = edge[1]
            reading = False
        else:
            algoname = edge[1].rstrip("_algo")
            product = edge[0]
            reading = True

        if algoname not in known_names:
            algorithms[algoname] = {"name":    algoname,
                                    "inputs":  [],
                                    "outputs": [],
                                    "runtimes": [1000],  # TODO dummy
                                    "runtimes_wall": [1000]  # TODO dummy
                                    }
            known_names.add(algoname)
        if reading:
            algorithms[algoname]["inputs"].append(product)
        else:
            algorithms[algoname]["outputs"].append(product)
            algorithms[algoname]["runtimes_wall"] = [
                gr.edge_weight(edge) / 100, ]
    out = open(filename, "w")
    algorithm_list = [item for item in algorithms.values()]
    workflow = {"algorithms": algorithm_list}
    out.write(workflow.__repr__())
    out.close()


##########################
if __name__ == "__main__":
    filename = "Athena.json"
    gr = read_graph_from_json(filename)

    # let's analysis and fix for loops:
    had_to_fix_cycles = analyze_and_fix_cycles(gr)
    # write file with fixed graph
    if had_to_fix_cycles:
        print_graph_to_json(gr, filename.replace(".json", "_loopfixed.json"))

    # see how many disconnected components are there
    analyze_connected_componets(gr)

    # let's check the minimum time required
    # and print achievable speedup
    analyze_critical_path(gr)
