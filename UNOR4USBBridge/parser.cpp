/*
    This file is part of chAT.
    Copyright (C) 2022-2023 Reimu NotMoe <reimu@sudomaker.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "chAT.hpp"

namespace SudoMaker::chAT {
	ATParser::ATParser() {
		reset();
	}

	void ATParser::reset() {
		malformed = false;
		state = ParseState::Keyword;
		cmd_mode = CommandMode::Run;
		keyword_count = 0;
		command.clear();
		args.clear();
		args_quote = false;
		args_escape_count = 0;
#ifdef CHAT_STRICT_CRLF
		last_data[0] = 0;
		last_data[1] = 0;
#endif
		bytes_parsed = 0;
	}

	void ATParser::show() {
		const char *mode_str = nullptr;

		switch (cmd_mode) {
			case CommandMode::Run:
				mode_str = "run";
				break;
			case CommandMode::Write:
				mode_str = "write";
				break;
			case CommandMode::Read:
				mode_str = "read";
				break;
			case CommandMode::Test:
				mode_str = "test";
				break;
		}

		printf("[parser] malformed: %s, mode: %s, args_count: %zu\n", malformed ? "true" : "false", mode_str, args.size());

		if (!command.empty()) {
			printf("[parser] command: %s\n", command.c_str());
		}

		if (!args.empty()) {
			printf("[parser] args: ");

			for (size_t i=0; i<args.size(); i++) {
				printf("[%zu]=%s ", i, args[i].c_str());
			}

			printf("\n");

		}
	}

	size_t ATParser::parse(const uint8_t *buf, size_t len) {
		static const uint8_t keyword[2] = {'A', 'T'};

		for (size_t i = 0; i < len; i++) {
			uint8_t c = buf[i];

#ifdef CHAT_STRICT_CRLF
			last_data[0] = last_data[1];
			last_data[1] = c;

			if (last_data[0] == '\r' && last_data[1] == '\n') {
#else
			if (c == '\n') {
#endif
				if (state == ParseState::Malformed) {
					malformed = true;
				}
				state = ParseState::Done;
				bytes_parsed += (i+1);
				return i+1;
			}

			switch (state) {
				case ParseState::Keyword: {
					uint8_t kc = keyword[keyword_count];

					if (c == kc) {
						keyword_count++;
					} else {
						state = ParseState::Malformed;
					}

					if (keyword_count == sizeof(keyword)) {
						state = ParseState::Command;
					}

					break;
				}

				case ParseState::Command: {
					switch (c) {
						case '?':
							if (cmd_mode == CommandMode::Run) {
								cmd_mode = CommandMode::Read;
							} else if (cmd_mode == CommandMode::Write) {
								cmd_mode = CommandMode::Test;
							} else {
								state = ParseState::Malformed;
							}
							break;
						case '=':
							if (cmd_mode == CommandMode::Run) {
								cmd_mode = CommandMode::Write;
							} else {
								state = ParseState::Malformed;
							}
							break;
						case '\r':
						case '\n':
							break;
						default:
							if (cmd_mode == CommandMode::Run) {
								command.push_back((char) c);
							} else {
								args.emplace_back();
								state = ParseState::Argument;
								i--;
							}
							break;
					}
					break;
				}

				case ParseState::Argument: {
					auto &cur_arg = args.back();
					if (args_escape_count) {
						cur_arg.push_back((char)c);
						args_escape_count--;
					} else {
						switch (c) {
							case '\\':
								args_escape_count++;
								break;
							case '"':
								args_quote = !args_quote;
								break;
							case ',':
								if (args_quote) {
									cur_arg.push_back(',');
								} else {
									args.emplace_back();
								}
								break;
							case '\r':
							case '\n':
								break;
							default:
								cur_arg.push_back((char)c);
								break;
						}
					}
					break;
				}

				default:
					break;
			}

		}

		bytes_parsed += len;
		return len;
	}
}
