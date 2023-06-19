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

	template<size_t S>
	class input_buffer {
	protected:
		mutable uint8_t data_[S];
		size_t usage_ = 0, position_ = 0;

	public:
		void reset() noexcept {
			usage_ = position_ = 0;
		}

		void reset_if_done() noexcept {
			if (usage_ == position_) {
				reset();
			}
		}

		[[nodiscard]] uint8_t *data() const noexcept {
			return data_;
		}

		[[nodiscard]] size_t fresh_size() const noexcept {
			return usage_ - position_;
		}

		[[nodiscard]] uint8_t *fresh_begin() const noexcept {
			return data_ + position_;
		}

		[[nodiscard]] uint8_t *fresh_end() const noexcept {
			return data_ + usage_;
		}

		[[nodiscard]] size_t& usage() noexcept {
			return usage_;
		}

		[[nodiscard]] size_t& position() noexcept {
			return position_;
		}

		[[nodiscard]] size_t left() const noexcept {
			return S - usage_;
		}
	};

	struct data_holder {
		uint8_t *data;
		size_t size, position;

		std::optional<std::variant<std::string, std::vector<uint8_t>>> holder;

		auto& as_str() {
			return std::get<std::string>(holder.value());
		}

		auto& as_vec8() {
			return std::get<std::vector<uint8_t>>(holder.value());
		}

		void resolve_holder() {
			if (holder.has_value()) {
				switch (holder.value().index()) {
					case 0:
						data = (uint8_t *) as_str().data();
						size = as_str().size();
						break;
					case 1:
						data = as_vec8().data();
						size = as_vec8().size();
						break;
					default:
						break;
				}
			}
		}
	};

	class ServerImpl {
	public:
		Server *pintf;
		io_interface io;
		input_buffer<1024> buf_read;
		std::deque<data_holder> buf_write;
		size_t buf_write_len = 0;
		size_t buf_write_len_limit = 16384;
		command_callback_t cmd_cb;
		bool read_inhibited = false;
		bool nonblocking = false;
		bool parser_debug = false;

		ATParser parser;

		void do_parse() {
			while (!read_inhibited) {
				auto ibuf_fresh_size = buf_read.fresh_size();

				if (ibuf_fresh_size) {
					size_t parsed_len = parser.parse(buf_read.fresh_begin(), ibuf_fresh_size);
					buf_read.position() += parsed_len;
					buf_read.reset_if_done();

					if (parser.state == ATParser::ParseState::Done) {
						if (parser.malformed) {
							write_error();
						} else {
							if (parser.command.empty()) {
								write_ok();
							} else {
								auto rc_cmd = cmd_cb(*pintf, parser.command);

								if (rc_cmd == CommandStatus::OK) {
									write_ok();
								} else if (rc_cmd == CommandStatus::ERROR) {
									write_error();
								}
							}
						}

						if (parser_debug)
							parser.show();

						parser.reset();
					}
				} else {
					break;
				}
			}
		}

		void write_raw(data_holder d) {
			while (buf_write_len + d.size > buf_write_len_limit) {
				if (buf_write.empty()) {
					return;
				} else {
					buf_write_len -= buf_write.front().size;
					buf_write.pop_front();
				}
			}

			auto &nd = buf_write.emplace_back(std::move(d));
			nd.resolve_holder();
			buf_write_len += nd.size;
		}

		std::vector<uint8_t> inhibit_read(size_t raw_data_len) {
			std::vector<uint8_t> ret;

			size_t consume_len = std::min(buf_read.fresh_size(), raw_data_len);

			if (consume_len) {
				ret.resize(consume_len);
				memcpy(ret.data(), buf_read.fresh_begin(), consume_len);
				buf_read.position() += consume_len;
				buf_read.reset_if_done();
			}

			read_inhibited = true;

			return ret;
		}

		void continue_read() noexcept {
			read_inhibited = false;
		}

		void write_data(const void *buf, size_t len) {
			write_raw({(uint8_t *) buf, len, 0});
		}

		void write_cstr(const char *buf, ssize_t len = -1) {
			write_data((void *) buf, len == -1 ? strlen(buf) : len);
		}

		void write_str(std::string str) {
			write_raw(data_holder{
				.position = 0,
				.holder = std::move(str),
			});
		}

		void write_vec8(std::vector<uint8_t> vec8) {
			write_raw(data_holder{
				.position = 0,
				.holder = std::move(vec8),
			});
		}

		void write_error() {
			static const char str[] = "ERROR\r\n";
			write_cstr(str, sizeof(str) - 1);
		}

		void write_ok() {
			static const char str[] = "OK\r\n";
			write_cstr(str, sizeof(str) - 1);
		}

		void write_response_prompt() {
			write_str(parser.command);
			static const char str[] = ": ";
			write_cstr(str, sizeof(str) - 1);
		}

		void write_error_prompt() {
			static const char str[] = "+ERROR: ";
			write_cstr(str, sizeof(str) - 1);
		}

		void write_line_end() {
			static const char str[] = "\r\n";
			write_cstr(str, sizeof(str) - 1);
		}

		using RunStatus = Server::RunStatus;

		RunStatus run() {
			Server::RunStatus ret = RunStatus::OK;

			// 0: nothing attempted, 1: success, 2: blocked
			int read_state = 0, write_state = 0;

			// Step 1: Read
			while (!read_inhibited) {
				auto ibuf_left = buf_read.left();

				if (ibuf_left) {
					ssize_t rc_read = io.callback_io_read(buf_read.fresh_end(), ibuf_left);

					if (rc_read > 0) {
						read_state = 1;
						buf_read.usage() += rc_read;

						if (!nonblocking) {
							do_parse();
							break;
						}
					} else {
						if (read_state == 0) {
							read_state = 2;
						}
						break;
					}
				} else {
					break;
				}
			}


			// Step 2: Parse
			do_parse();

			// Step 3: Write
			while (true) {
				if (!buf_write.empty()) {
					auto &last_data = buf_write.front();

					if (last_data.size) {
						ssize_t rc_write = io.callback_io_write(last_data.data + last_data.position, last_data.size - last_data.position);

						if (rc_write > 0) {
							write_state = 1;
							last_data.position += rc_write;
							if (last_data.position == last_data.size) {
								buf_write_len -= last_data.size;
								buf_write.pop_front();
							}
						} else {
							if (write_state == 0) {
								write_state = 2;
							}
							break;
						}
					} else {
						buf_write.pop_front();
					}
				} else {
					break;
				}
			}

			if (write_state == 2) {
				ret |= RunStatus::WantWrite;
			}

			if (read_state == 2) {
				ret |= RunStatus::WantRead;
			}

			return ret;
		}
	};

	Server::Server() {
		pimpl = std::make_unique<ServerImpl>();
		pimpl->pintf = this;
	}

	Server::~Server() {

	}

	ATParser &Server::parser() noexcept {
		return pimpl->parser;
	}

	void Server::set_command_callback(command_callback_t cmd_cb) {
		pimpl->cmd_cb = std::move(cmd_cb);
	}

	void Server::set_io_callback(io_interface io_cbs) {
		pimpl->io = std::move(io_cbs);
	}

	const io_interface &Server::get_io_callback() noexcept {
		return pimpl->io;
	}

	void Server::set_nonblocking_mode(bool v) noexcept {
		pimpl->nonblocking = v;
	}

	void Server::set_parser_debugging(bool v) noexcept {
		pimpl->parser_debug = v;
	}

	void Server::set_write_buffer_size_limit(size_t l) noexcept {
		pimpl->buf_write_len_limit = l;
	}

	Server::RunStatus Server::run() {
		return pimpl->run();
	}

	std::vector<uint8_t> Server::inhibit_read(size_t raw_data_len) {
		return pimpl->inhibit_read(raw_data_len);
	}

	void Server::continue_read() noexcept {
		pimpl->continue_read();
	}

	void Server::write_data(const void *buf, size_t len) {
		return pimpl->write_data(buf, len);
	}

	void Server::write_cstr(const char *buf, ssize_t len) {
		return pimpl->write_cstr(buf, len);
	}

	void Server::write_str(std::string str) {
		return pimpl->write_str(std::move(str));
	}

	void Server::write_vec8(std::vector<uint8_t> vec8) {
		return pimpl->write_vec8(std::move(vec8));
	}

	void Server::write_response(std::string str) {
		write_response_prompt();
		write_str(std::move(str));
		write_line_end();
	}

	void Server::write_response(const char *buf, ssize_t len) {
		write_response_prompt();
		write_cstr(buf, len);
		write_line_end();
	}

	void Server::write_error() {
		return pimpl->write_error();
	}

	void Server::write_error_reason(std::string str) {
		write_error_prompt();
		write_str(std::move(str));
		write_line_end();
	}

	void Server::write_error_reason(const char *buf, ssize_t len) {
		write_error_prompt();
		write_cstr(buf, len);
		write_line_end();
	}

	void Server::write_ok() {
		return pimpl->write_ok();
	}

	void Server::write_response_prompt() {
		return pimpl->write_response_prompt();
	}

	void Server::write_error_prompt() {
		return pimpl->write_error_prompt();
	}

	void Server::write_line_end() {
		return pimpl->write_line_end();
	}

}
