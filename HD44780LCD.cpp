#include "HD44780LCD.h"

/** get the lower nibble (4 least significant bits) of a byte */
#define             LO_NIBBLE(x)          (((x) >> 0) & 0x0f)

/** get the higher nibble (4 most significant bits) of a byte */
#define             HI_NIBBLE(x)          (((x) >> 4) & 0x0f)

// instruction to clear the display
constexpr uint8_t   LCD_CLEAR_DISPLAY   = 0x01;

/** instruction to set the position of the cursor back to origin */
constexpr uint8_t   LCD_SET_CURSOR_HOME = 0x02;

/** instruction to set the entry mode of the LCD display (direction of cursor/display movement) */
constexpr uint8_t   LCD_SET_ENTRY_MODE  = 0x04;
/** mask to denote that the cursor should move relative to the display, while the absolute position of the display remains constant */
constexpr uint8_t   LCD_CURSOR_MOVE     = 0x00;
/** mask to denote that the display should move relative to the cursor, while the absolute position of the cursor remains constant */
constexpr uint8_t   LCD_DISPLAY_MOVE    = 0x01;
/** mask to denote that the cursor should move left (DDRAM address decrements) each time a character is written */
constexpr uint8_t   LCD_CURSOR_POS_DEC  = 0x00;
/** mask to denote that the cursor should move right (DDRAM address increments) each time a character is written */
constexpr uint8_t   LCD_CURSOR_POS_INC  = 0x02;

/** instruction to control the LCD display (whether the display/cursor/blink is enabled or not) */
constexpr uint8_t   LCD_CONTROL_DISPLAY = 0x08;
/** mask to denote that the blinking cursor is disabled */
constexpr uint8_t   LCD_BLINK_DISABLE   = 0x00;
/** mask to denote that the blinking cursor is enabled */
constexpr uint8_t   LCD_BLINK_ENABLE    = 0x01;
/** mask to denote that the underscore cursor is disabled */
constexpr uint8_t   LCD_CURSOR_DISABLE  = 0x00;
/** mask to denote that the underscore cursor is enabled */
constexpr uint8_t   LCD_CURSOR_ENABLE   = 0x02;
/** mask to denote that the LCD Display is enabled */
constexpr uint8_t   LCD_DISPLAY_ENABLE  = 0x04;

/** instruction to move the cursor/display left or right */
constexpr uint8_t   LCD_SHIFT_CURSOR    = 0x10;
/** mask to denote that the cursor should move left */
constexpr uint8_t   LCD_CURSOR_MOVE_LT  = 0x00;
/** mask to denote that the cursor should move right */
constexpr uint8_t   LCD_CURSOR_MOVE_RT  = 0x04;
/** mask to denote that the display should move left */
constexpr uint8_t   LCD_DISPLAY_MOVE_LT = 0x08;
/** mask to denote that the display should move right */
constexpr uint8_t   LCD_DISPLAY_MOVE_RT = 0x0C;

/** instruction to set the function of the LCD (4-bit/8-bit bus, 1/2 line mode, 8/11 columns in the cell) */
constexpr uint8_t   LCD_SET_FUNCTION    = 0x20;
/** mask to denote that the count of dots in a column of the display is 8 */
constexpr uint8_t   LCD_DOT_COUNT_8     = 0x00;
/** mask to denote that the count of dots in a column of the display is 11 */
constexpr uint8_t   LCD_DOT_COUNT_11    = 0x04;
/** mask to denote that the number of lines in the LCD display is 1 */
constexpr uint8_t   LCD_LINE_COUNT_1    = 0x00;
/** mask to denote that the number of lines in the LCD display is 2 */
constexpr uint8_t   LCD_LINE_COUNT_2    = 0x08;
/** mask to denote that the bus is 4 lines wide */
constexpr uint8_t   LCD_BUS_SIZE_4      = 0x00;
/** mask to denote that the bus is 8 lines wide */
constexpr uint8_t   LCD_BUS_SIZE_8      = 0x10;

/** instruction to set the address in the CGRAM (where the subsequent data will be written to) */
constexpr uint8_t   LCD_SET_CGRAMADDR   = 0x40;

/** instruction to set the address in the DDRAM (where the subsequent data will be written to) */
constexpr uint8_t   LCD_SET_DDRAMADDR   = 0x80;

/** address of the first position in single-line mode */
constexpr uint8_t   LCD_ORIG_ADDR_SINGLE= 0x00;
/** address of the first position of the first line in two-line mode */
constexpr uint8_t   LCD_ORIG_ADDR_FIRST = 0x00;
/** address of the first position of the second line in two-line mode */
constexpr uint8_t   LCD_ORIG_ADDR_SECOND= 0x40;
/** the width of a single line of the LCD */
constexpr uint8_t   LCD_LINE_SIZE       = 0x28;

/** the index of the bit that manages the RS pin of the LCD */
constexpr uint8_t   RS_ID				= 0;
/** the index of the bit that manages the RW pin of the LCD */
constexpr uint8_t   RW_ID				= 1;
/** the index of the bit that manages the EN pin of the LCD */
constexpr uint8_t   EN_ID               = 2;
/** the index of the bit that manages the backlight of the LCD */
constexpr uint8_t   BACKLIGHT_ID        = 3;

// Constructors

HD44780LCD::HD44780LCD(PinName i2c_sda, PinName i2c_scl)
        : con(i2c_sda, i2c_scl)
        , cursorLoc {LCD_ORIG_ADDR_FIRST}
{}

// public methods

void
HD44780LCD::initialize() {

    ThisThread::sleep_for(50ms);

    displayState = LCD_DISPLAY_ENABLE | LCD_CURSOR_DISABLE | LCD_BLINK_DISABLE;
    cursorMovement = LCD_CURSOR_MOVE | LCD_CURSOR_POS_INC;

    for (uint32_t i = 0; i < 3; ++i) {

        con.send_nibble(HI_NIBBLE(LCD_SET_FUNCTION | LCD_BUS_SIZE_8), 0);
        ThisThread::sleep_for(5ms);
    }

    con.send_nibble(HI_NIBBLE(LCD_SET_FUNCTION | LCD_BUS_SIZE_4), 0);
    ThisThread::sleep_for(2ms);

    con.send_byte(LCD_SET_FUNCTION | LCD_BUS_SIZE_4 | LCD_DOT_COUNT_8 | LCD_LINE_COUNT_2, 0);
    ThisThread::sleep_for(2ms);

    con.send_byte(LCD_CLEAR_DISPLAY, 0);
    ThisThread::sleep_for(2ms);

    con.send_byte(LCD_SET_CURSOR_HOME, 0);
    ThisThread::sleep_for(2ms);

    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
    ThisThread::sleep_for(2ms);

    con.send_byte(LCD_SET_ENTRY_MODE | cursorMovement, 0);
    ThisThread::sleep_for(2ms);
}


void
HD44780LCD::send_data(const uint8_t byte) {

    con.send_byte(byte, 1);

    (cursorMovement & LCD_CURSOR_POS_INC)
    ? inc_cursor_loc()
    : dec_cursor_loc();
}

void
HD44780LCD::send_buffer(const uint8_t *buf, const uint32_t len) {

    for (const uint8_t *ptr = buf; ptr != &buf[len]; ++ptr) {
        send_data(*ptr);
    }
}

void
HD44780LCD::create_custom_char(const uint32_t loc, const uint8_t *glyph) {

    con.send_byte(LCD_SET_CGRAMADDR | (loc << 3), 0);

    for (const uint8_t *ptr = glyph; ptr != &glyph[8]; ++ptr) {
        con.send_byte(*ptr, 1);
    }

    auto r = get_cursor_row();
    auto c = get_cursor_col();
    set_cursor_pos(r, c);
}


void
HD44780LCD::enable_backlight() {
    con.enable_backlight();
}

void
HD44780LCD::disable_backlight() {
    con.disable_backlight();
}


void
HD44780LCD::toggle_backlight() {
    con.toggle_backlight();
}

bool
HD44780LCD::is_backlight_on() const {
    return con.is_backlight_on();
}


void
HD44780LCD::set_cursor_auto_dec() {

    cursorMovement = LCD_CURSOR_POS_DEC | LCD_CURSOR_MOVE;
    con.send_byte(LCD_SET_ENTRY_MODE | cursorMovement, 0);
}

void
HD44780LCD::set_cursor_auto_inc() {

    cursorMovement = LCD_CURSOR_POS_INC | LCD_CURSOR_MOVE;
    con.send_byte(LCD_SET_ENTRY_MODE | cursorMovement, 0);
}

void
HD44780LCD::set_display_auto_dec() {

    cursorMovement = LCD_CURSOR_POS_INC | LCD_DISPLAY_MOVE;
    con.send_byte(LCD_SET_ENTRY_MODE | cursorMovement, 0);
}

void
HD44780LCD::set_display_auto_inc() {

    cursorMovement = LCD_CURSOR_POS_DEC | LCD_DISPLAY_MOVE;
    con.send_byte(LCD_SET_ENTRY_MODE | cursorMovement, 0);
}

HD44780LCD::EntryMode
HD44780LCD::get_entry_mode() const {

    switch (cursorMovement) {

        case LCD_CURSOR_MOVE | LCD_CURSOR_POS_DEC:
            return CURSOR_DEC;

        case LCD_CURSOR_MOVE | LCD_CURSOR_POS_INC:
            return CURSOR_INC;

        case LCD_DISPLAY_MOVE | LCD_CURSOR_POS_INC:
            return DISPLAY_DEC;

        case LCD_DISPLAY_MOVE | LCD_CURSOR_POS_DEC:
            return DISPLAY_INC;

        default:
            return CURSOR_INC;
    }
}


void
HD44780LCD::clear_display() {
    con.send_byte(LCD_CLEAR_DISPLAY, 0);
}

void
HD44780LCD::enable_display() {

    displayState |= LCD_DISPLAY_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

void
HD44780LCD::disable_display() {

    displayState &= ~LCD_DISPLAY_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

void
HD44780LCD::toggle_display() {

    displayState ^= LCD_DISPLAY_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

bool
HD44780LCD::is_display_enabled() const {
    return displayState != 0;
}

void
HD44780LCD::set_cursor_home() {

    cursorLoc = LCD_ORIG_ADDR_FIRST;
    con.send_byte(LCD_SET_CURSOR_HOME, 0);
}

void
HD44780LCD::move_cursor_left() {

    con.send_byte(LCD_SHIFT_CURSOR | LCD_CURSOR_MOVE_LT, 0);
    dec_cursor_loc();
}

void
HD44780LCD::move_cursor_right() {

    con.send_byte(LCD_SHIFT_CURSOR | LCD_CURSOR_MOVE_RT, 0);
    inc_cursor_loc();
}

void
HD44780LCD::set_cursor_pos(const uint32_t r, const uint32_t c) {

    if (r > 1 || c > LCD_LINE_SIZE) {
        return;
    }

    cursorLoc = ((r != 0)
            ? (LCD_ORIG_ADDR_SECOND)
            : (LCD_ORIG_ADDR_FIRST))
            + c;

    update_display_cursor_pos();
}

uint32_t
HD44780LCD::get_cursor_row() const {

    return (cursorLoc >= LCD_ORIG_ADDR_SECOND)
    ? 1
    : 0;
}

uint32_t
HD44780LCD::get_cursor_col() const {

    return cursorLoc -
            ((cursorLoc >= LCD_ORIG_ADDR_SECOND)
            ? LCD_ORIG_ADDR_SECOND
            : LCD_ORIG_ADDR_FIRST);
}


void
HD44780LCD::enable_cursor_display() {

    displayState |= LCD_CURSOR_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

void
HD44780LCD::disable_cursor_display() {

    displayState &= ~LCD_CURSOR_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

void
HD44780LCD::toggle_cursor_display() {

    displayState ^= LCD_CURSOR_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

bool
HD44780LCD::is_cursor_displayed() const {
    return (displayState & LCD_CURSOR_ENABLE) != 0;
}

void
HD44780LCD::enable_blinking_cursor() {

    displayState |= LCD_BLINK_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

void
HD44780LCD::disable_blinking_cursor() {

    displayState &= ~LCD_BLINK_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

void
HD44780LCD::toggle_blinking_cursor() {

    displayState ^= LCD_BLINK_ENABLE;
    con.send_byte(LCD_CONTROL_DISPLAY | displayState, 0);
}

bool
HD44780LCD::is_blinking_cursor_displayed() const {
    return (displayState & LCD_BLINK_ENABLE) != 0;
}


void
HD44780LCD::scroll_display_left() {
    con.send_byte(LCD_SHIFT_CURSOR | LCD_DISPLAY_MOVE_LT, 0);
}

void
HD44780LCD::scroll_display_right() {
    con.send_byte(LCD_SHIFT_CURSOR | LCD_DISPLAY_MOVE_RT, 0);
}


HD44780LCD *HD44780LCD::get_stream() {

    initialize();
    return this;
}


int HD44780LCD::_putc(int c) {

    auto row = get_cursor_row();
    auto col = get_cursor_col();

    switch (c) {

        case '\n':

            set_cursor_pos(row ^ 1, col);
            return 0;

        case '\r':

            set_cursor_pos(row, 0);
            return 0;

        default:

            send_data((uint8_t)c);
            break;
    }

    return 0;
}

int HD44780LCD::_getc() {
    return -1;
}

// private methods

void
HD44780LCD::update_display_cursor_pos() {
    con.send_byte(LCD_SET_DDRAMADDR | cursorLoc, 0);
}

void
HD44780LCD::inc_cursor_loc() {

    ++cursorLoc;
    if (cursorLoc >= (LCD_ORIG_ADDR_SECOND + LCD_LINE_SIZE)) {
        cursorLoc = LCD_ORIG_ADDR_FIRST;
        //update_display_cursor_pos();
    }
    else if ((cursorLoc < LCD_ORIG_ADDR_SECOND)
                && (cursorLoc >= (LCD_ORIG_ADDR_FIRST + LCD_LINE_SIZE))) {
        cursorLoc = LCD_ORIG_ADDR_SECOND;
        //update_display_cursor_pos();
    }
}

void
HD44780LCD::dec_cursor_loc() {

    if (cursorLoc == LCD_ORIG_ADDR_FIRST) {
        cursorLoc = LCD_ORIG_ADDR_SECOND + LCD_LINE_SIZE - 1;
        //update_display_cursor_pos();
    }
    else if (cursorLoc == LCD_ORIG_ADDR_SECOND) {
        cursorLoc = LCD_ORIG_ADDR_FIRST + LCD_LINE_SIZE - 1;
        //update_display_cursor_pos();
    }
    else {
        --cursorLoc;
    }
}

// private class methods

HD44780LCD::I2CInterface::I2CInterface(PinName I2cSda, PinName I2cScl, uint8_t addr)
        : con(I2cSda, I2cScl)
        , addr {addr}
        , backlightMask {0}
{}


void
HD44780LCD::I2CInterface::send_byte(uint8_t byte, uint8_t isData) {

    static struct {

        uint8_t result;
        uint8_t buf[3];
    } pack;
    static_assert(sizeof(pack) == 4);

    for (uint32_t _ = 0; _ <= 4; _ += 4) {

        auto nibble = (byte >> (4 ^ _)) & 0xf;
        pack.result = (nibble << 4) | (isData << RS_ID) | backlightMask;

        pack.buf[0] = pack.result;
        pack.buf[1] = pack.result | (1 << EN_ID);
        pack.buf[2] = pack.result;

        for (const uint8_t *ptr = pack.buf; ptr != &pack.buf[3]; ++ptr) {

            con.write(addr, (const char *)ptr, 1);
            ThisThread::sleep_for(1ms);
        }
    }
}

void
HD44780LCD::I2CInterface::send_nibble(uint8_t nibble, uint8_t isData) {

    static struct {

        uint8_t result;
        uint8_t buf[3];
    } pack;
    static_assert(sizeof(pack) == 4);

    pack.result = (nibble << 4) | (isData << RS_ID) | backlightMask;

    pack.buf[0] = pack.result;
    pack.buf[1] = pack.result | (1 << EN_ID);
    pack.buf[2] = pack.result;

    for (const uint8_t *ptr = pack.buf; ptr != &pack.buf[3]; ++ptr) {

        con.write(addr, (const char *)ptr, 1);
        ThisThread::sleep_for(1ms);
    }
}

void
HD44780LCD::I2CInterface::enable_backlight() {

    backlightMask = (1 << BACKLIGHT_ID);
    con.write(addr, (const char *)(&backlightMask), 1);
}

void
HD44780LCD::I2CInterface::disable_backlight() {

    backlightMask &= ~(1 << BACKLIGHT_ID);
    con.write(addr, (const char *)(&backlightMask), 1);
}

void
HD44780LCD::I2CInterface::toggle_backlight() {

    backlightMask ^= (1 << BACKLIGHT_ID);
    con.write(addr, (const char *)(&backlightMask), 1);
}

bool
HD44780LCD::I2CInterface::is_backlight_on() const {
    return backlightMask != 0;
}
