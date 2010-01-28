// $Id: HistoAlgorithm.cpp,v 1.11 2006/11/30 10:35:27 mato Exp $


// Include files
#include "HistoAlgorithm.h"

#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IHistogramSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/DataObject.h"

#include <math.h>

DECLARE_ALGORITHM_FACTORY(HistoAlgorithm)


//------------------------------------------------------------------------------
HistoAlgorithm::HistoAlgorithm(const std::string& name,
							   ISvcLocator* pSvcLocator) :
							   Algorithm(name, pSvcLocator)
//------------------------------------------------------------------------------
{
  m_h1D = m_h1DVar = 0;
  m_h2D = m_h2DVar = 0;
  m_h3D = m_h3DVar = 0;
  m_p1D = 0;
  m_p2D = 0;
}


//------------------------------------------------------------------------------
StatusCode HistoAlgorithm::initialize()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );
  int i;
  std::vector<double> edges;
  for( i = 0; i <= 10; i++) edges.push_back( double(i)*double(i) );
  // Getting existing histograms
  SmartDataPtr<IHistogram1D> h1(histoSvc(),"InFile/1");
  if( h1 ) {
    log << MSG::INFO << "Got histogram from file" << endmsg;
    histoSvc()->print( h1);
  } else {
    log << MSG::ERROR << "Histogram h1 not found" << endmsg;
  }

  log << MSG::INFO << "Booking Histograms" << endmsg;

  // Book 1D histogram with fixed and variable binning
  m_h1D    = histoSvc()->book( "1", "1D fix binning", 50, 0., 100. );
  m_h1DVar = histoSvc()->book( "Var/1", "1D variable binning", edges );

  m_h2D    = histoSvc()->book( "2", "2D fix binning", 50, 0., 100.,
                                                      50, 0., 100.  );
  m_h2DVar = histoSvc()->book( "Var/2", "2D variable binning", edges, edges );

  m_h3D    = histoSvc()->book( "3", "3D fix binning", 50, 0., 100.,
                                                      50, 0., 100.,
                                                      10, 0., 100.  );
  m_h3DVar = histoSvc()->book( "Var/3", "3D variable binning", edges,
                                                               edges,
                                                               edges );
  m_p1D    = histoSvc()->bookProf( "Prof/1", "Profile 1D", 50, 0., 100. );
  m_p2D    = histoSvc()->bookProf( "Prof/2", "Profile 2D", 50, 0., 100.,
                                                           50, 0., 100. );

  if( 0 == m_h1D ||0 == m_h2D || 0 == m_h3D ||
      0 == m_h1DVar ||0 == m_h2DVar ||0 == m_h3DVar ||
      0 == m_p1D || 0 == m_p2D  ) {
    log << MSG::ERROR << "----- Cannot book or register histograms -----" << endmsg;
    return StatusCode::FAILURE;
  }
  log << MSG::INFO << "Finished booking Histograms" << endmsg;

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode HistoAlgorithm::execute()
//------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );

  static int n = 0;

  double x = sin(double(n)) * 52. + 50.;
  double y = cos(double(2*n)) * 52. + 50.;
  double z = cos(double(5*n)) * 52. + 50.;

  m_h1D->fill(x);
  m_h1DVar->fill(x);

  m_h2D->fill(x, y);
  m_h2DVar->fill(x, y);

  m_h3D->fill(x, y, z);
  m_h3DVar->fill(x, y, z);

  m_p1D->fill(x, y);
  m_p2D->fill(x, y, z);

  n++;
  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode HistoAlgorithm::finalize()
//------------------------------------------------------------------------------
{
  MsgStream log(msgSvc(), name());
  log << MSG::DEBUG << "Finalizing..." << endmsg;

  //1D HISTOGRAM STUFF
  log << MSG::DEBUG << "Info about Histogram 1D\n" << endmsg;
  //General Info:
	log << MSG::DEBUG << "title: " << m_h1D->title() <<  endmsg;
	log << MSG::DEBUG << "dim:   " << m_h1D->dimension() << endmsg;
  //Histo Info:
	log << MSG::DEBUG << "Entries (In): "  << m_h1D->entries()  << endmsg;
	log << MSG::DEBUG << "Entries (Out): " << m_h1D->extraEntries()  << endmsg;
	log << MSG::DEBUG << "Entries (All): " << m_h1D->allEntries()  << endmsg;
	log << MSG::DEBUG << "Entries (Eq.): " << m_h1D->equivalentBinEntries()  << endmsg;

	log << MSG::DEBUG << "Sum of Heights(In): "  << m_h1D->sumBinHeights()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(Out): " << m_h1D->sumExtraBinHeights()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(All): " << m_h1D->sumAllBinHeights() << endmsg;
	log << MSG::DEBUG << "Min. Heights(In): "    << m_h1D->minBinHeight()  << endmsg;
	log << MSG::DEBUG << "Max. Heights(In): "    << m_h1D->maxBinHeight()  << endmsg;
  //Statistics:
	log << MSG::DEBUG << "mean: " << m_h1D->mean() << endmsg;
	log << MSG::DEBUG << "rms: " <<  m_h1D->rms()  << endmsg;
  //Print Histogram stream in ASCII on screen:
	log << MSG::DEBUG << "Printing H1: " << endmsg;
	//histoSvc()->print( m_h1D );

  //1DVar HISTOGRAM STUFF
  log << MSG::DEBUG << "Info about Histogram 1DVar\n" << endmsg;
  //General Info:
	log << MSG::DEBUG << "title: " << m_h1DVar->title() <<  endmsg;
	log << MSG::DEBUG << "dim:   " << m_h1DVar->dimension() << endmsg;
  //Histo Info:
	log << MSG::DEBUG << "Entries (In): "  << m_h1DVar->entries()  << endmsg;
	log << MSG::DEBUG << "Entries (Out): " << m_h1DVar->extraEntries()  << endmsg;
	log << MSG::DEBUG << "Entries (All): " << m_h1DVar->allEntries()  << endmsg;
	log << MSG::DEBUG << "Entries (Eq.): " << m_h1DVar->equivalentBinEntries()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(In): "  << m_h1DVar->sumBinHeights()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(Out): " << m_h1DVar->sumExtraBinHeights()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(All): " << m_h1DVar->sumAllBinHeights() << endmsg;
	log << MSG::DEBUG << "Min. Heights(In): "    << m_h1DVar->minBinHeight()  << endmsg;
	log << MSG::DEBUG << "Max. Heights(In): "    << m_h1DVar->maxBinHeight()  << endmsg;
  //Statistics:
	log << MSG::DEBUG << "mean: " << m_h1DVar->mean() << endmsg;
	log << MSG::DEBUG << "rms: " <<  m_h1DVar->rms()  << endmsg;
  //Print Histogram stream in ASCII on screen:
	log << MSG::DEBUG << "Printing H1Var: " << endmsg;
	//histoSvc()->print( m_h1DVar );

  //2D HISTOGRAM STUFF
  log << MSG::DEBUG << "Info about Histogram 2D\n" << endmsg;
  //General Info:
	log << MSG::DEBUG << "title: " << m_h2D->title() <<  endmsg;
	log << MSG::DEBUG << "dim:   " << m_h2D->dimension() << endmsg;
  //Histo Info:
	log << MSG::DEBUG << "Entries (In): "  << m_h2D->entries()  << endmsg;
	log << MSG::DEBUG << "Entries (Out): " << m_h2D->extraEntries()  << endmsg;
	log << MSG::DEBUG << "Entries (All): " << m_h2D->allEntries()  << endmsg;
	log << MSG::DEBUG << "Entries (Eq.): " << m_h2D->equivalentBinEntries()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(In): "  << m_h2D->sumBinHeights()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(Out): " << m_h2D->sumExtraBinHeights()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(All): " << m_h2D->sumAllBinHeights() << endmsg;
	log << MSG::DEBUG << "Min. Heights(In): "    << m_h2D->minBinHeight()  << endmsg;
	log << MSG::DEBUG << "Max. Heights(In): "    << m_h2D->maxBinHeight()  << endmsg;
  //Statistics:
	log << MSG::DEBUG << "meanX: " << m_h2D->meanX() << endmsg;
	log << MSG::DEBUG << "meanY: " << m_h2D->meanY() << endmsg;
	log << MSG::DEBUG << "rmsX: " <<  m_h2D->rmsX()  << endmsg;
	log << MSG::DEBUG << "rmsY: " <<  m_h2D->rmsY()  << endmsg;
  //Print Histogram stream in ASCII on screen:
	log << MSG::DEBUG << "Printing H1Var: " << endmsg;
	//histoSvc()->print( m_h2D );

  //2DVar HISTOGRAM STUFF
  log << MSG::DEBUG << "Info about Histogram 2DVar\n" << endmsg;
  //General Info:
	log << MSG::DEBUG << "title: " << m_h2DVar->title() <<  endmsg;
	log << MSG::DEBUG << "dim:   " << m_h2DVar->dimension() << endmsg;
  //Histo Info:
	log << MSG::DEBUG << "Entries (In): "  << m_h2DVar->entries()  << endmsg;
	log << MSG::DEBUG << "Entries (Out): " << m_h2DVar->extraEntries()  << endmsg;
	log << MSG::DEBUG << "Entries (All): " << m_h2DVar->allEntries()  << endmsg;
	log << MSG::DEBUG << "Entries (Eq.): " << m_h2DVar->equivalentBinEntries()  << endmsg;

	log << MSG::DEBUG << "Sum of Heights(In): "  << m_h2DVar->sumBinHeights()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(Out): " << m_h2DVar->sumExtraBinHeights()  << endmsg;
	log << MSG::DEBUG << "Sum of Heights(All): " << m_h2DVar->sumAllBinHeights() << endmsg;
	log << MSG::DEBUG << "Min. Heights(In): "    << m_h2DVar->minBinHeight()  << endmsg;
	log << MSG::DEBUG << "Max. Heights(In): "    << m_h2DVar->maxBinHeight()  << endmsg;
  //Statistics:
	log << MSG::DEBUG << "meanX: " << m_h2DVar->meanX() << endmsg;
	log << MSG::DEBUG << "meanY: " << m_h2DVar->meanX() << endmsg;
	log << MSG::DEBUG << "rmsX: " <<  m_h2DVar->rmsX()  << endmsg;
	log << MSG::DEBUG << "rmsY: " <<  m_h2DVar->rmsY()  << endmsg;
  //Print Histogram stream in ASCII on screen:
//	log << MSG::DEBUG << "Printing H2Var: " << endmsg;
//	histoSvc()->print( m_h2DVar );

  // Create X projection of the Pt versus P histogram m_hPtvsP
//   IHistogram1D* xProj  = histoSvc()->histogramFactory()->projectionX( "Proj/101",  *m_h2D );
// 	IHistogram1D* yProj  = histoSvc()->histogramFactory()->projectionY( "Proj/102" , *m_h2D );
// 	IHistogram1D* xSlice = histoSvc()->histogramFactory()->sliceX( "Slice/101" , *m_h2D, 10);
// 	IHistogram1D* ySlice = histoSvc()->histogramFactory()->sliceY( "Slice/102" , *m_h2D, 20);
// 	log << MSG::DEBUG << "Projection X" << endmsg;
//   histoSvc()->print( xProj );
// 	log << MSG::DEBUG << "Projection Y" << endmsg;
//   histoSvc()->print( yProj );
// 	// Print the slices
// 	log << MSG::DEBUG << "Slice X:" << endmsg;
//   histoSvc()->print( xSlice );
// 	log << MSG::DEBUG << "Slice Y:" << endmsg;
//   histoSvc()->print( ySlice );


  return StatusCode::SUCCESS;
}
