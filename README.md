
TTY Attr
========

About
-----

This is a small Node.js module based on a native C++ part for
manipulating the terminal attributes via `termios` on file descriptor 0
(`stdin`). It allows to preserve and restore the terminal attributes
and to set the terminal into raw mode.

Motivation
----------

This NPM module is motivated by the problem that programs which
are spawning TUI tools like `tmux`, `lazygit` or `vim` with
the help of [node-pty](https://github.com/microsoft/node-pty/)
experience broken renderings of the TUI tools under Unix-like
operating systems. The origin of this problem was [already
determined](https://github.com/microsoft/node-pty/issues/430) in 2020
and traced back to the usually necessary call to Node.js's setRawMode()
uses the underlying libuv's `UV_TTY_MODE_RAW` which itself causes the
problem. This TTY Attr module performs raw `termios` `cfmakeraw()`
instead which does not cause this problem.

Installation
------------

```shell
$ npm install tty-attr
```

Usage Example
-------------

```ts
import fs          from "node:fs"
import process     from "node:process"

import * as pty    from "node-pty"
import ttyAttr     from "tty-attr"

async function main (): Promise<void> {
    /*  get command and its arguments  */
    const cmd  = process.argv[2]
    const args = process.argv.slice(3).map(String)

    /*  spawn the command in a PTY  */
    const term = pty.spawn(cmd, args, {
        name:     process.env["TERM"] ?? "xterm-color",
        cols:     process.stdout.columns ?? 80,
        rows:     process.stdout.rows    ?? 24,
        cwd:      process.cwd(),
        env:      process.env as Record<string, string>,
        encoding: null
    })

    /*  pipe PTY output through to stdout  */
    term.onData((data: Buffer | string) => {
        const buf = Buffer.isBuffer(data) ? data : Buffer.from(data)
        for (let offset = 0; offset < buf.length; )
            offset += fs.writeSync(1, buf, offset, buf.length - offset)
    })

    /*  preserve terminal attributes and switch to raw mode  */
    if (process.stdin.isTTY) {
        ttyAttr.preserveAttr()
        ttyAttr.setRawMode()
    }

    /*  pipe stdin to the PTY input  */
    process.stdin.on("data", (data: Buffer) => {
        term.write(data)
    })
    process.stdin.resume()

    /*  handle terminal resize  */
    process.stdout.on("resize", () => {
        term.resize(
            process.stdout.columns ?? 80,
            process.stdout.rows    ?? 24
        )
    })

    /*  handle command exit  */
    term.onExit(({ exitCode }: { exitCode: number }) => {
        /*  restore terminal attributes  */
        if (process.stdin.isTTY)
            ttyAttr.restoreAttr()

        /*  pause stdin  */
        process.stdin.pause()

        /*  terminate gracefully  */
        process.exit(exitCode)
    })
}
main().catch((err: unknown) => {
    /*  print errors and terminate with error exit code  */
    process.stderr.write(`pass: ERROR: ${err instanceof Error ? err.message : String(err)}\n`)
    process.exit(1)
})
```

```sh
$ npx tsx example.ts ls -l
$ npx tsx example.ts vim
$ npx tsx example.ts tmux
```

License
-------

Copyright &copy; 2026 Dr. Ralf S. Engelschall (http://engelschall.com/)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

