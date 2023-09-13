/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// Convert text referencing JIRA or Savannah items into links to the items.
// Requires jQuery.

function isTextNode() { return this.nodeType == Node.TEXT_NODE; }

// http://stackoverflow.com/a/1219983
function htmlEncode(value) {
  // create a in-memory div, set it's inner text(which jQuery automatically encodes)
  // then grab the encoded contents back out.  The div never exists on the page.
  return $('<div/>').text(value).html();
}

var JIRA_ITEM_REGEX = /\b([A-Z]+)-[0-9]+/g;
var SAVANNAH_ITEM_REGEX = /\b([Pp]atch|[Tt]ask|[Bb]ug) #([0-9]+)/g;
var MERGE_REQUESTS_REGEX = /(\b[a-zA-Z0-9_/-]+)?!([0-9]+)\b/g;
var GIT_COMMITS_REGEX = /\b[a-f0-9]{7,40}\b/g;

jQuery.fn.linkToSavannah = function() {
  this.contents().filter(isTextNode).replaceWith(function() {
    return htmlEncode(this.nodeValue).replace(SAVANNAH_ITEM_REGEX, function(match, p1, p2) {
      p1 = p1.toLowerCase();
      return '<a href="https://savannah.cern.ch/' + p1 + '/index.php?' + p2 + '">' + match + '</a>';
    });
  });
  return this;
};

jQuery.fn.linkToJIRA = function() {
  this.contents().filter(isTextNode).replaceWith(function() {
    return htmlEncode(this.nodeValue).replace(JIRA_ITEM_REGEX, function(match, p1) {
      var url = "https://its.cern.ch/jira/browse/";
      if (p1 == "ROOT" || p1 == "SPI" || p1 == "PF" || p1 == "CFHEP" || p1 == "CVM")
        url = "https://sft.its.cern.ch/jira/browse/";
      return '<a href="' + url + match + '">' + match + '</a>';
    });
  });
  return this;
};

jQuery.fn.linkToGitLabCommit = function() {
  this.contents().filter(isTextNode).replaceWith(function() {
    return htmlEncode(this.nodeValue).replace(GIT_COMMITS_REGEX, function(match) {
      return '<a href="https://gitlab.cern.ch/gaudi/Gaudi/commit/' + match + '">' + match + '</a>';
    });
  });
  return this;
};

jQuery.fn.linkToGitLabMR = function() {
  this.contents().filter(isTextNode).replaceWith(function() {
    return htmlEncode(this.nodeValue).replace(MERGE_REQUESTS_REGEX, function(match, p1, p2) {
      if (!p1)
        p1 = "gaudi/Gaudi";
      return '<a href="https://gitlab.cern.ch/' + p1 + '/merge_request/' + p2 + '">' + match + '</a>';
    });
  });
  return this;
};

$(function() {
  $(":not(a)", ".textblock").linkToJIRA();
  $(":not(a)", ".textblock").linkToSavannah();
  $(":not(a)", ".textblock").linkToGitLabCommit();
  $(":not(a)", ".textblock").linkToGitLabMR();
});
