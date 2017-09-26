def test():
    try:
        from Gaudi.Configuration import ApplicationMgr, OutputStream
        ApplicationMgr().OutStream = OutputStream("Test")
    except ValueError:
        return  # success
    assert False, "exception ValueError not raised"
