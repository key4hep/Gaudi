def validate(stdout,stderr,result,causes):
    expected = {
           'map<int,double>': {1: 0.1,
                               2: 0.2,
                               3: 0.3},
           'map<int,int>': {1: 10, 2: 20, 3: 30},
           'map<int,string>': {-1: 'minus one', 0: 'zero', 2: 'two'},
           'map<string,double>': {'aa': 0.1,
                                  'bb': 0.2,
                                  'cc': 3.0},
           'map<string,int>': {'a': 1, 'b': 2, 'c': 3},
           'map<string,string>': {'a': 'sddsgsgsdgdggf',
                                  'b': 'sddsgsgsdgdggf',
                                  'c': 'sddsgsgsdgdggf'},
           'map<string,vector<double> >': {'aaa': [1.0, 2.0, 3.0],
                                           'bbb': [1.0, 2.0, 3.0],
                                           'ccc': [0.1, 0.2, 0.3]},
           'map<string,vector<int> >': {'aaa': [1, 2, 3],
                                        'bbb': [4, 5, 6],
                                        'ccc': [7, 8, 9]},
           'map<string,vector<string> >': {'aaa': ['a', 'b', 'c'],
                                           'bbb': ['a', 'b', 'c'],
                                           'ccc': ['a', 'b', 'c']},
           'map<unsignedint,string>': {0: 'UZero', 1: 'UOne', 2: 'UTwo'},
           'pair<double,double>': (3141.592, 2.1828183),
           'pair<int,int>': (3, 2),
           'vector<pair<double,double> >': [(0.0, 1.0),
                                            (1.0, 2.0),
                                            (2.0, 3.0),
                                            (3.0, 4.0),
                                            (4.0, 5.0),
                                            (5.0, 6.0),
                                            (6.0, 7.0),
                                            (7.0, 8.0)],
           'vector<pair<int,int> >': [(1, 1), (2, 1), (3, 2), (4, 3), (5, 5)],
           'vector<vector<double> >': [[0.0, 1.0, 2.0], [0.0, -0.5, -0.25]],
           'vector<vector<string> >': [['a', 'b', 'c'], ['A', 'B', 'C']],
           'EmptyMap' : {},
           'EmptyVector' : []
               }

    # Hack for win32: 'string' become 'basic_string...', so I have to
    #                 modify the keys of the dictionary
    import os
    if "winxp" in os.environ['CMTCONFIG'].split("-"):
        for k in expected.keys():
            if 'string' in k:
                expected[k.replace('string','basic_string<char,char_traits<char> >').replace('>>','> >')] = expected[k]
                del expected[k]

    import re
    signature = re.compile(r'xProps\s*SUCCESS\s*My Properties :')
    prop_sign = re.compile(r'''xProps\s*SUCCESS\s*'([^']*)':(.*)''')

    output = map(lambda l: l.rstrip(), stdout.splitlines())
    i = output.__iter__()
    try:
        while not signature.match(i.next()):
            pass

        properties = {}
        for l in i:
            m = prop_sign.match(l)
            if m:
                properties[m.group(1)] = eval(m.group(2))
            else:
               break

        if properties != expected:
            causes.append('properties')
            from pprint import PrettyPrinter
            pp = PrettyPrinter()
            result['GaudiTest.properties.expected'] = result.Quote(pp.pformat(expected))
            result['GaudiTest.properties.found'] = result.Quote(pp.pformat(properties))
            pkeys = set(properties)
            ekeys = set(expected)
            missing = sorted(ekeys - pkeys)
            if missing:
                result['GaudiTest.properties.missing_keys'] = result.Quote(pp.pformat(missing))
            extra = sorted(pkeys - ekeys)
            if extra:
                result['GaudiTest.properties.extra_keys'] = result.Quote(pp.pformat(extra))
            mismatch = sorted([k for k in ekeys.intersection(pkeys) if expected[k] != properties[k]])
            if mismatch:
                result['GaudiTest.properties.mismatch_keys'] = result.Quote(pp.pformat(mismatch))

    except StopIteration:
        causes.append('missing signature')
