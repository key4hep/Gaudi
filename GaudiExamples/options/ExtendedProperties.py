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
# Translation of ExtendedProperties.opts into python job options.
# @author Marco Clemencic
from Gaudi.Configuration import *
from Configurables import ExtendedProperties

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

from GaudiKernel.SystemOfUnits import *

# BEGIN of xProp configuration

xProps = ExtendedProperties("xProps")
xProps.PairDD = (3.141592 * m, 2.18281828)  # std::pair<double,double>
xProps.PairII = (3, 2)  # std::pair<int,int>

# std::vector<std::pair<double,double> >
xProps.VectorOfPairsDD += [(0, 1), (1, 2), (2, 3), (3, 4)]
xProps.VectorOfPairsDD += [(4, 5), (5, 6), (6, 7), (7, 8)]

# std::vector<std::vector<std::string> >
xProps.VectorOfVectorsString = [["a", "b", "c"], ["A", "B", "C"]]

# std::vector<std::vector<double> >
xProps.VectorOfVectorsDouble = [[0, 1, 2], [0, -0.5, -0.25]]

# std::map<int,double>
xProps.MapIntDouble = {1: 0.1, 2: 0.2, 3: 0.3}

# std::map<std::string,std::string>
xProps.MapStringString = {
    'a': 'sddsgsgsdgdggf',
    'b': 'sddsgsgsdgdggf',
    'c': 'sddsgsgsdgdggf'
}

# std::map<std::string,int>
xProps.MapStringInt = {'a': 1, 'b': 2, "c": 3}

# std::map<std::string,int>
xProps.MapStringDouble = {'aa': 0.1, 'bb': 0.2, "cc": 3}

# std::map<std::string,std::vector<std::string> >
xProps.MapStringVectorOfStrings = {
    'aaa': ['a', 'b', 'c'],
    'bbb': ['a', 'b', 'c'],
    'ccc': ['a', 'b', 'c']
}

# std::map<std::string,std::vector<double> >
xProps.MapStringVectorOfDoubles = {
    'aaa': [1, 2, 3],
    'bbb': [1., 2., 3.],
    'ccc': [0.1, 0.2, 0.3]
}

# std::map<std::string,std::vector<int> >
xProps.MapStringVectorOfInts = {
    'aaa': [1, 2, 3],
    'bbb': [4, 5, 6],
    'ccc': [7, 8, 9]
}

# std::map<int,int>
xProps.MapIntInt = {1: 10, 2: 20, 3: 30}

# std::vector<std::pair<int,int> >
xProps.VectorOfPairsII = [(1, 1), (2, 1), (3, 2), (4, 3), (5, 5)]

# std::map<int,std::string>
xProps.MapIntString = {0: "zero", 2: "two", -1: "minus one"}

# std::map<unsigned int,std::string>
xProps.MapUIntString = {0: "UZero", 2: "UTwo", 1: "UOne"}

xProps.EmptyMap = {}
xProps.EmptyVector = []

xProps.TupleStringIntDouble = ("hello", 10, 0.001)
xProps.TupleString = ("hello", )
xProps.StdArrayDouble3 = (3.3, 2.2, 1.1)
xProps.StdArrayInt1 = (42, )

xProps.GaudiMapSS = {'a': '1', 'b': '2'}

# END of xProp configuration

app = ApplicationMgr(
    TopAlg=[xProps], EvtMax=1, EvtSel="NONE", HistogramPersistency="NONE")
