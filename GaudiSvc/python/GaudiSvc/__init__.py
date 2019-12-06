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
