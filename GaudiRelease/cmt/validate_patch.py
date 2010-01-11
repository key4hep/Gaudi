#!/usr/bin/env python
"""
Simple script for automatic validation of a patch.

Usage:
   validate_patch.py <savannah patch id>
   validate_patch.py file.patch
"""

__author__ = "Marco Clemencic <marco.clemencic@cern.ch>"

import os, sys
import logging
from shutil import rmtree
from tempfile import mkdtemp
from subprocess import Popen, PIPE

from HTMLParser import HTMLParser
from urllib import urlopen

class PatchData(object):
    def __init__(self, id = None, title = None, files = None):
        self.id = None
        self.title = None
        if files is None:
            self.files = []
    def __repr__(self):
        r = self.__class__.__name__ + "("
        fields = []
        if self.id is not None:
            fields.append("id=%r" % self.id)
        if self.title is not None:
            fields.append("title=%r" % self.title)
        if self.files:
            fields.append("files=%r" % self.files)
        return "%s(%s)" % (self.__class__.__name__, ",".join(fields))

## parse the 
class SavannahParser(HTMLParser):
    __attachments_id__ = "hidsubpartcontentattached"
    def __init__(self):
        HTMLParser.__init__(self)
        # data collected
        self.patch = None
        
        # parsing flags and temporary data
        self._parsingAttachments = 0 # depth of span tag in the attachment block
        self._currentFileId = None
        self._currentFileData = ""
        
        # temporary storage
    
    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if tag == "html":
            # new file: new patch data 
            self.patch = PatchData()
        elif tag == "span":
            if attrs.get("id", None) == self.__attachments_id__:
                self._parsingAttachments = 1
            elif self._parsingAttachments:
                self._parsingAttachments += 1
        elif (self._parsingAttachments
              and tag == "a"
              and "file_id=" in attrs.get("href", "")):
            self._currentFileId = attrs["href"].split("file_id=")[-1]
    def handle_endtag(self, tag):
        if tag == "span" and self._parsingAttachments:
            self._parsingAttachments -= 1
        elif tag == "a" and self._currentFileId:
            #print self._currentFileData
            filename = self._currentFileData.split(":")[-1].strip()
            #print filename, self._currentFileId
            self.patch.files.append((filename, int(self._currentFileId)))
            self._currentFileId = None
            self._currentFileData = ""
            
    def handle_data(self, data):
        if self._currentFileId:
            data = data.replace("&nbsp;", " ")
            self._currentFileData += data

def get_patch_info_x(patch):
    patch = int(patch)
    server = "savannah.cern.ch"
    path =  "/patch/?%d" % patch
    conn = httplib.HTTPSConnection(server)
    conn.request("GET", path)
    r = conn.getresponse()
    if r.status == 200:
        pass
    else:
        raise RuntimeError(r.status, r.reason, "https://" + server + path)
    parser = SavannahParser()
    parser.feed(r.read())
    parser.close()
    conn.close()
    return parser.patch

def get_patch_info(patch):
    patch = int(patch)
    parser = SavannahParser()
    parser.feed(urlopen("https://savannah.cern.ch/patch/?%d" % patch).read())
    parser.close()
    return parser.patch

def get_patch_data(file_id):
    file_id = int(file_id)
    return urlopen("https://savannah.cern.ch/patch/download.php?file_id=%d" % file_id).read()

class TempDir(object):
    """Class to create a temporary directory."""
    def __init__(self, suffix="", prefix="tmp", dir=None, keep_var="KEEPTEMPDIR"):
        """Constructor.
        
        'keep_var' is used to define which environment variable will prevent the
        deletion of the directory.
        
        The other arguments are the same as tempfile.mkdtemp.
        """
        self._keep_var = keep_var 
        self._name = mkdtemp(suffix, prefix, dir)

    def getName(self):
        """Returns the name of the temporary directory"""
        return self._name
    
    def __str__(self):
        """Convert to string."""
        return self.getName()

    def __del__(self):
        """Destructor.
        
        Remove the temporary directory.
        """
        if self._name:
            if self._keep_var in os.environ:
                logging.info("%s set: I do not remove the temporary directory '%s'",
                             self._keep_var, self._name)
                return
            rmtree(self._name)

def check_out_gaudi(path):
    return Popen(["svn", "co", "http://svnweb.cern.ch/guest/gaudi/Gaudi/trunk", os.path.join(path, "Gaudi")]).wait()

def apply_patch(patch_data, path):
    proc = Popen(["patch", "-p0", "--batch"], cwd = path, stdin = PIPE)
    proc.communicate(patch_data)
    return proc.returncode

def check(path):
    return Popen(" ".join(["cmt", "show", "projects"]), shell = True, cwd = path).wait()

def build(path):
    if "LBCONFIGURATIONROOT" in os.environ:
        cmd = ["make",
               "-f", os.path.join(os.environ["LBCONFIGURATIONROOT"], "data", "Makefile")]
        if "use-distcc" in os.environ.get("CMTEXTRATAGS",""): 
               cmd += ["-j", "6"]
    else:
        cmd = ["cmt", "-pack=GaudiRelease", "broadcast", "cmt", "make", "all_groups"]
    return Popen(" ".join(cmd),
                 shell = True,
                 cwd = path).wait()
def test(path):
    cmd = ["cmt", "-pack=GaudiRelease", "TestProject"]
    proc = Popen(" ".join(cmd),
                 stdout = PIPE,
                 shell = True,
                 cwd = path)
    output = []
    while proc.poll() is None:
        chunk = proc.stdout.read(256)
        output.append(chunk) 
        sys.stdout.write(chunk)
        sys.stdout.flush()
    chunk = proc.stdout.read(256)
    output.append(chunk) 
    sys.stdout.write(chunk)
    sys.stdout.flush()
    if proc.returncode:
        # immediately return in case of failure
        return proc.returncode
    # look for failures in the output
    output = ("".join(output)).splitlines()
    for l in output:
        l = l.strip()
        if ": FAIL" in l or ": ERROR" in l:
            return 1 
    return 0

def main():
    logging.basicConfig()
    if len(sys.argv) != 2:
        print """Usage:
   validate_patch.py <savannah patch id>
   validate_patch.py file.patch
"""
        return 2
    patch_id = sys.argv[1]
    if os.path.isfile(patch_id):
        patch_data = open(patch_id, "rb").read()
    else:
        patch = get_patch_info(patch_id)
        patch_file_id = patch.files[0][1]
        patch_data = get_patch_data(patch_file_id)
    
    td = TempDir(prefix = patch_id + "-")
    if check_out_gaudi(str(td)) != 0:
        print "Sorry, problems checking out Gaudi. Try again."
        return 0
    top_dir = os.path.join(str(td), "Gaudi")
    open(os.path.join(top_dir, patch_id) ,"wb").write(patch_data)
    
    revision = -1
    for l in Popen(["svn", "info", top_dir], stdout = PIPE).communicate()[0].splitlines():
        if l.startswith("Revision:"):
            revision = int(l.split()[-1])
            break
    
    actions = [(lambda path: apply_patch(patch_data, path), "application of the patch"),
               (check, "check of the configuration"),
               (build, "build"),
               (test, "test"),
               ]
    failure = False
    for action, title in actions:
        if action(top_dir) != 0:
            failure = title
            break
    
    if failure:
        print "*** Patch %s failed during %s (using revision r%d) ***" % (patch_id, failure, revision)
        return 1
    
    print "*** Patch %s succeeded (using revision r%d) ***" % (patch_id, revision)
    return 0

if __name__ == "__main__":
    sys.exit(main())
