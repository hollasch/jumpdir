JUMPDIR Milestone 0
====================================================================================================

Dev Plan
---------

    (x) Print usage info

    (x) Print debug output

    (x) Flip slashes

    (x) Auto aggregation of words with spaces

    (x) Change directory straight

    ( ) Establish base test framework. Data files should be Unicode.

    ( ) Read/write data file (Unicode)

    ( ) Implement data file history size read/write

    ( ) Implement data file dir echo     read/write

    ( ) Implement data file net search   read/write

    (x) Error reporting routine

    ( ) Change drive letter additionally

    ( ) Set history size

    ( ) Print state variables

    ( ) Set report switch

    ( ) Option -list without wildcards

    ( ) Go to numbered historical entry

    ( ) Option -clean to purge non-existent locations

    ( ) Wildcard matches

    ( ) Option -list with wildcards

    ( ) Option -purge with wildcards

    ( ) Set netsearch switch

    ( ) Guard against attempt to reuse remembered connections

    ( ) Centralized memory alloc failure handling


Strategies
-----------

1. Trivial change (null directory, or non-wildcarded & begins with "." or "..").

2. Straight non-wildcarded directory - attempt to change directly to that directory.

3. Wildcarded path (for example, "Z:/foo/.../bar", or the tail fragment "...oo/bar" or
   "xy/.../glitz"). Match against historical entries. When historical matches are found, select the
   entries nearest the current directory, and then select the most recent of those. If no historical
   directory matches, use wildcard directly (if enabled) and select first match. Halt on failure.

4. Historical partial path - match against partial historical paths. For example, if we've been to
   T:/qux/foo/bar/baz, then "oo/bar" will match against the "bar" directory on this path.

5. Absolute non-wildcard directory-letter path (for example, "g:/foo/bar") - attempt to change
   verbatim. Halt on failure.

6. Absolute non-wildcard UNC share path (for example, "//host/share/foo/bar").

7. Rooted non-wildcard path - First attempt relative to current drive. On failure, match against
   previously visited drives.

8. Peer match - If the front part of the specification matches against the tail of a historical
   path, try the extension. For example, given "tox/clu/fle", and we've been to "H:/injex/botox",
   then try "H:/injex/botox/clu/fle".


Jump Data File
---------------

    %JUMPDAT%                 (user-defined)
    %USERPROFILE%/jumpdir.dat (default)

Use binary data file, with optional XML import/export.
