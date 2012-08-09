from cmt2cmake import CMTParser
from pyparsing import ParseException

def _test_package():
    tokens = list(CMTParser.parseString('package Test'))
    assert tokens == ['package', 'Test']

def test_version():
    tokens = list(CMTParser.parseString('version v1r2p3'))
    assert tokens == ['version', 'v1r2p3']

def test_library():
    statement = 'library  GaudiKernelLib  *.cpp  -no_static  -import=Boost'
    tokens = list(CMTParser.parseString(statement))
    print tokens
    assert tokens == ['library', 'GaudiKernelLib', '*.cpp', '-no_static', '-import=Boost']

    statement = 'library  TestLib  test/lib/*.cpp  a_variable=some_value  -group=tests'
    tokens = list(CMTParser.parseString(statement))
    print tokens
    assert tokens == ['library', 'TestLib', 'test/lib/*.cpp', 'a_variable=some_value', '-group=tests']

def test_macro():
    s = 'macro_append ROOT_linkopts "some" WIN32 "other"'
    tokens = list(CMTParser.parseString(s))
    print tokens
    assert tokens == ['macro_append', 'ROOT_linkopts', '"some"', 'WIN32', '"other"']

    s = 'macro a_var "value"'
    tokens = list(CMTParser.parseString(s))
    print tokens
    assert tokens == ['macro', 'a_var', '"value"']

    s = 'macro a_var'
    try:
        tokens = list(CMTParser.parseString(s))
        assert False, 'parsing should have failed: %r' % s
    except ParseException:
        pass
