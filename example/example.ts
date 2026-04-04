
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

