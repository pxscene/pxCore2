var result = {

    "XRE_S_OK": 0,
    "XRE_E_FAILED": 1,
    "XRE_E_CONNECTION_FAILED": 2,
    "XRE_E_BAD_URL": 3,
    "XRE_E_OUT_OF_MEMORY": 4,
    "XRE_E_NOT_INITIALIZED": 5,
    "XRE_E_COULD_NOT_CREATE_OBJECT": 6,
    "XRE_E_INVALID_ID": 7,
    "XRE_E_OBJECT_DOES_NOT_IMPLEMENT_METHOD": 8,
    "XRE_E_INVALID_PARENT_ID": 9,
    "XRE_E_UNSUPPORTED_OBJECT_TYPE": 10,
    "XRE_E_COULD_NOT_CREATE_FILE": 11,
    "XRE_E_UNSUPPORTED_PROPERTY": 12
};

var key = {
    "F1": 0x01000030,
    "F2": 0x01000031,
    "F3": 0x01000032,
    "F4": 0x01000033,
    "F5": 0x01000034,
    "F6": 0x01000035,
    "F7": 0x01000036,
    "F8": 0x01000037,
    "F9": 0x01000038,
    "F10": 0x01000039,
    "F11": 0x0100003a,
    "F12": 0x0100003b,
    "F13": 0x0100003c,
    "F14": 0x0100003d,
    "F15": 0x0100003e,
    "Left": 0x25,
    "Up": 0x26,
    "Right": 0x27,
    "Down": 0x28,
    "PageUp": 0x01000016,
    "PageDown": 0x01000017,
    "VolumeDown": 0x01000070,
    "VolumeMute": 0x01000071,
    "VolumeUp": 0x01000072,
    "MediaStop": 0x01000081,
    "MediaPrevious": 0,//0x01000082,
    "MediaNext": 0,//0x01000083,
    "MediaTogglePlayPause": 0x1000086,
    "AudioRewind": 0x010000c5,
    "AudioForward": 0x01000102,
    "NumLock": 0x01000025,
    "Enter": 13,
    "Asterisk": 0x2a,
    "Plus": 0x2b,
    "Minus": 0x2d,
    "Period": 0x2e,
    "Slash": 0x2f,
    "Zero": 0x30,
    "One": 0x31,
    "Two": 0x32,
    "Three": 0x33,
    "Four": 0x34,
    "Five": 0x35,
    "Six": 0x36,
    "Seven": 0x37,
    "Eight": 0x38,
    "Nine": 0x39,
    "Apostrophe": 0x27,
    "Return": 0x01000004,
    "Backspace": 0x01000003,
    "Space": 0x20,
    "Delete": 0x01000007,
    "ParenLeft": 0x28,
    "ParenRight": 0x29,
    "Exclam": 0x21,
    "At": 0x40,
    "QuoteDbl": 0x22,
    "NumberSign": 0x23,
    "Dollar": 0x24,
    "Percent": 0x25,
    "Ampersand": 0x26,
    "AsciiCircum": 0x5e,
    "Semicolon": 0x3b,
    "QuoteLeft": 0x60,
    "Backslash": 0x5c,
    "Comma": 0x2c,
    "Less": 0x3c,
    "Equal": 0x3d,
    "Greater": 0x3e,
    "Question": 0x3f,
    "Colon": 0x3a,
    "AsciiTilde": 0x7e,
    "Bar": 0x7c,
    "Underscore": 0x5f,
    "BracketLeft": 0x5b,
    "BracketRight": 0x5d,
    "BraceLeft": 0x7b,
    "BraceRight": 0x7d,
    "Red": 0x01000114,
    "Green": 0x01000115,
    "Yellow": 0x01000116,
    "Blue": 0x01000117,
    "Escape" : 0x1B,
    "A": 0x41,
    "B": 0x42,
    "C": 0x43,
    "D": 0x44,
    "E": 0x45,
    "F": 0x46,
    "G": 0x47,
    "H": 0x48,
    "I": 0x49,
    "J": 0x4a,
    "K": 0x4b,
    "L": 0x4c,
    "M": 0x4d,
    "N": 0x4e,
    "O": 0x4f,
    "P": 0x50,
    "Q": 0x51,
    "R": 0x52,
    "S": 0x53,
    "T": 0x54,
    "U": 0x55,
    "V": 0x56,
    "W": 0x57,
    "X": 0x58,
    "Y": 0x59,
    "Z": 0x5a
};
/* Object to hold truncation styles */
var textTruncStyle = {
    "NONE": "NONE",
    "ELLIPSIS": "ELLIPSIS",
    "ELLIPSIS_AT_WORD_BOUNDARY": "ELLIPSIS_AT_WORD_BOUNDARY"
};
/* Object to hold font styles */
var fontStyle = {
    "BOLD": 0,
    "NORMAL": 1,
    "ITALIC": 2
};
/* Object to hold horizontal alignment styles*/
var horizontalAlignStyle = {
    "LEFT": 0,
    "CENTER": 1,
    "RIGHT": 2
};
/* Object to hold vertical alignment styles*/
var verticalAlignStyle = {
    "TOP": 0,
    "CENTER": 1,
    "BOTTOM": 2
};
/* Arrays to hold font files for each font family (in order bold,normal,italic)*/
var dancingScript = ["DancingScript-Bold.ttf", "DancingScript-Regular.ttf", "FreeSans.ttf"];
var freeSans = ["FreeSans.ttf", "FreeSans.ttf", "FreeSans.ttf"];
var xfinitySansReg = ["XFINITYSansTT-New-BoldCond.ttf", "XFINITYSansTT-New-BoldCond.ttf", "XFINITYSansTT-New-BoldCond.ttf"];;
var withoutSerif = ["DancingScript-Bold.ttf", "DancingScript-Bold.ttf", "DancingScript-Bold.ttf"];
var xfinitySansLgt = ["XFINITYSansTT-New-Lgt.ttf", "XFINITYSansTT-New-Lgt.ttf", "XFINITYSansTT-New-Lgt.ttf"];
var xfinitySansMed = ["XFINITYSansTT-New-Med.ttf", "XFINITYSansTT-New-Med.ttf", "XFINITYSansTT-New-Med.ttf"];
var xfinitySansMedCond = ["XFINITYSansTT-New-MedCond.ttf", "XFINITYSansTT-New-MedCond.ttf", "XFINITYSansTT-New-MedCond.ttf"];

/* Object holding font map */
var fontMap = {
    "dancing": dancingScript,
    "freeSans": freeSans,
    "XFINITY Sans Reg": xfinitySansReg,
    //"without-serif": withoutSerif,
    "XFINITY Sans Lgt": xfinitySansLgt,
    "XFINITY Sans Med": xfinitySansMed,
    "XFINITY Sans Med Cond": xfinitySansMedCond,
};


module.exports = {
    RESULT: result,
    KEY: key,
    TEXT_TRUNC_STYLE: textTruncStyle,
    FONT_MAP: fontMap,
    FONT_STYLE: fontStyle,
    HORIZONTAL_ALIGN_STYLE: horizontalAlignStyle,
    VERTICAL_ALIGN_STYLE: verticalAlignStyle
};
