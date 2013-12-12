#!/usr/bin/env python

__author__ = "Marco Clemencic <Marco.Clemencic@cern.ch>"
__version__ = "$Id: update_versions.py,v 1.3 2008/11/10 19:43:31 marcocle Exp $"

import os, re, sys, time

# special_packages = [ "Gaudi", "GaudiSys", "GaudiExamples" ]

# guess current version
_req_version_pattern = re.compile(r"^\s*version\s*(v[0-9]+r[0-9]+(?:p[0-9]+)?)\s*$")
_cml_version_pattern = re.compile(r"^\s*gaudi_subdir\s*\(\s*\S+\s+(v[0-9]+r[0-9]+(?:p[0-9]+)?)\)\s*$")
_hwaf_hpyscript_version_pattern = re.compile(r"^\s*[\"']version[\"']\s*:\s*[\"'](v[0-9]+r[0-9]+(?:p[0-9]+)?)[\"'].*$")
_hwaf_ymlscript_version_pattern = re.compile(r"^\s*version\s*:\s*([\"']|)(v[0-9]+r[0-9]+(?:p[0-9]+)?)([\"']|).*$")

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

def change_cml_version(cml, newversion):
    if os.path.exists(cml):
        out = []
        changed = False
        for l in open(cml):
            m = _cml_version_pattern.match(l)
            if m and m.group(1) != newversion:
                print "%s: %s -> %s"%(cml, m.group(1), newversion)
                l = l.replace(m.group(1), newversion)
                changed = True
            out.append(l)
        if changed:
            open(cml, "w").writelines(out)

def change_hwaf_version(pkgdir, newversion):
    hname = os.path.join(pkgdir, "hscript.py")
    yname = os.path.join(pkgdir, "hscript.yml")
    fname = None
    pat = None
    if os.path.exists(hname):
        pat   = _hwaf_hpyscript_version_pattern
        fname = hname
    elif os.path.exists(yname):
        pat   = _hwaf_ymlscript_version_pattern
        fname = yname
    else:
        print ("*** package [%s] has no hwaf script" % pkgdir)
        return
    
    out = []
    changed = False
    for l in open(hname):
        m = pat.match(l)
        if m and m.group(1) != newversion:
            print "%s: %s -> %s"%(fname, m.group(1), newversion)
            l = l.replace(m.group(1), newversion)
            changed = True
        out.append(l)
    if changed:
        open(fname, "w").writelines(out)
    return

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
    # update CMakeLists.txt
    cml = os.path.normpath(os.path.join(packagedir, "..", "CMakeLists.txt"))
    change_cml_version(cml, newversion)
    if "GaudiKernel" in packagedir:
        cml = os.path.normpath(os.path.join(packagedir, "..", "src", "Util", "CMakeLists.txt"))
        change_cml_version(cml, newversion)

    # update hscripts
    change_hwaf_version(packagedir, newversion)
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

def extract_recent_rel_notes(filename):
    changelog_entry = re.compile(r'^(! [0-9]{4}-[0-9]{2}-[0-9]{2} -)|!?============')
    separator_entry = re.compile(r'^!?============')
    notes = []
    state = "searching"
    for l in open(filename):
        # looking for the first changelog entry
        if state == "searching":
            if changelog_entry.match(l):
                state = "found"
        # when found, we start collecting lines until the next separator
        if state == "found":
            if not separator_entry.match(l):
                notes.append(l)
            else:
                break
    # remove trailing empty lines
    while notes and not notes[-1].strip(): notes.pop()
    return "".join(notes)

def add_release_separator_bar(filename, pkg, version):
    changelog_entry = re.compile(r'^(! [0-9]{4}-[0-9]{2}-[0-9]{2} -)|============')
    title = " %s %s " % (pkg, version)
    letf_chars = (78 - len(title)) / 2
    right_chars = 78 - letf_chars - len(title)
    separator = ("=" * letf_chars) + title + ("=" * right_chars) + "\n"
    out = []
    found = False
    for l in open(filename):
        # looking for the first changelog entry
        if not found:
            if changelog_entry.match(l):
                out.append(separator)
                found = True
        # if found, just go on appending lines
        out.append(l)
    if found:
        open(filename,"w").writelines(out)
    else:
        print "Warning: could not update release.notes in %s" % pkg

def main():

    # Find the version of LCGCMT
    m = re.search("use\s*LCGCMT\s*LCGCMT_(\S*)",open(os.path.join("..","..","cmt","project.cmt")).read())
    if m:
        LCGCMTVers = m.group(1)
        print "Using LCGCMT", LCGCMTVers
    else:
        print "Cannot find LCGCMT version"
        sys.exit(1)

    # Collect all the packages in the project with their directory
    # (I want to preserve the order that cmt broadcast gives)
    all_packages_tmp = []
    exec(os.popen(r"""cmt broadcast 'echo "all_packages_tmp.append((\"<package>\"", \"$PWD\""))"'""","r").read())
    all_packages_names = []
    all_packages = {}
    for k,v in all_packages_tmp:
        all_packages_names.append(k)
        all_packages[k] = v

    # Packages which version must match the version of the project
    special_packages = ["Gaudi", "GaudiExamples", "GaudiSys", "GaudiRelease"]

    # Ask for the version of the project
    old_version = extract_version("requirements")
    new_version = raw_input("The old version of the project is %s, which is the new one? " % old_version)

    old_versions = {}
    release_notes = {}
    new_versions = {}
    # for each package in the project check if there were changes and ask for the new version number
    for pkg in all_packages_names:
        reqfile = os.path.join(all_packages[pkg], "requirements")
        relnotefile = os.path.join(all_packages[pkg], "..", "doc", "release.notes")
        old_versions[pkg] = extract_version(reqfile)
        if os.path.exists(relnotefile): # ignore missing release.notes
            release_notes[pkg] = extract_recent_rel_notes(relnotefile)
        else:
            release_notes[pkg] = ""
        if pkg in special_packages:
            new_versions[pkg] = new_version
        else:
            if release_notes[pkg]:
                new_versions[pkg] = raw_input("\nThe old version of %s is %s, this are the changes:\n%s\nWhich version you want (old is %s)? " % (pkg, old_versions[pkg], release_notes[pkg], old_versions[pkg]))
            else:
                new_versions[pkg] = old_versions[pkg]
        # update infos
        if new_versions[pkg] != old_versions[pkg]:
            change_version(all_packages[pkg], new_versions[pkg])
            if os.path.exists(relnotefile):
                add_release_separator_bar(relnotefile, pkg, new_versions[pkg])
        print "=" * 80
    # The changes in the GaudiRelease requirements for the other packages can be postponed to now
    reqfile = os.path.join(all_packages["GaudiRelease"], "requirements")
    out = []
    for l in open(reqfile):
        sl = l.strip().split()
        if sl and sl[0] == "use":
            if sl[1] in new_versions:
                if sl[2] != new_versions[sl[1]]:
                    l = l.replace(sl[2], new_versions[sl[1]])
        out.append(l)
    open(reqfile, "w").writelines(out)

    # update the global release notes
    new_lines = []
    new_lines.append("<!-- ====================================================================== -->")
    data = { "vers": new_version, "date": time.strftime("%Y-%m-%d") }
    new_lines.append('<h2><a name="%(vers)s">Gaudi %(vers)s</a> (%(date)s)</h2>' % data)
    data = { "vers": LCGCMTVers }
    new_lines.append('<h3>Externals version: <a href="http://lcgsoft.cern.ch/index.py?page=cfg_overview&cfg=%(vers)s">LCGCMT_%(vers)s</a></h3>' % data)
    new_lines.append("<h3>General Changes</h3>")
    new_lines.append('<ul>\n<li><br/>\n    (<span class="author"></span>)</li>\n</ul>')
    new_lines.append("<h3>Packages Changes</h3>")
    new_lines.append("<ul>")
    for pkg in all_packages_names:
        if release_notes[pkg]:
            new_lines.append('<li>%s (%s):\n<ul>\n<li><br/>\n    (<span class="author"></span>)</li>\n</ul>\n<pre>'%(pkg,new_versions[pkg]))
            new_lines.append(release_notes[pkg].replace('&','&amp;') \
                                               .replace('<','&lt;') \
                                               .replace('>','&gt;') + "</pre>")
            new_lines.append("</li>")
    new_lines.append("</ul>")

    global_rel_notes = os.path.join("..", "doc", "release.notes.html")
    out = []
    separator = re.compile("<!-- =+ -->")
    block_added = False
    for l in open(global_rel_notes):
        if not block_added and separator.match(l.strip()):
            out.append("\n".join(new_lines) + "\n")
            block_added = True
        out.append(l)
    open(global_rel_notes, "w").writelines(out)

    # update the global CMakeLists.txt
    global_cmakelists = os.path.join("..","..","CMakeLists.txt")
    out = []
    for l in open(global_cmakelists):
        if l.strip().startswith('gaudi_project'):
            l = 'gaudi_project(Gaudi %s)\n' % new_version
        out.append(l)
    open(global_cmakelists, "w").writelines(out)

if __name__ == '__main__':
    main()
