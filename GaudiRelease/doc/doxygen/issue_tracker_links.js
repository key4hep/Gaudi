// Convert text referencing JIRA or Savannah items into links to the items.
// Requires jQuery.

function isTextNode() {
  return this.nodeType == Node.TEXT_NODE;
}

JIRA_ITEM_REGEX = /\b([A-Z]+)-[0-9]+/g;
SAVANNAH_ITEM_REGEX = /\b([Pp]atch|[Tt]ask|[Bb]ug) #([0-9]*)/g;

function conatinsSavannahItems(text) {
  return SAVANNAH_ITEM_REGEX.test(text);
}

jQuery.fn.linkToSavannah = function () {
  this.contents()
      .filter(isTextNode)
      .filter(function() {return conatinsSavannahItems(this.nodeValue);})
      .replaceWith(function() {
        return this.nodeValue.replace(SAVANNAH_ITEM_REGEX,
    	       function(match, p1, p2) {
    		     p1 = p1.toLowerCase();
    		     return '<a href="https://savannah.cern.ch/' + p1 +
    		            '/index.php?' + p2 + '">' + match + '</a>';
        });
  });
  return this;
}

function conatinsJIRAItems(text) {
  return JIRA_ITEM_REGEX.test(text);
}

jQuery.fn.linkToJIRA = function () {
  this.contents()
      .filter(isTextNode)
      .filter(function() {return conatinsJIRAItems(this.nodeValue);})
      .replaceWith(function() {
        return this.nodeValue.replace(/\b([A-Z]+)-[0-9]+/g,
               function(match, p1) {
                 var url = "https://its.cern.ch/jira/browse/";
                 if (p1 == "ROOT" || p1 == "SPI" || p1 == "PF" || p1 == "CFHEP" ||
                     p1 == "CVM")
                   url = "https://sft.its.cern.ch/jira/browse/";
                 return '<a href="' + url + match + '">' + match + '</a>';
               });
        });
  return this;
};

$(function() {
  $(":not(a)", ".textblock")
    .linkToJIRA()
    .linkToSavannah();
});
