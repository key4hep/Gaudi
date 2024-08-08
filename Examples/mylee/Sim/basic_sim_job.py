#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
def gen_elastic(nevt):
    """
    Configure a simple job to generate `nevt` elastic scattering events.
    """
    from GaudiConfig2.Configurables import (
        ApplicationMgr,
        EvtDataSvc,
        FileSvc,
        Gaudi,
        mylee,
    )

    files = FileSvc(
        Config={
            "NTuple": "sim_output.root?mode=create",
        },
    )

    # define nodes od data flow graph
    gen = mylee.mc.gen.elastic(
        "elastic",
    )
    gen_init = mylee.mc.gen.init(
        "gen_init",
        Generator=gen.name,
        BaseSeed=1234,
    )
    # define edges of data flow graph
    gen.header = gen_init.header.path()

    writer = Gaudi.NTuple.GenericWriter(
        OutputFile="NTuple",
        NTupleName="events",
        ExtraInputs=[(obj.type(), obj.path()) for obj in [gen_init.header, gen.event]],
    )

    algorithms = [gen_init, gen, writer]

    # application configuration
    app = ApplicationMgr(
        # list of algorithms to execute
        TopAlg=algorithms,
        # simulation jobs do not read events from input files
        EvtSel="NONE",
        # number of events to produce
        EvtMax=nevt,
        # tune the overall verbosity
        # OutputLevel=1,
    )
    # Transient Event Store config
    tes = EvtDataSvc("EventDataSvc")
    # automatic creation of intermediate directories
    tes.ForceLeaves = True

    # we have to return all objects we configured
    return [app, tes, files] + algorithms


def gen_1000_elastic():
    return gen_elastic(1000)
