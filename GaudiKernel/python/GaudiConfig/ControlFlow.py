'''
Classes for the implementation of the Control Flow Structure Syntax.

@see:
https://github.com/lhcb/scheduling-event-model/tree/master/controlflow_syntax
'''


class ControlFlowNode(object):
    '''
    Basic entry in the control flow graph.
    '''
    def __and__(self, rhs):
        if rhs is CFTrue:
            return self
        elif rhs is CFFalse:
            return CFFalse
        return AndNode(self, rhs)

    def __or__(self, rhs):
        if rhs is CFFalse:
            return self
        elif rhs is CFTrue:
            return CFTrue
        return OrNode(self, rhs)

    def __invert__(self):
        return InvertNode(self)

    def __rshift__(self, rhs):
        return OrderedNode(self, rhs)

    def visitNode(self, visitor):
        visitor.enter(self)
        self._visitSubNodes(visitor)
        visitor.leave(self)

    def _visitSubNodes(self, visitor):
        pass

    def __eq__(self, other):
        return (repr(self) == repr(other))

    def getFullName(self):
        '''
        Allow use of an expression as an algorihtm/sequence in a Gaudi job
        configuration.

        Convert the expression in nested sequencers and return the full name of
        the top one.
        '''
        if not hasattr(self, '_fullname'):
            from GaudiKernel.Configurable import makeSequences
            self._fullname = makeSequences(self).getFullName()
        return self._fullname


class ControlFlowLeaf(ControlFlowNode):
    '''
    Class used to identify a note without sub-nodes.
    '''
    pass


class ControlFlowBool(ControlFlowLeaf):
    def __init__(self, value):
        self.value = value

    def __and__(self, rhs):
        return rhs if self.value else self

    def __or__(self, rhs):
        return self if self.value else rhs

    def __invert__(self):
        return CFFalse if self.value else CFTrue

    def __repr__(self):
        return 'CFTrue' if self.value else 'CFFalse'

CFTrue = ControlFlowBool(True)
CFFalse = ControlFlowBool(False)
del ControlFlowBool


class OrderedNode(ControlFlowNode):
    '''
    Represent order of execution of nodes.
    '''
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __str__(self):
        return ">> "

    def __repr__(self):
        return "(%r >> %r)" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class AndNode(ControlFlowNode):
    '''
    And operation between control flow nodes.
    '''
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __str__(self):
        return "& "

    def __repr__(self):
        return "(%r & %r)" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class OrNode(ControlFlowNode):
    '''
    Or operation between control flow nodes.
    '''
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs

    def __str__(self):
        return "| "

    def __repr__(self):
        return "(%r | %r)" % (self.lhs, self.rhs)

    def _visitSubNodes(self, visitor):
        self.lhs.visitNode(visitor)
        self.rhs.visitNode(visitor)


class InvertNode(ControlFlowNode):
    '''
    Invert logic (negation) of a control flow node.
    '''
    def __init__(self, item):
        self.item = item

    def __str__(self):
        return "! "

    def __repr__(self):
        return "~%r" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class ign(ControlFlowNode): # noqa
    '''
    Treat a control flow node as always successful, equivalent to (a | ~ a).
    '''
    def __init__(self, item):
        self.item = item

    def __str__(self):
        return "^ "

    def __repr__(self):
        return "ign(%r)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class par(ControlFlowNode): # noqa
    def __init__(self, item):
        self.item = item

    def __str__(self):
        return "= "

    def __repr__(self):
        return "par(%r)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class seq(ControlFlowNode): # noqa
    def __init__(self, item):
        self.item = item

    def __str__(self):
        return "- "

    def __repr__(self):
        return "seq(%r)" % self.item

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class line(ControlFlowNode): # noqa
    def __init__(self, name, item):
        self.name = name
        self.item = item

    def __str__(self):
        return self.name + ' '

    def __repr__(self):
        return "line(%r, %r)" % (self.name, self.item)

    def _visitSubNodes(self, visitor):
        self.item.visitNode(visitor)


class PolishVisitor(object):
    def __init__(self):
        self.sequence = ''

    def enter(self, visitee):
        self.sequence = str(visitee) + self.sequence

    def leave(self, visitee):
        pass


class _TestVisitor(object):
    def __init__(self):
        self.depths = 0
        self.polishNotation = ''

    def enter(self, visitee):
        self.depths += 1
        print "%sEntering %s" % (self.depths * " ", type(visitee))
        self.polishNotation += str(type(visitee))
        if isinstance(visitee, ControlFlowLeaf):
            print "%s Algorithm name: %s" % (" " * self.depths, visitee)
            self.polishNotation += str(visitee)

    def leave(self, visitee):
        print "%sLeaving %s" % (self.depths * " ", type(visitee))
        self.depths -= 1


class _TestAlgorithm(ControlFlowLeaf):
    def __init__(self, name):
        self._name = name

    def __str__(self):
        return self._name + ' '

    def __repr__(self):
        return self._name

    def name(self):
        return self._name


class DotVisitor(object):
    def __init__(self):
        self.nodes = []
        self.edges = []
        self.number = 0
        self.stack = []
        self.ids = {}

    def enter(self, visitee):
        if visitee not in self.ids:
            self.number += 1
            dot_id = self.ids[visitee] = 'T%s' % self.number
        dot_id = self.ids[visitee]
        mother = None
        if self.is_needed(visitee):
            if isinstance(visitee, ControlFlowLeaf):
                entry = '%s [label="%s", shape=box]' % (dot_id, visitee.name())
            elif isinstance(visitee, OrNode):
                entry = '%s [label="OR", shape=invhouse]' % dot_id
            elif isinstance(visitee, AndNode):
                entry = '%s [label="AND", shape=invhouse]' % dot_id
            elif isinstance(visitee, OrderedNode):
                entry = '%s [label=">>", shape=circle, color=blue]' % dot_id
            elif isinstance(visitee, InvertNode):
                entry = '%s [label="NOT", shape=circle, color=red]' % dot_id
            elif isinstance(visitee, par):
                entry = '%s [label="PAR", shape=circle]' % dot_id
            elif isinstance(visitee, seq):
                entry = '%s [label="SEQ", shape=circle]' % dot_id
            else:
                entry = '%s [label="%s", shape=circle]' %\
                        (dot_id, type(visitee))
            self.nodes.append(entry)
            if len(self.stack) != 0:
                mother = self.collapse_identical_ancestors(type
                                                           (self.stack[-1][0]))
                if not mother:
                    mother = self.stack[-1][1]
                edge = "%s->%s" % (dot_id, mother)
                self.edges.append(edge)
        self.stack.append((visitee, dot_id))

    def leave(self, visitee):
        self.stack.pop()

    def collapse_identical_ancestors(self, thetype):
        '''
        If AND nodes are inside AND nodes, the graph could be simplified
        to not contain those (same true for OR and ordered)
        '''
        # counter = 0
        if len(self.stack) != 0:
            mother = self.stack[-1][1]
            for entry in self.stack[::-1]:
                if type(entry[0]) != thetype:
                    break
                mother = entry[1]
            return mother
        return None

    def is_needed(self, visitee):
        '''
        Check whether this node is actually needed
        '''
        if len(self.stack) != 0:
            return not isinstance(visitee, type(self.stack[-1][0]))
        return True

    def write(self, filename):
        output = """
digraph graphname {
rankdir=LR

%s

%s

}
""" % ("\n".join(self.nodes), "\n".join(self.edges))

        with open(filename, "w") as outfile:
            outfile.write(output)


def test():
    algorithm = _TestAlgorithm

    a = algorithm("a")
    b = algorithm("b")
    c = algorithm("c")
    d = algorithm("d")
    e = algorithm("e")
    f = algorithm("f")
    g = algorithm("g")
    sequence = seq(b >> a >> f)
    expression = sequence | ~c & par(d & e & g)
    a = (expression == expression)
    aline = line("MyTriggerPath", expression)
    visitor = _TestVisitor()
    visitor2 = DotVisitor()
    print "\nPrinting trigger line:"
    print repr(aline)
    print "\nPrinting expression:"
    print repr(expression)
    print "\nTraversing through expression:\n"
    expression.visitNode(visitor)
    expression.visitNode(visitor2)
    visitor2.write("out.dot")

    pvisitor = PolishVisitor()
    aline.visitNode(pvisitor)
    print pvisitor.sequence


def test2():
    algorithm = _TestAlgorithm

    a = algorithm("a")
    b = algorithm("b")
    c = algorithm("c")

    expression = a & b & c

    aline = line("MyTriggerline", expression)
    visitor = PolishVisitor()

    aline.visitNode(visitor)
    print visitor.sequence
