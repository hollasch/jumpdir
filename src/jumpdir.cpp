//======================================================================================================================
// jumpdir - A smart directory navigation command
//
// Copyright 2017 Steve Hollasch. All rights reserved.
//======================================================================================================================

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <direct.h>
#include <assert.h>
#include <PathMatcher.h>
#include <fileSystemProxyWindows.h>


using namespace PMatcher;
using namespace FSProxy;


// Program Parameters and Constants

bool fDebug = false;


// Version and Usage Information

static const char* usage[] = {
    "",
    "jumpdir v0.0.10 / (c) 2017 Steve Hollasch",
    "",
    "jumpdir: Adaptive directory navigation for the command line",
    "Usage:   jumpdir <directory>",
    "",
    "    This command changes the directory as specified.",
    0
};



//======================================================================================================================
// Utility Functions
//======================================================================================================================

void PrintUsage () {

    // Prints out usage information for this tool.
    //----------------------------------------------------------------------------------------------

    int i;

    for (i=0;  usage[i];  ++i) {
        if (usage[i][0] == 0)
            puts ("echo.");
        else
            printf ("echo %s\n", usage[i]);
    }
}


//--------------------------------------------------------------------------------------------------
void DPrint (const char* format, ...) {

    // Prints one line of debug information. The formatted string does not need a carriage return.
    //
    // 'format' - a printf-style format string
    // ...      - the remainder of the printf-style arguments
    //----------------------------------------------------------------------------------------------

    if (!fDebug) return;

    va_list vl;
    va_start (vl, format);

    fputs ("# ", stderr);
    vfprintf_s (stderr, format, vl);
    fputc ('\n', stderr);

    va_end (vl);
}


//--------------------------------------------------------------------------------------------------
void ErrorPrint (const char* format, ...) {

    // Prints a printf-style message plus arguments to the error output stream, followed by a
    // carriage return.
    //----------------------------------------------------------------------------------------------

    va_list vl;
    va_start (vl, format);

    fputs ("jumpdir: ", stderr);
    vfprintf_s (stderr, format, vl);
    fputc ('\n', stderr);

    va_end (vl);
}


//--------------------------------------------------------------------------------------------------
inline bool streqic (const char *str1, const char *str2) {

    // Returns true if the two strings are non-null and equal, ignoring case. Either string
    // argument may be null.
    //----------------------------------------------------------------------------------------------

    return (str1 != 0) && (str2 != 0) && (0 == _stricmp(str1,str2));
}


//--------------------------------------------------------------------------------------------------
void SlashForward (char* str) {

    // Given a string pointer, converts all back slashes to forward slashes.
    //----------------------------------------------------------------------------------------------

    char* ptr;

    for (ptr=str;  *ptr;  ++ptr) {
       if (*ptr == '\\')
            *ptr = '/';
    }
}


//--------------------------------------------------------------------------------------------------
void SlashBackward (char* str) {

    // Given a string pointer, converts all forward slashes to back slashes.
    //----------------------------------------------------------------------------------------------

    char* ptr;

    for (ptr=str;  *ptr;  ++ptr) {
       if (*ptr == '/') *ptr = '\\';
    }
}



//--------------------------------------------------------------------------------------------------
static bool isWildStr (const char* str) {

    // Returns true if the given string begins with a wildcard character sequence.
    // This includes '?', '*', '**', or '...'.
    //----------------------------------------------------------------------------------------------

    return (*str == '?')
        || (*str == '*')
        || ((str[0] == '.') && (str[1] == '.') && (str[2] == '.'));
}



//======================================================================================================================
// Struct DirEntry
//======================================================================================================================

struct DirEntry {
    unsigned int m_dvol_name;          // Offset of Volume Name
    unsigned int m_dvol_label;         // Offset of Volume Label
    unsigned int m_dpath;              // Offset of Path
    time_t       m_lastverified;       // Date Last Verified
    DWORD        m_serialnum;          // Volume Serial Number
    bool         m_valid;              // Entry Valid
    DirEntry    *m_next;               // Next Directory Entry
};



//======================================================================================================================
// Class JDMemPool
//======================================================================================================================

class JDMemPool {
  public:
    JDMemPool () : m_size{0}, m_heap{0} { }

    void Attach (void* block, unsigned int size);

  private:
    unsigned int  m_size;
    void         *m_heap;
};


//--------------------------------------------------------------------------------------------------
void JDMemPool::Attach (void *block, unsigned int size) {

    // This method sets the block and size for the memory pool, as read from the data file. This
    // memory pool can be effectively detached by calling it with (0,0) as the arguments.
    //
    // Ensure that the block is non-null if the size is non-zero.
    //----------------------------------------------------------------------------------------------

    assert ((size == 0) == (block == NULL));

    m_heap = block;
    m_size = size;
}



//======================================================================================================================
// Class JDFileHeader
//======================================================================================================================

class JDFileHeader {
  public:

    JDFileHeader ()
      : maxHistSize    {-1},
        dirEcho        {false},
        verbose        {false},
        netSearch      {true},
        automap        {true},
        numHistEntries {0}
    {
    }

    // Data Fields

    int          maxHistSize;     // Max Number of History Entries
    bool         dirEcho;         // Echo new directories?
    bool         verbose;         // Echo resultant shell commands?
    bool         netSearch;       // Search network paths?
    bool         automap;         // Automatically map network drives?
    unsigned int numHistEntries;  // Current count of path history entries
};



//======================================================================================================================
// Class JumpData
//======================================================================================================================

class JumpData {
  public:

    JumpData() : m_rawData{nullptr} {};
    ~JumpData();

    bool Load  (const string& filename);
    bool Store (const string& filename) const;

  private:
    JDFileHeader *m_header;
    DirEntry     *m_dirEntries; // Directory Entry List
    char*         m_strpool;

  private:
    void *m_rawData;         // Raw Data File Contents
};


//--------------------------------------------------------------------------------------------------
JumpData::~JumpData () {
    if (m_rawData) {
        delete [] m_rawData;
    }
}


//--------------------------------------------------------------------------------------------------
bool JumpData::Load (const string& filename) {
    DPrint ("Reading jumpdata from \"%s\".", filename.c_str());

    m_header = new JDFileHeader;

    // If the data file does not exist, then silently assume that we're starting
    // from scratch.

    // ifstream dbStream {filename.c_str()};

    if (0 != _access(filename.c_str(), 0))
        return true;

    // Read in the entire contents of the jump data file into the data buffer.
    // Below open in "rb" mode to get an accurate (untranslated) size of the
    // entire data file, so we can slurp the entire thing into memory at once.
    // If the file is opened in text mode, the size won't be accurate since
    // carriage-return/linefeeds may be logically collapsed into a single
    // entity.

    FILE* dataFile;

    if (0 != fopen_s (&dataFile, filename.c_str(), "r,ccs=UTF-8")) {
        ErrorPrint ("Couldn't open data file \"%s\".", filename);
        return false;
    }

    // Seek to the end to determine the size of the entire data file.

    if (0 != fseek (dataFile, 0, SEEK_END)) {
        ErrorPrint ("Couldn't seek in data file \"%s\".", filename);
        return false;
    }

    auto dataCount = ftell (dataFile);
    rewind (dataFile);

    // Allocate the buffer for the data file contents.

    m_rawData = new char [dataCount + 100];                                            // !!! TEMPORARY !!! buffer for experimentation

    if (m_rawData == nullptr) {
        ErrorPrint ("Couldn't allocate %ld bytes for jumpdir data.\n", dataCount);
        return false;
    }

    // Read the data file into the data block and close the file.

    size_t nItemsRead = fread_s (m_rawData, dataCount, 1, dataCount, dataFile);

    if (nItemsRead != dataCount) {
        ErrorPrint ("Read failed on data file \"%s\".", filename);
        return false;
    }

    fclose (dataFile);

    return true;
}


//--------------------------------------------------------------------------------------------------
bool JumpData::Store (const string& filename) const {
                                                                                                        // !!! TEMPORARY !!!
    DPrint ("Skipping DB store for development.");
    return true;

    #if 0
        FILE *datafile;

        if (0 != _wfopen_s (&datafile, filename, L"wb")) {
            ErrorPrint (L"Couldn't create data file \"%s\".", filename);
            return false;
        }


        if (1 != fwrite (m_header, sizeof(JDFileHeader), 1, datafile)) {
            ErrorPrint (L"Error while writing data file.");
            return false;
        }
    #endif

    #if 0
        for (int i=0;  i <= 0xff;  ++i) {
            unsigned char c = static_cast<unsigned char> (i);
            if (1 != fwrite (&c, sizeof(unsigned char), 1, datafile)) {
                ErrorPrint (L"Error while writing data file.");
                return false;
            }
        }

        unsigned int x = 0x11223344;
        if (1 != fwrite (&x, sizeof(unsigned int), 1, datafile)) {
            ErrorPrint (L"Error while writing data file.");
            return false;
        }

        if (0 > fputws (L"[# Hello world!\n]", datafile)) {
            ErrorPrint (L"Error while writing data file.");
            return false;
        }
    #endif

    #if 0
        fclose (datafile);
    #endif

    return true;
}



//======================================================================================================================
// Class JDContext
//======================================================================================================================

class JDContext {
    //--------------------------------------------------------------------------
    // The main data context structure for the jumpdir program.
    //--------------------------------------------------------------------------

  public:

    static const int c_numDrives = 26;

    JDContext (FileSysProxy&);
    ~JDContext() {};

    bool ParseArgs (int argc, const char* const argv[]);
    bool ScanEnvironment ();
    bool ScanDrives ();

    bool Load ();
    bool Store ();

    void EnumerateNetMaps ();

    bool HandleTrivialChange();
    bool Jump ();

  private:

    FileSysProxy& m_fsProxy;           // File System Proxy

    bool AppendDest (const char*);     // Appends Path to Destination

    string   m_dbFilename;             // Jumpdir Data File Name
    JumpData m_jumpData;               // Jump Directory Data

    PathMatcher m_pathMatcher;         // Wildcard Path Matcher

    char     m_cwd[MAX_PATH+1];        // Current Working Directory
    char*    m_driveMaps[c_numDrives]; // Drive Network Mappings

    char     m_dest[MAX_PATH+1];       // Specified Destination
    int      m_destlen;                // Dest String Length
    bool     m_destwild;               // Destination Contains Wildcards
};


//--------------------------------------------------------------------------------------------------
JDContext::JDContext (FileSysProxy& fsProxy)
  : m_fsProxy{fsProxy}, m_pathMatcher{fsProxy} {

    int drive;

    for (drive=0;  drive < c_numDrives;  ++drive)
        m_driveMaps[drive] = 0;

    m_dest[0]  = 0;
    m_destlen  = 0;
    m_destwild = false;
}


//--------------------------------------------------------------------------------------------------
bool JDContext::ParseArgs (int argc, const char* const argv[]) {

    // Parses the command-line arguments before executing commands. This
    // function is also responsible for reading in the data file.
    //
    // Parameter 'context' is the jumpdir context object. 'argc' and 'argv' are
    // the standard command line arguments.
    //
    // Returns true if the parse succeeded, otherwise false.
    //----------------------------------------------------------------------------------------------

    int argi;     // Argument Index

    for (argi=1;  argi < argc;  ++argi) {
        if (  streqic (argv[argi], "-?")
           || streqic (argv[argi], "/?")
           || streqic (argv[argi], "-h")
           || streqic (argv[argi], "/h")
           || streqic (argv[argi], "--help")
           )
        {
            PrintUsage();
            continue;
        }

        if (argv[argi][0] == '-') {

            switch (argv[argi][1]) {

                case 'd': case 'D': {
                    fDebug = 1;
                    break;
                }

                default: {
                    ErrorPrint ("Unrecognized command option (%s).", argv[argi]);
                    exit (1);
                    break;
                }
            }
        } else {
            if (!AppendDest (argv[argi]))
                return false;
        }
    }

    DPrint ("Debug flag on");
    DPrint ("Destination \"%s\"", m_dest);
    DPrint ("Destination is %swild.", m_destwild ? "" : "not ");

    // Return true to indicate success.

    return true;
}


//--------------------------------------------------------------------------------------------------
bool JDContext::ScanEnvironment () {

    // Scans the current environment and initializes the context object
    // accordingly.  This includes the current working directory, the current
    // drive mappings, the data file, and so forth.
    //
    // Returns true if the scan/initialization succeeded, otherwise false.
    //--------------------------------------------------------------------------

    // Load the current working directory.

    if (!_getcwd (m_cwd, _countof(m_cwd)))
        return false;

    SlashForward (m_cwd);

    if (!ScanDrives()) return false;

    if (!Load()) return false;

    return true;
}


//--------------------------------------------------------------------------------------------------
bool JDContext::ScanDrives () {

    // Scan the currently mapped drives for type and other info. Returns True if
    // the scan was successful, otherwise false.
    //----------------------------------------------------------------------------------------------

    return true;
}


//--------------------------------------------------------------------------------------------------
bool JDContext::Load () {

    // Loads the jumpdir data file and initializes the related fields in the JDContext object.
    //
    // Returns True if the scan was successful, otherwise false.
    //----------------------------------------------------------------------------------------------

    DPrint ("Loading jump data.");

    size_t jumpDataStrLength;

    getenv_s (&jumpDataStrLength, NULL, 0, "JUMPDATA");

    if (jumpDataStrLength) {

        auto dbFilename = new char [jumpDataStrLength];

        if (0 != getenv_s (&jumpDataStrLength, dbFilename, jumpDataStrLength, "JUMPDATA"))
            return false;

        m_dbFilename = dbFilename;

        DPrint ("Using %%JUMPDATA%%=\"%s\"", m_dbFilename);
    } else {
        DPrint ("JUMPDATA is not defined.");

        size_t userProfileStrLength;
        getenv_s (&userProfileStrLength, NULL, 0, "USERPROFILE");

        if (!userProfileStrLength) {
            DPrint ("USERPROFILE is not defined. Hmmm, didn't expect THAT.");
            ErrorPrint ("Couldn't find jumpdir.dat file. Neither JUMPDATA nor\nUSERPROFILE is defined.");
            exit (1);
        }

        auto  userProfilePath = new char [userProfileStrLength];

        if (0 != getenv_s (&userProfileStrLength, userProfilePath, userProfileStrLength, "USERPROFILE"))
            return false;

        m_dbFilename = userProfilePath;
        m_dbFilename += "\\jumpdir.dat";

        DPrint ("Using \"%s\"", m_dbFilename);
    }

    return m_jumpData.Load(m_dbFilename);
}


//--------------------------------------------------------------------------------------------------
bool JDContext::Store () {
    return m_jumpData.Store(m_dbFilename);
}


//--------------------------------------------------------------------------------------------------
void JDContext::EnumerateNetMaps () {
    DPrint ("Enumerating network drive mappings.");

    // char driveStr[] = "X:";
    // char netName [MAX_PATH + 1];
    int  driveNum;

    for (driveNum=0;  driveNum < c_numDrives;  ++driveNum) {
    }
}


//--------------------------------------------------------------------------------------------------
bool JDContext::HandleTrivialChange () {

    // This routine handles trivial target directories. This includes null directories, and
    // non-wildcard directories that begin with '.' or '..'. This routine assumes that the
    // m_destwild member variable has already been set properly.
    //----------------------------------------------------------------------------------------------

    DPrint ("Trivial change?");

    // Bail out if the destination path contains wildcard characters.

    if (m_destwild) {
        DPrint ("Not trivial; destination path contains wildcards.");
        return false;
    }

    // If no directory was specified, and if no other commands were supplied,
    // then echo the current directory.

    if (m_dest[0] == 0) {
        DPrint ("Null directory; echo current.");
        printf ("echo %s\n", m_cwd);
        return true;
    }

    // If this is not a dot directory; then handle elsewhere.

    if (m_dest[0] != '.') {
        DPrint ("Not trivial (not null and not dot directory).");
        return false;
    }

    // If the directory is literally ".", we're already there, so return true.

    if (0 == strcmp(m_dest, ".")) {
        DPrint ("Literally '.'; no action.");
        return true;
    }

    // If this is a path that begins with a single dot directory, then use that
    // path directly.

    if (m_dest[1] == '/') {
        SlashBackward (m_dest);
        printf ("cd \"%s\"\n", m_dest);
        return true;
    }

    if (m_dest[1] != '.')   // Something else that begins with a dot.
        return false;

    // If the first subdirectory is "..", then use that path directly.

    if ((m_dest[2] == 0) || (m_dest[2] == '/')) {
        SlashBackward (m_dest);
        printf ("cd \"%s\"\n", m_dest);
        return true;
    }

    return false;           // For all other paths, handle elsewhere.
}


//--------------------------------------------------------------------------------------------------
bool JDContext::Jump () {

    // Jumps to the destination directory.
    //
    // Returns true if a match was found, and the function successfully changed to that matching
    // directory.
    //----------------------------------------------------------------------------------------------

    if (HandleTrivialChange()) return true;

    DPrint ("Seeing if new target matches current directory.");

    if (0 == _stricmp(m_cwd, m_dest)) {
        DPrint ("Tried to change to same directory as current.");
        return false;
    }

    DPrint ("Attempting to change to \"%s\".", m_dest);

    if (0 == _chdir(m_dest)) {
        DPrint ("Successfully changed to \"%s\".", m_dest);
        printf ("cd /d %s\n", m_dest);
        return true;
    }

    DPrint ("Attempt failed.");

    return false;
}


//--------------------------------------------------------------------------------------------------
bool JDContext::AppendDest (const char* path) {
    size_t roomleft = sizeof(m_dest) - m_destlen - 1;

    if (m_destlen > 0) roomleft -= 1;

    if (strlen(path) > roomleft) return false;

    if (m_destlen > 0)
        m_dest[m_destlen++] = ' ';

    for (;  *path != 0;  ++path, ++m_destlen) {

        if (!m_destwild && isWildStr(path))
            m_destwild = true;

        if (*path == '\\')
            m_dest[m_destlen] = '/';
        else
            m_dest[m_destlen] = *path;
    }

    m_dest[m_destlen] = 0;

    return true;
}



//======================================================================================================================
// Main Program
//======================================================================================================================

// int wmain (int argc, const wchar_t* const argv[]) {
int main (int argc, const char* const argv[]) {

    // The main entry point for the jumpdir program. Returns the return code for the entire program.

    assert ((sizeof(DirEntry) & 0x7) == 0);

    FileSysProxyWindows fsProxy;

    JDContext context {fsProxy};

    if (!context.ParseArgs(argc, argv)) return 1;
    if (!context.ScanEnvironment()) return 1;
    if (!context.Jump()) return 1;
    if (!context.Store()) return 1;

    return 0;
}
