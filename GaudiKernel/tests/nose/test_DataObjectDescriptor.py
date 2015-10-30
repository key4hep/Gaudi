from GaudiKernel.DataObjectDescriptor import *

def test_str():
    dod = DataObjectDescriptor
    for s in ['',
              'tracks|/evt/rec/tracks|0|0',
              'tracks|/evt/rec/tracks|1|0',
              'tracks|/evt/rec/tracks|0|1',
              'multi|a&b&c|0|0']:
        print 'check DataObjectDescriptor(%r)' % s
        assert str(dod(s)) == s, 'expected %r, found %r' % (s, dod(s))

    for args, s in [(tuple(), ''),
                    (('tracks', '/evt/rec/tracks', False, dod.READ),
                     'tracks|/evt/rec/tracks|0|0'),
                    (('multi', 'a&b&c', True, dod.UPDATE),
                      'multi|a&b&c|1|2')]:
        print 'check DataObjectDescriptor(%s)' % ', '.join(map(repr, args))
        assert str(dod(*args)) == s, 'expected %r, found %r' % (s, dod(*args))

def test_eq():
    dod = DataObjectDescriptor
    for s in ['',
              'tracks|/evt/rec/tracks|0|0',
              'tracks|/evt/rec/tracks|1|0',
              'tracks|/evt/rec/tracks|0|1',
              'multi|a&b&c|0|0']:
        d1 = dod(s)
        d2 = dod(s)
        assert d1 == d2, 'equality check failed for %r' % d1

def test_coll_str():
    dodc = DataObjectDescriptorCollection
    for s in ['',
              'a|/evt/a|0|0',
              'a|/evt/a|0|0#b|/evt/b|0|2',
              'a|/evt/a|0|0#b|/evt/b|1|0#c|/evt/c&/evt/c1|0|0']:
        assert str(dodc(s)) == s, 'expected %r, found %r' % (s, dodc(s))

def test_coll_eq():
    dodc = DataObjectDescriptorCollection
    for s in ['',
              'a|/evt/a|0|0',
              'a|/evt/a|0|0#b|/evt/b|0|2',
              'a|/evt/a|0|0#b|/evt/b|1|0#c|/evt/c&/evt/c1|0|0']:
        d1 = dodc(s)
        d2 = dodc(s)
        assert d1 == d2, 'equality check failed for %r' % d1

def test_coll_access():
    dod = DataObjectDescriptor
    dodc = DataObjectDescriptorCollection

    d = dodc('')
    try:
        d.a
        assert False, 'exception expected'
    except AttributeError:
        pass

    try:
        d.a = dod('a|/evt/a|0|0')
        assert False, 'exception expected'
    except AttributeError:
        pass

    d = dodc('a|/evt/a|0|0#b|/evt/b|0|2')
    assert d.a
    assert d.b
    try:
        d.c = 'c|path|0|0'
        assert False, 'exception expected'
    except AttributeError:
        pass

    assert d.b.Path == '/evt/b'

    d.b = 'b|new|0|2'
    assert d.b.Path == 'new'

    d.b = dod('b|another|0|2')
    assert d.b.Path == 'another'

    try:
        d.b = dod('c|path|0|0')
        assert False, 'exception expected'
    except ValueError:
        pass
