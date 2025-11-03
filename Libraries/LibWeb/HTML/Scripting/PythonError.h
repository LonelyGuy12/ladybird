#include <AK/Error.h>
#include <AK/Optional.h>
#include <AK/String.h>
#include <AK/ByteString.h>

namespace Web::HTML {

class PythonError : public Error {
public:
    static ErrorOr<String> format_python_exception();
    static Error from_python_exception();
    
    PythonError(String message, String type, String traceback)
        : Error(Error::from_string_literal("PythonError"))
        , m_message(move(message))
        , m_message_bytes(m_message.bytes_as_string_view().to_byte_string())
        , m_type(move(type))
        , m_traceback(move(traceback))
    {
    }
    
    String const& message() const { return m_message; }
    String const& type() const { return m_type; }
    String const& traceback() const { return m_traceback; }
    
    StringView string_literal() const { return m_message_bytes.view(); }
    
private:
    String m_message;
    ByteString m_message_bytes;
    String m_type;
    String m_traceback;
};

} // namespace Web::HTML
