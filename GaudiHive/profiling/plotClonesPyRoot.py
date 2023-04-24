#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import re
import sys

from ROOT import TCanvas, TGraph, TLatex, kBlue, kFullStar

"""
Prepare the clones plot.
Parses the end of the output file, printed by the destructors of the CPUCruncher.
A line with the pattern "Summary: name=" is looked for.
The number of Events in flight is parsed as well.
"""

# configure
Color = kBlue
MarkerStyle = kFullStar
MarkerSize = 3.8

NEventsInFlight = 0
NThreads = -1


def parseLog(logfilename):
    global NEventsInFlight
    global NThreads
    ifile = open(logfilename, "r")
    lines = ifile.readlines()
    vals = []
    for line in lines:
        if "Summary: name=" in line:
            runtime_nclones = map(
                float,
                re.match(
                    ".* avg_runtime= ([0-9]*.[0-9]*|[0-9]*.[0-9]*e-[0-9]*) n_clones= ([0-9]).*",
                    line,
                ).groups(),
            )
            vals.append(runtime_nclones)
        elif "Running with" in line:
            NEventsInFlight, NThreads = map(
                int,
                re.match(
                    ".* Running with ([0-9]*) parallel events.*algorithms, ([0-9]*) threads",
                    line,
                ).groups(),
            )

    return vals


def createGraph(vals):
    graph = TGraph(len(vals))
    counter = 0
    for runtime, nclones in vals:
        graph.SetPoint(counter, runtime, nclones)
        counter += 1

    graph.SetMarkerStyle(MarkerStyle)
    graph.SetMarkerSize(MarkerSize)
    graph.SetMarkerColor(Color)
    graph.SetTitle(
        "GaudiHive Speedup (Brunel, 100 evts);Algorithm Runtime [s];Number of Clones"
    )
    return graph


def getText(x, y, text, scale, angle, colour, font, NDC=False):
    lat = TLatex(
        float(x),
        float(y),
        "#scale[%s]{#color[%s]{#font[%s]{%s}}}" % (scale, colour, font, text),
    )

    lat.SetNDC(NDC)
    if angle != 0.0:
        lat.SetTextAngle(angle)
    return lat


def getCountLatexes(vals, xmax):
    # print vals
    def getNclones(runtime_nclones):
        return runtime_nclones[1]

    max_nclones = int(max(vals, key=getNclones)[1])

    latexes = []
    for i in range(1, max_nclones + 1):
        n_algos = len(filter(lambda runtime_nclones: runtime_nclones[1] == i, vals))
        latexes.append(getText(xmax * 1.01, i, n_algos, 0.7, 0, 600, 12))

    label = getText(0.95, 0.55, "Total", 0.8, 270, 600, 12, True)

    latexes.append(label)

    return latexes


def doPlot(logfilename):
    global NEventsInFlight
    global NThreads
    vals = parseLog(logfilename)
    graph = createGraph(vals)
    nalgorithms = len(vals)

    canvas = TCanvas("Clones", "Clones", 1024, 768)
    canvas.cd()
    canvas.SetGrid()
    graph.Draw("AP")

    # Latex

    countLatexes = getCountLatexes(vals, graph.GetXaxis().GetXmax())
    map(lambda latex: latex.Draw(), countLatexes)
    evtsIf = getText(
        0.6,
        0.365,
        "#splitline{#splitline{%s Simultaneous Events}{%s Threads}}{%s Algorithms}"
        % (NEventsInFlight, NThreads, nalgorithms),
        0.8,
        0,
        2,
        12,
        True,
    )
    evtsIf.Draw()

    input("press enter to continue")
    canvas.Print("PlotClones.png")


if __name__ == "__main__":
    argc = len(sys.argv)
    if argc != 2:
        print("Usage: plotClonesPyRoot.py logfilename")
        sys.exit(1)
    doPlot(sys.argv[1])
