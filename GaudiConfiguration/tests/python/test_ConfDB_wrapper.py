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


def test_confdb(monkeypatch, tmp_path):
    import os
    import shelve

    import GaudiConfig2._db

    db = shelve.open(tmp_path.joinpath("test1.confdb2").as_posix())
    db.update(
        ("Cls{}".format(i), {"properties": {"IntA": ("int", i)}}) for i in range(5)
    )
    db.close()
    db = shelve.open(tmp_path.joinpath("test2.confdb2").as_posix())
    db.update(
        ("Cls{}".format(i), {"properties": {"IntB": ("int", i)}}) for i in range(3, 8)
    )
    db.close()

    monkeypatch.setenv(
        "GAUDI_PLUGIN_PATH", "{0}{1}{0}_not_there".format(tmp_path, os.pathsep)
    )
    monkeypatch.delenv("LD_LIBRARY_PATH", raising=False)
    monkeypatch.delenv("DYLD_LIBRARY_PATH", raising=False)

    db = GaudiConfig2._db.ConfDB2()
    assert set(db) == set("Cls{}".format(i) for i in range(8))
    assert "Cls1" in db and "Cls9" not in db
    assert "IntA" in db["Cls1"]["properties"]
    assert "IntA" in db["Cls4"]["properties"]
    assert "IntB" in db["Cls7"]["properties"]
