#####################################################################################
# (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import pytest


def test_import():
    import GaudiConfig2.Configurables
    from GaudiConfig2 import Configurables

    assert Configurables
    assert GaudiConfig2.Configurables is Configurables


def test_entries():
    from GaudiConfig2 import Configurables as C

    assert set(C.__all__) == set(["MyAlgorithm", "MyOtherAlg", "Gaudi", "TestConf"])
    assert set(C.Gaudi.__all__) == set(["SomeAlg", "TestSuite", "Foundation"])
    assert set(C.Gaudi.TestSuite.__all__) == set(["Test1"])
    assert set(C.Gaudi.Foundation.__all__) == set(["Service"])

    try:
        C.Gaudi.NotThere
        assert False, "AttributeError expected"
    except AttributeError:
        pass

    assert "SomeAlg" not in dir(C.Gaudi)  # not loaded yet
    assert hasattr(C.Gaudi, "SomeAlg")  # this loads it
    assert "SomeAlg" in dir(C.Gaudi)  # now it's loaded

    assert C.TestConf.MyAlg.__cpp_type__ == "TestConf::MyAlg"

    name = "TemplatedAlg<int, std::vector<std::string, std::allocator<std::string> > >"
    alt_name = (
        "TemplatedAlg_int_std__vector_std__string_std__allocator_std__string_s_s_"
    )
    assert getattr(C.TestConf, name) is getattr(C.TestConf, alt_name)


def test_get_by_full_type():
    from GaudiConfig2 import Configurables as C

    assert C.getByType("TestConf::MyAlg") is C.TestConf.MyAlg
    name = "TemplatedAlg<int, std::vector<std::string, std::allocator<std::string> > >"
    assert C.getByType("TestConf::" + name) is getattr(C.TestConf, name)


def test_get_by_full_type_ignore_spaces():
    from GaudiConfig2 import Configurables as C

    name = "TemplatedAlg<int, std::vector<std::string, std::allocator<std::string> > >"
    stripped_name = name.replace(" ", "")
    assert C.getByType("TestConf::" + name) is C.getByType("TestConf::" + stripped_name)
    mix_name = name.replace(" ", "", 1)
    assert C.getByType("TestConf::" + name) is C.getByType("TestConf::" + mix_name)


def get_get_by_full_type_missing():
    from GaudiConfig2 import Configurables as C

    with pytest.raises(AttributeError):
        C.getByType("Gaudi::NotThere")


def test_confdb():
    import os
    import shelve
    import sys

    try:
        from tempfile import TemporaryDirectory
    except ImportError:
        from contextlib import contextmanager

        @contextmanager
        def TemporaryDirectory():
            from shutil import rmtree
            from tempfile import mkdtemp

            name = mkdtemp()
            try:
                yield name
            finally:
                rmtree(name)

    import GaudiConfig2._db

    with TemporaryDirectory() as tmpdir:
        db = shelve.open(os.path.join(tmpdir, "test1.confdb2"))
        db.update(
            ("Cls{}".format(i), {"properties": {"IntA": ("int", i)}}) for i in range(5)
        )
        db.close()
        db = shelve.open(os.path.join(tmpdir, "test2.confdb2"))
        db.update(
            ("Cls{}".format(i), {"properties": {"IntB": ("int", i)}})
            for i in range(3, 8)
        )
        db.close()

        pathvar = "DYLD_LIBRARY_PATH" if sys.platform == "darwin" else "LD_LIBRARY_PATH"
        old_path = os.environ.get(pathvar)
        os.environ[pathvar] = "{0}{1}{0}_not_there".format(tmpdir, os.pathsep)

        db = GaudiConfig2._db.ConfDB2()
        assert set(db) == set("Cls{}".format(i) for i in range(8))
        assert "Cls1" in db and "Cls9" not in db
        assert "IntA" in db["Cls1"]["properties"]
        assert "IntA" in db["Cls4"]["properties"]
        assert "IntB" in db["Cls7"]["properties"]

        if old_path is None:
            del os.environ[pathvar]
        else:
            os.environ[pathvar] = old_path
