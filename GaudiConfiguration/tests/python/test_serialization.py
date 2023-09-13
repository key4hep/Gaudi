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
from GaudiConfig2 import Configurable, all_options
from GaudiConfig2.Configurables.TestConf import (
    AlgWithMaps,
    AlgWithVectors,
    MyAlg,
    SimpleOptsAlgTool,
)


class MyDerivedAlg(MyAlg):
    """Derived algorithm with __init__ override"""

    def __init__(self):
        super(MyDerivedAlg, self).__init__()
        self.AnIntProp = 42


def test_repr(with_global_instances):
    p = MyAlg(AnIntProp=42)

    q = eval(repr(p))
    assert q.AnIntProp == 42
    assert repr(q) == repr(p)

    p.name = "p"
    r = repr(p)
    del p.name
    q = eval(r)
    assert q.name == "p"
    assert q.AnIntProp == 42
    del q.name
    assert repr(q) == repr(p)

    MyAlg("Parent")
    t = SimpleOptsAlgTool("Tool", parent="Parent", Bool=True)
    orig_name = t.name
    r = repr(t)
    del t.name
    t1 = eval(r)
    assert t1.name == orig_name
    assert t1.Bool == t.Bool

    p = AlgWithVectors(VS="abc")
    q = eval(repr(p))
    assert list(q.VS) == list("abc")
    assert repr(q) == repr(p)

    p = AlgWithMaps(MSS={"a": "B"})
    q = eval(repr(p))
    assert dict(q.MSS) == {"a": "B"}
    assert repr(q) == repr(p)


def test_pickle(with_global_instances):
    from pickle import dumps, loads

    p = MyAlg(AnIntProp=42)

    q = loads(dumps(p))
    assert q.AnIntProp == 42
    assert dumps(q) == dumps(p)

    p.name = "p"
    r = dumps(p)
    del p.name
    q = loads(r)
    assert q.name == "p"
    assert q.AnIntProp == 42
    del q.name
    assert dumps(q) == dumps(p)

    MyAlg("Parent")
    t = SimpleOptsAlgTool("Tool", parent="Parent", Bool=True)
    orig_name = t.name
    r = dumps(t)
    del t.name
    t1 = loads(r)
    assert t1.name == orig_name
    assert t1.Bool == t.Bool

    p = AlgWithVectors(VS="abc")
    q = loads(dumps(p))
    assert list(q.VS) == list("abc")
    assert dumps(q) == dumps(p)

    p = AlgWithMaps(MSS={"a": "B"})
    q = loads(dumps(p))
    assert dict(q.MSS) == {"a": "B"}
    assert dumps(q) == dumps(p)

    p = MyDerivedAlg()
    q = loads(dumps(p))
    assert q.AnIntProp == 42
    assert dumps(q) == dumps(p)


def test_full_serialization_repr(with_global_instances):
    MyAlg("Alg1", AnIntProp=1)
    SimpleOptsAlgTool("ToolA", parent=MyAlg("Alg2", AnIntProp=2))
    AlgWithVectors("AV", VS="abc")
    AlgWithMaps("AM", MSS={"a": "B"})

    serial = repr(list(Configurable.instances.values()))
    opts = all_options(explicit_defaults=True)
    Configurable.instances.clear()
    eval(serial)
    assert all_options(explicit_defaults=True) == opts


def test_full_serialization_pickle(with_global_instances):
    from pickle import dumps, loads

    MyAlg("Alg1", AnIntProp=1)
    SimpleOptsAlgTool("ToolA", parent=MyAlg("Alg2", AnIntProp=2))
    AlgWithVectors("AV", VS="abc")
    AlgWithMaps("AM", MSS={"a": "B"})

    serial = dumps(list(Configurable.instances.values()))
    opts = all_options(explicit_defaults=True)
    Configurable.instances.clear()
    loads(serial)
    assert all_options(explicit_defaults=True) == opts
