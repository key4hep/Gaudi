#!/usr/bin/env python
# =============================================================================
# $Id: ExtendedProperties.py,v 1.4 2007/08/07 13:00:28 marcocle Exp $
# =============================================================================
# CVS tag $Name:  $, version $Revision: 1.4 $
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example (identical to C++ ExtendedProperties.opts) which illustrates *
* the extended job-properties and their C++/Python intercommunication         *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV ibelyaev@physics.syr.edu'
# =============================================================================
## @file
#  Simple example (identical to C++ ExtendedProperties.opts) which illustrates
#  the extended job-properties and their C++/Python intercommunication
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2007-02-13
# =============================================================================

import gaudimodule

SUCCESS = gaudimodule.SUCCESS

# =============================================================================
## the configurtaion of the job
def configure ( gaudi = None ) :
    """ the configurtaion of the job """

    ## create application manager if not done yet
    if not gaudi : gaudi = gaudimodule.AppMgr()

    ## read main configuration files
    gaudi.config ( files = [ '../options/Common.opts' ] )

    ## private algorithm configuration options

    gaudi.TopAlg = [ "ExtendedProperties/xProps" ]
    gaudi.EvtSel = 'NONE'
    gaudi.HistogramPersistency = 'NONE'

    xProps = gaudi.algorithm( 'xProps' )

    # std::pair<double,double>
    # xProps.PairDD = ( 3.141592 , 2.18281828 )
    # std::pair<int,int>
    # xProps.PairII = ( 3        , 2          )

    # std::vector<std::pair<double,double> >
    xProps.VectorOfPairsDD = [ (0,1), (1,2), (2,3), (3,4) ]

    # std::vector<std::vector<std::string> >
    xProps.VectorOfVectorsString = [
        [ "a", "b" , "c" ] , [ "A", "B" , "C" ] ]

    # std::vector<std::vector<double> >
    xProps.VectorOfVectorsDouble = [
        [ 0 , 1 , 2 ] , [ 0 , -0.5 , -0.25 ] ]


    # std::map<int,double>
    xProps.MapIntDouble = {
        1 : 0.1 , 2 : 0.2 , 3 : 0.3 }

    # std::map<std::string,std::string>
    xProps.MapStringString = {
        'a' : 'sddsgsgsdgdggf' ,
        'b' : 'sddsgsgsdgdggf' ,
        'c' : 'sddsgsgsdgdggf' }


    # std::map<std::string,int>
    xProps.MapStringInt = {
        'a' : 1 ,
        'b' : 2 ,
        "c" : 3 }

    # std::map<std::string,double>
    xProps.MapStringDouble = {
        'aa' : 0.1 ,
        'bb' : 0.2 ,
        "cc" : 3   }

    # std::map<std::string,std::vector<std::string> >
    xProps.MapStringVectorOfStrings = {
        'aaa' : [ 'a' , 'b' ,'c' ] ,
        'bbb' : [ 'a' , 'b' ,'c' ] ,
        'ccc' : [ 'a' , 'b' ,'c' ] }

    # std::map<std::string,std::vector<double> >
    xProps.MapStringVectorOfDoubles = {
        'aaa' : [ 1   , 2   , 3   ] ,
        'bbb' : [ 1.  , 2.  , 3.  ] ,
        'ccc' : [ 0.1 , 0.2 , 0.3 ] }

    # std::map<std::string,std::vector<int> >
    xProps.MapStringVectorOfInts = {
        'aaa' : [ 1 , 2 , 3 ] ,
        'bbb' : [ 4 , 5 , 6 ] ,
        'ccc' : [ 7 , 8 , 9 ] }

    return SUCCESS

# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__ :

    print __doc__ , __author__

    gaudi = gaudimodule.AppMgr()
    configure( gaudi )
    gaudi.run(1)

    alg = gaudi.algorithm( 'xProps' )

    ## get all properties throught python
    #
    # IT DOES NOT WORK ANYMORE after the
    # reimplementation of
    # gaudimodule.iProperty.properties using
    # new class PropertyEntry
    #
    props = alg.properties()

    print 'All Properties of %s ' % alg.name()
    for p in props :
        v = props[p].value()
        t = type(v).__name__
        print "Python: Name/Value:  '%s' / '%s' "%(p,v)

    ## get the properties in the form of python dictionary:
    print 'All Properties of %s ' % alg.name()
    properties = {}
    for p in props : properties[p] = props[p].value()

    for p in properties :
        print "Python: Name/Value:  '%s' / '%s' "%(p,properties[p])


# =============================================================================
# The END
# =============================================================================
