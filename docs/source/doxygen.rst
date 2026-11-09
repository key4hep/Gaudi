API documentation (Doxygen)
===========================

The reference documentation of the Gaudi code (classes, headers, release notes)
is generated from the sources with `Doxygen <https://www.doxygen.nl/>`_ for the
development version (``master``) and for each release.

.. raw:: html

   <p id="doxygen-versions-status"></p>
   <ul id="doxygen-versions"></ul>
   <noscript>
     <p>The list of available versions is at
     <a href="doxygen/">doxygen/</a>.</p>
   </noscript>
   <script>
   // The Doxygen documentation is not part of this Sphinx project: it is
   // published separately (at each release) under doxygen/, together with the
   // list of the available versions in doxygen/list.txt, which we read here so
   // that this page does not need to be rebuilt for each new release.
   (function () {
     var status = document.getElementById("doxygen-versions-status");
     var list = document.getElementById("doxygen-versions");
     fetch("doxygen/list.txt")
       .then(function (response) {
         if (!response.ok) { throw new Error(response.statusText); }
         return response.text();
       })
       .then(function (text) {
         text.split("\n").filter(Boolean).forEach(function (name) {
           var item = document.createElement("li");
           var link = document.createElement("a");
           link.href = "doxygen/" + name + "/index.html";
           link.textContent = name;
           item.appendChild(link);
           if (name === "master") {
             item.appendChild(document.createTextNode(" (development version)"));
           }
           list.appendChild(item);
         });
       })
       .catch(function () {
         status.innerHTML = 'The list of available versions cannot be loaded, ' +
           'see <a href="doxygen/">doxygen/</a>.';
       });
   })();
   </script>
