from ROOT import *

'''
Script to parse all the logs and produce the speedup plot.
Usage:
plotSpeedupsPyRoot.py --> vanilla plot
plotSpeedupsPyRoot.py 1 --> HT scaled plot
The variable fname_template contains the template of the file names to be 
opened and parsed and is FIXED for all the logs.
The word "seconds" is looked for in the log and then the total runtime of the 
event loop is extracted. This allows to discard the time spent in calibration 
and so on.
'''

# Configuration ----------------------------------------------------------------
fname_template = "measurement_BrunelScenario_n100_eif%s_aif100_nthreads%s_c%s_dqFalse_v5.log"
# Number of events in flight
neif_l = [1, 2, 3, 5, 20, 30]
# Number of Threads
nts = [2, 3, 5, 10, 11, 12, 13, 15, 23]
# Clone Flag
cFlags = ["True", "False"]

ScalarTime = 1640.87

# Style
LegendDrawOpts = "lp"
LineColours = [kRed,
               kBlue,
               kGreen + 2,
               kOrange,
               kPink + 10,
               kViolet + 10]
MarkerStyles = [kFullCircle,
                kOpenCross,
                kFullTriangleUp,
                kOpenStar,
                kFullCross,
                kOpenCircle]
MarkerSize = 4
LineWidth = 6
LineStyle = 7
graph_counter = 0

PhCores = 11
TotalCores = 24
HtCoreWeight = 0.4

LabelsFont = 12
LabelsSize = .6
#--------------------


def scaleCores(n_threads):
    effective_n_threads = n_threads
    if effective_n_threads > PhCores:
        ht_cores = n_threads - PhCores
        effective_n_threads = PhCores + ht_cores * HtCoreWeight
    return effective_n_threads

#--------------------


def getText(x, y, text, scale, angle, colour, font):
    lat = TLatex(x, y, "#scale[%s]{#color[%s]{#font[%s]{%s}}}" % (
        scale, colour, font, text))
    if angle != 0.:
        lat.SetTextAngle(angle)
    return lat

#--------------------


def formatGraphs(graph, graphc):
    global graph_counter
    graphc.SetLineStyle(LineStyle)
    graphs = (graph, graphc)
    for g in graphs:
        g.SetLineWidth(LineWidth)
        g.SetMarkerSize(MarkerSize)
        g.SetMarkerStyle(MarkerStyles[graph_counter])
        g.SetLineColor(LineColours[graph_counter])
        g.SetMarkerColor(LineColours[graph_counter])
    graph_counter += 1

#--------------------


def createFname(neif, nt, cFlag):
    return fname_template % (neif, nt, cFlag)

#--------------------


def xtractTiming(neif, nt, cFlag):
    filename = createFname(neif, nt, cFlag)
    ifile = open(filename, "r")
    seconds = -1
    for line in ifile:
        if "seconds" in line:
            line = line[:-1]
            seconds = float(line.split(" ")[-1])
            break
    ifile.close()
    if seconds == -1:
        seconds = xtractTiming(neif, nts[nts.index(nt) - 1], cFlag)
    return seconds

#--------------------


import sys
scaleThreads = False
if len(sys.argv) > 1:
    scaleThreads = True


# main loop: just printouts
for neif in neif_l:
    print "Events in flight: %s" % neif
    for tn in nts:
        print "%s %s %s" % (tn, xtractTiming(
            neif, tn, False), xtractTiming(neif, tn, True))


len_nt = len(nts) + 1
# Prepare ideal speedup graph
idealSpeedup = TGraph(2)
idealSpeedup.SetPoint(0, 1, 1)
idealSpeedup.SetPoint(1, TotalCores, TotalCores)
scaled_s = ""
if scaleThreads:
    scaled_s = " (scaled for HT)"
idealSpeedup.SetTitle(
    "GaudiHive Speedup (Brunel, 100 evts);Thread Pool Size%s;Speedup wrt Serial Case" % scaled_s)
idealSpeedup.SetLineWidth(4)
idealSpeedup.SetLineColor(kGray - 2)
idealSpeedup.SetLineStyle(2)

# Main Loop: fill all graphs
neif_graphs = []
for neif in neif_l:  # One graph per number of events in flight
    graph = TGraph(len_nt)
    graph.SetName("%s" % neif)
    graph.SetPoint(0, 1, 1)

    graphc = TGraph(len_nt)
    graphc.SetName("%s clone" % neif)
    graphc.SetPoint(0, 1, 1)
    counter = 1
    for tn in nts:
        scaled_tn = tn
        if scaleThreads:
            scaled_tn = scaleCores(tn)
        time = xtractTiming(neif, tn, False)
        graph.SetPoint(counter, scaled_tn, ScalarTime / time)
        timec = xtractTiming(neif, tn, True)
        graphc.SetPoint(counter, scaled_tn, ScalarTime / timec)
        counter += 1
    formatGraphs(graph, graphc)
    neif_graphs.append([neif, graph, graphc])

neif_graphs.reverse()

# Now that all are complete, let's make the plot
canvas = TCanvas("Speedup", "Speedup", 2048, 1800)
canvas.cd()
canvas.SetGrid()
idealSpeedup.Draw("APL")
idealSpeedup.GetYaxis().SetRangeUser(0.1, TotalCores + 1)  # only one 0

# Line
line = TLine(11, 0, 11, 25)
line.SetLineColor(kRed)
line.SetLineWidth(4)
line.SetLineStyle(2)
line.Draw()

for neif, graph, graphc in neif_graphs:
    graph.Draw("SamePL")
    graphc.Draw("SamePL")

# Prepare Legend
legend = TLegend(.1, .45, .38, .9)
legend.SetFillColor(kWhite)
legend.SetHeader("# Simultaneous Evts")
for neif, graph, graphc in neif_graphs:
    legend.AddEntry(graph, "%s" % neif, LegendDrawOpts)
    legend.AddEntry(graphc, "%s (clone)" % neif, LegendDrawOpts)
legend.Draw()

# Labels
ph_cores = getText(10.5, 15, "Physical Cores", LabelsSize, 90, 2, LabelsFont)
ph_cores.Draw()
ht_cores = getText(12., 15, "Hardware Threaded Regime",
                   LabelsSize, 90, 2, LabelsFont)
ht_cores.Draw()
is_text = getText(16, 16.5, "Ideal (linear) Speedup",
                  LabelsSize, 45, 918, LabelsFont)
is_text.Draw()
ht_weight = 0
if scaleThreads:
    ht_weight = getText(
        18.5, 8, "#splitline{Hardware threaded}{cores weight: %s}" % HtCoreWeight, LabelsSize, 0, 600, LabelsFont)
    ht_weight.Draw()

if scaleThreads:
    scaled_s = "_HTScaled"
canvas.Print("GaudiHivePerfBrunelAllPoints%s.png" % scaled_s)
