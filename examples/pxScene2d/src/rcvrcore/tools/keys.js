// keys.js .... JavaScript Keycodes

var keys =
{
    ENTER        : 13,
    BACKSPACE    : 8,
    TAB          : 9,
    CANCEL       : 10001,  //TODO
    CLEAR        : 10002, //TODO
    SHIFT        : 16,
 //   SHIFT_LEFT   : 112, //10003 //TODO - special
 //   SHIFT_RIGHT  : 113, //10003 //TODO - special
    CONTROL      : 17,
    ALT          : 18,
    PAUSE        : 19,
    CAPSLOCK     : 20,
    ESCAPE       : 27,
    SPACE        : 32,
    PAGEUP       : 33,
    PAGEDOWN     : 34,
    END          : 35,
    HOME         : 36,
    LEFT         : 37,
    UP           : 38,
    RIGHT        : 39,
    DOWN         : 40,
    COMMA        : 188,
    PERIOD       : 190,
    SLASH        : 191,
    ZERO         : 48,
    ONE          : 49,
    TWO          : 50,
    THREE        : 51,
    FOUR         : 52,
    FIVE         : 53,
    SIX          : 54,
    SEVEN        : 55,
    EIGHT        : 56,
    NINE         : 57,
    SEMICOLON    : 186,
    EQUALS       : 187,
    A            : 65,
    B            : 66,
    C            : 67,
    D            : 68,
    E            : 69,
    F            : 70,
    G            : 71,
    H            : 72,
    I            : 73,
    J            : 74,
    K            : 75,
    L            : 76,
    M            : 77,
    N            : 78,
    O            : 79,
    P            : 80,
    Q            : 81,
    R            : 82,
    S            : 83,
    T            : 84,
    U            : 85,
    V            : 86,
    W            : 87,
    X            : 88,
    Y            : 89,
    Z            : 90,
    OPENBRACKET  : 219,
    BACKSLASH    : 220,
    CLOSEBRACKET : 221,
    NUMPAD0      : 96,
    NUMPAD1      : 97,
    NUMPAD2      : 98,
    NUMPAD3      : 99,
    NUMPAD4      : 100,
    NUMPAD5      : 101,
    NUMPAD6      : 102,
    NUMPAD7      : 103,
    NUMPAD8      : 104,
    NUMPAD9      : 105,
    MULTIPLY     : 106,
    ADD          : 107,
    SEPARATOR    : 27,
    SUBTRACT     : 109,
    DIVIDE       : 111,
    F1           : 112,
    F2           : 113,
    F3           : 114,
    F4           : 115,
    F5           : 116,
    F6           : 117,
    F7           : 118,
    F8           : 119,
    F9           : 120,
    F10          : 121,
    F11          : 122,
    F12          : 123,
    DELETE       : 46,
    NUMLOCK      : 144,
    SCROLLLOCK   : 145,
//    PRINTSCREEN  : 10036 //TODO
    INSERT       : 45,
//    HELP         : 10038 //TODO
    DECIMAL      : 190,
    BACKQUOTE    : 192,
    QUOTE        : 222
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

keys.name = function(key)
{
  var all_keys = Object.keys(this);

  for(var k in all_keys)
  {
    if( this[all_keys[k]] == key)
    {
      return all_keys[k];
    }
  }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Helper functions for testing Key Modifer FLAGS...

keys.is_CTRL = function(f)
{
//    console.log("\n\n ### keys.is_CTRL .... f = " + f + " (16) \n\n");

    return (f==16);
}

keys.is_CTRL_ALT = function(f)
{
    return (f==48);
}

keys.is_SHIFT = function(f)
{
    return (f==8);
}
keys.is_CTRL_SHIFT = function(f)
{
//    console.log("\n\n ### keys.is_CTRL_SHIFT .... f = " + f + " (24) \n\n");

    return (f==24);
}

keys.is_CTRL_ALT_SHIFT = function(f)
{
    return (f==56);
}


keys.is_CMD = function(f)   // OSX
{
//    console.log("\n\n ### keys.is_CMD .... f = " + f + " (64) \n\n");

    return (f==64);
}

keys.is_CMD_SHIFT = function(f)   // OSX
{
//    console.log("\n\n ### keys.is_CMD_SHIFT .... f = " + f + " (72) \n\n");

    return (f==72);
}

keys.is_CMD_OPTION_SHIFT = function(f)   // OSX
{
    return (f==104);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

module.exports = keys;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


