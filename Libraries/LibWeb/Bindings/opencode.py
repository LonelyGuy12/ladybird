
def test_document_is_accessible():
    assert "document" in globals(), "Document object should be in global scope"
    doc = globals()["document"]
    assert doc is not None, "Document object should not be null"
    print("test_document_is_accessible passed")

if __name__ == "__main__":
    test_document_is_accessible()
