#!/usr/bin/env python

# @file: GaudiPolicy/cmt/fragments/merge_files.py
# @purpose: merge_files <fragment> file into a 'per-project' <merged> file
# @author: Sebastien Binet <binet@cern.ch>

import os
import sys
from datetime import datetime
import locker

def mergeFiles( fragFileName, mergedFileName, commentChar, doMerge ):

    isNewFile = not os.path.exists(mergedFileName)
    
    # create an empty file if it does not exist
    # "append mode" ensures that, in case of two processes trying to
    # create the file, they do not truncate each other file
    if isNewFile:
        # check if the destination directory exists
        path_to_file = os.path.split(mergedFileName)[0]
        if not os.path.isdir(path_to_file):
            # if doesn't exist, create it
            os.makedirs(path_to_file)
        open(mergedFileName,'a')
    
    mergedFile = open( mergedFileName, 'r+' )

    # locking file, gaining exclusive access to it
    lock = locker.lock( mergedFile )
    try:
    

        startMark = "%s --Beg %s" % ( commentChar,
                                      os.path.basename(fragFileName) )
        timeMark  = "%s --Date inserted: %s" % ( commentChar,
                                                 str(datetime.now()) )
        endMark   = "%s --End %s" % ( commentChar,
                                      os.path.basename(fragFileName) )

        newLines = [ ]
        skipBlock = False
        for line in mergedFile.readlines():
            if line.startswith(startMark):
                skipBlock = True
                # remove all the empty lines occurring before the start mark
                while (len(newLines) > 0) and (newLines[-1].strip() == ''):
                    newLines.pop()
            if not skipBlock:
                newLines.append(line)
            if line.startswith(endMark):
                skipBlock = False
        if skipBlock:
            print "WARNING: missing end mark ('%s')"%endMark
    
        if doMerge:
            # I do not want to add 2 empty lines at the beginning of a file
            if not isNewFile:
                newLines.append('\n\n')
            newLines.append(startMark+'\n')
            newLines.append(timeMark+'\n')
    
            for line in open( fragFileName, 'r' ).readlines():
                newLines.append(line)
        
            newLines.append(endMark+'\n')
    
        mergedFile.seek(0)
        mergedFile.truncate(0)
        mergedFile.writelines(newLines)

    finally:
        # unlock file
        locker.unlock( mergedFile )
    
    return 0

if __name__ == "__main__":

    from optparse import OptionParser
    parser = OptionParser(usage="usage: %prog [options]")
    parser.add_option(
        "-i",
        "--input-file",
        dest = "fragFileName",
        default = None,
        help = "The path and name of the file one wants to merge into the 'master' one"
        )
    parser.add_option(
        "-m",
        "--merged-file",
        dest = "mergedFileName",
        default = None,
        help = "The path and name of the 'master' file which will hold the content of all the other fragment files"
        )
    parser.add_option(
        "-c",
        "--comment-char",
        dest = "commentChar",
        default = "#",
        help = "The type of the commenting character for the type of files at hand (this is an attempt at handling the largest possible use cases)"
        )
    parser.add_option(
        "--do-merge",
        dest = "doMerge",
        action = "store_true",
        default = True,
        help = "Switch to actually carry on with the merging procedure"
        )
    parser.add_option(
        "--un-merge",
        dest = "unMerge",
        action = "store_true",
        default = False,
        help = "Switch to remove our fragment file from the 'master' file"
        )
    parser.add_option(
        "--stamp-dir",
        dest = "stampDir",
        action = "store",
        default = None,
        help = "Create the stamp file in the specified directory. If not specified"
              +" the directory of the source file is used." 
        )
    
    (options, args) = parser.parse_args()

    # ensure consistency...
    options.doMerge = not options.unMerge

    # allow command line syntax as
    #   merge_files.py [options] <fragment file> <merged file>
    if len(args) > 0 and args[0][0] != "-": options.fragFileName   = args[0]
    if len(args) > 1 and args[1][0] != "-": options.mergedFileName = args[1]
    
    sc = 1
    if not options.fragFileName or \
       not options.mergedFileName :
        str(parser.print_help() or "")
        print "*** ERROR ***",sys.argv
        sys.exit(sc)
        pass

    if options.stampDir is None:
        stampFileName = options.fragFileName + ".stamp"
    else:
        stampFileName = os.path.join(options.stampDir,
                                     os.path.basename(options.fragFileName)
                                         + ".stamp")
    # Configure Python logging
    import logging
    logging.basicConfig(level = logging.INFO)
    
    if "GAUDI_BUILD_LOCK" in os.environ:
        globalLock = locker.LockFile(os.environ["GAUDI_BUILD_LOCK"], temporary =  True) 
    else:
        globalLock = None
    
    try:
        sc = mergeFiles( options.fragFileName, options.mergedFileName,
                         options.commentChar,
                         doMerge = options.doMerge )
        stamp = open( stampFileName, 'w' )
        stamp.close()
    except IOError, err:
        print "ERROR:",err
    except Exception, err:
        print "ERROR:",err
    except:
        print "ERROR: unknown error !!"
    
    del globalLock
    
    sys.exit( sc )
