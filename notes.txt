JUMPDIR Feature List
=====================

________________________________________________________________________________
Option: --clean [pattern]

    When auto clean is implemented. Examine all historical entries that match
    the given pattern (default pattern is "..."). Entries that no longer exist
    are removed from the history.

________________________________________________________________________________
Note: Guard against attempt to reuse remembered connections

    > net use x: \\foo\bar /persistent yes
    > reboot
    > net use x: \\qux\qax
        --> prompt / error message

    Since jumpdir is its own type of persistent connection, all jumpdir-mapped
    connections should not be persistent.

________________________________________________________________________________
Increase Default History Size?

    Right now the default is 200 entries. I'd suggest bumping this to at
    least 500, and possibly higher.

    Perhaps a better option, when paired with auto cleanup, would be to set a
    maximum age. You should also probably be able to set a maximum age for
    removable drives, since these can't be automatically cleaned up when the
    drive is gone.

________________________________________________________________________________
Note: Skip . and .. prefixed paths

    Paths beginning with . and .. should not be matched against historical
    entries -- it should be interpreted literally only.

________________________________________________________________________________
Feature: Bare History List

    Add an option to give a bare list of the historical entries. This would
    be identical to the --list option, except that output lines would not be
    prefixed with the slot number.

________________________________________________________________________________
Nicknamed Directories

    The ability to nickname a location for future reference.  Don't require
    magic syntax, just examine list of known nicknames first.

    Should be able to list all known nicknames.
    Should be able to remove/change a nickname.

________________________________________________________________________________
Context Bubbling

    The ability to bubble-up entries according to some condition.

    This could be useful, for example, when you navigate amongst three
    enlistments to the same tree.  Here, you could bubble up all entries
    under one of the trees, so that tree would have priority when
    searching. See also the "Search along current directory" entry below. For
    example:

    > go --prefer c:/srh/...

    would move all matching paths to the top of the history stack.

________________________________________________________________________________
Search Along Current Directory / Prefer Nearest

    Today setdir works like this:

        D:\family\FCES\PTSA> go ck...origin
        D:\family\CKMS\PTSA\directory\2008\data\origin> go ptsa
        D:\family\FCES\PTSA>

    Since we went to a directory under CKMS/, the second go command might
    prefer that path, rather than jumping to the FCES/ variant.

    Another way of describing this is to switch priority from the usual
    chronological priority to an ancestral priority (that is, choose the
    resulting match with the nearest relation distance (up to common ancestor
    directory and back down to the new directory).

    In the example above with the second go command {ptsa},
    D:\family\FCES\PTSA\directory\2008\data\origin is eight hops away, while
    D:\family\ CES\PTSA\directory\2008\data\origin would be four hops away.

    Perhaps a "--nearest" switch would trigger a nearest match search.

________________________________________________________________________________
Duplicate Entries from Shortened Names

    These two entries show up independently in the history.

        C:/DOCUME~1/v-stevho
        C:/Documents and Settings/v-stevho

    This tool should recognize both as being equivalent. Probably canonicalize
    each path to the full names.
________________________________________________________________________________
Add an "aggressiveness" option to control how aggressive search is

    Ordered by potential time impact. For example, net search is high, hitting
    historically visited paths is low.

________________________________________________________________________________
Implement Sparse Matching

    As an alternative to using ellipsis path matching, provide a way to make a
    sparse match which matches characters in order with any number of
    interleaving characters (equivalent to putting ".*" between every character
    in the match). Perhaps prefixed with an escape character to denote a sparse
    match.
