.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapDetd:

Detector Description
======================

Overview
------------

    This chapter is a place holder for documenting how to access the
    detector description data in the Gaudi transient detector data
    store. A detector description implementation based on XML exists in
    the LHCb extensions to Gaudi [Chytracek:2000]_ but it is not distributed with the
    framework.

    The Gaudi architecture aims to shield the applications from the
    details of the persistent detector description and calibration
    databases. Ideally, the detector will be described in a logically
    unique detector description database (DDDB), containing data from
    many sources (e.g. editors and CAD tools for geometry data,
    calibration and alignment programs, detector control system for
    environmental data) as shown in :numref:`fig-detdesc`. The job of the Gaudi
    detector data service is to populate the transient detector data
    store with a snapshot of the detector description, which is valid
    for the event currently being analysed. Conversion services can be
    invoked to provide different transient representations of the same
    persistent data, appropriate to the specific application. For
    example, detector simulation, reconstruction and event display all
    require a geometry description of the detector, but with different
    levels of detail. In the Gaudi architecture it is possible to have a
    single, generic, persistent geometry description, from which a set
    of different representations can be extracted and made available to
    the data processing applications..

    The LHCb implementation of the detector description database
    describes the logical structure of the detector in terms of a
    hierarchy of detector elements and the basic geometry in terms of
    volumes, solids and materials, and provides facilities for
    customizing the generic description to many specific detector needs.
    This should allow to develop detector specific code which can
    provide geometry answers to questions from the physics algorithms.
    The persistent representation of the LHCb detector description is
    based on text files in XML format. An XML editor that understands
    the detector description semantics has been developed.

    .. figure:: images/GDG_DetDescriptiona.png
        :name: fig-detdesc

        Overview of the Detector Description model
