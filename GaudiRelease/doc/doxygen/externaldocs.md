Related external libraries    {#externaldocs}
==========================

\htmlonly
<h3><center>(from LCGCMT version $(LCGCMTVERS))</h3></center>
 <UL>
  <li>LCG Application Area:
  <ul>
    <LI> <a href="http://lcgapp.cern.ch/doxygen/CORAL/$(CORALVERS)/doxygen/html/" target="_blank">$(CORALVERS)</A> (relational access)
    <LI> <a href="http://lcgapp.cern.ch/doxygen/COOL/$(COOLVERS)/doxygen/html/" target="_blank">$(COOLVERS)</A> (conditions database)
    <LI> <a href="http://lcgapp.cern.ch/doxygen/ROOT/$(ROOTVERS)/doxygen/html/" target="_blank">ROOT $(ROOTVERS)</A> (persistency)
  </ul></li>
  <li>Externals:
  <UL>
    <script type="text/javascript" language="JavaScript">
      var boostVers = "$(BoostVERS)";
      var url = "http://www.boost.org/doc/libs/" + boostVers.replace(/\./g,"_") + "/libs/libraries.htm";
      document.write("<LI> <a href=\"" + url + "\" target=\"_blank\">Boost " + boostVers + "</a>");
    </script>
    <LI> <a href="http://lcgsoft.cern.ch/index.py?page=pkg_overview&pkg=uuid" target="_blank">uuid $(uuidVERS)</A>
    <LI> <a href="http://lcgsoft.cern.ch/index.py?page=pkg_overview&pkg=gccxml" target="_blank">GCC-XML $(GCCXMLVERS)</A>
    <LI> <a href="http://aida.freehep.org/doc/v$(AIDAVERS)/api/index.html" target="_blank">AIDA $(AIDAVERS)</A> (histogramming)
    <script type="text/javascript" language="JavaScript">
      var xercesVers = "$(XercesCVERS)";
      var url = "http://xml.apache.org/xerces-c/apiDocs-" + xercesVers[0] + "/index.html";
      document.write("<LI> <a href=\"" + url + "\" target=\"_blank\">Xerces-C " + xercesVers + "</a>");
    </script>
    <LI> <a href="http://www.gnu.org/software/gsl/manual/html_node/index.html" target="_blank">GNU Scientific Library (GSL) $(GSLVERS)</A>
    <LI> <a href="http://www.python.org/doc/$(PythonVERS)/" target="_blank">Python $(PythonVERS)</A> (scripting and interactivity)
    <LI> <a href="http://lcgapp.cern.ch/project/simu/HepMC/" target="_blank">HepMC $(HepMCVERS)</A>
    <LI> <a href="http://www.codesourcery.com/qmtest/" target="_blank">QMTest $(QMtestVERS)</A>
  </UL></li>
</UL>
See also:
  Standard Template Library
    (<a href="http://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/index.html" target="_blank">gcc-4.3</a>,
     <a href="http://gcc.gnu.org/onlinedocs/gcc-4.6.3/libstdc++/api/" target="_blank">gcc-4.6</a>)
\endhtmlonly
