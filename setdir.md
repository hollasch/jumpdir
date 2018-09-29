`setdir`
====================================================================================================

`setdir` - Extended replacement for the `cd` command

    setdir [-?,-help] [-debug]
           [-histsize=num] [-netsearch=bool] [-automap=bool] [-report=bool]
           [-verbose=bool] [-state]
           [-list pattern] [-purge pattern] [-clean]
           [:num] [path]


Description
------------
`setdir` offers a much improved replacement for the `cd` command. It keeps track of directories you
visit and allows you to jump from directory to directory quickly and easily. It also connects you to
network drives automatically and removes the need to change drives explicitly. This differs from
other approaches which use hard-coded aliases or scripts to hop to specific directories, since
`setdir` dynamically keeps track of your movements from directory to directory, and learns your
environment without any maintenance needed on your part.

`setdir` works by maintaining a list of places you've been, most recent first. Though old
directories eventually fall off the bottom of the list, the size of the history is easily
configurable. (This size is 200 by default, but I regularly use 800 entries without problem. My
history size is set to 1000, though I tend to top out at about 600-800 active directories).

This tool actually comes in two parts: the setdir executable, and the `go` batch file (which you
will normally use). `setdir` determines the commands needed to change to the target path (including
drive letter changes and remote share connections), and `go` executes these commands. _(This
indirection is necessary because environment changes in an executable file don't change the shell's
environment.)_

To use `setdir`, place the files `setdir.exe` and `go.bat` in a directory on your executable path.
If you immediately use `setdir`, it will create the file `%USERPROFILE%\setdir.dat` to track your
directory history. Alternatively, you can set the `SETDIR` environment variable to a filename of
your choosing, and `setdir` will use that as the setdir history data file. Now just use <b>go</b>
instead of <b>cd</b> when you want to change directories.

Several arguments can take wildcard patterns. All patterns are case-insensitive, and may contain the
characters `*` (matches zero or more of any character) and `?` (matches exactly one character).


Command-Line Options
---------------------

### `-?, -help`
Print help reference information.


### `-debug`
Print debug output. This allows you to watch over `setdir`'s shoulder to see what it's doing, and is
also useful for understanding the heuristics it employs to search for the matching path.


### `-clean`
Removes all historic paths that no longer exist. This is useful when you have a good sized history
and want to clean out all obsolete paths. Running this occasionally will speed up your directory
searches.


### `-state`
Print the `setdir` state (_e.g._ history size, automap, and so forth).


### `-histsize=num`
Set the size of the history stack. By default this is set to 200, though I like running with 1000
myself. The tradeoff of large stack sizes is the time it takes to look through all directories and
all guesses before finding or failing to find anything.


### `-netsearch=bool`
Network path searches can be expensive, particularly if you are connected over a remote connection,
or some of the net shares are down. If netsearch is enabled, then `setdir` will perform all of the
various searches on network drives as it does for local drives. If netsearch is disabled (the
default), then it will only attempt those searches that have good odds of matching (_e.g._ if
you've already got a historical match).


### `-automap=bool`
If this is set true, and if you specify a network path, then
`setdir` will automatically map the network drive for you, and place
you in the requested directory. So, for example, you can do
`go //foo/bar/xyzzy` and it will work even if you don't currently have that
share mapped to any logical drive. `setdir` is fairly flexible in the
strings it takes for boolean values. For example, you can denote true with
`true`, `yes`, `y`, `1`, or `on`.


### `-report=bool`
If this switch is on, then the new directory will be printed when changing to
a new directory. This is useful if you don't print the path in your shell
prompt, for example. Reporting is off by default.


### `-verbose=bool`
In verbose mode, `setdir` prints all shell commands before executing them.
Verbose is disabled by default.


### `-list pattern`
If _pattern_ is not given, prints all history entries. If _pattern_
<u>is</u> supplied, then print all matching history entries. The pattern is
implicitly prefixed with `*`.


### `-purge pattern`
Purges all entries from the directory history matching the given pattern. The
pattern must be supplied. This command is useful to remove all entries for a
particular network share, for example. For any given pattern, all entries
that match in the `-list` command will also match in the `-purge` command, so
you can double-check with `-list` before purging entries.


### `:num`
Go to history entry number _num_. To print history entries, use the
`-list` option.


### `path`
The path to change to. Separating slashes in the directory paths may be either forward or backward
slashes. Also, if paths have single spaces in their names, you need not quote them (so you can do
something like `go A dir name with spaces` and `go` will automatically insert the spaces for you.


Paths may be one of the following types:

### Absolute Local Paths
These are paths that begin with a drive letter and an absolute path, such
    as `G:/abc/def/ghi`.

### UNC Paths
These are network paths that begin with two slashes, a machine name, another slash, and a share
point (_e.g._ `//nostromo/mother/directives`).

### Rooted Paths
These are paths that begin with a slash. `setdir` will try to find the rooted path from every drive
you've been to. If netsearch is enabled, it will try this for network paths as well.

### Simple Relative Paths
Paths that have no slashes at all, but are just simple strings.

### Compound Relative Paths
Paths that don't begin with a root of any kind, but have multiple components (_e.g._
`xy/zzy/upstr`).

### Wildcard Paths
If you supply a wildcard pattern, then `setdir` will test the pattern against all historical
entries. Note that the wildcard characters span slashes, so that `foo*bar` will match against
`D:/aaa/foober/blah/bar`.

`setdir` will explicitly <strong>not</strong> do speculative directory searches based on the
pattern; it will only check against places you've already been. This is to keep `setdir` as fast as
possible.


Methodology
------------

If you're interested in how `setdir` works, this section covers the basic strategy. Before searching
for the specified path, the slashes are normalized, the path fragments on the command line are
concatenated with spaces if necessary, and environment information (such as mapped network drives)
is gathered.

These are the following steps `setdir` takes as it tries to find the path (the first successful
match is the winner):

### [1] Wildcard Match
If the path string contains wildcard characters, then only the wildcard match is attempted. The
given pattern is tested against all historical entries. If a pattern doesn't match an entry, but the
same pattern concatenated with `/*` <em>does</em>, then the subportion of that path is tested and
used if available. For example, suppose the pattern is `b*ghi`, and we find an entry
`C:\abc\def\ghi\jkl\mno`, then it will fail to match, but `b*ghi/*` <em>will</em> match. In this
case, `setdir` will choose `C:\abc\def\ghi` as the winning entry.

_The remaining tests are executed only if the path does **not** contain wildcard characters._

### [2] Straight Match
Try the path as is. This applies to all path types.

### [3] Rooted Match
If the path is a rooted path (begins with a slash), then try rooting it to all visited local drives,
and (if _netsearch_ is enabled) all visited network paths.

### [4] Tail Match
See if the path matches the last portion of any historical path. For example, if the given path is
`yon`, we'll match on `//harold/purple/crayon` if that path is the most recent in the history that
ends in that string.

### [5] Partial Path Match
See if the path matches any partial path of a path we've visited. For example, if the given path is
`ox/yz`, we've been to C:/misc/things/box/morethings/stuff, <em>and</em> the directory
C:/misc/things/box/yz exists, then we'll match on the latter path.

### [6] Child Match
See if the given path is a child of any directory we've visited. If _netsearch_ is enabled,
then do this for network paths as well.

### [7] Failed Match
If none of these attempts turn up anything, then we report failure and halt.


Also note that `setdir` will ignore matches that evaluate to the current directory. This allows you
to toggle between two directories that have the same name, like `G:/jim/jam` and `X:/raspberry/jam`,
by typing `go jam` from either of the directories.


Examples
---------

One of the best ways to learn how `setdir` works is to turn on debugging output (via the -debug
switch) while changing to a given directory. You'll be able to watch which directories it tries and
which it guesses at. Here's an example session:

    D:\srh\source\setdir&gt; <b>go -debug icecap</b>
    # Looking for setdir data file.
    # Trying %SETDIR%.
    # SETDIR is defined as "D:\srh\setdir.dat"; using that.
    # Debugging output enabled.
    # Current working directory is "D:/srh/source/setdir".
    # Given path is "icecap".
    # Current dir is "D:/srh/source/setdir".
    # Straight Match?
    # Trying icecap
    # No.
    # Tail Match: Been somewhere ending in "icecap"?
    # No.
    # Partial Path:  A partial path of a historical path?
    # No.
    # Child Match: Subdirs of visited paths?
    # (Skipping network paths.)
    # Trying D:/srh/source/setdir/icecap
    # Trying F:/appel2/tools/x86/sweeper/icecap
    # Trying F:/appel3/src/qvlib/icecap
    # Trying F:/ss/dxm/java/test/unit/3d/spin/icecap
    # Trying F:/appel3/src/icecap
    # Trying F:/appel2/src/icecap
    # Trying F:/appelles/src/icecap
    # Trying F:/appel2/src/appel/icecap
    # Trying F:/appel2/tools/x86/perl/icecap
    # Trying F:/appel2/tools/x86/directx/icecap
    # Trying //itg1/contrib/gnu/tools/icecap
    # Trying F:/appelles/src/appel/icecap
    # Trying D:/apps/devstudio/vc/include/icecap
    # Trying //lerp/c$/dxsdk/icecap
    # Trying F:/appel2/icecap
    # Trying F:/appel3/icecap
    # Trying F:/appel3/tools/icecap
    # Trying F:/appel2/src/appel/values/icecap
    # Trying F:/appel2/src/appel/values/geom/icecap
    # Trying F:/appel2/tools/x86/directx/inc/icecap
    # Trying D:/srh/source/drives/icecap
    # Trying F:/appelles/src/make/icecap
    # Trying F:/appelles/icecap
    # Trying F:/appelles/build/icecap
    # Trying F:/appelles/build/win/icecap
    # Trying F:/appelles/build/win/ship/bin/icecap
    # Trying E:/nt/system32/icecap
    # Trying E:/nt/icecap
    # Trying F:/appel2/build/win/debug/bin/icecap
    # Trying F:/icecap
    # Trying F:/appel2/tools/x86/icecap
    # Succeeded
    # Writing history global for new path "F:/appel2/tools/x86/icecap".
    # Wrote 710 entries.
    # Writing commands to change to F:/appel2/tools/x86/icecap
    # Local or mapped path.

    F:\appel2\tools\x86\icecap&gt;

Finally, here's a dump of a sample session to show some of the features of `setdir`. Note that we're
starting from scratch, with no prior history.

    // This is our initial startup state.

    C:\> go -state
    histsize:  200
    netsearch: false
    automap:   true
    report:    false
    verbose:   false

    // Let's increase our history to 1000 entries.

    C:\> go -histsize=1000
    // Adding the first couple of paths to our history.

    C:\> go /

    C:\> go D:/

    D:\> go c:/windows/system32

    C:\WINDOWS\system32> go d:\bin\perl\bin

    // Now list the history so far. Note that the entries are ordered newest
    // first.

    D:\bin\perl\bin> go -list
       0:  D:/bin/perl/bin
       1:  C:/WINDOWS/system32
       2:  D:
       3:  C:

    // Now let's go to a UNC path. We don't currently have
    // \\tikfiltoolbox\tools mapped, so `setdir` maps it to the first available
    // drive letter and takes us there.

    D:\bin\perl\bin> go //tkfiltoolbox/tools/586
    The command completed successfully.

    \\tkfiltoolbox\tools E:\586>
    // Now let's try some shortcuts. Go to `32`, and note that it takes us to
    // the most recent historial path that ended in `32`.

    \\tkfiltoolbox\tools E:\586> go 32

    C:\WINDOWS\system32>

    // Now we'll go directly to a given directory entry.
    // We'll use the second entry.

    C:\WINDOWS\system32> go -list
       0:  C:/windows/system32
       1:  //tkfiltoolbox/tools/586
       2:  D:/bin/perl/bin
       3:  D:
       4:  C:

    C:\WINDOWS\system32> go :2

    D:\bin\perl\bin>

    // Note that bad paths are handled as follows:

    D:\bin\perl\bin> go foo
    No matching directory found for `foo`.

    // We're already at a directory ending in `bin`, so going to that string
    // will take us to the most recent _different_ match.

    D:\bin\perl\bin> go bin

    D:\bin> go d:/srh/bin

    D:\srh\bin> go c:/

    C:\> go p*bin

    D:\bin\perl\bin> go p*bin
    No matching directory found for `p*bin`.

    D:\bin\perl\bin> go s*bin

    D:\srh\bin> go -list
       0:  D:/srh/bin
       1:  D:/bin/perl/bin
       2:  C:
       3:  D:/bin
       4:  C:/windows/system32
       5:  //tkfiltoolbox/tools/586
       6:  D:

    D:\srh\bin> go ..

    D:\srh> go /

    // In the following example, setdir finds a new directory that is under a
    // historical directory.

    D:\> go notes

    D:\srh\notes>

    // Now purge all entries under with `C:/windows`. This is a good way to
    // clear all entries to network paths you no longer care about. You can
    // also use the -clean option to automatically purge all directories that
    // no longer exist.

    D:\srh\notes> go -purge c:/windows/*

    Purging:
    - C:/windows/system32
    1 entry purged.

    D:\srh\notes> go -list

       0:  D:/srh/notes
       1:  D:
       2:  D:/srh
       3:  D:/srh/bin
       4:  D:/bin/perl/bin
       5:  C:
       6:  D:/bin
       7:  //tkfiltoolbox/tools/586

----------------------------------------------------------------------------------------------------
Steve Hollasch / 2003-12-09
