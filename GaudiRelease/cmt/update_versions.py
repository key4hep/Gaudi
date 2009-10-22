#!/usr/bin/env python

__author__ = "Marco Clemencic <Marco.Clemencic@cern.ch>"
__version__ = "$Id: update_versions.py,v 1.3 2008/11/10 19:43:31 marcocle Exp $"

import os, re, sys, time

# special_packages = [ "Gaudi", "GaudiSys", "GaudiExamples" ]

# guess current version
_req_version_pattern = re.compile(r"^\s*version\s*(v[0-9]+r[0-9]+(?:p[0-9]+)?)\s*$")
def extract_version(f):
    """
    Find the version number in a requirements file.
    """ 
    global _req_version_pattern
    for l in open(f):
        m = _req_version_pattern.match(l)
        if m:
            return m.group(1)
    return None

def change_version(packagedir, newversion):
    """
    Compare the version of the package with the new one and update the package if
    needed.
    
    Returns true if the package have been modified. 
    """
    global _req_version_pattern
    changed = False
    out = []
    req = os.path.join(packagedir,"requirements")
    for l in open(req):
        m = _req_version_pattern.match(l)
        if m:
            if m.group(1) != newversion:
                print "%s: %s -> %s"%(packagedir,m.group(1),newversion)
                l = l.replace(m.group(1),newversion)
                changed = True
        out.append(l)
    if changed:
        open(req,"w").writelines(out)
    # verify the version.cmt file
    ver = os.path.join(packagedir,"version.cmt")
    if os.path.exists(ver):
        current = open(ver).read().strip()
        if current != newversion:
            open(ver,"w").write(newversion + "\n")
    return changed

_use_pattern = re.compile(r"^\s*use\s*(\w+)\s*(v[0-9]+r[0-9]+(?:p[0-9]+)?)\s*(\w+)?\s*$")
def gather_new_versions(f):
    global _use_pattern
    versions = {}
    for l in open(f):
        m = _use_pattern.match(l)
        if m:
            versions[m.group(1)] = m.group(2)
    return versions

def update_release_notes(filename, pkg, version):
    changelog_entry = re.compile(r'^(! [0-9]{4}-[0-9]{2}-[0-9]{2} -)|============')
    separator_entry = re.compile(r'^============')
    separator = "================ %s %s ====================================\n"
    out = []
    notes = []
    state = "searching"
    for l in open(filename):
        if state == "searching":
            if changelog_entry.match(l):
                out.append(separator%(pkg,version))
                state = "found"
        if state == "found":
            if not separator_entry.match(l):
                notes.append(l)
            else:
                state = "over"
        out.append(l)
    if state != "searching":
        open(filename,"w").writelines(out)
    else:
        print "Warning: could not update release.notes in %s"%pkg
    # remove trailing empty lines
    while notes and not notes[-1].strip(): notes.pop()
    return "".join(notes)

def main():
    
    m = re.search("use\s*LCGCMT\s*LCGCMT_(\S*)",open(os.path.join("..","..","cmt","project.cmt")).read())
    if m:
        LCGCMTVers = m.group(1)
    else:
        print "Cannot find LCGCMT version"
        sys.exit(1)  
    
    all_packages = {}
    exec(os.popen(r"""cmt broadcast 'echo "all_packages[\"<package>\"]" = \"$PWD\"'""","r").read())

    versions = gather_new_versions("requirements")
    release_notes = {}
    for pkg in all_packages:
        if pkg in versions:
            print "Updating %s"%pkg
            if change_version(all_packages[pkg],versions[pkg]):
                relnotes = os.path.join(all_packages[pkg],"..","doc","release.notes")
                release_notes[pkg] = update_release_notes(relnotes, pkg, versions[pkg])
            else:
                print "%s unchanged"%pkg

    print "<!-- ====================================================================== -->"
    data = { "vers": versions["Gaudi"], "date": time.strftime("%Y-%m-%d") }
    print '<h2><a name="%(vers)s">Gaudi %(vers)s</a> (%(date)s)</h2>' % data
    data = { "vers": LCGCMTVers }
    print '<h3>Externals version: <a href="http://lcgsoft.cern.ch/index.py?page=cfg_overview&cfg=%(vers)s">LCGCMT_%(vers)s</a></h3>' % data
    print "<h3>Packages Changes</h3>"
    print "<ul>"
    for pkg in release_notes:
        if release_notes[pkg]:
            print '<li>%s (%s):\n<ul>\n<li><br/>\n    (<span class="author"></span>)</li>\n</ul>\n<pre>'%(pkg,versions[pkg])
            print release_notes[pkg].replace('&','&amp;') \
                                    .replace('<','&lt;') \
                                    .replace('>','&gt;') + "</pre>"
            print "</li>"
    print "</ul>"

if __name__ == '__main__':
    main()