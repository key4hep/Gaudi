Procedure to Release Gaudi
==========================

## Updated versions and release notes

1. use the script `GaudiRelease/prepare_gaudi_release.py` to update the
   versions of all packages and prepare their release notes
2. update `GaudiRelease/doc/release.notes.html` from the release notes
   in JIRA (see the [list of versions](https://its.cern.ch/jira/browse/GAUDI/?selectedTab=com.atlassian.jira.jira-projects-plugin:versions-panel))
3. update/correct the release notes of the packages or of the project if
   needed
4. push the changes to a branch and create a merge request


## Tagging

1. using the GitLab interface, [create the tag](https://gitlab.cern.ch/gaudi/Gaudi/tags/new)
2. close the corresponding milestone from the [list of milestones](https://gitlab.cern.ch/gaudi/Gaudi/milestones)
3. release the version in JIRA (see the [list of versions](https://its.cern.ch/jira/browse/GAUDI/?selectedTab=com.atlassian.jira.jira-projects-plugin:versions-panel))


## Update the web page

After the release is installed in AFS, run the following commands:

    cd /afs/.cern.ch/sw/Gaudi/www
    ./add_release.sh vXrY
    afs_admin vos_release .


## Synchronizing SVN

1. synchronize the version in the trunk:

    cd /afs/cern.ch/sw/Gaudi/git/gateway/Gaudi
    git checkout master
    git pull --ff-only
    git checkout gw/trunk
    git merge --no-ff -c 'synchronize with Gaudi vXrY' master
    git svn dcommit
    git checkout master
    git svn fetch
    git push

2. create the svn tags:

    cd /afs/cern.ch/sw/Gaudi/git/gateway/Gaudi
    GaudiRelease/cmt/svn_tag_release.py
    git checkout master
    git svn fetch
    git push

