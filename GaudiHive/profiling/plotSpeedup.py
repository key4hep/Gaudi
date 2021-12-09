#! /usr/bin/env python
#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Script that fetches all the logfiles from disk and reads the timings.
Parameters can be changed according to the working points considered.
Some parameters of the plots are hardcoded.
"""
from __future__ import print_function

from ROOT import *

n_algos_in_flight_l = [1, 2, 5, 7, 10, 16, 20, 22]
n_evts_in_flight_l = [1, 2, 4, 6, 7, 8, 9, 11, 13, 14, 15]
colour_l = [kRed, kBlue, kOrange, kGreen, kMagenta, kCyan] * 2
line_style_l = [1] * 6 + [2] * 6
cloneFlag_l = [True, False]
"""
To be filled with files on disk of the form
timing_measurement_BrunelScenario_n150_eif2_aif20_nthreads24_cFalse_dqFalse.log
"""
filename_scheleton = (
    "timing_measurement_BrunelScenario_n150_eif%s_aif%s_nthreads24_c%s_dqFalse.log"
)


def getRuntime(n_algos_in_flight, n_evts_in_flight, cloneFlag):
    filename = filename_scheleton % (n_evts_in_flight, n_algos_in_flight, cloneFlag)
    print(filename)
    rt = 0.0
    for line in open(filename, "r").readlines():
        rt = float(line[:-1])
        # print filename
        # print rt
    return rt


"""
Dictionary with
o Key = [n_algos_in_flight,  n_evts_in_flight, cloneFlag]
o Val = Runtime
"""


def getRuntimes():
    runtimes = {}
    for n_algos_in_flight in n_algos_in_flight_l:
        for n_evts_in_flight in n_evts_in_flight_l:
            for cloneFlag in cloneFlag_l:
                rt = getRuntime(n_algos_in_flight, n_evts_in_flight, cloneFlag)
                runtimes[n_algos_in_flight, n_evts_in_flight, cloneFlag] = rt
    return runtimes


def getGraphPoints(n_evts_in_flight, cloneFlag, runtimes):
    points = []
    for key, rt in runtimes.items():
        this_n_algos_in_flight, this_n_evts_in_flight, this_cloneFlag = key
        if this_n_evts_in_flight == n_evts_in_flight and this_cloneFlag == cloneFlag:
            points.append((this_n_algos_in_flight, rt))
    return sorted(points)


def getSingleGraph(n_evts_in_flight, cloneFlag, runtimes, colour, style):
    points = getGraphPoints(n_evts_in_flight, cloneFlag, runtimes)
    graph = TGraph(len(points))
    graph.GetXaxis().SetTitle("Maximum # in flight algos")
    graph.GetXaxis().SetRangeUser(0, 23)
    graph.GetYaxis().SetTitle("Runtime [s]")
    graph.GetYaxis().SetTitleOffset(1.45)
    graph.GetYaxis().SetRangeUser(0.1, 275)
    graph.GetYaxis().SetNdivisions(12, 5, 1)
    graph.SetLineWidth(3)
    graph.SetLineColor(colour)
    graph.SetMarkerColor(colour)
    graph.SetLineStyle(style)
    graph.SetFillColor(kWhite)
    point_n = 0
    # print points
    for x, y in points:
        graph.SetPoint(point_n, x, y)
        point_n += 1
    return graph


def make_plot(runtimes, cloneFlag):
    title = "Brunel 150 events"
    clone_string = ""
    if cloneFlag:
        clone_string = "_clone"
        title += " (Cloning)"
    plotname = "runtime%s.pdf" % clone_string

    canvas = TCanvas(plotname, "plot", 500, 400)
    canvas.SetGrid()
    canvas.cd()

    graphs = []
    first = True
    for colour, n_evts_in_flight, line_style in zip(
        colour_l, n_evts_in_flight_l, line_style_l
    ):
        print(n_evts_in_flight)
        graph = getSingleGraph(
            n_evts_in_flight, cloneFlag, runtimes, colour, line_style
        )
        opts = "LSame"
        if first:
            opts = "AL"
        first = False
        graph.SetTitle(title)
        graph.Draw(opts)
        graphs.append(graph)

    # Make Legend
    legend = TLegend(0.499, 0.45, 0.9, 0.9, "# Parallel Events")
    legend.SetTextSize(0.04)
    legend.SetFillColor(kWhite)
    # evil
    for graph, n in zip(graphs, n_evts_in_flight_l):
        legend.AddEntry(graph, "%s" % n)
    legend.Draw()

    # Add some text
    l = TLatex(0.13, 0.16, "#font[12]{#scale[.8]{24 Threads}}")
    l.SetNDC()
    l.Draw()

    dummy = raw_input("Press enter to save...")
    canvas.Print(plotname)


# -------------------------------------------------------------------------------
runtimes = getRuntimes()

make_plot(runtimes, True)

make_plot(runtimes, False)
