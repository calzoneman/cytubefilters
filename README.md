node-re2
========

node-re2 is a node.js binding for Google's [re2](https://code.google.com/p/re2/) regular expression engine.  It is currently being developed and has not been tested yet.

## Why?

From the Project Home page for re2:

>Backtracking engines are typically full of features and convenient syntactic sugar but can be forced into taking exponential amounts of time on even small inputs. RE2 uses automata theory to guarantee that regular expression searches run in time linear in the size of the input. RE2 implements memory limits, so that searches can be constrained to a fixed amount of memory.

>On large inputs, RE2 is often much faster than backtracking engines; its use of automata theory lets it apply optimizations that the others cannot.


## Compatibility

I have only tested this on my Arch Linux machine so far.  I plan to try it on other platforms once I get more of the API work done.

## Contributing

I don't have that much experience with node.js bindings.  If you see something that could be done cleaner/better/more efficiently feel free to open an issue or a pull request.

## Copyright

The code for this binding is copyright 2014 Calvin Montgomery.  It is released under the MIT license.  See the included LICENSE file for details.

RE2 is released under the BSD 3-clause license:

```
// Copyright (c) 2009 The RE2 Authors. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```
