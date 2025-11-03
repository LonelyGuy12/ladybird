#include <AK/Error.h>
#include <AK/String.h>

namespace Web::HTML {

class PythonError : public Error {
public:
    static ErrorOr<String> format_python_exception();
    static Error from_python_exception();
    
    PythonError(String message, String type, String traceback)
        : Error(Error::from_string_literal("PythonError"))
        , m_message(move(message))
        , m_type(move(type))
        , m_traceback(move(traceback))
    {
    }
    
    String const& message() const { return m_message; }
    String const& type() const { return m_type; }
    String const& traceback() const { return m_traceback; }
    
    StringView string_literal() const { return m_message.view(); }
    
private:
    String m_message;
    String m_type;
    String m_traceback;
};

} // namespace Web::HTML
