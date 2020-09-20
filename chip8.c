#include <stdio.h>
#include <stdbool.h>

extern const int SCALE;

typedef struct {
	uint8_t display[64*32]; // display de 64x32 pixels
	uint8_t memory[4096];	// chip-8 possui 4 KB (4096 bytes) de memória total.
	uint8_t regs[16];
	uint16_t reg_I;
	uint8_t DT;	// delay timer
	uint8_t ST; // sound timer

	bool keypad[16]; // o keypad hexadecimal do Chip8.

	bool waiting_for_keypress;
	uint8_t waiting_for_keypress_reg;

	uint16_t pc; // program counter
	uint8_t sp; // stack pointer
	uint16_t stack[16]; // stack (16 valores de 16 bits)

	bool debug_autorun; // executar o main loop automaticamente?
} Chip;

void Chip8_Load(Chip *c, char *path) {
	printf("Carregando ROM:\t%s\n", path);

	long rom_length;
	uint8_t *rom_buffer;

	FILE *rom = fopen(path, "rb");

    if (!rom) {
    	fprintf(stderr, "Impossível abrir a ROM para leitura!\n");
    	exit(1);
    }

    fseek(rom, 0, SEEK_END);
    rom_length = ftell(rom);
    rewind(rom);
    rom_buffer = (uint8_t*) malloc(sizeof(uint8_t) * rom_length);
    fread(rom_buffer, sizeof(uint8_t), rom_length, rom);

    // escrever o conteúdo da ROM na RAM a partir de 0x200
    for (int i = 0; i < rom_length; i++) {
    	c->memory[0x200 + i] = rom_buffer[i];
    }

	fclose(rom);
	free(rom_buffer);

	printf("%ld bytes lidos\n", rom_length);

	c->debug_autorun = 1;
	c->pc = 0x200;

	// sprite fonts
	c->memory[0x00 + 0] = 0xF0;	// 0
	c->memory[0x00 + 1] = 0x90;
	c->memory[0x00 + 2] = 0x90;
	c->memory[0x00 + 3] = 0x90;
	c->memory[0x00 + 4] = 0xF0;

	c->memory[0x10 + 0] = 0x20; // 1
	c->memory[0x10 + 1] = 0x60;
	c->memory[0x10 + 2] = 0x20;
	c->memory[0x10 + 3] = 0x20;
	c->memory[0x10 + 4] = 0x70;

	c->memory[0x20 + 0] = 0xF0; // 2
	c->memory[0x20 + 1] = 0x10;
	c->memory[0x20 + 2] = 0xF0;
	c->memory[0x20 + 3] = 0x80;
	c->memory[0x20 + 4] = 0xF0;

	c->memory[0x30 + 0] = 0xF0; // 3
	c->memory[0x30 + 1] = 0x10;
	c->memory[0x30 + 2] = 0xF0;
	c->memory[0x30 + 3] = 0x10;
	c->memory[0x30 + 4] = 0xF0;

	c->memory[0x40 + 0] = 0x90; // 4
	c->memory[0x40 + 1] = 0x90;
	c->memory[0x40 + 2] = 0xF0;
	c->memory[0x40 + 3] = 0x10;
	c->memory[0x40 + 4] = 0x10;

	c->memory[0x50 + 0] = 0xF0; // 5
	c->memory[0x50 + 1] = 0x80;
	c->memory[0x50 + 2] = 0xF0;
	c->memory[0x50 + 3] = 0x10;
	c->memory[0x50 + 4] = 0xF0;

	c->memory[0x60 + 0] = 0xF0; // 6
	c->memory[0x60 + 1] = 0x80;
	c->memory[0x60 + 2] = 0xF0;
	c->memory[0x60 + 3] = 0x90;
	c->memory[0x60 + 4] = 0xF0;

	c->memory[0x70 + 0] = 0xF0; // 7
	c->memory[0x70 + 1] = 0x10;
	c->memory[0x70 + 2] = 0x20;
	c->memory[0x70 + 3] = 0x40;
	c->memory[0x70 + 4] = 0x40;

	c->memory[0x80 + 0] = 0xF0; // 8
	c->memory[0x80 + 1] = 0x90;
	c->memory[0x80 + 2] = 0xF0;
	c->memory[0x80 + 3] = 0x90;
	c->memory[0x80 + 4] = 0xF0;

	c->memory[0x90 + 0] = 0xF0; // 9
	c->memory[0x90 + 1] = 0x90;
	c->memory[0x90 + 2] = 0xF0;
	c->memory[0x90 + 3] = 0x10;
	c->memory[0x90 + 4] = 0xF0;

	c->memory[0xA0 + 0] = 0xF0; // A
	c->memory[0xA0 + 1] = 0x90;
	c->memory[0xA0 + 2] = 0xF0;
	c->memory[0xA0 + 3] = 0x90;
	c->memory[0xA0 + 4] = 0x90;

	c->memory[0xB0 + 0] = 0xE0; // B
	c->memory[0xB0 + 1] = 0x90;
	c->memory[0xB0 + 2] = 0xE0;
	c->memory[0xB0 + 3] = 0x90;
	c->memory[0xB0 + 4] = 0xE0;

	c->memory[0xC0 + 0] = 0xF0; // C
	c->memory[0xC0 + 1] = 0x80;
	c->memory[0xC0 + 2] = 0x80;
	c->memory[0xC0 + 3] = 0x80;
	c->memory[0xC0 + 4] = 0xF0;

	c->memory[0xD0 + 0] = 0xE0; // D
	c->memory[0xD0 + 1] = 0x90;
	c->memory[0xD0 + 2] = 0x90;
	c->memory[0xD0 + 3] = 0x90;
	c->memory[0xD0 + 4] = 0xE0;

	c->memory[0xE0 + 0] = 0xF0; // E
	c->memory[0xE0 + 1] = 0x80;
	c->memory[0xE0 + 2] = 0xF0;
	c->memory[0xE0 + 3] = 0x80;
	c->memory[0xE0 + 4] = 0xF0;

	c->memory[0xF0 + 0] = 0xF0; // F
	c->memory[0xF0 + 1] = 0x80;
	c->memory[0xF0 + 2] = 0xF0;
	c->memory[0xF0 + 3] = 0x80;
	c->memory[0xF0 + 4] = 0x80;
}

void Chip8_Update(Chip *c) {
	if (c->waiting_for_keypress) {
		return;
	}

	uint16_t op = (c->memory[c->pc]  << 8) | c->memory[c->pc + 1];
	//printf("PC %#.3x : %#.4x \n", c->pc, op);

	uint16_t arg_nnn = op & 0x0FFF;
	uint8_t arg_kk = op & 0x00FF;
	uint8_t arg_x = (op & 0x0F00) >> 8;
	uint8_t arg_y = (op & 0x00F0) >> 4;
	uint8_t arg_n = (op & 0x000F);

	if (op == 0x00E0) {
		// CLS limpar display
		for (int i = 0; i < 32*64; i++) {
			c->display[i] = 0x00;
		}
	} else if (op == 0x00EE) {
		// RET retornar de uma subrotina
		c->sp--;
		c->pc = c->stack[c->sp];
	} else if ((op & 0xF000) == 0x1000) {
		// JP pular para nnn
		c->pc = arg_nnn - 0x02;
	} else if ((op & 0xF000) == 0x2000) {
		// CALL
		c->stack[c->sp] = c->pc;
		c->sp++;
		c->pc = arg_nnn - 0x02;
	} else if ((op & 0xF000) == 0x3000) {
		// SE Vx == kk
		if (c->regs[arg_x] == arg_kk)
			c->pc += 0x02;
	} else if ((op & 0xF000) == 0x4000) {
		// SNE Vx != kk
		if (c->regs[arg_x] != arg_kk)
			c->pc += 0x02;
	} else if ((op & 0xF000) == 0x5000) {
		// SE Vx == Vy
		if (c->regs[arg_x] == c->regs[arg_y])
			c->pc += 0x02;
	} else if ((op & 0xF000) == 0x6000) {
		// LD Vx, kk
		c->regs[arg_x] = arg_kk;
	} else if ((op & 0xF000) == 0x7000) {
		// ADD Vx, kk
		c->regs[arg_x] += arg_kk;
	} else if ((op & 0xF00F) == 0x8000) {
		// LD Vx, Vy
		c->regs[arg_x] = c->regs[arg_y];
	} else if ((op & 0xF00F) == 0x8001) {
		// OR Vx, Vy
		c->regs[arg_x] = c->regs[arg_x] | c->regs[arg_y];
	} else if ((op & 0xF00F) == 0x8002) {
		// AND Vx, Vy
		c->regs[arg_x] = c->regs[arg_x] & c->regs[arg_y];
	} else if ((op & 0xF00F) == 0x8003) {
		// XOR Vx, Vy
		c->regs[arg_x] = c->regs[arg_x] ^ c->regs[arg_y];
	} else if ((op & 0xF00F) == 0x8004) {
		// ADD Vx, Vy
		int sum = c->regs[arg_x] + c->regs[arg_y];
		c->regs[0xF] = (sum > 255) ? 1 : 0;
		c->regs[arg_x] = sum & 0xFF;
	} else if ((op & 0xF00F) == 0x8005) {
		// SUB Vx, Vy
		c->regs[0xF] = (c->regs[arg_x] > c->regs[arg_y]) ? 1 : 0;
		c->regs[arg_x] = (c->regs[arg_x] - c->regs[arg_y]) & 0xFF;
	} else if ((op & 0xF00F) == 0x8006) {
		// SHR Vx
		c->regs[0xF] = (c->regs[arg_x] & 1 == 1) ? 1 : 0;
		c->regs[arg_x] = c->regs[arg_x] >> 1;
	} else if ((op & 0xF00F) == 0x8007) {
		// SUBN Vx, Vy
		c->regs[0xF] = (c->regs[arg_y] > c->regs[arg_x]) ? 1 : 0;
		c->regs[arg_x] = (c->regs[arg_y] - c->regs[arg_x]) & 0xFF;
	} else if ((op & 0xF00F) == 0x800E) {
		// SHL Vx
		if ((c->regs[arg_x] & 0x80) == 0x80) {
			c->regs[0xF] = 1;
		} else {
			c->regs[0xF] = 0;
		}

		c->regs[arg_x] = c->regs[arg_x] << 1;
	} else if ((op & 0xF00F) == 0x9000) {
		// SNE Vx, Vy
		if (c->regs[arg_x] != c->regs[arg_y])
			c->pc += 0x02;
	} else if ((op & 0xF000) == 0xA000) {
		// LD I, addr
		c->reg_I = arg_nnn;
	} else if ((op & 0xF000) == 0xB000) {
		// JP V0, addr
		c->pc = c->regs[0] + arg_nnn - 0x02;
	} else if ((op & 0xF000) == 0xC000) {
		// RND Vx, byte
		c->regs[arg_x] = (rand() & 0xFF) & arg_kk;
	} else if ((op & 0xF000)== 0xD000) {
		// DRW Vx, Vy, nibble

		uint8_t target_v_reg_x = arg_x;
		uint8_t target_v_reg_y = arg_y;
		uint8_t x_location = c->regs[target_v_reg_x];
		uint8_t y_location = c->regs[target_v_reg_y];
		uint8_t sprite_height = arg_n;
		uint8_t pixel;

		c->regs[0xF] = 0;
		for (int y_coordinate = 0; y_coordinate < sprite_height; y_coordinate++) {
        	pixel = c->memory[c->reg_I + y_coordinate];
        	for (int x_coordinate = 0; x_coordinate < 8; x_coordinate++) {
        	    if ((pixel & (0x80 >> x_coordinate)) != 0) {
        	    	uint8_t _y = y_location + y_coordinate;
        	    	uint8_t _x = x_location + x_coordinate;
        	        if (c->display[_y * 64 + _x] == 1) {
        	            c->regs[0xF] = 1;
        	        }
        	        c->display[_y * 64 + _x]  ^= 1;
        	    }
        	}
    	}

	} else if ((op & 0xF0FF) == 0xE09E) {
		// SKP Vx
		if (c->keypad[c->regs[arg_x]] == 0x1)
			c->pc += 0x2;
	} else if ((op & 0xF0FF) == 0xE0A1) {
		// SKNP Vx
		if (c->keypad[c->regs[arg_x]] == 0x0)
			c->pc += 0x2;
	} else if ((op & 0xF0FF) == 0xF007) {
		// LD Vx, DT
		c->regs[arg_x] = c->DT;
	} else if ((op & 0xF0FF) == 0xF00A) {
		// LD Vx, K
		// FIXME: implementar teclado
		c->waiting_for_keypress = 0x1;
		c->waiting_for_keypress_reg = arg_x;
		printf("LD Vx, K ..... Aguardando keypress...\n");
	} else if ((op & 0xF0FF) == 0xF015) {
		// LD DT, Vx
		c->DT = c->regs[arg_x];
	} else if ((op & 0xF0FF) == 0xF018) {
		// LD ST, Vx
		c->ST = c->regs[arg_x];
	} else if ((op & 0xF0FF) == 0xF01E) {
		// ADD I, Vx
		c->reg_I += c->regs[arg_x];
	} else if ((op & 0xF0FF) == 0xF029) {
		// LD F, Vx
		c->reg_I = c->regs[arg_x] << 4;
	} else if ((op & 0xF0FF) == 0xF033) {
		// LD B, Vx
		c->memory[c->reg_I] = c->regs[arg_x] / 100;
		c->memory[c->reg_I+1] = (c->regs[arg_x] / 10) % 10;
		c->memory[c->reg_I+2] = (c->regs[arg_x] % 100) % 10;
	} else if ((op & 0xF0FF) == 0xF055) {
		// LD [I], Vx
		for (int i = 0; i <= arg_x; i++) {
			c->memory[c->reg_I + i] = c->regs[i];
		}
		//c->reg_I += arg_x + 1;
	} else if ((op & 0xF0FF) == 0xF065) {
		// LD Vx, [I]
		for (int i = 0; i <= arg_x; i++) {
			c->regs[i] = c->memory[c->reg_I + i];
		}
		//c->reg_I += arg_x + 1;
	}

	c->pc += 0x02;
	if (c->DT > 0) c->DT--;
	if (c->ST > 0) c->ST--;
}

void Chip8_Render(Chip *c, SDL_Renderer *renderer) {
	for (int y = 0; y < 32; y++) {
		for (int x = 0; x < 64; x++) {
			if (c->display[x + y * 64] > 0) {
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			} else {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			}

			SDL_Rect r;
			r.x = x * SCALE;
			r.y = y * SCALE;
			r.w = 1 * SCALE;
			r.h = 1 * SCALE;

			SDL_RenderFillRect(renderer, &r);
		}
	}
}

void Chip8_HandleKeyboard(Chip *c, int key, int down) {
	uint8_t ch8_key;

	switch (key) {
		case SDLK_NUMLOCKCLEAR:
			ch8_key = 0x1;
			break;
		case SDLK_KP_DIVIDE:
			ch8_key = 0x2;
			break;
		case SDLK_KP_MULTIPLY:
			ch8_key = 0x3;
			break;
		case SDLK_KP_MINUS:
			ch8_key = 0xC;
			break;
		case SDLK_KP_7:
			ch8_key = 0x4;
			break;
		case SDLK_KP_8:
			ch8_key = 0x5;
			break;
		case SDLK_KP_9:
			ch8_key = 0x6;
			break;
		case SDLK_KP_PLUS:
			ch8_key = 0xD;
			break;
		case SDLK_KP_4:
			ch8_key = 0x7;
			break;
		case SDLK_KP_5:
			ch8_key = 0x8;
			break;
		case SDLK_KP_6:
			ch8_key = 0x9;
			break;
		case SDLK_KP_COMMA:
			ch8_key = 0xE;
			break;
		case SDLK_KP_1:
			ch8_key = 0xA;
			break;
		case SDLK_KP_2:
			ch8_key = 0x0;
			break;
		case SDLK_KP_3:
			ch8_key = 0xB;
			break;
		case SDLK_KP_ENTER:
			ch8_key = 0xF;
			break;
		default:
			return;
	}

	if (c->waiting_for_keypress) {
		c->waiting_for_keypress = false;
		c->regs[c->waiting_for_keypress_reg] = ch8_key;
	}

	c->keypad[ch8_key] = (down > 0) ? true : false;
}
