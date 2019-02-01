from ROOT import *
import sys
import re
"""
Produces the backlog plot, parsing the output of the EventLoopManager.
Lines with the pattern "Event backlog" are looked for.
Events in flight are looked for as well.
"""

# configuration

NEventsInFlight = -1
NThreads = -1

LineStyles = [1, 2]
LineWidth = 3
Colors = [kBlue, kGreen + 2]
MarkerStyles = [kOpenCircle, kFullCircle]
MarkesSize = 1.5

graphCounter = 0
maxY = -1

LegendDrawOpts = "lp"


def parseLog(logfilename):
    # a line looks like
    #"HiveSlimEventLoopMgr  SUCCESS Event backlog (max= 3, min= 0 ) = 3"
    global NEventsInFlight
    global NThreads
    ifile = open(logfilename, "r")
    lines = ifile.readlines()
    ifile.close()
    content = []
    for line in lines:
        if "Event backlog" in line:
            content.append(
                re.match(".* \(max= ([0-9]*), min= ([0-9]*) \) = ([0-9]*).*",
                         line).groups())
        elif "Running with" in line:
            NEventsInFlight, NThreads = map(
                int,
                re.match(".*Running with ([0-9]*).* ([0-9]*) threads",
                         line).groups())

    return content


def createGraph(max_min_blog_vals):
    global graphCounter
    global maxY
    graph = TGraph(len(max_min_blog_vals))
    counter = 0
    for maxn, minn, blog in max_min_blog_vals:
        blog = float(blog)
        graph.SetPoint(counter, counter + 1, float(blog))
        if maxY < blog:
            maxY = blog
        counter += 1

    graph.SetMarkerSize(MarkesSize)
    graph.SetMarkerStyle(MarkerStyles[graphCounter])
    graph.SetMarkerColor(Colors[graphCounter])
    graph.SetLineWidth(LineWidth)
    graph.SetLineColor(Colors[graphCounter])
    graph.SetLineStyle(LineStyles[graphCounter])

    graphCounter += 1

    return graph


def createInFlightGraph(nevts):
    global NEventsInFlight
    graph = TGraph(2)
    graph.SetPoint(0, 0., float(NEventsInFlight))
    graph.SetPoint(1, float(nevts) + 1, float(NEventsInFlight))
    graph.SetLineWidth(3)
    graph.SetLineColor(kRed)
    graph.SetLineStyle(2)
    graph.SetTitle(
        "GaudiHive Backlog (Brunel, 100 evts);Events Finished;Event Backlog")
    print NEventsInFlight
    return graph


def getText(x, y, text, scale, angle, colour, font, NDC=False):
    lat = TLatex(
        float(x), float(y),
        "#scale[%s]{#color[%s]{#font[%s]{%s}}}" % (scale, colour, font, text))
    if (NDC):
        lat.SetNDC()
    if angle != 0.:
        lat.SetTextAngle(angle)
    return lat


def doPlot(logfilename, logfilename_copy):
    global NEventsInFlight
    global maxY
    global NThreads
    vals = parseLog(logfilename)
    vals_c = parseLog(logfilename_copy)
    n_vals = len(vals)
    inFlightgraph = createInFlightGraph(n_vals)
    graph = createGraph(vals)
    graph_c = createGraph(vals_c)

    canvas = TCanvas("Backlog", "Backlog", 1100, 900)
    canvas.cd()
    canvas.SetGrid()
    inFlightgraph.Draw("APL")
    inFlightgraph.GetYaxis().SetRangeUser(0., maxY * 1.2)
    inFlightgraph.GetXaxis().SetRangeUser(0., float(n_vals + 1))
    graph.Draw("PLSame")
    graph_c.Draw("PLSame")

    # Labels
    eventInFlightLabel = getText(
        float(n_vals + 1) * 1.03, NEventsInFlight,
        "#splitline{# Simultaneous}{       Events}", .6, 270, 2, 12)
    eventInFlightLabel.Draw()
    nThreadsLabel = getText(.15, .7, "%s Threads" % NThreads, .6, 0, 2, 12,
                            True)
    nThreadsLabel.Draw()

    # Build a Legend
    legend = TLegend(.7, .75, .9, .9)
    legend.SetFillColor(kWhite)
    legend.SetHeader("Algo Management")
    legend.AddEntry(graph, "No Cloning", LegendDrawOpts)
    legend.AddEntry(graph_c, "Cloning enabled", LegendDrawOpts)
    legend.Draw()

    a = raw_input("press enter to continue")

    canvas.Print("EventBacklog.png")


if __name__ == "__main__":
    argc = len(sys.argv)
    if argc != 3:
        print "Usage: plotBacklogPyRoot.py logfilename logfilename_copy"
        sys.exit(1)
    doPlot(sys.argv[1], sys.argv[2])
