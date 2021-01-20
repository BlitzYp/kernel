#include <cstddef>
#include <cstdint>
#include "vga.hpp"

namespace bin {
	enum OPCODES {
		PUTCHAR = 1
	};

	template <class T>
	void parse(T& code) {
		for (size_t i = 0; i < (sizeof code / sizeof *code); i++) {
			if (code[i][0] == (uint8_t) OPCODES::PUTCHAR) {
				// vga_putchar((char) code[i][1], VGA_COLOR::WHITE);
			}
		}
	}
};
