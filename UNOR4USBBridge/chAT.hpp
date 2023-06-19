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

#pragma once

#include <tuple>
#include <variant>
#include <vector>
#include <deque>
#include <string>
#include <functional>
#include <memory>
#include <optional>

#include <cstdio>
#include <cstring>
#include <cinttypes>

namespace SudoMaker::chAT {
	enum class CommandMode : unsigned int {
		Run, Write, Read, Test
	};

	class ATParser {
	public:
		enum class ParseState : unsigned int {
			Keyword, Command, Argument, Done, Malformed
		};

		bool malformed;
		ParseState state;
		CommandMode cmd_mode;
		size_t keyword_count;
		std::string command;
		std::vector<std::string> args;
		bool args_quote;
		size_t args_escape_count;
#ifdef CHAT_STRICT_CRLF
		uint8_t last_data[2];
#endif
		size_t bytes_parsed;

		ATParser();

		void reset();
		void show();
		size_t parse(const uint8_t *buf, size_t len);
	};

	struct io_interface {
		typedef std::function<ssize_t(uint8_t *, size_t)> io_callback_t;

		io_callback_t callback_io_read, callback_io_write;
	};

	enum class CommandStatus {
		OK, ERROR, CUSTOM
	};

	class Server;
	class ServerImpl;

	typedef std::function<CommandStatus(Server& srv, const std::string& command)> command_callback_t;

	class Server {
	private:
		std::unique_ptr<ServerImpl> pimpl;
	public:
		enum RunStatus : unsigned {
			OK = 0, WantRead = 0x1, WantWrite = 0x2,
		};

		Server();
		~Server();

		ATParser &parser() noexcept;

		void set_command_callback(command_callback_t cmd_cb);
		void set_io_callback(io_interface io_cbs);
		const io_interface& get_io_callback() noexcept;
		void set_nonblocking_mode(bool v) noexcept;
		void set_parser_debugging(bool v) noexcept;
		void set_write_buffer_size_limit(size_t l = 16384) noexcept;

		std::vector<uint8_t> inhibit_read(size_t raw_data_len);
		void continue_read() noexcept;

		void write_data(const void *buf, size_t len);
		void write_cstr(const char *buf, ssize_t len = -1);
		void write_str(std::string str);
		void write_vec8(std::vector<uint8_t> vec8);

		void write_response(std::string str);
		void write_response(const char *buf, ssize_t len = -1);
		void write_error_reason(std::string str);
		void write_error_reason(const char *buf, ssize_t len = -1);
		void write_error();
		void write_ok();

		void write_response_prompt();
		void write_error_prompt();
		void write_line_end();

		RunStatus run();
	};

	inline Server::RunStatus operator|(Server::RunStatus a, Server::RunStatus b) {
		return static_cast<Server::RunStatus>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
	}

	inline Server::RunStatus operator&(Server::RunStatus a, Server::RunStatus b) {
		return static_cast<Server::RunStatus>(static_cast<unsigned>(a) & static_cast<unsigned>(b));
	}

	inline Server::RunStatus& operator|=(Server::RunStatus& a, Server::RunStatus b) {
		a = a | b;
		return a;
	}

	inline Server::RunStatus& operator&=(Server::RunStatus& a, Server::RunStatus b) {
		a = a & b;
		return a;
	}
}
