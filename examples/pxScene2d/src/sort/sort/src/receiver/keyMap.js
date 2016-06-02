var KEY = require('../utils/enum.js').KEY;
var XRElogger = require('../utils/xrelogger.js');

var XREKeyMap = function() {
    var eventLogger = XRElogger.getLogger("event");
    eventLogger.log("info","---------------------------XREKeyMap");
    eventLogger.log("info",KEY.Ampersand);
    var m_keys = {};
    var m_controlKeys = {};
    var m_externKeys = {};
    m_keys[KEY.PageUp] = "PAGE_UP";
    m_keys[KEY.PageDown] = "PAGE_DOWN";
    m_keys[KEY.A] = "A";
    m_keys[KEY.B] = "B";
    m_keys[KEY.C] = "C";
    m_keys[KEY.D] = "D";
    m_keys[KEY.E] = "E";
    m_keys[KEY.F] = "F";
    m_keys[KEY.G] = "G";
    m_keys[KEY.H] = "H";
    m_keys[KEY.I] = "I";
    m_keys[KEY.J] = "J";
    m_keys[KEY.K] = "K";
    m_keys[KEY.L] = "L";
    m_keys[KEY.M] = "M";
    m_keys[KEY.N] = "N";
    m_keys[KEY.O] = "O";
    m_keys[KEY.P] = "P";
    m_keys[KEY.Q] = "Q";
    m_keys[KEY.R] = "R";
    m_keys[KEY.S] = "S";
    m_keys[KEY.T] = "T";
    m_keys[KEY.U] = "U";
    m_keys[KEY.V] = "V";
    m_keys[KEY.W] = "W";
    m_keys[KEY.X] = "X";
    m_keys[KEY.Y] = "Y";
    m_keys[KEY.Z] = "Z";
    m_keys[KEY.Zero] = "NUMBER_0";
    m_keys[KEY.One] = "NUMBER_1";
    m_keys[KEY.Two] = "NUMBER_2";
    m_keys[KEY.Three] = "NUMBER_3";
    m_keys[KEY.Four] = "NUMBER_4";
    m_keys[KEY.Five] = "NUMBER_5";
    m_keys[KEY.Six] = "NUMBER_6";
    m_keys[KEY.Seven] = "NUMBER_7";
    m_keys[KEY.Eight] = "NUMBER_8";
    m_keys[KEY.Nine] = "NUMBER_9";
    m_keys[KEY.Left] = "LEFT";
    m_keys[KEY.Up] = "UP";
    m_keys[KEY.Right] = "RIGHT";
    m_keys[KEY.Down] = "DOWN";
    m_keys[KEY.Escape] = "ESCAPE";
    m_keys[KEY.Tab] = "TAB";
    m_keys[KEY.Backtab] = "TAB";
    m_keys[KEY.Backspace] = "BACKSPACE";
    m_keys[KEY.Space] = "SPACE";
    m_keys[KEY.Enter] = "ENTER";
    m_keys[KEY.Return] = "ENTER";
    m_keys[KEY.Asterisk] = "NUMPAD_MULTIPLY";

    //TODO: need to map keys
    /*m_keys.insert(Qt::Key_Exclam, "NUMBER_1");
        m_keys.insert(Qt::Key_At, "NUMBER_2");
        m_keys.insert(Qt::Key_NumberSign, "NUMBER_3");
        m_keys.insert(Qt::Key_Dollar, "NUMBER_4");
        m_keys.insert(Qt::Key_Percent, "NUMBER_5");
        m_keys.insert(Qt::Key_AsciiCircum, "NUMBER_6");
        m_keys.insert(Qt::Key_Ampersand, "NUMBER_7");
        m_keys.insert(Qt::Key_Asterisk, "NUMBER_8");
        m_keys.insert(Qt::Key_ParenLeft, "NUMBER_9");
        m_keys.insert(Qt::Key_ParenRight, "NUMBER_0");

        m_keys.insert(Qt::Key_Period, "PERIOD");
        m_keys.insert(Qt::Key_Greater, "PERIOD");
        m_keys.insert(Qt::Key_Comma, "COMMA");
        m_keys.insert(Qt::Key_Less, "COMMA");
        m_keys.insert(Qt::Key_Semicolon, "SEMICOLON");
        m_keys.insert(Qt::Key_Colon, "SEMICOLON");
        m_keys.insert(Qt::Key_Slash, "SLASH");
        m_keys.insert(Qt::Key_Question, "SLASH");
        m_keys.insert(Qt::Key_Minus, "MINUS");
        m_keys.insert(Qt::Key_Underscore, "MINUS");
        m_keys.insert(Qt::Key_Plus, "EQUAL");
        m_keys.insert(Qt::Key_Equal, "EQUAL");
        m_keys.insert(Qt::Key_Apostrophe, "QUOTE");
        m_keys.insert(Qt::Key_QuoteDbl, "QUOTE");
        m_keys.insert(Qt::Key_QuoteLeft, "BACKQUOTE");
        m_keys.insert(Qt::Key_AsciiTilde, "BACKQUOTE");
        m_keys.insert(Qt::Key_Backslash, "BACKSLASH");
        m_keys.insert(Qt::Key_Bar, "BACKSLASH");
        m_keys.insert(Qt::Key_BracketLeft, "LEFTBRACKET");
        m_keys.insert(Qt::Key_BraceLeft, "LEFTBRACKET");
        m_keys.insert(Qt::Key_BracketRight, "RIGHTBRACKET");
        m_keys.insert(Qt::Key_BraceRight, "RIGHTBRACKET");

*/
    m_keys[KEY.F1] = "F1";
    m_keys[KEY.F2] = "F2";
    m_keys[KEY.F3] = "F3";
    m_keys[KEY.F4] = "F4";
    m_keys[KEY.F5] = "F5";
    m_keys[KEY.F6] = "F6";
    m_keys[KEY.F7] = "F7";
    m_keys[KEY.F8] = "F8";
    m_keys[KEY.F9] = "F9";
    m_keys[KEY.F10] = "F10";
    m_keys[KEY.F11] = "F11";
    m_keys[KEY.F12] = "F12";

    m_keys[KEY.VolumeMute] = "MUTE";
    m_keys[KEY.VolumeDown] = "VOLUME_DOWN";
    m_keys[KEY.VolumeUp] = "VOLUME_UP";
    m_keys[KEY.MediaNext] = "CHANNEL_UP";
    m_keys[KEY.MediaPrevious] = "CHANNEL_DOWN";
    m_keys[KEY.MediaStop] = "STOP";
    m_keys[KEY.MediaTogglePlayPause] = "PLAY";
    m_keys[KEY.AudioRewind] = "REWIND";
    m_keys[KEY.AudioForward] = "FAST_FORWARD";
    // Platform::setupXREKeyMap(m_keys);

    //Emulate key on a remote control using CTL+KEY
    m_controlKeys[KEY.M] = "MENU";
    m_controlKeys[KEY.I] = "INFO";
    m_controlKeys[KEY.E] = "EXIT";
    m_controlKeys[KEY.N] = "NEXT_FAV_CHAN";
    m_controlKeys[KEY.L] = "PREV";
    m_controlKeys[KEY.O] = "ON_DEMAND";
    m_controlKeys[KEY.P] = "PLAY";
    m_controlKeys[KEY.T] = "PLAY";
    m_controlKeys[KEY.A] = "PAUSE";
    m_controlKeys[KEY.S] = "STOP";
    m_controlKeys[KEY.R] = "RECORD";
    m_controlKeys[KEY.F] = "FAST_FORWARD";
    m_controlKeys[KEY.W] = "REWIND";
    m_controlKeys[KEY.U] = "VOLUME_UP";
    m_controlKeys[KEY.D] = "VOLUME_DOWN";
    m_controlKeys[KEY.Zero] = "COLOR_KEY_0";
    m_controlKeys[KEY.One] = "COLOR_KEY_1";
    m_controlKeys[KEY.Two] = "COLOR_KEY_2";
    m_controlKeys[KEY.Three] = "COLOR_KEY_3";
    m_controlKeys[KEY.G] = "GUIDE";
    m_controlKeys[KEY.Y] = "MUTE";
    m_controlKeys[KEY.Up] = "CHANNEL_UP";
    m_controlKeys[KEY.Down] = "CHANNEL_DOWN";
    clog("init key .........................");
    this.getVK = function(key, modifiers) {
        //TODO:
        /*(void)nativeKey;
        //Check for remapping of native keys
        if( m_externKeys.contains(qtKey) && m_externKeys[qtKey].ctl == ctl && m_externKeys[qtKey].alt == alt && m_externKeys[qtKey].shift==shift )
        {
                return m_externKeys[qtKey].name;
        }
        else if( (ctl && !alt && !shift) && m_controlKeys.contains(qtKey) )
        {
                return m_controlKeys[qtKey];
        }
        else
        {*/
        if ((modifiers.ctrl && !modifiers.alt && !modifiers.shift) && m_controlKeys[key]) {
            clog("ctrl pressed----------------------");
            return m_controlKeys[key];
        } else if (m_keys[key]) {
            clog("no modifiers pressed----------------------");
            return m_keys[key];
        } else{
            return "unknown";
        }
    };
};

//TODO:
/*void XREKeyMap::LoadExternalKeys( QIODevice * keyMapData )
{
        QXmlStreamReader xml(keyMapData);
        while(!(xml.hasError() || xml.atEnd()))
        {
                QXmlStreamReader::TokenType type = xml.readNext();

                switch(type)
                {
                 case QXmlStreamReader::StartElement:
                        if(xml.name() == "key")
                        {
                                QXmlStreamAttributes  attributes = xml.attributes();
                                bool ok=true;
                                int code = xml.readElementText().toInt(&ok,16);

                                XRE_KEY key;
                                key.name = attributes.value("name").toString();
                                if(attributes.hasAttribute("ctl"))              key.ctl = attributes.value("ctl") == QString::fromAscii("true");
                                if(attributes.hasAttribute("alt"))              key.alt = attributes.value("alt") == QString::fromAscii("true");
                                if(attributes.hasAttribute("shift"))    key.shift = attributes.value("shift") == QString::fromAscii("true");
                                m_externKeys.insert( code, key );
                                Q_ASSERT(ok);
                                break;
                        }
                default:
                        break;
                }
        }
}*/
module.exports = XREKeyMap;