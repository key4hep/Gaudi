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
      name = algo["name"]+"_algo" #to differentiate algos from equally named products
      gr.add_nodes([name])
      for input in algo["inputs"]:
        if input == "": input = "dummy"
        if not gr.has_node(input):         
          gr.add_nodes([input])
        if not gr.has_edge((input,name)):
          gr.add_edge((input,name),wt=0)
      for output in algo["outputs"]:
        if output == "":output ="dummy" 
        if not gr.has_node(output):  
          gr.add_nodes([output])
        if not gr.has_edge((name,output)):
          #the output edge gets the weigth corresponding to the runtime
          gr.add_edge((name,output),wt=algo["runtimes_wall"][0]*100)
    return gr

def analyze_and_fix_cycles(gr):
    has_loop = True
    n_cycles = 0
    while has_loop:
      cycle = find_cycle(gr)
      if len(cycle) == 0 : 
        has_loop = False
        continue
      n_cycles += 1
      print cycle
      print "Removed loop by deleting edge (%s,%s)" %(cycle[-1],cycle[0])
      gr.del_edge((cycle[-1],cycle[0]))
    print "\nIN TOTAL %i CYCLES\n" %(n_cycles)

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
    print "NUMBER OF CONNECTED COMPONENTS: %i" %(len(cc_size.keys()))


def analyze_critical_path(gr):
    total_time = 0
    critical_time = 0
    cp = critical_path(gr)
    # calculate total time
    for edge in gr.edges():
     total_time += gr.edge_weight(edge)
    
    # calculate time of critical path
    edges = [tuple(cp[i:i+2]) for i in range(0, len(cp))]
    for edge in edges:
      critical_time += gr.edge_weight(edge)

    print "Total time   : %s" %total_time
    print "Critical path: %s" %critical_time
    print "POSSIBLE SPEEDUP: %s" %(total_time/critical_time)
 

##########################  
if __name__ == "__main__":
  filename = "lhcb.json"
  gr = read_graph_from_json(filename)
  
  # let's analysis and fix for loops:
  analyze_and_fix_cycles(gr)

  # see how many disconnected components are there
  analyze_connected_componets(gr)

  # let's check the minimum time required
  # and print achievable speedup
  analyze_critical_path(gr)
