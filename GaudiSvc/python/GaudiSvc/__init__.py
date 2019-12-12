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
def getMetaData(path):
    '''
    Extract job metadata recorded by Gaudi::MetaDataSvc from a file.
    '''
    from cppyy.gbl import TFile
    f = TFile.Open(path)
    if not f:
        raise RuntimeError('cannot open {0}'.format(path))
    try:
        # return dict((i.first, i.second) for i in f.info)
        # FIXME: workaround for https://sft.its.cern.ch/jira/browse/ROOT-10474
        import cppyy
        cppyy.gbl.gInterpreter.Declare(r'''
        #include <map>
        #include <string>
        #include <sstream>
        auto get_keys(const std::map<std::string, std::string>& m) {
            std::stringstream s;
            for(auto& i: m) {
                s << i.first << '\n';
            }
            return s.str();
        }
        ''')
        keys = str(cppyy.gbl.get_keys(f.info)).strip().split('\n')
        return dict((k, f.info[k]) for k in keys)
    except AttributeError:
        return {}
