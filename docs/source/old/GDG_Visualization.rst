.. include:: ./global.rst

|newpage|

|Gaudi logo|

.. _chapVisu:

Visualization Facilities
===========================

Overview
-------------

    In this chapter we describe how visualization facilities are
    provided to the applications based on the Gaudi framework. A
    prototype implementation (GaudiLab) exists in LHCb but is not
    distributed with the framework. It is based on the packages that
    constitute the Open Scientist suite (OpenGL, OpenInventor(soFree),
    Lab,... see http://www.lal.in2p3.fr/OpenScientist/). An event and
    geometry display application has been built using these facilities.

The data visualization model
---------------------------------

    The Gaudi architecture envisaged implementing data visualization
    using a similar pattern to data persistency. We do not want to
    implement visualization methods in each data object. In other words,
    we do not want to tell an object to "draw" itself. Instead we would
    implement converters as separate entities that are able to create
    specific graphical representations for each type of data object and
    for each graphical package that we would like to use. In that way,
    as for the persistency case, we decouple the definition and
    behaviour of the data objects from the various technologies for
    graphics. We could configure at run time to have 2D or 3D graphics
    depending on the needs of the end-user at that moment.

    :numref:`fig-visu` illustrates the
    components that need to be included in an application to make it
    capable of visualizing data objects (the "So" prefix in the names is
    taken from the GaudiLab implementation). The interactive user
    interface is a Service which allows the end-user to interact with
    all the components of the application. The user could select which
    objects to display, which algorithms to run, what properties of
    which algorithm to inspect and modify, etc. This interaction can be
    implemented using a graphical user interface or by using a scripting
    language.

    The User interface service is also in charge of managing one or more
    GUI windows where views of the graphical representations are going
    to be displayed.

    .. figure:: images/visualmodel.png
        :name: fig-visu

        Components for visualization**

    The other main component is a Conversion Service that handles the
    conversion of objects into their graphical representation. This
    service requires the help of a number of specialized converters, one
    for each type of data object that needs to be graphically displayed.
    The transient store of graphical representations is shared by the
    conversion service, together with the converters, and the user
    interface component. The form of this transient store depends on the
    choice of graphics package. Typically it is the user interface
    component that would trigger the conversion service to start the
    conversion of a number of objects (next event), but this service can
    also be triggered by any algorithm that would like to display some
    objects.
