/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include <TFile.h>
#include <TTree.h>
#include <map>
#include <stdio.h>
#include <string>

#include <TApplication.h>
#include <TColor.h>
#include <TDatime.h>
#include <TGColorSelect.h>
#include <TGComboBox.h>
#include <TGHtml.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TROOT.h>
#include <TTimeStamp.h>

using namespace std;

class Widget {
private:
  TString fHeader;    // HTML header
  TString fFooter;    // HTML footer
  TString fHtml_text; // output HTML string

  TGMainFrame* fMain; // main frame
  TGHtml*      fHtml; // html widget to display HTML calendar
public:
  void MakeHeader();
  void MakeFooter();
  Widget( map<string, string> md );
  virtual ~Widget();
  TString Html() const { return fHtml_text; }
  ClassDef( Widget, 0 );
};
void Widget::MakeHeader() {
  fHeader = "<html><head><title>Meta Data Viewer</title></head>";
  fHeader += "<body>\n";
  fHeader += "<center><H2>Meta Data Viewer</H2></center>";
}
void Widget::MakeFooter() { fFooter = "</body></html>"; }

Widget::Widget( map<string, string> md ) {
  MakeHeader();
  MakeFooter();

  map<string, string>::iterator iter;

  fHtml_text = fHeader;
  fHtml_text += "<table border='1' style='width:100%'>";

  for ( iter = md.begin(); iter != md.end(); iter++ ) {
    fHtml_text += "<tr><td><font color='black'><b>";
    fHtml_text += iter->first;
    fHtml_text += "</b></font></td><td><font color='blue'><b>";
    fHtml_text += iter->second;
    fHtml_text += "</b></font></td></tr>";
  }
  fHtml_text += "</table>";
  fHtml_text += fFooter;

  // Main  window.
  fMain = new TGMainFrame( gClient->GetRoot(), 10, 10, kVerticalFrame );
  fMain->SetCleanup( kDeepCleanup ); // delete all subframes on exit
  // create HTML widget
  fHtml = new TGHtml( fMain, 1, 1 );
  fMain->AddFrame( fHtml, new TGLayoutHints( kLHintsExpandX | kLHintsExpandY, 5, 5, 2, 2 ) );
  fHtml->ParseText( (char*)fHtml_text.Data() );
  // terminate ROOT session when window is closed
  fMain->Connect( "CloseWindow()", "TApplication", gApplication, "Terminate()" );
  fMain->DontCallClose();

  fMain->MapSubwindows();
  fMain->Resize( 700, 700 );

  // set  minimum size of main window
  fMain->SetWMSizeHints( fMain->GetDefaultWidth(), fMain->GetDefaultHeight(), 1000, 1000, 0, 0 );
  fMain->MapRaised();
}
Widget::~Widget() { delete fMain; }
void readMetaData() {
  TFile                mFile( "TupleEx.root" ); // TODO update name of the file
  map<string, string>* mdMap  = (map<string, string>*)mFile.Get( "info" );
  map<string, string>  newMap = *mdMap;
  new Widget( newMap );
}
