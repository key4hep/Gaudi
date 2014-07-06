function LinkToSavannah()
{
  patch = "<a href='https://savannah.cern.ch/patch/index.php?$2'>$1&nbsp;#$2</a>";
  bug = "<a href='https://savannah.cern.ch/bug/index.php?$2'>$1&nbsp;#$2</a>";
  task = "<a href='https://savannah.cern.ch/task/index.php?$2'>$1&nbsp;#$2</a>";
  s = document.getElementsByTagName("body")[0].innerHTML;
  s = s.replace(/([Pp]atch) #([0-9]*)/g,patch);
  s = s.replace(/([Tt]ask) #([0-9]*)/g,task);
  s = s.replace(/([Bb]ug) #([0-9]*)/g,bug);
  document.getElementsByTagName("body")[0].innerHTML = s;
}

// replaces the occurrences of ([A-Z]+-[0-9]+) with the link to the JIRA issue
function LinkToJIRA()
{
  s = document.getElementsByTagName("body")[0].innerHTML;
  s = s.replace(/([A-Z]+)-[0-9]+/g, function(match, p1) {
    var url = "https://its.cern.ch/jira/browse/";
    if (p1 == "ROOT" || p1 == "SPI" || p1 == "PF" || p1 == "CFHEP" ||
        p1 == "CVM")
        url = "https://sft.its.cern.ch/jira/browse/";
    else if (p1 == "GUG")
    	return match;
    return '<a href="' + url + match + '">' + match + '</a>';
  });
  document.getElementsByTagName("body")[0].innerHTML = s;
}

$(function () {
  LinkToSavannah();
  LinkToJIRA();
});
