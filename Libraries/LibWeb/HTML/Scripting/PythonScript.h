/*
 * Copyright (c) 2025, Your Name <your.email@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibJS/Forward.h>
#include <LibWeb/Export.h>
#include <LibWeb/Forward.h>
#include <LibWeb/ForwardPython.h>
#include <LibWeb/HTML/Scripting/Script.h>

namespace Web::HTML {

// https://html.spec.whatwg.org/multipage/webappapis.html#python-script
class WEB_API PythonScript final : public Script {
    GC_CELL(PythonScript, Script);
    GC_DECLARE_ALLOCATOR(PythonScript);

public:
    virtual ~PythonScript() override;

    enum class MutedErrors {
        No,
        Yes,
    };
    static GC::Ref<PythonScript> create(ByteString filename, StringView source, JS::Realm&, URL::URL base_url, MutedErrors = MutedErrors::No);

    // Python-specific methods
    PyObject* script_record() { return m_script_record; }
    PyObject const* script_record() const { return m_script_record; }

    enum class RethrowErrors {
        No,
        Yes,
    };
    JS::Completion run(RethrowErrors = RethrowErrors::No, GC::Ptr<JS::Environment> lexical_environment_override = {});

    MutedErrors muted_errors() const { return m_muted_errors; }

private:
    PythonScript(URL::URL base_url, ByteString filename, JS::Realm&);

    virtual bool is_python_script() const final { return true; }

    virtual void visit_edges(Cell::Visitor&) override;

    PyObject* m_script_record { nullptr };
    PyObject* m_globals { nullptr };
    PyObject* m_locals { nullptr };
    MutedErrors m_muted_errors { MutedErrors::No };
};

}