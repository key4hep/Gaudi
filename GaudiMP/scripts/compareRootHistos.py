#! /usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
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
from optparse import OptionParser

# Well known trick
backupArgv = sys.argv[:]
sys.argv = []
from ROOT import TFile

sys.argv = backupArgv

# The list of histograms not to check. Expressed as regexps
gRegexBlackList = []
histos = ["TH1D", "TH1F", "TH2D", "TH2F", "TProfile"]
ref = "REFERENCE"
test = "TEST"

# =============================================================================
# Method   : rec( o, path=None, lst=None )
#
# @param o    : a ROOT object
# @param path : a string like a transient store path; ie '/stat/CaloPIDs/ECALPIDE'
# @param lst  : a list to hold (path, object) tuples
#
# function    : recursively pull apart a ROOT file, making a list of (path, TObject) tuples
#               This is done by GetListOfKeys method, which lets one work down through directories
#               until you hit the Histo at the end of the path.  The list of tuples is returned
#


def rec(o, path=None, lst=None):
    if not path:
        path = "/stat"
        lst = []
    else:
        path = path + "/" + o.GetName()
    lst.append((path, o))
    if "GetListOfKeys" in dir(o):
        keys = o.GetListOfKeys()
        for k in keys:
            name = k.GetName()
            rec(o.Get(name), path, lst)
    else:
        pass
    return lst


# =============================================================================

# =============================================================================
# Method   : composition( t )
#
# @param t : a tuple of ( type, d ) where type is either 'REFERENCE' or 'TEST'
#            and d is a dictionary of ROOT objects, with each key = ROOT path
#
# function : deduce the composition, (objects/histos) counts
#


def composition(t):
    typ, d = t
    hists = 0
    objs = 0
    for k in d.keys():
        if d[k].__class__.__name__ in histos:
            hists += 1
        else:
            objs += 1
    return objs, hists


# =============================================================================

# =============================================================================
# Method        : comparePaths( t1, t2 )
#
# @param t1, t2 : a tuple of ( type, d ) where type is either 'REFERENCE' or 'TEST'
#                 and d is a dictionary of ROOT objects, with each key = ROOT path
#
# function      : compare the paths between the two histo files.  If the files are identical, they
#                 should have the same set of paths.  The Test file should definitely have the
#                 same paths as the Reference.  Perhaps the Reference file will have some more paths due
#                 to extra histos added as part of Application Sequencer finalisation
#                 Arguments t1 and t2 are checked and the test/reference auto-detected
#


def comparePaths(t1, t2):
    if t1[0] == ref:
        ds = t1[1]
        dp = t2[1]
    elif t2[0] == ref:
        ds = t2[1]
        dp = t1[1]
    else:
        print("Neither tuple is Reference Root file reference?")
        return

    dsks = list(ds.keys())
    dpks = list(dp.keys())
    dsks.sort()
    dpks.sort()

    sset = set(dsks)
    pset = set(dpks)
    os, hs = composition((ref, ds))
    op, hp = composition((test, dp))
    print("\n" + "=" * 80)
    print("Comparison of Paths : Reference vs Test ROOT files")
    print("-" * 80)
    print(
        f"Number of paths in Reference file : {len(dsks)} (objects, histos) = ( {os}, {hs} )"
    )
    print(
        f"Number of paths in Test file : {len(dpks)} (objects, histos) = ( {op}, {hp} )"
    )
    matching = sset.intersection(pset)
    matchingHistos = 0
    for n in matching:
        if ds[n].__class__.__name__ in histos:
            matchingHistos += 1
    print(f"\nMatching paths                 : {len(matching)}")
    uSer = sset - pset
    # work out histos unique to test file
    uniqueReferenceHistos = 0
    for n in uSer:
        if ds[n].__class__.__name__ in histos:
            uniqueReferenceHistos += 1
    print(
        f"Paths unique to Reference file : {len(uSer)} ( {uniqueReferenceHistos} Histos )"
    )
    if uSer:
        for n in uSer:
            print(f"\t{ds[n]} : \t{n}")
    uPar = pset - sset
    uniqueTestHistos = 0
    for n in uPar:
        if dp[n].__class__.__name__ in histos:
            uniqueTestHistos += 1
    print(f"Paths unique to Test file : {len(uPar)} ( {uniqueTestHistos} Histos )")
    if uPar:
        for n in uPar:
            print(f"\t{dp[n]} : \t{n}")
    print(f"Matching Histos to test : {matchingHistos}")
    print("=" * 80 + "\n")
    return (
        ((os, hs), (op, hp)),
        (uSer, uniqueReferenceHistos),
        (uPar, uniqueTestHistos),
        matchingHistos,
    )


# =============================================================================

# =============================================================================
# Method        : bin2binIdentity(h1,h2)
#
# @param h1, h2 : The two histogtams to compare
# function      : Return the number of different bins


def bin2binIdentity(h1, h2):
    def getNbins(h):
        biny = h.GetNbinsY()
        if biny > 1:
            biny += 1
        binz = h.GetNbinsZ()
        if binz > 1:
            binz += 1
        return (h.GetNbinsX() + 1) * (biny) * (binz)

    nbins = getNbins(h1)
    diffbins = 0
    for ibin in range(0, nbins):
        h1bin = h1.GetBinContent(ibin)
        h2bin = h2.GetBinContent(ibin)
        diffbins += h1bin != h2bin
    return diffbins


# =============================================================================
# Method        : compareHistos( t1, t2 )
#
# @param t1, t2 : a tuple of ( type, d ) where type is either 'REFERENCE' or 'TEST'
#                 and d is a dictionary of ROOT objects, with each key = ROOT path
#
# function      : compare the histograms in Reference/Test ROOT files.  First, go through each
#                 dict to collect the histos (ignore TDirectory objects, etc).  Then the histos
#                 in the test file (experimental) are compared to their equivalents in the
#                 reference file (definitely correct) using 3 methods.
#                 1) The entries are checked, they should be equal
#                 2) If entries are equal, check the Integral(); should be equal
#                 3) If integrals are equal, check the KolmogorovTest() ; should be 1
#                 4) If identity flag is there and KS test is performed, perform bin2bin identity test
#                 Arguments t1 and t2 are checked and the test/reference auto-detected
#
def compareHistos(t1, t2, state, checkBin2BinIdentity):
    (
        ((referenceObjects, referenceHistos), (parallObjects, parallHistos)),
        (uniqueSerPaths, uniqueSerHistos),
        (uniqueParPaths, uniqueParHistos),
        mh,
    ) = state

    # deduce which one is test, which reference
    if t1[0] == ref:
        ds = t1[1]
        dp = t2[1]
    elif t2[0] == ref:
        ds = t2[1]
        dp = t1[1]
    else:
        print("Neither tuple is Reference Root file reference?")
        return

    # histocount, objectcount for test/reference
    hcp = 0
    pHistos = []
    hcs = 0
    sHistos = []

    omit = [re.compile(regex) for regex in gRegexBlackList]

    # find the histos in the reference file
    for k in ds.keys():
        if not any(regex.search(k) is not None for regex in omit):
            if ds[k].__class__.__name__ in histos:
                hcs += 1
                sHistos.append(k)
    # same for test
    for k in dp.keys():
        if not any(regex.search(k) is not None for regex in omit):
            if dp[k].__class__.__name__ in histos:
                hcp += 1
                pHistos.append(k)

    cEntries = 0
    xEntries = 0
    diffEntries = []
    xIntegrals = 0
    diffIntegrals = []
    passedKol = 0
    failedKol = 0
    diffKols = []
    passedIdentity = 0
    failedIdentity = 0
    diffIdentity = []
    identityDiffBins = {}
    kTested = 0
    kTestResults = {}
    notfound = 0
    integralMatch = 0
    otherTest = 0
    zeroIntegralMatch = 0
    for h in sHistos:
        if h in pHistos:
            # matching histos to check
            cEntries += 1
            sh = ds[h]
            ph = dp[h]
            # first check entries
            if sh.GetEntries() != ph.GetEntries():
                diffEntries.append(h)
                xEntries += 1
                continue
            # check for (non-zero sum of bin error) && (non-zero integrals) for K-Test
            sBinError = 0.0
            pBinError = 0.0
            for i in range(sh.GetNbinsX()):
                sBinError += sh.GetBinError(i)
            for i in range(ph.GetNbinsX()):
                pBinError += ph.GetBinError(i)
            sint = sh.Integral()
            pint = ph.Integral()
            doKS = (bool(sint) and bool(pint)) and (sBinError > 0 and pBinError > 0)
            if checkBin2BinIdentity and doKS:
                diffBins = bin2binIdentity(sh, ph)
                if diffBins == 0:
                    passedIdentity += 1
                else:
                    failedIdentity += 1
                    diffIdentity.append(h)
                    identityDiffBins[h] = diffBins
            if (bool(sint) and bool(pint)) and (sBinError > 0 and pBinError > 0):
                kTested += 1
                kTest = sh.KolmogorovTest(ph)
                kTestResults[h] = kTest
                if int(kTest):
                    passedKol += 1
                else:
                    # ; print 'KTest result : ', kTest
                    failedKol += 1
                    diffKols.append(h)
            else:
                # try the integral test?
                otherTest += 1
                if all((sint, pint)) and (sint == pint):
                    integralMatch += 1
                elif sint == pint:
                    zeroIntegralMatch += 1
                else:
                    diffIntegrals.append(h)
                    xIntegrals += 1
        else:
            notfound += 1
            print("not found? ", h)

    # report on Failed Entry-Checks
    print("\n\n" + "-" * 80)
    print("Summary of histos with different Entries")
    print("-" * 80)
    if diffEntries:
        diffEntries.sort()
        for e in diffEntries:
            print(f"\t\t\t{e}:\t{int(ds[e].GetEntries())} != {int(dp[e].GetEntries())}")
    print("-" * 80)

    # report on Failed Kolmogorov Tests
    print("\n\n" + "-" * 60)
    print("Summary of histos which failed Kolmogorov Test")
    print("-" * 60)
    if diffKols:
        diffKols.sort()
        for e in diffKols:
            result = kTestResults[e]  # DP Calculated twice ARGH!!
            print(f"{ds[e].ClassName()}\t\t{e} :\tK-Test Result :\t {result:5.16f}")
    print("-" * 60)

    # report on Failed Integral Checks
    print("\n\n" + "-" * 60)
    print("Summary of histos which failed Integral Check")
    print("-" * 60)
    if diffIntegrals:
        diffIntegrals.sort()
        for e in diffIntegrals:
            diff = dp[e].Integral() - ds[e].Integral()
            pc = (diff * 100) / ds[e].Integral()
            print(
                f"{ds[e].ClassName()}\t\t{e}:\t Diff = {diff:5.6f}\tPercent Diff to Reference : {pc:5.6f} "
            )
    print("-" * 60 + "\n")
    print("=" * 80 + "\n")

    # Report on failed bin2bin identity
    if checkBin2BinIdentity:
        # report on b2b checks
        print("\n\n" + "-" * 80)
        print("Summary of histos with at least one bin with different Entries")
        print("-" * 80)
        if diffIdentity:
            diffIdentity.sort()
            for e in diffIdentity:
                print(
                    f"{ds[e].ClassName()}\t\t{e}: {identityDiffBins[e]} different bins"
                )
            print("-" * 80)

    print("\n" + "=" * 80)
    print("Comparison : Reference/Test ROOT Histo files")
    print("\n\t\tReference\tTest")
    print(
        f"\tObjects : {referenceObjects}\t{parallObjects}\t\t( p-s = {parallObjects - referenceObjects} )"
    )
    print(
        f"\tHistos  : {referenceHistos}\t{parallHistos}\t\t( p-s = {parallHistos - referenceHistos} )"
    )
    print("\t          __________")
    print(
        f"\tTotal   : {referenceHistos + referenceObjects}\t{parallHistos + parallObjects}"
    )
    print(
        f"Objects/Histos unique to Reference File : {len(uniqueSerPaths) - uniqueSerHistos} / {uniqueSerHistos}"
    )
    print(
        f"Objects/Histos unique to Test File : {len(uniqueParPaths) - uniqueParHistos} / {uniqueParHistos}"
    )
    print(f"\nMatching Histograms valid for Comparison : {mh}")
    print("\nOmissions' patterns : ")
    for entry in gRegexBlackList:
        print(f"\t{entry}")
    print(
        f"\nHistograms for Comparison (after Omissions) : {mh - len(gRegexBlackList)}"
    )
    print("\n\tHISTOGRAM TESTS : ")
    print(f"\t\tKOLMOGOROV TEST      : {kTested}")
    print(f"\t\tINTEGRAL TEST        : {otherTest}")
    print(f"\t\tENTRIES TEST         : {xEntries}")
    if checkBin2BinIdentity:
        print(f"\t\tBIN2BIN TEST         : {passedIdentity}")
    print("\t\t                       ____")
    print(f"\t\tTested               : {cEntries}")

    print("\n\tDISCREPANCIES : ")
    print(f"\t\tK-Test      : {failedKol}")
    print(f"\t\tIntegrals   : {xIntegrals}")
    print(f"\t\tEntries     : {xEntries}")
    retval = failedKol + xIntegrals + xEntries + failedIdentity
    if retval != 0:
        print("\nThe two sets of histograms were not identical")
    print("\n" + "=" * 80)
    return retval


# =============================================================================


def extractBlacklist(listString):
    global gRegexBlackList
    if listString:
        for blackRegexp in listString.split(","):
            gRegexBlackList.append(blackRegexp)
    else:
        gRegexBlackList = []


# =============================================================================

if __name__ == "__main__":
    usage = "usage: %prog testFile.root referenceFile.root [options]"
    parser = OptionParser()
    parser.add_option(
        "-b",
        dest="blacklist",
        help='Comma separated list of regexps matching histograms to skip (for example -b"MemoryTool,ProcTime")',
    )

    parser.add_option(
        "-i",
        action="store_true",
        dest="bin2bin",
        default=False,
        help="Check for bin to bin identity",
    )
    (options, args) = parser.parse_args()

    if len(args) != 2:
        print("Wrong number of rootfiles. Usage:")
        print(usage)
        sys.exit(1)

    extractBlacklist(options.blacklist)

    testFile, referenceFile = args

    tfs = TFile(testFile, "REC")
    print(f"opening Test File : {testFile}")
    tfp = TFile(referenceFile, "REC")
    print(f"opening Reference File : {referenceFile}")

    # get structure of TFiles in a list of (path, object) tuples
    lref = rec(tfs)
    ltest = rec(tfp)
    # make a dictionary of lser and lpar.  keys=paths
    dref = dict([(n, o) for n, o in lref])
    dtest = dict([(n, o) for n, o in ltest])
    # make a tuple of (type, dict) where type is either 'reference' or 'test'
    ts = (ref, dref)
    tp = (test, dtest)

    # check objs/histos in each file
    composition(ts)
    composition(tp)

    # compare paths from each file
    state = comparePaths(ts, tp)

    # compare histos from each file
    retval = compareHistos(ts, tp, state, checkBin2BinIdentity=options.bin2bin)

    # finished with TFiles
    tfs.Close()
    tfp.Close()

    sys.exit(retval)
