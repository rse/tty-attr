/*
**  TTY-Attr -- TTY Attribute Functions
**  Copyright (c) 2026 Dr. Ralf S. Engelschall <rse@engelschall.com>
**
**  Permission is hereby granted, free of charge, to any person obtaining
**  a copy of this software and associated documentation files (the
**  "Software"), to deal in the Software without restriction, including
**  without limitation the rights to use, copy, modify, merge, publish,
**  distribute, sublicense, and/or sell copies of the Software, and to
**  permit persons to whom the Software is furnished to do so, subject to
**  the following conditions:
**
**  The above copyright notice and this permission notice shall be included
**  in all copies or substantial portions of the Software.
**
**  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
**  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
**  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
**  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
**  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
**  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
**  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <memory>
#include <node.h>
#include <termios.h>
#include <unistd.h>

/*  internal terminal attribute store  */
std::unique_ptr<termios> saved_termios;

/*  preserve terminal attributes  */
static void preserveAttr (const v8::FunctionCallbackInfo<v8::Value> &args) {
    if (!saved_termios) {
        saved_termios = std::make_unique<termios>();
        tcgetattr(STDIN_FILENO, saved_termios.get());
    }
}

/*  set terminal into raw mode  */
static void setRawMode (const v8::FunctionCallbackInfo<v8::Value> &args) {
    termios raw_termios;
    tcgetattr(STDIN_FILENO, &raw_termios);
    cfmakeraw(&raw_termios);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_termios);
}

/*  restore terminal attributes  */
static void restoreAttr (const v8::FunctionCallbackInfo<v8::Value> &args) {
    if (saved_termios) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, saved_termios.get());
    }
}

/*  module initialization  */
static void initialize(v8::Local<v8::Object> exports, v8::Local<v8::Value> module, void *priv) {
    NODE_SET_METHOD(exports, "preserveAttr",  preserveAttr);
    NODE_SET_METHOD(exports, "setRawMode",    setRawMode);
    NODE_SET_METHOD(exports, "restoreAttr",   restoreAttr);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, initialize)

