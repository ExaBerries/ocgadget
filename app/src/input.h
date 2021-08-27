#pragma once
#include <variant>

namespace ocgadget {
	struct keyboard_input {
		enum {
			DOWN,
			UP
		} action = DOWN;
		enum {
			ESCAPE,
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			PRINT_SCREEN,
			SCROLL_LOCK,
			PAUSE,
			ACUTE,
			ONE,
			TWO,
			THREE,
			FOUR,
			FIVE,
			SIX,
			SEVEN,
			EIGHT,
			NINE,
			ZERO,
			MINUS,
			EQUALS,
			BACKSPACE,
			INSERT,
			HOME,
			PAGE_UP,
			NUM_LOCK,
			NUMPAD_SLASH,
			NUMPAD_ASTERISK,
			NUMPAD_MINUS,
			TAB,
			Q,
			W,
			E,
			R,
			T,
			Y,
			U,
			I,
			O,
			P,
			LEFT_SQURE_BRACKET,
			RIGHT_SQUARE_BRACKET,
			BACKSLASH,
			DELETE,
			END,
			PAGE_DOWN,
			NUMPAD_SEVEN,
			NUMPAD_EIGHT,
			NUMPAD_NINE,
			NUMPAD_PLUS,
			CAPS_LOCK,
			A,
			S,
			D,
			F,
			G,
			H,
			J,
			K,
			L,
			SEMICOLON,
			APOSTROPHE,
			ENTER,
			NUMPAD_FOUR,
			NUMPAD_FIVE,
			NUMPAD_SIX,
			RIGHT_SHIFT,
			Z,
			X,
			C,
			V,
			B,
			N,
			M,
			COMMA,
			PEROID,
			SLASH,
			LEFT_SHIFT,
			UP_ARROW,
			NUMPAD_ONE,
			NUMPAD_TWO,
			NUMPAD_THREE,
			NUMPAD_ENTER,
			LEFT_CONTROL,
			LEFT_OS,
			LEFT_ALT,
			SPACEBAR,
			RIGHT_ALT,
			RIGHT_OS,
			RIGHT_CLICK,
			RIGHT_CONTROL,
			LEFT_ARROW,
			DOWN_ARROW,
			RIGHT_ARROW,
			NUMPAD_ZERO,
			NUMPAD_PEROID
		} key = A;
	};

	struct mouse_movement_input {
		double x = 0;
		double y = 0;
	};

	struct mouse_button_input {
		enum {
			DOWN,
			UP
		} action = DOWN;
		enum {
			LEFT,
			MIDDLE,
			RIGHT
		} button = LEFT;
	};

	struct mouse_scroll_input {
		double x = 0.0;
		double y = 0.0;
	};

	using input_event = std::variant<keyboard_input, mouse_movement_input, mouse_button_input, mouse_scroll_input>;
} // namespace ocgadget
