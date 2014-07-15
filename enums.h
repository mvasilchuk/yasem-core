#ifndef ENUM_H
#define ENUM_H

#include <QString>

static const QString DB_TAG_PROFILE = "profile";
static const QString DB_TAG_RDIR = "RDIR";
static const QString DB_TAG_ENV = "ENV";
static const QString DB_TAG_USER = "USER";
static const QString DB_TAG_SFG = "SFG";

enum MediaPlayingState
{
    StoppedState,
    PlayingState,
    PausedState
};

enum TOP_GUI_OBJECT {
    BROWSER = 0,
    VIDEO = 1
};

enum ASPECT_RATIO {
    ASPECT_RATIO_AUTO = 0
};

enum PLUGIN_ERROR_CODES {
    PLUGIN_ERROR_UNKNOWN_ERROR = -1,
    PLUGIN_ERROR_NO_ERROR = 0,
    PLUGIN_ERROR_NOT_INITIALIZED = 1,
    PLUGIN_ERROR_CYCLIC_DEPENDENCY = 2,
    PLUGIN_ERROR_DEPENDENCY_MISSING = 3,
    PLUGIN_ERROR_NO_PLUGIN_ID = 4,
    PLUGIN_ERROR_DIR_NOT_FOUND = 5
};

enum PLUGIN_STATE {
    PLUGIN_STATE_UNKNOWN = -1,
    PLUGIN_STATE_DISABLED = 0,
    PLUGIN_STATE_LOADED = 1,
    PLUGIN_STATE_INITIALIZED = 2,
    PLUGIN_STATE_UNLOADED = 3,
    PLUGIN_STATE_WAITING_FOR_DEPENDENCY = 4
};

enum DB_ERRORS {
    DB_ERROR_UNKNOWN_ERROR = -1,
    DB_ERROR_NO_ERROR = 0,
    DB_ERROR_NOT_FOUND = 1,
    DB_ERROR_CORRUPTED = 2,
    DB_ERROR_TABLE_NOT_FOUND = 2
};

enum RC_ERRORS {
    RC_UNKNOWN_ERROR = -1,
    RC_NO_ERROR = 0,
    RC_KEYCODE_NOT_FOUND = 1
};

enum RC_KEY_EVENT_TYPE {
    onKeyPress = 0,
    onKeyDown = 1,
    onKeyUp = 2
};

enum RC_KEY {
    RC_KEY_NO_KEY = 0,
    RC_KEY_OK = 1,

    RC_KEY_RIGHT = 2,
    RC_KEY_LEFT = 3,
    RC_KEY_UP = 4,

    RC_KEY_DOWN = 5,
    RC_KEY_PAGE_UP = 6,
    RC_KEY_PAGE_DOWN = 7,

    RC_KEY_MENU = 8,
    RC_KEY_BACK = 9,
    RC_KEY_REFRESH = 10,

    RC_KEY_RED = 11,
    RC_KEY_GREEN = 12,
    RC_KEY_YELLOW = 13,
    RC_KEY_BLUE = 14,

    RC_KEY_CHANNEL_PLUS = 15,
    RC_KEY_CHANNEL_MINUS = 16,

    RC_KEY_SERVICE = 17,
    RC_KEY_TV = 18,
    RC_KEY_PHONE = 19,
    RC_KEY_WEB = 20,
    RC_KEY_FRAME = 21,

    RC_KEY_VOLUME_PLUS = 22,
    RC_KEY_VOLUME_MINUS = 23,

    RC_KEY_REWIND = 24,
    RC_KEY_FAST_FORWARD = 25,
    RC_KEY_STOP = 26,
    RC_KEY_PLAY_PAUSE = 27,
    RC_KEY_REC = 28,

    RC_KEY_MIC = 29,
    RC_KEY_MUTE = 30,
    RC_KEY_POWER = 31,
    RC_KEY_INFO = 32,

    RC_KEY_NUMBER_0 = 33,
    RC_KEY_NUMBER_1 = 34,
    RC_KEY_NUMBER_2 = 35,
    RC_KEY_NUMBER_3 = 36,
    RC_KEY_NUMBER_4 = 37,
    RC_KEY_NUMBER_5 = 38,
    RC_KEY_NUMBER_6 = 39,
    RC_KEY_NUMBER_7 = 40,
    RC_KEY_NUMBER_8 = 41,
    RC_KEY_NUMBER_9 = 42,
    RC_KEY_EXIT = 99
};

enum CMD_ARGUMENTS {
    COLOR_OUTPUT,
    FULLSCREEN_APP,
    DEVELOPER_TOOLS
};

static QString arguments[] = {
    "--color",
    "--fullscreen",
    "--developer-tools"
};

enum MOUSE_POSITION {
    MIDDLE  = 0,
    LEFT    = 1,
    TOP     = 2,
    RIGHT   = 4,
    BOTTOM  = 8
};

enum PluginFlag {
    PLUGIN_FLAG_NONE = 0,
    PLUGIN_FLAG_CLIENT = 1,
    PLUGIN_FLAG_SYSTEM = 2,
    PLUGIN_FLAG_HIDDEN = 4,
    PLUGIN_FLAG_GUI = 8
};

enum PluginRole {
    ROLE_UNKNOWN,
    ROLE_UNSPECIFIED,
    ROLE_GUI,
    ROLE_MEDIA,
    ROLE_STB_API,
    ROLE_STB_API_SYSTEM,
    ROLE_BROWSER,
    ROLE_DATASOURCE
};

#endif // ENUM_H
