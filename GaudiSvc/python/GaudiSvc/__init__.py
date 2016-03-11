def getMetaData(path):
    '''
    Extract job metadata recorded by Gaudi::MetaDataSvc from a file.
    '''
    from cppyy.gbl import TFile
    f = TFile.Open(path)
    if not f:
        raise RuntimeError('cannot open {0}'.format(path))
    try:
        return dict((i.first, i.second) for i in f.info)
    except AttributeError:
        return {}
