// Copyright (c) 2015, the Fletch project authors. Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE.md file.

import 'dart:_fletch_system' as fletch;

const patch = "patch";

@patch bool identical(Object a, Object b) {
  return false;
}

@patch class Object {
  @patch String toString() => '[object Object]';

  @patch noSuchMethod(Invocation invocation) {
    // TODO(kasperl): Extract information from the invocation
    // so we can construct the right NoSuchMethodError.
    throw "NoSuchMethod";
  }

  // The noSuchMethod helper is automatically called from the
  // trampoline and it is passed the selector. The arguments
  // to the original call are still present on the stack, so
  // it is possible to dig them out if need be.
  _noSuchMethod(selector) => noSuchMethod(null);

  // The noSuchMethod trampoline is automatically generated
  // by the compiler. It calls the noSuchMethod helper and
  // takes care off removing an arbitrary number of arguments
  // from the caller stack before it returns.
  external _noSuchMethodTrampoline();
}
