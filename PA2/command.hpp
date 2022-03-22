/*
 * Copyright (c) 2022, Justin Bradley
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <iostream>
#include <string>
#include <vector>

/// Input stream mode for a command.
enum class istream_mode {
    term, ///< From the terminal.
    file, ///< From a file.
    pipe  ///< From the previous command.
};

/// Output stream mode for a command.
enum class ostream_mode {
    term,   ///< To the terminal.
    file,   ///< To a file.
    append, ///< To a file (append).
    pipe    ///< To the next command.
};

/// Execution of the next command.
enum class next_command_mode {
    always,     ///< Execute unconditionally.
    on_success, ///< Execute if the current command returns zero.
    on_fail     ///< Execute if the current command returns nonzero.
};

/// A single shell command.
struct shell_command {
    /// Name of the command (e.g., echo, ls, cat).
    std::string cmd;

    /// Arguments following the command name.
    std::vector<std::string> args;

    /// Input stream mode.
    istream_mode cin_mode = istream_mode::term;

    /// Input stream filename (if applicable).
    std::string cin_file;

    /// Output stream mode.
    ostream_mode cout_mode = ostream_mode::term;

    /// Output stream filename (if applicable).
    std::string cout_file;

    /// Condition of the next command execution.
    next_command_mode next_mode = next_command_mode::always;
};

/// Pretty-prints istream_mode.
inline std::ostream& operator<<(std::ostream& os, const istream_mode& x)
{
    const char* text[] = {"term", "file", "pipe"};
    return os << text[static_cast<size_t>(x)];
}

/// Pretty-prints ostream_mode.
inline std::ostream& operator<<(std::ostream& os, const ostream_mode& x)
{
    const char* text[] = {"term", "file", "append", "pipe"};
    return os << text[static_cast<size_t>(x)];
}

/// Pretty-prints next_command_mode.
inline std::ostream& operator<<(std::ostream& os, const next_command_mode& x)
{
    const char* text[] = {"always", "on_success", "on_fail"};
    return os << text[static_cast<size_t>(x)];
}

/// Pretty-prints a single command.
inline std::ostream& operator<<(std::ostream& os, const shell_command& x)
{
    os << "cmd: " << x.cmd << "\n";
    for (const auto& arg : x.args) {
        os << "arg: " << arg << "\n";
    }
    os << "cin_file: " << x.cin_file << "\n";
    os << "cin_mode: " << x.cin_mode << "\n";
    os << "cout_file: " << x.cout_file << "\n";
    os << "cout_mode: " << x.cout_mode << "\n";
    os << "next_mode: " << x.next_mode << "\n";
    return os;
}

#endif
