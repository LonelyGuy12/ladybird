/*
 * Copyright (c) 2025, Ladybird Browser Project
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibTest/TestCase.h>
#include <LibWeb/HTML/Scripting/IndependentPythonEngine.h>
#include <LibWeb/HTML/Scripting/PythonSecurityModel.h>
#include <LibWeb/HTML/Scripting/PythonStdLib.h>
#include <LibWeb/Bindings/PythonDOMBindings.h>
#include <LibJS/Runtime/VM.h>

namespace Web::Tests {

class PythonIntegrationTest : public LibTest::TestCase {
public:
    PythonIntegrationTest();
    virtual ~PythonIntegrationTest() override;

protected:
    virtual void setup() override;
    virtual void teardown() override;
    
    NonnullOwnPtr<HTML::IndependentPythonEngine> m_python_engine;
    JS::NonnullGCPtr<JS::VM> m_js_vm;
};

} // namespace Web::Tests
