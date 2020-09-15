import os


struct Emulator {
	stare Gamestate
	file  os.File
}

enum Gamestate { running paused terminated }

struct CHIP8 {
mut:
	mem []byte
	reg []byte // 8-bit registers V0-VF
	gx  []byte // screen buffer
	op  u16 // opcode to be executed
	i   u16 // index register
	pc  u16 // program counter

	dt byte // delay timer
	st byte // sound timer

	stack []u16 // the stack
	sp    u16

	key   []byte // keypad
	
}

fn new_chip8() &CHIP8 {

	game := os.read_file_array<byte>('breakout.ch8')
	mut mem1 := []byte{len: 4096, init: 0}

	for i in 0..game.len {
		mem1[i + 512] = game[i]
	}

	return &CHIP8{
		mem : mem1
		reg : []byte{len: 16, init: 0}
		gx  : []byte{len: 64*32, init: 0}
		op  : 0
		i   : 0
		pc  : 0x200
		dt  : 0
		st  : 0

		stack : []u16{len: 16, init: 0}
		sp    : 0
		key   : []byte{len: 16, init: 0}
	}


}

fn (c CHIP8) cycle() {
	opcode := u16(c.mem[c.pc] << 8 | c.mem[c.pc + 1])
	println(opcode.hex())


}


fn main() {

	mut emu := new_chip8()
	emu.cycle()

}
