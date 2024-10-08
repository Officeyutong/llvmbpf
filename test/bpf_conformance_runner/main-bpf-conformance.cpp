#include "llvmbpf.hpp"
#include "ebpf_inst.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

using namespace bpftime;

/**
 * @brief Read in a string of hex bytes and return a vector of bytes.
 *
 * @param[in] input String containing hex bytes.
 * @return Vector of bytes.
 */
static inline std::vector<uint8_t> base16_decode(const std::string &input)
{
	std::vector<uint8_t> output;
	std::stringstream ss(input);
	std::string value;
	while (std::getline(ss, value, ' ')) {
		try {
			output.push_back(std::stoi(value, nullptr, 16));
		} catch (...) {
			// Ignore invalid values.
		}
	}
	return output;
}

/**
 * @brief Convert a vector of bytes to a vector of ebpf_inst.
 *
 * @param[in] bytes Vector of bytes.
 * @return Vector of ebpf_inst.
 */
std::vector<ebpf_inst> bytes_to_ebpf_inst(std::vector<uint8_t> bytes)
{
	std::vector<ebpf_inst> instructions(bytes.size() / sizeof(ebpf_inst));
	memcpy(instructions.data(), bytes.data(), bytes.size());
	return instructions;
}

uint64_t empty_helper_func(uint64_t _a, uint64_t _b, uint64_t _c, uint64_t _d,
	       uint64_t _e)
{
	return 0;
}

int main(int argc, char **argv)
{
	// bool debug = false;
	// bool elf = false;
	std::vector<std::string> args(argv, argv + argc);
	if (args.size() > 0) {
		args.erase(args.begin());
	}
	std::string program_string;
	std::string memory_string;

	if (args.size() > 0 && args[0] == "--help") {
		std::cout
			<< "usage: " << argv[0]
			<< " [--program <base16 program bytes>] [<base16 memory bytes>] [--debug] [--elf]"
			<< std::endl;
		return 1;
	}

	if (args.size() > 1 && args[0] == "--program") {
		args.erase(args.begin());
		program_string = args[0];
		args.erase(args.begin());
	} else {
		std::getline(std::cin, program_string);
	}

	// Next parameter is optional memory contents.
	if (args.size() > 0 && !args[0].starts_with("--")) {
		memory_string = args[0];
		args.erase(args.begin());
	}

	if (args.size() > 0 && args[0] == "--debug") {
		// debug = true;
		args.erase(args.begin());
	}

	if (args.size() > 0 && args[0] == "--elf") {
		// elf = true;
		args.erase(args.begin());
	}

	if (args.size() > 0 && args[0].size() > 0) {
		std::cerr << "Unexpected arguments: " << args[0] << std::endl;
		return 1;
	}

	std::vector<uint8_t> memory = base16_decode(memory_string);
	std::vector<ebpf_inst> program =
		bytes_to_ebpf_inst(base16_decode(program_string));
	std::string log;

	int err;
	auto vm = llvmbpf_vm();
	err = vm.load_code(&program[0], program.size() * sizeof(ebpf_inst));
	if (err < 0) {
		std::cerr << "Error: " << vm.get_error_message() << std::endl;
		return -1;
	}
	// add 1000 pesudo helpers so it can be tested with helpers
	for (int i = 0; i < 1000; i++) {
		vm.register_external_function(i, "helper_" + std::to_string(i),
					      (void*)empty_helper_func);
	}
	auto func = vm.compile();
	assert(func);
	uint64_t res;
	vm.exec(&memory[0], memory.size(), res);
	std::cout << std::hex << res << std::endl;
	return 0;
}
