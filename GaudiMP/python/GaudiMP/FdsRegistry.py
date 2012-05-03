# @file GaudiMP.FdsRegistry.py
# @purpose file descriptor registration and handling
# @author Mous Tatarkhanov <tmmous@berkeley.edu>

_O_ACCMODE = 3  #access-mode check for file flags.

import logging
msg = logging.getLogger( 'FdsRegistry' )

class FdsDict(dict):
    name = "fds_dict"
    curdir = None
    
    def __missing__(self, key):
        self[key] = ""
        return ""

    def fname(self,i):
        if i in self:
            return self[i][0]
        else:
            msg.warning ("fds_dict:fname: No Key %s" % i)
            return ""

    def fds(self, fname):
        return [i for i, v in self.iteritems() if v[0]==fname]

    def has_name(self, fname):
        for v in self.values():
            if (v[0] == fname):
                return True
        return False
    
    # i - is the fd index (int)
    def add(self, i, fname, iomode, flags):
        self[i] = (fname, iomode, flags)
        return 

    def iomode(self,i):
        if i in self:
            return self[i][1]
        else:
            msg.warning ("fds_dict:iomode: No Key %s" % i)
            return ""
    
    def get_output_fds(self):
        return [i for i in self.keys() if self[i][1]=='<OUTPUT>']
        
    def get_input_fds(self):
        return [i for i in self.keys() if self[i][1]=='<INPUT>']
    
    def get_fds_in_dir(self, dir=""):
        import os
        if dir == "" and self.curdir is not None:
            dir = self.curdir
        msg.debug("get_fds_in_dir(%s)" % dir)
        return [i for i in self.keys() 
                if os.path.samefile(os.path.dirname(self[i][0]), dir) ]
    
    def create_symlinks(self, wkdir=""):
        """
        create necessary symlinks in worker's dir if the fd is <INPUT>
        otherwise copy <OUTPUT> file 
        """
        import os,shutil
        msg.info("create_symlinks: %s" % self.get_fds_in_dir()) 
        #some files expected to be in curdir
        for fd in self.get_fds_in_dir():   
            src = self[fd][0]
            iomode = self[fd][1]
            dst = os.path.join(wkdir, os.path.basename(src))
            if iomode == "<INPUT>":
                if os.path.exists(dst):
                    # update_io_registry took care of this
                    msg.debug("fds_dict.create_symlink:update_io_registry took care of src=%s" % src)
                    pass
                else:
                    msg.debug("fds_dict.create_symlink:(symlink) src=%s, iomode=%s" % (src,iomode))
                    os.symlink(src, dst)
            else:
                msg.debug("fds_dict.create_symlink: (copy) src=%s, dst=%s" % (src, dst))
                shutil.copy(src, dst)
                pass
        return
    
    def extract_fds(self, dir=""):
        """parse the fds of the processs -> build fds_dict
        """
        import os, fcntl
        msg.info("extract_fds: making snapshot of parent process file descriptors")
        self.curdir = os.path.abspath(os.curdir)
        iomode = '<INPUT>'

        procfd = '/proc/self/fd'
        fds = os.listdir(procfd)
        for i in fds:
            fd = int(i)       # spurious entries should raise at this point
            if (fd==1 or fd==2):
                #leave stdout and stderr to redirect_log
                continue
            elif (fd==0):
                #with only a single controlling terminal, leave stdin alone
                continue

            try:
                realname = os.path.realpath(os.path.join(procfd,i))
            except (OSError, IOError, TypeError):
                # can fail because the symlink resolution (why is that needed
                # anyway?) may follow while a temp file disappears
                msg.debug( "failed to resolve: %s ... skipping", os.path.join(procfd,i) )
                continue

            if os.path.exists(realname):
                try:
                    flags = fcntl.fcntl(fd, fcntl.F_GETFL)
                    if (flags & _O_ACCMODE) == 0: #read-only --> <INPUT>
                        iomode = "<INPUT>"
                    else:
                        iomode = "<OUTPUT>"
                
                    self.add(fd, realname, iomode, flags)
                except (OSError, IOError):
                    # likely saw a temoorary file; for now log a debug
                    # message, but this is fine if silently ignored
                    msg.debug( "failed access to: %s ... skipping", realname )
                    continue

                # at this point the list of fds may still include temp files
                # TODO: figure out if they can be identified (seeing /tmp is
                # not enough as folks like to run with data from /tmp b/c of
                # space constraints on /afs

        msg.debug( "extract_fds.fds_dict=%s" % self)
        return
        
    pass #FdsDict

